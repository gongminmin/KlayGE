#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/PostProcess.hpp>

#include <sstream>

#include "SampleCommon.hpp"
#include "CausticsMap.hpp"

using namespace KlayGE;
using namespace std;

namespace
{
	const uint32_t CAUSTICS_GRID_SIZE = 256;
	uint32_t const SHADOW_MAP_SIZE = 512;
	uint32_t const ENV_CUBE_MAP_SIZE = 512;

	enum
	{
		Depth_WODT_Pass,
		Position_Pass,
		Depth_Front_WODT_Pass,
		Depth_Back_WODT_Pass,
		Normal_Front_WODT_Pass,
		Normal_Back_WODT_Pass,
		Position_Normal_Front_Pass,
		Position_Normal_Back_Pass,
		Single_Caustics_Pass,
		Dual_Caustics_Pass,
		Gen_Shadow_Pass,
		Refract_Pass,
		Default_Pass
	};

	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};


	bool use_gs = false;


	class ReceivePlane : public RenderablePlane
	{
	public:
		ReceivePlane(float length, float width)
			: RenderablePlane(length, width, 1, 1, true, true)
		{
			RenderEffectPtr effect = SyncLoadRenderEffect("Scene.fxml");
			technique_ = effect->TechniqueByName("DistanceMapping2a");
			default_tech_ = technique_;
			if (!technique_->Validate())
			{
				technique_ = effect->TechniqueByName("DistanceMapping20");
				BOOST_ASSERT(technique_->Validate());
			}

			*(effect->ParameterByName("diffuse_tex")) = ASyncLoadTexture("diffuse.dds", EAH_GPU_Read | EAH_Immutable);
			*(effect->ParameterByName("normal_tex")) = ASyncLoadTexture("normal.dds", EAH_GPU_Read | EAH_Immutable);
			*(effect->ParameterByName("distance_tex")) = ASyncLoadTexture("distance.dds", EAH_GPU_Read | EAH_Immutable);

			depth_wodt_pass_ = effect->TechniqueByName("DepthTexWODT");
			BOOST_ASSERT(depth_wodt_pass_->Validate());
			position_pass_ = effect->TechniqueByName("PositionTex");
			BOOST_ASSERT(position_pass_->Validate());

			effect = SyncLoadRenderEffect("ShadowCubeMap.fxml");
			gen_cube_sm_tech_ = effect->TechniqueByName("GenCubeShadowMap");
			BOOST_ASSERT(gen_cube_sm_tech_->Validate());
		}

		void BindLight(LightSourcePtr light)
		{
			light_ = light;
		}

		void CausticsLight(LightSourcePtr light)
		{
			caustics_light_ = light;
		}

		void SetSMTexture(TexturePtr sm_texture)
		{
			sm_texture_ = sm_texture;
		}

		void SetCausticsMap(TexturePtr caustics_map)
		{
			caustics_map_ = caustics_map;
		}

		void Pass(uint32_t pass)
		{
			pass_ = pass;
			if (Depth_WODT_Pass == pass_)
			{
				technique_ = depth_wodt_pass_;
			}
			else if (Position_Pass == pass_)
			{
				technique_ = position_pass_;
			}
			else if (Gen_Shadow_Pass == pass_)
			{
				technique_ = gen_cube_sm_tech_;
			}
			else
			{
				technique_ = default_tech_;
			}
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & model = MathLib::rotation_x(DEG90);

			*(technique_->Effect().ParameterByName("mvp")) = model * camera.ViewProjMatrix();
			*(technique_->Effect().ParameterByName("model")) = model;
			*(technique_->Effect().ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());

			*(technique_->Effect().ParameterByName("pos_center")) = pos_aabb_.Center();
			*(technique_->Effect().ParameterByName("pos_extent")) = pos_aabb_.HalfSize();

			if (light_)
			{
				float const scale_factor = 8.0f;
				float light_inv_range = 1.0f / (light_->SMCamera(0)->FarPlane() - light_->SMCamera(0)->NearPlane());
				*(technique_->Effect().ParameterByName("esm_scale_factor")) = scale_factor * light_inv_range;
			}

			if ((Depth_WODT_Pass == pass_) || (Position_Pass == pass_))
			{
			}
			else if (Gen_Shadow_Pass == pass_)
			{
				float4x4 light_model = MathLib::to_matrix(light_->Rotation()) * MathLib::translation(light_->Position());
				float4x4 inv_light_model = MathLib::inverse(light_model);

				*(technique_->Effect().ParameterByName("obj_model_to_light_model")) = model * inv_light_model;
			}
			else
			{
				float4x4 light_model = MathLib::to_matrix(light_->Rotation()) * MathLib::translation(light_->Position());
				float4x4 inv_light_model = MathLib::inverse(light_model);
				float4x4 first_light_view = light_->SMCamera(0)->ViewMatrix();

				*(technique_->Effect().ParameterByName("light_pos")) = float3(MathLib::transform(light_->Position(), MathLib::inverse(MathLib::rotation_x(DEG90))));
				*(technique_->Effect().ParameterByName("light_color")) = float3(light_->Color());
				*(technique_->Effect().ParameterByName("light_falloff")) = light_->Falloff();
				*(technique_->Effect().ParameterByName("light_vp")) = caustics_light_->SMCamera(0)->ViewMatrix() * caustics_light_->SMCamera(0)->ProjMatrix();
				*(technique_->Effect().ParameterByName("eye_pos")) = float3(MathLib::transform(app.ActiveCamera().EyePos(), MathLib::inverse(model)));
				*(technique_->Effect().ParameterByName("shadow_cube_tex")) = sm_texture_;
				*(technique_->Effect().ParameterByName("caustics_tex")) = caustics_map_;
				*(technique_->Effect().ParameterByName("obj_model_to_light_model")) = model * inv_light_model;
				*(technique_->Effect().ParameterByName("obj_model_to_light_view")) = model * first_light_view;
			}
		}

	private:
		uint32_t pass_;
		RenderTechniquePtr default_tech_;
		RenderTechniquePtr depth_wodt_pass_;
		RenderTechniquePtr position_pass_;
		RenderTechniquePtr gen_cube_sm_tech_;
		LightSourcePtr light_;
		LightSourcePtr caustics_light_;
		TexturePtr sm_texture_;
		TexturePtr caustics_map_;
	};

	class PlaneObject : public SceneObjectHelper
	{
	public:
		PlaneObject(float length, float width)
			: SceneObjectHelper(MakeSharedPtr<ReceivePlane>(length, width), SOA_Moveable)
		{
		}

		void Pass(uint32_t pass)
		{
			checked_pointer_cast<ReceivePlane>(renderable_)->Pass(pass);
		}
	};

	class RefractMesh : public StaticMesh
	{
	public:
		RefractMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
		{
			RenderEffectPtr effect = SyncLoadRenderEffect("Scene.fxml");
			depth_wodt_tech_f_ = effect->TechniqueByName("DepthTexWODTFront");
			BOOST_ASSERT(depth_wodt_tech_f_->Validate());
			depth_wodt_tech_b_ = effect->TechniqueByName("DepthTexWODTBack");
			BOOST_ASSERT(depth_wodt_tech_b_->Validate());
			normal_wodt_tech_f_ = effect->TechniqueByName("NormalTexWODTFront");
			BOOST_ASSERT(normal_wodt_tech_f_->Validate());
			normal_wodt_tech_b_ = effect->TechniqueByName("NormalTexWODTBack");
			BOOST_ASSERT(normal_wodt_tech_b_->Validate());
			caustics_input_tech_f_ = effect->TechniqueByName("PosNormTexFront");
			BOOST_ASSERT(caustics_input_tech_f_->Validate());
			caustics_input_tech_b_ = effect->TechniqueByName("PosNormTexBack");
			BOOST_ASSERT(caustics_input_tech_b_->Validate());

			refract_tech_ = effect->TechniqueByName("RefractEffect");
			BOOST_ASSERT(refract_tech_->Validate());

			effect = SyncLoadRenderEffect("ShadowCubeMap.fxml");
			gen_cube_sm_tech_ = effect->TechniqueByName("GenCubeShadowMap");
			BOOST_ASSERT(gen_cube_sm_tech_->Validate());
		}

		void BindLight(LightSourcePtr const & light)
		{
			light_ = light;
		}

		void SceneTexture(TexturePtr const & scene_texture)
		{
			scene_texture_ = scene_texture;
		}

		void SetEnvCubeCamera(CameraPtr const & camera)
		{
			env_cube_camera_ = camera;
		}

		void SetEnvCube(TexturePtr const & texture)
		{
			env_cube_ = texture;
		}

		void OnRenderBegin()
		{
			CausticsMapApp& app = *checked_cast<CausticsMapApp*>(&Context::Instance().AppInstance());
			Camera const & camera = app.ActiveCamera();

			*(technique_->Effect().ParameterByName("mvp")) = model_mat_ * camera.ViewProjMatrix();
			*(technique_->Effect().ParameterByName("model")) = model_mat_;
			
			AABBox const & pos_bb = this->PosBound();
			*(technique_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
			*(technique_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();

			switch (pass_)
			{
			case Depth_Front_WODT_Pass:
			case Depth_Back_WODT_Pass:
			case Normal_Front_WODT_Pass:
			case Normal_Back_WODT_Pass:
			case Position_Normal_Front_Pass:
			case Position_Normal_Back_Pass:
				break;

			case Gen_Shadow_Pass:
				{
					float4x4 light_model = MathLib::to_matrix(light_->Rotation()) * MathLib::translation(light_->Position());
					float4x4 inv_light_model = MathLib::inverse(light_model);

					*(technique_->Effect().ParameterByName("obj_model_to_light_model")) = model_mat_ * inv_light_model;
					*(technique_->Effect().ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
				}
				break;

			case Refract_Pass:
				{
					float refract_idx = app.RefractIndex();
					float3 absorption_idx = float3(0.1f, 0.1f, 0.1f);
					
					*(technique_->Effect().ParameterByName("vp")) = camera.ViewProjMatrix();
					*(technique_->Effect().ParameterByName("eye_pos")) = camera.EyePos();				
					*(technique_->Effect().ParameterByName("background_texture")) = scene_texture_;
					*(technique_->Effect().ParameterByName("env_cube")) = env_cube_;
					*(technique_->Effect().ParameterByName("refract_idx")) = float2(refract_idx, 1.0f / refract_idx);
					*(technique_->Effect().ParameterByName("absorption_idx")) = absorption_idx;
				}
				break;
			}
		}

		void Pass(uint32_t pass)
		{
			pass_ = pass;

			switch (pass_)
			{
			case Depth_Front_WODT_Pass:
				technique_ = depth_wodt_tech_f_;
				break;

			case Depth_Back_WODT_Pass:
				technique_ = depth_wodt_tech_b_;
				break;

			case Normal_Front_WODT_Pass:
				technique_ = normal_wodt_tech_f_;
				break;

			case Normal_Back_WODT_Pass:
				technique_ = normal_wodt_tech_b_;
				break;

			case Position_Normal_Front_Pass:
				technique_ = caustics_input_tech_f_;
				break;

			case Position_Normal_Back_Pass:
				technique_ = caustics_input_tech_b_;
				break;

			case Gen_Shadow_Pass:
				technique_ = gen_cube_sm_tech_;
				break;

			case Refract_Pass:
				technique_ = refract_tech_;
				break;
			}
		}

	private:
		RenderTechniquePtr refract_tech_;
		RenderTechniquePtr depth_wodt_tech_f_;
		RenderTechniquePtr depth_wodt_tech_b_;
		RenderTechniquePtr normal_wodt_tech_f_;
		RenderTechniquePtr normal_wodt_tech_b_;
		RenderTechniquePtr caustics_input_tech_f_;
		RenderTechniquePtr caustics_input_tech_b_;
		RenderTechniquePtr gen_cube_sm_tech_;
		uint32_t pass_;
		LightSourcePtr light_;
		TexturePtr scene_texture_;
		CameraPtr env_cube_camera_;
		TexturePtr env_cube_;
	};

	class RefractModel : public RenderModel
	{
	public:
		explicit RefractModel(std::wstring const & name)
			: RenderModel(name)
		{
		}

		void BindLight(LightSourcePtr const & light)
		{
			for (auto const & mesh : subrenderables_)
			{
				checked_pointer_cast<RefractMesh>(mesh)->BindLight(light);
			}
		}

		void SceneTexture(TexturePtr const & scene_texture)
		{
			for (auto const & mesh : subrenderables_)
			{
				checked_pointer_cast<RefractMesh>(mesh)->SceneTexture(scene_texture);
			}
		}

		void ModelMatrix(float4x4 const & model)
		{
			for (auto const & mesh : subrenderables_)
			{
				checked_pointer_cast<RefractMesh>(mesh)->ModelMatrix(model);
			}
		}

		void SetEnvCube(TexturePtr const & texture)
		{
			for (auto const & mesh : subrenderables_)
			{
				checked_pointer_cast<RefractMesh>(mesh)->SetEnvCube(texture);
			}
		}

		void Pass(uint32_t pass)
		{
			for (auto const & mesh : subrenderables_)
			{
				checked_pointer_cast<RefractMesh>(mesh)->Pass(pass);
			}
		}
	};

	class CausticsGrid : public RenderableHelper
	{
	public:
		CausticsGrid()
			: RenderableHelper(L"CausticsGrid")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			
			std::vector<float2> xys(CAUSTICS_GRID_SIZE * CAUSTICS_GRID_SIZE);
			for (uint32_t i = 0; i < CAUSTICS_GRID_SIZE; ++ i)
			{
				for (uint32_t j = 0; j < CAUSTICS_GRID_SIZE; ++ j)
				{
					xys[i * CAUSTICS_GRID_SIZE + j] = float2(static_cast<float>(j), static_cast<float>(i)) / static_cast<float>(CAUSTICS_GRID_SIZE);
				}
			}

			rl_ = rf.MakeRenderLayout();

			RenderEffectPtr effect = SyncLoadRenderEffect("Caustics.fxml");
			*(effect->ParameterByName("pt_texture")) = ASyncLoadTexture("point.dds", EAH_GPU_Read | EAH_Immutable);
			if (use_gs)
			{
				rl_->TopologyType(RenderLayout::TT_PointList);

				ElementInitData init_data;
				init_data.row_pitch = static_cast<uint32_t>(xys.size() * sizeof(xys[0]));
				init_data.slice_pitch = 0;
				init_data.data = &xys[0];
				GraphicsBufferPtr point_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
				rl_->BindVertexStream(point_vb, std::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

				single_caustics_pass_ = effect->TechniqueByName("GenSingleFaceCausticsMapWithGS");
				BOOST_ASSERT(single_caustics_pass_->Validate());
				dual_caustics_pass_ = effect->TechniqueByName("GenDualFaceCausticsMapWithGS");
				BOOST_ASSERT(dual_caustics_pass_->Validate());
			}
			else
			{
				rl_->TopologyType(RenderLayout::TT_TriangleStrip);

				ElementInitData init_data;
				init_data.row_pitch = static_cast<uint32_t>(xys.size() * sizeof(xys[0]));
				init_data.slice_pitch = 0;
				init_data.data = &xys[0];
				GraphicsBufferPtr point_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
				rl_->BindVertexStream(point_vb, std::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)), RenderLayout::ST_Instance, 1);

				float2 texs[] =
				{
					float2(-1.0f, 1.0f),
					float2(1.0f, 1.0f),
					float2(-1.0f, -1.0f),
					float2(1.0f, -1.0f)
				};

				uint16_t indices[] =
				{
					0, 1, 2, 3
				};

				init_data.row_pitch = sizeof(texs);
				init_data.slice_pitch = 0;
				init_data.data = texs;
				GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
				rl_->BindVertexStream(tex_vb, std::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)), RenderLayout::ST_Geometry, static_cast<uint32_t>(xys.size()));

				init_data.row_pitch = sizeof(indices);
				init_data.slice_pitch = 0;
				init_data.data = indices;
				GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
				rl_->BindIndexStream(ib, EF_R16UI);

				single_caustics_pass_ = effect->TechniqueByName("GenSingleFaceCausticsMap");
				BOOST_ASSERT(single_caustics_pass_->Validate());
				dual_caustics_pass_ = effect->TechniqueByName("GenDualFaceCausticsMap");
				if (!dual_caustics_pass_->Validate())
				{
					dual_caustics_pass_ = single_caustics_pass_;
				}
			}

			pos_aabb_ = AABBox(float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 0.0f));
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		}

		void Pass(uint32_t pass)
		{
			pass_ = pass;
			if (Single_Caustics_Pass == pass_)
			{
				technique_ = single_caustics_pass_;
			}
			else //if (Dual_Caustics_Pass == pass_)
			{
				technique_ = dual_caustics_pass_;
			}
		}

		void OnRenderBegin()
		{
			CausticsMapApp& app = *checked_cast<CausticsMapApp*>(&Context::Instance().AppInstance());
			Camera const & camera = app.ActiveCamera();
			float4x4 const & light_view = camera.ViewMatrix();
			float4x4 const & light_proj = camera.ProjMatrix();
			float4x4 const & light_view_proj = camera.ViewProjMatrix();
			float4x4 const & inv_light_view = camera.InverseViewMatrix();
			float4x4 const & inv_light_proj = camera.InverseProjMatrix();
			float pt_size = app.PointSize();
			float refract_idx = app.RefractIndex();
			float3 absorption_idx = float3(0.1f, 0.1f, 0.1f);

			float3 light_pos = app.GetLightSource()->Position();
			float3 light_clr = app.GetLightSource()->Color();
			float light_density = app.LightDensity();

			if ((Single_Caustics_Pass == pass_) || (Dual_Caustics_Pass == pass_))
			{			
				*(technique_->Effect().ParameterByName("light_view")) = light_view;
				*(technique_->Effect().ParameterByName("light_proj")) = light_proj;
				*(technique_->Effect().ParameterByName("light_vp")) = light_view_proj;
				*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
				*(technique_->Effect().ParameterByName("light_color")) = light_clr;
				*(technique_->Effect().ParameterByName("light_density")) = light_density;
				*(technique_->Effect().ParameterByName("point_size")) = pt_size;
				*(technique_->Effect().ParameterByName("refract_idx")) = float2(refract_idx, 1.0f / refract_idx);
				*(technique_->Effect().ParameterByName("absorption_idx")) = absorption_idx;
				*(technique_->Effect().ParameterByName("inv_occlusion_pixs")) = 1.0f / (CAUSTICS_GRID_SIZE * CAUSTICS_GRID_SIZE / 2);

				*(technique_->Effect().ParameterByName("t_background_depth")) = app.GetBackgroundDepthTex();
				*(technique_->Effect().ParameterByName("t_first_depth")) = app.GetRefractObjDepthFrontTex();
				*(technique_->Effect().ParameterByName("t_first_normals")) = app.GetRefractObjNormalFrontTex();

				float3 ttow_center = MathLib::transform_normal(MathLib::transform_coord(float3(0, 0, 1), inv_light_proj), inv_light_view);
				float3 ttow_left = MathLib::transform_coord(MathLib::transform_coord(float3(-1, 0, 1), inv_light_proj), inv_light_view);
				float3 ttow_right = MathLib::transform_coord(MathLib::transform_coord(float3(+1, 0, 1), inv_light_proj), inv_light_view);
				float3 ttow_upper = MathLib::transform_coord(MathLib::transform_coord(float3(0, +1, 1), inv_light_proj), inv_light_view);
				float3 ttow_lower = MathLib::transform_coord(MathLib::transform_coord(float3(0, -1, 1), inv_light_proj), inv_light_view);
				*(technique_->Effect().ParameterByName("ttow_z")) = float4(ttow_center.x(), ttow_center.y(), ttow_center.z(), MathLib::length(ttow_center));
				*(technique_->Effect().ParameterByName("ttow_x")) = ttow_right - ttow_left;
				*(technique_->Effect().ParameterByName("ttow_y")) = ttow_lower - ttow_upper;
				*(technique_->Effect().ParameterByName("far_plane")) = float2(camera.FarPlane(), 1 / camera.FarPlane());

				if (Dual_Caustics_Pass == pass_)
				{
					*(technique_->Effect().ParameterByName("t_second_depth")) = app.GetRefractObjDepthBackTex();
					*(technique_->Effect().ParameterByName("t_second_normals")) = app.GetRefractObjNormalBackTex();
				}
			}
		}

	private:
		uint32_t pass_;
		RenderTechniquePtr single_caustics_pass_;
		RenderTechniquePtr dual_caustics_pass_;
	};
}

int SampleMain()
{
	CausticsMapApp app;
	app.Create();
	app.Run();

	return 0;
}

CausticsMapApp::CausticsMapApp()
	: App3DFramework("Caustics Map")
{
	ResLoader::Instance().AddPath("../../Samples/media/ShadowCubeMap");
	ResLoader::Instance().AddPath("../../Samples/media/CausticsMap");
}

bool CausticsMapApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < ShaderModel(3, 0))
	{
		return false;
	}
	return true;
}

void CausticsMapApp::OnCreate()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
	use_gs = caps.gs_support;
	depth_texture_support_ = caps.depth_texture_support;

	//Font config
	font_ = SyncLoadFont("gkai00mp.kfont");

	//Default Camera
	this->LookAt(float3(30, 30, -30), float3(0.0f, 0.0f, 0.0f));
	this->Proj(0.1f, 100);
	trackball_controller_.AttachCamera(this->ActiveCamera());
	trackball_controller_.Scalers(0.05f, 0.1f);

	//Light
	light_ = MakeSharedPtr<SpotLightSource>();
	light_->Attrib(0);
	light_->Color(float3(10, 10, 10));
	light_->Falloff(float3(1, 0, 0.01f));
	light_->Position(float3(0, 30, 0));
	light_->Direction(MathLib::normalize(-light_->Position()));
	light_->OuterAngle(PI / 8);
	light_->AddToSceneManager();

	//dummy light for Shadow
	dummy_light_ = MakeSharedPtr<PointLightSource>();
	dummy_light_->Attrib(0);
	dummy_light_->Color(light_->Color());
	dummy_light_->Falloff(light_->Falloff());
	dummy_light_->Position(light_->Position());

	//for env map generating
	dummy_light_env_ = MakeSharedPtr<PointLightSource>();
	dummy_light_env_->Position(float3(0.0f, 20.0f, 0.0f));

	light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
	light_proxy_->AddToSceneManager();

	//Input Bind
	InputEngine& ie(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap action_map;
	action_map.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));
	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(std::bind(&CausticsMapApp::InputHandler, this, std::placeholders::_1, std::placeholders::_2));
	ie.ActionMap(action_map, input_handler);

	//Model
	plane_object_ = MakeSharedPtr<PlaneObject>(50.f, 50.f);
	plane_object_->AddToSceneManager();

	RenderablePtr model_sphere = SyncLoadModel("sphere_high.7z//sphere_high.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RefractModel>(), CreateMeshFactory<RefractMesh>());
	sphere_ = MakeSharedPtr<SceneObjectHelper>(model_sphere, SceneObjectHelper::SOA_Cullable);
	sphere_->ModelMatrix(MathLib::scaling(200.0f, 200.0f, 200.0f) * MathLib::translation(0.0f, 10.0f, 0.0f));
	sphere_->AddToSceneManager();
	sphere_->Visible(false);

	RenderablePtr model_bunny = SyncLoadModel("bunny.7z//bunny.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RefractModel>(), CreateMeshFactory<RefractMesh>());
	bunny_ = MakeSharedPtr<SceneObjectHelper>(model_bunny, SceneObjectHelper::SOA_Cullable);
	bunny_->ModelMatrix(MathLib::scaling(320.0f, 320.0f, 320.0f) * MathLib::translation(3.0f, 2.0f, 0.0f));
	bunny_->AddToSceneManager();
	bunny_->Visible(false);

	refract_obj_ = sphere_;

	caustics_grid_ = MakeSharedPtr<CausticsGrid>();

	//Sky Box
	y_cube_map_ = SyncLoadTexture("uffizi_cross_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	c_cube_map_ = SyncLoadTexture("uffizi_cross_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>(0);
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map_, c_cube_map_);
	sky_box_->AddToSceneManager();

	copy_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");
	if (depth_texture_support_)
	{
		depth_to_linear_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToLinear");
	}

	this->InitUI();

	this->InitBuffer();

	this->InitCubeSM();

	this->InitEnvCube();

	caustics_map_pps_ = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess>>(5, 1.0f);
	caustics_map_pps_->InputPin(0, caustics_texture_);
	caustics_map_pps_->OutputPin(0, caustics_texture_filtered_);
}

void CausticsMapApp::InitBuffer()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	
	ElementFormat normal_fmt;
	if (caps.rendertarget_format_support(EF_A2BGR10, 1, 0))
	{
		normal_fmt = EF_A2BGR10;
	}
	else
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			normal_fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

			normal_fmt = EF_ARGB8;
		}
	}
	ElementFormat depth_fmt;
	if (caps.pack_to_rgba_required)
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			depth_fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			depth_fmt = EF_ARGB8;
		}
	}
	else
	{
		if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			depth_fmt = EF_R16F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_R32F, 1, 0));
			depth_fmt = EF_R32F;
		}
	}
	ElementFormat ds_fmt;
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		ds_fmt = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

		ds_fmt = EF_D16;
	}

	RenderViewPtr refract_obj_ds_view_f;
	RenderViewPtr refract_obj_ds_view_b;
	RenderViewPtr background_ds_view;
	if (depth_texture_support_)
	{
		refract_obj_ds_tex_f_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		refract_obj_ds_tex_b_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		background_ds_tex_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

		refract_obj_ds_view_f = rf.Make2DDepthStencilRenderView(*refract_obj_ds_tex_f_, 0, 1, 0);
		refract_obj_ds_view_b = rf.Make2DDepthStencilRenderView(*refract_obj_ds_tex_b_, 0, 1, 0);
		background_ds_view = rf.Make2DDepthStencilRenderView(*background_ds_tex_, 0, 1, 0);
	}
	else
	{
		refract_obj_ds_view_f = rf.Make2DDepthStencilRenderView(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, EF_D16, 1, 0);
		refract_obj_ds_view_b = rf.Make2DDepthStencilRenderView(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, EF_D16, 1, 0);
		background_ds_view = rf.Make2DDepthStencilRenderView(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, EF_D16, 1, 0);
	}

	refract_obj_N_texture_f_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, normal_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	refract_obj_N_texture_b_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, normal_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	refract_obj_depth_tex_f_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	refract_obj_depth_tex_b_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	background_depth_tex_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	background_fb_ = rf.MakeFrameBuffer();
	background_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*background_depth_tex_, 0, 1, 0));
	background_fb_->Attach(FrameBuffer::ATT_DepthStencil, background_ds_view);

	refract_obj_fb_d_f_ = rf.MakeFrameBuffer();
	refract_obj_fb_d_f_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*refract_obj_depth_tex_f_, 0, 1, 0));
	refract_obj_fb_d_f_->Attach(FrameBuffer::ATT_DepthStencil, refract_obj_ds_view_f);

	refract_obj_fb_d_b_ = rf.MakeFrameBuffer();
	refract_obj_fb_d_b_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*refract_obj_depth_tex_b_, 0, 1, 0));
	refract_obj_fb_d_b_->Attach(FrameBuffer::ATT_DepthStencil, refract_obj_ds_view_b);

	refract_obj_fb_f_ = rf.MakeFrameBuffer();
	refract_obj_fb_f_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*refract_obj_N_texture_f_, 0, 1, 0));
	refract_obj_fb_f_->Attach(FrameBuffer::ATT_DepthStencil, refract_obj_ds_view_f);
	
	refract_obj_fb_b_ = rf.MakeFrameBuffer();
	refract_obj_fb_b_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*refract_obj_N_texture_b_, 0, 1, 0));
	refract_obj_fb_b_->Attach(FrameBuffer::ATT_DepthStencil, refract_obj_ds_view_b);

	ElementFormat fmt;
	if (caps.fp_color_support)
	{
		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));
			fmt = EF_ABGR16F;
		}
	}
	else
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			fmt = EF_ARGB8;
		}
	}
	caustics_texture_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	caustics_texture_filtered_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, caustics_texture_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	caustics_fb_ = rf.MakeFrameBuffer();
	caustics_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*caustics_texture_, 0, 1, 0));

	scene_fb_ = rf.MakeFrameBuffer();
	scene_fb_->GetViewport()->camera = re.DefaultFrameBuffer()->GetViewport()->camera;
}

void CausticsMapApp::InitEnvCube()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	ElementFormat ds_fmt;
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		ds_fmt = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

		ds_fmt = EF_D16;
	}
	RenderViewPtr depth_view = rf.Make2DDepthStencilRenderView(ENV_CUBE_MAP_SIZE, ENV_CUBE_MAP_SIZE, ds_fmt, 1, 0);
	env_cube_buffer_ = rf.MakeFrameBuffer();
	ElementFormat fmt;
	if (caps.fp_color_support)
	{
		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));
			fmt = EF_ABGR16F;
		}
	}
	else
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			fmt = EF_ARGB8;
		}
	}
	env_tex_ = rf.MakeTexture2D(ENV_CUBE_MAP_SIZE, ENV_CUBE_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	env_cube_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*env_tex_, 0, 1, 0));
	env_cube_buffer_->Attach(FrameBuffer::ATT_DepthStencil, depth_view);

	env_cube_tex_ = rf.MakeTextureCube(ENV_CUBE_MAP_SIZE, 1, 1, env_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	for (int i = 0; i < 6; ++ i)
	{
		env_filter_pps_[i] = SyncLoadPostProcess("Copy.ppml", "copy");

		env_filter_pps_[i]->InputPin(0, env_tex_);
		env_filter_pps_[i]->OutputPin(0, env_cube_tex_, 0, 0, i);
	}
}

void CausticsMapApp::InitCubeSM()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	ElementFormat ds_fmt;
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		ds_fmt = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

		ds_fmt = EF_D16;
	}
	RenderViewPtr depth_view = rf.Make2DDepthStencilRenderView(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, ds_fmt, 1, 0);
	shadow_cube_buffer_ = rf.MakeFrameBuffer();
	ElementFormat fmt;
	if (caps.pack_to_rgba_required)
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			fmt = EF_ARGB8;
		}
	}
	else
	{
		if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			fmt = EF_R16F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_R32F, 1, 0));
			fmt = EF_R32F;
		}
	}
	shadow_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	shadow_cube_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*shadow_tex_, 0, 1, 0));
	shadow_cube_buffer_->Attach(FrameBuffer::ATT_DepthStencil, depth_view);

	shadow_cube_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	for (int i = 0; i < 6; ++ i)
	{
		sm_filter_pps_[i] = MakeSharedPtr<LogGaussianBlurPostProcess>(3, true);
		sm_filter_pps_[i]->InputPin(0, shadow_tex_);
		sm_filter_pps_[i]->OutputPin(0, shadow_cube_tex_, 0, 0, i);
	}
}

void CausticsMapApp::InitUI()
{
	//UI Settings
	UIManager::Instance().Load(ResLoader::Instance().Open("Caustics.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	int ui_id = 0;
	ui_id = dialog_->IDFromName("Light_Density_Slider");
	dialog_->Control<UISlider>(ui_id)->OnValueChangedEvent().connect(std::bind(&CausticsMapApp::LightDensityHandler, this, std::placeholders::_1));
	LightDensityHandler(*(dialog_->Control<UISlider>(ui_id)));

	ui_id = dialog_->IDFromName("Refraction_Index_Slider");
	dialog_->Control<UISlider>(ui_id)->OnValueChangedEvent().connect(std::bind(&CausticsMapApp::RefractIndexHandler, this, std::placeholders::_1));
	RefractIndexHandler(*(dialog_->Control<UISlider>(ui_id)));

	ui_id = dialog_->IDFromName("Point_Size_Slider");
	dialog_->Control<UISlider>(ui_id)->OnValueChangedEvent().connect(std::bind(&CausticsMapApp::PointSizeHandler, this, std::placeholders::_1));
	PointSizeHandler(*(dialog_->Control<UISlider>(ui_id)));

	ui_id = dialog_->IDFromName("Dual_Caustics_Checkbox");
	dialog_->Control<UICheckBox>(ui_id)->OnChangedEvent().connect(std::bind(&CausticsMapApp::DualFaceCausticsCheckBoxHandler, this, std::placeholders::_1));
	DualFaceCausticsCheckBoxHandler(*(dialog_->Control<UICheckBox>(ui_id)));

	ui_id = dialog_->IDFromName("Model_Combobox");
	dialog_->Control<UIComboBox>(ui_id)->OnSelectionChangedEvent().connect(std::bind(&CausticsMapApp::ModelSelectionComboBox, this, std::placeholders::_1));
	ModelSelectionComboBox(*(dialog_->Control<UIComboBox>(ui_id)));
}

void CausticsMapApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	ElementFormat fmt;
	if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
	{
		fmt = EF_B10G11R11F;
	}
	else
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(re.DeviceCaps().rendertarget_format_support(EF_ARGB8, 1, 0));

			fmt = EF_ARGB8;
		}
	}
	scene_texture_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	scene_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_texture_, 0, 1, 0));
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		fmt = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

		fmt = EF_D16;
	}
	scene_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(width, height, fmt, 1, 0));

	copy_pp_->InputPin(0, scene_texture_);
}

void CausticsMapApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

//UI Handler
void CausticsMapApp::RefractIndexHandler(KlayGE::UISlider const & sender)
{
	float idx_min = 1.0f;
	float idx_max = 2.0f;

	int min_val, max_val;
	sender.GetRange(min_val, max_val);

	refract_idx_ = (static_cast<float>(sender.GetValue()) - min_val) / (max_val - min_val) * (idx_max - idx_min) + idx_min;
}

void CausticsMapApp::LightDensityHandler(KlayGE::UISlider const & sender)
{
	float density_min = 1000.0f;
	float density_max = 20000.0f;

	int min_val, max_val;
	sender.GetRange(min_val, max_val);

	light_density_ = (static_cast<float>(sender.GetValue()) - min_val) / (max_val - min_val) * (density_max - density_min) + density_min;
}

void CausticsMapApp::PointSizeHandler(KlayGE::UISlider const & sender)
{
	float pt_min = 0.01f;
	float pt_max = 0.1f;

	int min_val, max_val;
	sender.GetRange(min_val, max_val);

	point_size_ = (static_cast<float>(sender.GetValue()) - min_val) / (max_val - min_val) * (pt_max - pt_min) + pt_min;
}

void CausticsMapApp::DualFaceCausticsCheckBoxHandler(KlayGE::UICheckBox const & sender)
{
	enable_dual_face_caustics_ = sender.GetChecked();
}

void CausticsMapApp::ModelSelectionComboBox(KlayGE::UIComboBox const & sender)
{
	switch (sender.GetSelectedIndex())
	{
	case 0:
		refract_obj_->Visible(false);
		refract_obj_ = sphere_;
		refract_obj_->Visible(true);
		break;

	default:
		refract_obj_->Visible(false);
		refract_obj_ = bunny_;
		refract_obj_->Visible(true);
		break;
	}

	dummy_light_env_->Position(MathLib::transform_coord(refract_obj_->GetRenderable()->PosBound().Center(), refract_obj_->ModelMatrix()));
}

void CausticsMapApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	wostringstream st;
	st.precision(2);
	st << fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Caustics Map", 16);
	font_->RenderText(0, 16, Color(1, 1, 0, 1), st.str().c_str(), 16);
	font_->RenderText(0, 32, Color(1, 1, 0, 1), re.ScreenFrameBuffer()->Description(), 16);
}

uint32_t CausticsMapApp::DoUpdate(uint32_t pass)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	sky_box_->Visible(false);
	refract_obj_->Visible(false);

	uint32_t sm_start_pass = 4;
	uint32_t env_start_pass = 10;

	if (!depth_texture_support_)
	{
		sm_start_pass += 2;
		env_start_pass += 2;
	}

	//Pass 0 ~ 3 Caustics Map
	if (0 == pass)
	{
		if (depth_texture_support_)
		{
			checked_pointer_cast<PlaneObject>(plane_object_)->Pass(Position_Pass);
		}
		else
		{
			checked_pointer_cast<PlaneObject>(plane_object_)->Pass(Depth_WODT_Pass);
		}

		re.BindFrameBuffer(background_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
		re.CurFrameBuffer()->GetViewport()->camera = light_->SMCamera(0);

		if (depth_texture_support_)
		{
			float q = light_->SMCamera(0)->FarPlane() / (light_->SMCamera(0)->FarPlane() - light_->SMCamera(0)->NearPlane());
			float4 near_q_far(light_->SMCamera(0)->NearPlane() * q, q, light_->SMCamera(0)->FarPlane(), 1 / light_->SMCamera(0)->FarPlane());
			depth_to_linear_pp_->SetParam(0, near_q_far);
		}

		return App3DFramework::URV_NeedFlush;
	}
	else if (1 == pass)
	{
		plane_object_->Visible(false);
		refract_obj_->Visible(true);
		if (depth_texture_support_)
		{
			checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->Pass(Position_Normal_Front_Pass);

			depth_to_linear_pp_->InputPin(0, background_ds_tex_);
			depth_to_linear_pp_->OutputPin(0, background_depth_tex_);
			depth_to_linear_pp_->Apply();

			re.BindFrameBuffer(refract_obj_fb_f_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->GetViewport()->camera = light_->SMCamera(0);
		}
		else
		{
			checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->Pass(Depth_Front_WODT_Pass);

			re.BindFrameBuffer(refract_obj_fb_d_f_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(light_->SMCamera(0)->FarPlane(), 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->GetViewport()->camera = light_->SMCamera(0);
		}

		return App3DFramework::URV_NeedFlush;
	}
	else if (2 == pass)
	{
		if (depth_texture_support_)
		{
			depth_to_linear_pp_->InputPin(0, refract_obj_ds_tex_f_);
			depth_to_linear_pp_->OutputPin(0, refract_obj_depth_tex_f_);
			depth_to_linear_pp_->Apply();

			if (enable_dual_face_caustics_)
			{
				refract_obj_->Visible(true);
				checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->Pass(Position_Normal_Back_Pass);

				re.BindFrameBuffer(refract_obj_fb_b_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
				re.CurFrameBuffer()->GetViewport()->camera = light_->SMCamera(0);
				return App3DFramework::URV_NeedFlush;
			}

			return 0;
		}
		else
		{
			plane_object_->Visible(false);
			refract_obj_->Visible(true);

			checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->Pass(Normal_Front_WODT_Pass);

			re.BindFrameBuffer(refract_obj_fb_f_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->GetViewport()->camera = light_->SMCamera(0);

			return App3DFramework::URV_NeedFlush;
		}
	}
	else if (3 == pass)
	{
		if (depth_texture_support_)
		{
			if (enable_dual_face_caustics_)
			{
				checked_pointer_cast<CausticsGrid>(caustics_grid_)->Pass(Dual_Caustics_Pass);

				depth_to_linear_pp_->InputPin(0, refract_obj_ds_tex_b_);
				depth_to_linear_pp_->OutputPin(0, refract_obj_depth_tex_b_);
				depth_to_linear_pp_->Apply();
			}
			else
			{
				checked_pointer_cast<CausticsGrid>(caustics_grid_)->Pass(Single_Caustics_Pass);
			}

			re.BindFrameBuffer(caustics_fb_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->GetViewport()->camera = light_->SMCamera(0);

			caustics_grid_->Render();
			caustics_map_pps_->Apply();
		}
		else
		{
			if (enable_dual_face_caustics_)
			{
				refract_obj_->Visible(true);
				checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->Pass(Depth_Back_WODT_Pass);

				re.BindFrameBuffer(refract_obj_fb_d_b_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(light_->SMCamera(0)->FarPlane(), 0.0f, 0.0f, 0.0f), 1.0f, 0);
				re.CurFrameBuffer()->GetViewport()->camera = light_->SMCamera(0);
				return App3DFramework::URV_NeedFlush;
			}
		}

		return 0;
	}
	else if (!depth_texture_support_ && (4 == pass))
	{
		if (enable_dual_face_caustics_)
		{
			refract_obj_->Visible(true);
			checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->Pass(Normal_Back_WODT_Pass);

			re.BindFrameBuffer(refract_obj_fb_b_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->GetViewport()->camera = light_->SMCamera(0);
			return App3DFramework::URV_NeedFlush;
		}

		return 0;
	}
	else if (!depth_texture_support_ && (5 == pass))
	{
		if (enable_dual_face_caustics_)
		{
			checked_pointer_cast<CausticsGrid>(caustics_grid_)->Pass(Dual_Caustics_Pass);
		}
		else
		{
			checked_pointer_cast<CausticsGrid>(caustics_grid_)->Pass(Single_Caustics_Pass);
		}

		re.BindFrameBuffer(caustics_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
		re.CurFrameBuffer()->GetViewport()->camera = light_->SMCamera(0);

		caustics_grid_->Render();
		caustics_map_pps_->Apply();

		return 0;
	}
	else if (pass - sm_start_pass < 6)
	{
		//Shadow Cube Map Passes
	
		if (pass > sm_start_pass)
		{
			checked_pointer_cast<LogGaussianBlurPostProcess>(sm_filter_pps_[pass - sm_start_pass - 1])->ESMScaleFactor(8.0f, light_->SMCamera(0));
			sm_filter_pps_[pass - sm_start_pass - 1]->Apply();
		}

		refract_obj_->Visible(true);
		plane_object_->Visible(true);

		checked_pointer_cast<PlaneObject>(plane_object_)->Pass(Gen_Shadow_Pass);
		checked_pointer_cast<ReceivePlane>(plane_object_->GetRenderable())->BindLight(dummy_light_);
		checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->Pass(Gen_Shadow_Pass);
		checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->BindLight(dummy_light_);

		re.BindFrameBuffer(shadow_cube_buffer_);
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
		shadow_cube_buffer_->GetViewport()->camera = dummy_light_->SMCamera(pass - sm_start_pass);

		return App3DFramework::URV_NeedFlush;
	}
	else if (pass - env_start_pass < 6)
	{
		if (6 == pass - sm_start_pass)
		{
			checked_pointer_cast<LogGaussianBlurPostProcess>(sm_filter_pps_[pass - sm_start_pass - 1])->ESMScaleFactor(8.0f, light_->SMCamera(0));
			sm_filter_pps_[pass - sm_start_pass - 1]->Apply();
		}

		// env map for reflect

		if (pass > env_start_pass)
		{
			env_filter_pps_[pass - env_start_pass - 1]->Apply();
		}

		refract_obj_->Visible(false);
		plane_object_->Visible(true);
		sky_box_->Visible(true);

		checked_pointer_cast<PlaneObject>(plane_object_)->Pass(Default_Pass);
		checked_pointer_cast<ReceivePlane>(plane_object_->GetRenderable())->BindLight(dummy_light_);
		checked_pointer_cast<ReceivePlane>(plane_object_->GetRenderable())->CausticsLight(light_);
		checked_pointer_cast<ReceivePlane>(plane_object_->GetRenderable())->SetSMTexture(shadow_cube_tex_);
		checked_pointer_cast<ReceivePlane>(plane_object_->GetRenderable())->SetCausticsMap(caustics_texture_filtered_);

		re.BindFrameBuffer(env_cube_buffer_);
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
		env_cube_buffer_->GetViewport()->camera = dummy_light_env_->SMCamera(pass - env_start_pass);
		
		return App3DFramework::URV_NeedFlush;
	}
	else if (6 == pass - env_start_pass)
	{
		if (pass - env_start_pass == 6)
		{
			env_filter_pps_[pass - env_start_pass - 1]->Apply();
		}

		// render background for refract

		refract_obj_->Visible(false);
		plane_object_->Visible(true);
		sky_box_->Visible(true);

		checked_pointer_cast<PlaneObject>(plane_object_)->Pass(Default_Pass);
		checked_pointer_cast<ReceivePlane>(plane_object_->GetRenderable())->BindLight(dummy_light_);
		checked_pointer_cast<ReceivePlane>(plane_object_->GetRenderable())->CausticsLight(light_);
		checked_pointer_cast<ReceivePlane>(plane_object_->GetRenderable())->SetSMTexture(shadow_cube_tex_);
		checked_pointer_cast<ReceivePlane>(plane_object_->GetRenderable())->SetCausticsMap(caustics_texture_filtered_);

		re.BindFrameBuffer(scene_fb_);
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

		return App3DFramework::URV_NeedFlush;
	}
	else
	{
		//Scene

		sky_box_->Visible(false);
		refract_obj_->Visible(true);
		plane_object_->Visible(true);

		checked_pointer_cast<PlaneObject>(plane_object_)->Pass(Default_Pass);
		checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->Pass(Refract_Pass);
		checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->SceneTexture(scene_texture_);
		checked_pointer_cast<RefractModel>(refract_obj_->GetRenderable())->SetEnvCube(env_cube_tex_);

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

		copy_pp_->Apply();

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}
