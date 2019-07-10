#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>
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
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			scene_effect_ = SyncLoadRenderEffect("Scene.fxml");
			technique_ = scene_effect_->TechniqueByName("DistanceMapping2a");
			default_tech_ = technique_;
			if (!technique_->Validate())
			{
				technique_ = scene_effect_->TechniqueByName("DistanceMapping20");
				BOOST_ASSERT(technique_->Validate());
			}

			mtl_ = MakeSharedPtr<RenderMaterial>();
			mtl_->Albedo(float4(1, 1, 1, 1));
			mtl_->Texture(RenderMaterial::TS_Albedo, rf.MakeTextureSrv(ASyncLoadTexture("diffuse.dds", EAH_GPU_Read | EAH_Immutable)));
			mtl_->Texture(RenderMaterial::TS_Normal, rf.MakeTextureSrv(ASyncLoadTexture("normal.dds", EAH_GPU_Read | EAH_Immutable)));

			*(scene_effect_->ParameterByName("distance_tex")) = ASyncLoadTexture("distance.dds", EAH_GPU_Read | EAH_Immutable);

			depth_wodt_pass_ = scene_effect_->TechniqueByName("DepthTexWODT");
			BOOST_ASSERT(depth_wodt_pass_->Validate());
			position_pass_ = scene_effect_->TechniqueByName("PositionTex");
			BOOST_ASSERT(position_pass_->Validate());

			gen_cube_sm_effect_ = SyncLoadRenderEffect("ShadowCubeMap.fxml");
			gen_cube_sm_tech_ = gen_cube_sm_effect_->TechniqueByName("GenCubeShadowMap");
			BOOST_ASSERT(gen_cube_sm_tech_->Validate());
		}

		void BindLight(LightSourcePtr const& light)
		{
			light_ = light;
		}

		void CausticsLight(LightSourcePtr const& light)
		{
			caustics_light_ = light;
		}

		void SetSMTexture(TexturePtr const& sm_texture)
		{
			sm_texture_ = sm_texture;
		}

		void SetCausticsMap(TexturePtr const& caustics_map)
		{
			caustics_map_ = caustics_map;
		}

		void CausticsPass(uint32_t pass)
		{
			pass_ = pass;
			if (Depth_WODT_Pass == pass_)
			{
				technique_ = depth_wodt_pass_;
				effect_ = scene_effect_;
			}
			else if (Position_Pass == pass_)
			{
				technique_ = position_pass_;
				effect_ = scene_effect_;
			}
			else if (Gen_Shadow_Pass == pass_)
			{
				technique_ = gen_cube_sm_tech_;
				effect_ = gen_cube_sm_effect_;
			}
			else
			{
				technique_ = default_tech_;
				effect_ = scene_effect_;
			}
		}

		void OnRenderBegin()
		{
			RenderablePlane::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & model = MathLib::rotation_x(DEG90);

			*(effect_->ParameterByName("mvp")) = model * camera.ViewProjMatrix();
			*(effect_->ParameterByName("model")) = model;
			*(effect_->ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());

			if (light_)
			{
				float const scale_factor = 8.0f;
				float light_inv_range = 1.0f / (light_->SMCamera(0)->FarPlane() - light_->SMCamera(0)->NearPlane());
				*(effect_->ParameterByName("esm_scale_factor")) = scale_factor * light_inv_range;
			}

			if ((Depth_WODT_Pass == pass_) || (Position_Pass == pass_))
			{
			}
			else if (Gen_Shadow_Pass == pass_)
			{
				float4x4 light_model = light_->BoundSceneNode()->TransformToParent();
				float4x4 inv_light_model = MathLib::inverse(light_model);

				*(effect_->ParameterByName("obj_model_to_light_model")) = model * inv_light_model;
			}
			else
			{
				float4x4 light_model = light_->BoundSceneNode()->TransformToParent();
				float4x4 inv_light_model = MathLib::inverse(light_model);
				float4x4 first_light_view = light_->SMCamera(0)->ViewMatrix();

				*(effect_->ParameterByName("light_pos")) = MathLib::transform_coord(light_->Position(), MathLib::rotation_x(-DEG90));
				*(effect_->ParameterByName("light_color")) = float3(light_->Color());
				*(effect_->ParameterByName("light_falloff")) = light_->Falloff();
				*(effect_->ParameterByName("light_vp")) = caustics_light_->SMCamera(0)->ViewMatrix() * caustics_light_->SMCamera(0)->ProjMatrix();
				*(effect_->ParameterByName("eye_pos")) = float3(MathLib::transform(app.ActiveCamera().EyePos(), MathLib::inverse(model)));
				*(effect_->ParameterByName("shadow_cube_tex")) = sm_texture_;
				*(effect_->ParameterByName("caustics_tex")) = caustics_map_;
				*(effect_->ParameterByName("obj_model_to_light_model")) = model * inv_light_model;
				*(effect_->ParameterByName("obj_model_to_light_view")) = model * first_light_view;
			}
		}

	private:
		uint32_t pass_;
		RenderEffectPtr scene_effect_;
		RenderTechnique* default_tech_;
		RenderTechnique* depth_wodt_pass_;
		RenderTechnique* position_pass_;
		RenderEffectPtr gen_cube_sm_effect_;
		RenderTechnique* gen_cube_sm_tech_;
		LightSourcePtr light_;
		LightSourcePtr caustics_light_;
		TexturePtr sm_texture_;
		TexturePtr caustics_map_;
	};

	class RefractMesh : public StaticMesh
	{
	public:
		explicit RefractMesh(std::wstring_view name)
			: StaticMesh(name)
		{
			scene_effect_ = SyncLoadRenderEffect("Scene.fxml");
			depth_wodt_tech_f_ = scene_effect_->TechniqueByName("DepthTexWODTFront");
			BOOST_ASSERT(depth_wodt_tech_f_->Validate());
			depth_wodt_tech_b_ = scene_effect_->TechniqueByName("DepthTexWODTBack");
			BOOST_ASSERT(depth_wodt_tech_b_->Validate());
			normal_wodt_tech_f_ = scene_effect_->TechniqueByName("NormalTexWODTFront");
			BOOST_ASSERT(normal_wodt_tech_f_->Validate());
			normal_wodt_tech_b_ = scene_effect_->TechniqueByName("NormalTexWODTBack");
			BOOST_ASSERT(normal_wodt_tech_b_->Validate());
			caustics_input_tech_f_ = scene_effect_->TechniqueByName("PosNormTexFront");
			BOOST_ASSERT(caustics_input_tech_f_->Validate());
			caustics_input_tech_b_ = scene_effect_->TechniqueByName("PosNormTexBack");
			BOOST_ASSERT(caustics_input_tech_b_->Validate());

			refract_tech_ = scene_effect_->TechniqueByName("RefractEffect");
			BOOST_ASSERT(refract_tech_->Validate());

			gen_cube_sm_effect_ = SyncLoadRenderEffect("ShadowCubeMap.fxml");
			gen_cube_sm_tech_ = gen_cube_sm_effect_->TechniqueByName("GenCubeShadowMap");
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
			CausticsMapApp& app = checked_cast<CausticsMapApp&>(Context::Instance().AppInstance());
			Camera const & camera = app.ActiveCamera();

			*(scene_effect_->ParameterByName("mvp")) = model_mat_ * camera.ViewProjMatrix();
			*(scene_effect_->ParameterByName("model")) = model_mat_;
			
			AABBox const & pos_bb = this->PosBound();
			*(scene_effect_->ParameterByName("pos_center")) = pos_bb.Center();
			*(scene_effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();

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
					float4x4 light_model = light_->BoundSceneNode()->TransformToParent();
					float4x4 inv_light_model = MathLib::inverse(light_model);

					*(scene_effect_->ParameterByName("obj_model_to_light_model")) = model_mat_ * inv_light_model;
					*(scene_effect_->ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
				}
				break;

			case Refract_Pass:
				{
					float refract_idx = app.RefractIndex();
					float3 absorption_idx = float3(0.1f, 0.1f, 0.1f);
					
					*(scene_effect_->ParameterByName("vp")) = camera.ViewProjMatrix();
					*(scene_effect_->ParameterByName("eye_pos")) = camera.EyePos();				
					*(scene_effect_->ParameterByName("background_texture")) = scene_texture_;
					*(scene_effect_->ParameterByName("env_cube")) = env_cube_;
					*(scene_effect_->ParameterByName("refract_idx")) = float2(refract_idx, 1.0f / refract_idx);
					*(scene_effect_->ParameterByName("absorption_idx")) = absorption_idx;
				}
				break;
			}
		}

		void CausticsPass(uint32_t pass)
		{
			pass_ = pass;

			switch (pass_)
			{
			case Depth_Front_WODT_Pass:
				technique_ = depth_wodt_tech_f_;
				effect_ = scene_effect_;
				break;

			case Depth_Back_WODT_Pass:
				technique_ = depth_wodt_tech_b_;
				effect_ = scene_effect_;
				break;

			case Normal_Front_WODT_Pass:
				technique_ = normal_wodt_tech_f_;
				effect_ = scene_effect_;
				break;

			case Normal_Back_WODT_Pass:
				technique_ = normal_wodt_tech_b_;
				effect_ = scene_effect_;
				break;

			case Position_Normal_Front_Pass:
				technique_ = caustics_input_tech_f_;
				effect_ = scene_effect_;
				break;

			case Position_Normal_Back_Pass:
				technique_ = caustics_input_tech_b_;
				effect_ = scene_effect_;
				break;

			case Gen_Shadow_Pass:
				technique_ = gen_cube_sm_tech_;
				effect_ = gen_cube_sm_effect_;
				break;

			case Refract_Pass:
				technique_ = refract_tech_;
				effect_ = scene_effect_;
				break;
			}
		}

	private:
		RenderEffectPtr scene_effect_;
		RenderTechnique* refract_tech_;
		RenderTechnique* depth_wodt_tech_f_;
		RenderTechnique* depth_wodt_tech_b_;
		RenderTechnique* normal_wodt_tech_f_;
		RenderTechnique* normal_wodt_tech_b_;
		RenderTechnique* caustics_input_tech_f_;
		RenderTechnique* caustics_input_tech_b_;
		RenderEffectPtr gen_cube_sm_effect_;
		RenderTechnique* gen_cube_sm_tech_;
		uint32_t pass_;
		LightSourcePtr light_;
		TexturePtr scene_texture_;
		CameraPtr env_cube_camera_;
		TexturePtr env_cube_;
	};

	class CausticsGrid : public Renderable
	{
	public:
		CausticsGrid()
			: Renderable(L"CausticsGrid")
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

			rls_[0] = rf.MakeRenderLayout();

			effect_ = SyncLoadRenderEffect("Caustics.fxml");
			*(effect_->ParameterByName("pt_texture")) = ASyncLoadTexture("point.dds", EAH_GPU_Read | EAH_Immutable);
			if (use_gs)
			{
				rls_[0]->TopologyType(RenderLayout::TT_PointList);

				GraphicsBufferPtr point_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					static_cast<uint32_t>(xys.size() * sizeof(xys[0])), &xys[0]);
				rls_[0]->BindVertexStream(point_vb, VertexElement(VEU_Position, 0, EF_GR32F));

				single_caustics_pass_ = effect_->TechniqueByName("GenSingleFaceCausticsMapWithGS");
				BOOST_ASSERT(single_caustics_pass_->Validate());
				dual_caustics_pass_ = effect_->TechniqueByName("GenDualFaceCausticsMapWithGS");
				BOOST_ASSERT(dual_caustics_pass_->Validate());
			}
			else
			{
				rls_[0]->TopologyType(RenderLayout::TT_TriangleStrip);

				GraphicsBufferPtr point_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					static_cast<uint32_t>(xys.size() * sizeof(xys[0])), &xys[0]);
				rls_[0]->BindVertexStream(point_vb, VertexElement(VEU_Position, 0, EF_GR32F), RenderLayout::ST_Instance, 1);

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

				GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(texs), texs);
				rls_[0]->BindVertexStream(tex_vb, VertexElement(VEU_TextureCoord, 0, EF_GR32F),
					RenderLayout::ST_Geometry, static_cast<uint32_t>(xys.size()));

				GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(indices), indices);
				rls_[0]->BindIndexStream(ib, EF_R16UI);

				single_caustics_pass_ = effect_->TechniqueByName("GenSingleFaceCausticsMap");
				BOOST_ASSERT(single_caustics_pass_->Validate());
				dual_caustics_pass_ = effect_->TechniqueByName("GenDualFaceCausticsMap");
				if (!dual_caustics_pass_->Validate())
				{
					dual_caustics_pass_ = single_caustics_pass_;
				}
			}

			pos_aabb_ = AABBox(float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 0.0f));
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		}

		void CausticsPass(uint32_t pass)
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
			CausticsMapApp& app = checked_cast<CausticsMapApp&>(Context::Instance().AppInstance());
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
				*(effect_->ParameterByName("light_view")) = light_view;
				*(effect_->ParameterByName("light_proj")) = light_proj;
				*(effect_->ParameterByName("light_vp")) = light_view_proj;
				*(effect_->ParameterByName("light_pos")) = light_pos;
				*(effect_->ParameterByName("light_color")) = light_clr;
				*(effect_->ParameterByName("light_density")) = light_density;
				*(effect_->ParameterByName("point_size")) = pt_size;
				*(effect_->ParameterByName("refract_idx")) = float2(refract_idx, 1.0f / refract_idx);
				*(effect_->ParameterByName("absorption_idx")) = absorption_idx;
				*(effect_->ParameterByName("inv_occlusion_pixs")) = 1.0f / (CAUSTICS_GRID_SIZE * CAUSTICS_GRID_SIZE / 2);

				*(effect_->ParameterByName("t_background_depth")) = app.GetBackgroundDepthTex();
				*(effect_->ParameterByName("t_first_depth")) = app.GetRefractObjDepthFrontTex();
				*(effect_->ParameterByName("t_first_normals")) = app.GetRefractObjNormalFrontTex();

				float3 ttow_center = MathLib::transform_normal(MathLib::transform_coord(float3(0, 0, 1), inv_light_proj), inv_light_view);
				float3 ttow_left = MathLib::transform_coord(MathLib::transform_coord(float3(-1, 0, 1), inv_light_proj), inv_light_view);
				float3 ttow_right = MathLib::transform_coord(MathLib::transform_coord(float3(+1, 0, 1), inv_light_proj), inv_light_view);
				float3 ttow_upper = MathLib::transform_coord(MathLib::transform_coord(float3(0, +1, 1), inv_light_proj), inv_light_view);
				float3 ttow_lower = MathLib::transform_coord(MathLib::transform_coord(float3(0, -1, 1), inv_light_proj), inv_light_view);
				*(effect_->ParameterByName("ttow_z")) = float4(ttow_center.x(), ttow_center.y(), ttow_center.z(), MathLib::length(ttow_center));
				*(effect_->ParameterByName("ttow_x")) = ttow_right - ttow_left;
				*(effect_->ParameterByName("ttow_y")) = ttow_lower - ttow_upper;
				*(effect_->ParameterByName("far_plane")) = float2(camera.FarPlane(), 1 / camera.FarPlane());

				if (Dual_Caustics_Pass == pass_)
				{
					*(effect_->ParameterByName("t_second_depth")) = app.GetRefractObjDepthBackTex();
					*(effect_->ParameterByName("t_second_normals")) = app.GetRefractObjNormalBackTex();
				}
			}
		}

	private:
		uint32_t pass_;
		RenderTechnique* single_caustics_pass_;
		RenderTechnique* dual_caustics_pass_;
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

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	light_node->TransformToParent(MathLib::inverse(MathLib::look_at_lh(float3(0, 30, 0), float3(0, 0, 0), float3(0, 0, 1))));
	root_node.AddChild(light_node);

	//Light
	light_ = MakeSharedPtr<SpotLightSource>();
	light_->Attrib(0);
	light_->Color(float3(10, 10, 10));
	light_->Falloff(float3(1, 0, 0.01f));
	light_->OuterAngle(PI / 8);
	light_node->AddComponent(light_);

	auto light_proxy = LoadLightSourceProxyModel(light_);
	light_node->AddChild(light_proxy->RootNode());

	//dummy light for Shadow
	dummy_light_ = MakeSharedPtr<PointLightSource>();
	dummy_light_->Attrib(0);
	dummy_light_->Color(light_->Color());
	dummy_light_->Falloff(light_->Falloff());
	light_node->AddComponent(dummy_light_);

	//for env map generating
	auto dummy_light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
	dummy_light_env_ = MakeSharedPtr<PointLightSource>();
	dummy_light_node->AddComponent(dummy_light_env_);
	dummy_light_node->OnMainThreadUpdate().Connect([this](SceneNode& node, float app_time, float elapsed_time) {
		KFL_UNUSED(app_time);
		KFL_UNUSED(elapsed_time);

		node.TransformToParent(MathLib::translation(refract_model_->RootNode()->PosBoundWS().Center()));
	});
	root_node.AddChild(dummy_light_node);

	//Input Bind
	InputEngine& ie(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap action_map;
	action_map.AddActions(actions, actions + std::size(actions));
	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	ie.ActionMap(action_map, input_handler);

	// Model
	plane_renderable_ = MakeSharedPtr<ReceivePlane>(50.0f, 50.0f);
	plane_obj_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(plane_renderable_), SceneNode::SOA_Moveable);
	root_node.AddChild(plane_obj_);

	sphere_obj_ = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	sphere_obj_->TransformToParent(MathLib::scaling(200.0f, 200.0f, 200.0f) * MathLib::translation(0.0f, 10.0f, 0.0f));
	sphere_obj_->Visible(false);
	root_node.AddChild(sphere_obj_);
	sphere_model_ = ASyncLoadModel("sphere_high.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable,
		[this](RenderModel& model)
		{
			AddToSceneHelper(*sphere_obj_, model);
		},
		CreateModelFactory<RenderModel>, CreateMeshFactory<RefractMesh>);

	bunny_obj_ = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	bunny_obj_->TransformToParent(MathLib::scaling(320.0f, 320.0f, 320.0f) * MathLib::translation(3.0f, 2.0f, 0.0f));
	bunny_obj_->Visible(false);
	root_node.AddChild(bunny_obj_);
	bunny_model_ = ASyncLoadModel("bunny.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable,
		[this](RenderModel& model)
		{
			AddToSceneHelper(*bunny_obj_, model);
		},
		CreateModelFactory<RenderModel>, CreateMeshFactory<RefractMesh>);

	refract_obj_ = sphere_obj_;
	refract_model_ = sphere_model_;

	caustics_grid_ = MakeSharedPtr<CausticsGrid>();

	// Sky Box
	auto y_cube_map = ASyncLoadTexture("uffizi_cross_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	auto c_cube_map = ASyncLoadTexture("uffizi_cross_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	auto skybox_renderable = MakeSharedPtr<RenderableSkyBox>();
	skybox_renderable->CompressedCubeMap(y_cube_map, c_cube_map);
	skybox_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox_renderable), SceneNode::SOA_NotCastShadow);
	root_node.AddChild(skybox_);

	copy_pp_ = SyncLoadPostProcess("Copy.ppml", "Copy");
	if (depth_texture_support_)
	{
		depth_to_linear_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToLinear");
	}

	this->InitUI();

	this->InitBuffer();

	this->InitCubeSM();

	this->InitEnvCube();

	caustics_map_pps_ = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess>>(5, 1.0f);
	caustics_map_pps_->InputPin(0, caustics_srv_);
	caustics_map_pps_->OutputPin(0, rf.Make2DRtv(caustics_texture_filtered_, 0, 1, 0));
}

void CausticsMapApp::InitBuffer()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	
	auto const normal_fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_A2BGR10, EF_ABGR8, EF_ARGB8}), 1, 0);
	BOOST_ASSERT(normal_fmt != EF_Unknown);

	auto const depth_fmt = caps.BestMatchTextureRenderTargetFormat(
		caps.pack_to_rgba_required ? MakeSpan({EF_ABGR8, EF_ARGB8}) : MakeSpan({EF_R16F, EF_R32F}), 1, 0);
	BOOST_ASSERT(depth_fmt != EF_Unknown);

	DepthStencilViewPtr refract_obj_ds_dsv_f;
	DepthStencilViewPtr refract_obj_ds_dsv_b;
	DepthStencilViewPtr background_ds_dsv;
	if (depth_texture_support_)
	{
		auto const ds_fmt = caps.BestMatchRenderTargetFormat(MakeSpan({EF_D24S8, EF_D16}), 1, 0);
		BOOST_ASSERT(ds_fmt != EF_Unknown);

		refract_obj_ds_tex_f_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		refract_obj_ds_srv_f_ = rf.MakeTextureSrv(refract_obj_ds_tex_f_);
		refract_obj_ds_dsv_f = rf.Make2DDsv(refract_obj_ds_tex_f_, 0, 1, 0);
		refract_obj_ds_tex_b_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		refract_obj_ds_srv_b_ = rf.MakeTextureSrv(refract_obj_ds_tex_b_);
		refract_obj_ds_dsv_b = rf.Make2DDsv(refract_obj_ds_tex_b_, 0, 1, 0);
		background_ds_tex_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		background_ds_srv_ = rf.MakeTextureSrv(background_ds_tex_);
		background_ds_dsv = rf.Make2DDsv(background_ds_tex_, 0, 1, 0);
	}
	else
	{
		refract_obj_ds_dsv_f = rf.Make2DDsv(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, EF_D16, 1, 0);
		refract_obj_ds_dsv_b = rf.Make2DDsv(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, EF_D16, 1, 0);
		background_ds_dsv = rf.Make2DDsv(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, EF_D16, 1, 0);
	}

	refract_obj_N_texture_f_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, normal_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	refract_obj_N_texture_b_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, normal_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	refract_obj_depth_tex_f_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	refract_obj_depth_rtv_f_ = rf.Make2DRtv(refract_obj_depth_tex_f_, 0, 1, 0);
	refract_obj_depth_tex_b_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	refract_obj_depth_rtv_b_ = rf.Make2DRtv(refract_obj_depth_tex_b_, 0, 1, 0);
	background_depth_tex_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	background_depth_rtv_ = rf.Make2DRtv(background_depth_tex_, 0, 1, 0);

	background_fb_ = rf.MakeFrameBuffer();
	background_fb_->Attach(FrameBuffer::Attachment::Color0, background_depth_rtv_);
	background_fb_->Attach(background_ds_dsv);

	refract_obj_fb_d_f_ = rf.MakeFrameBuffer();
	refract_obj_fb_d_f_->Attach(FrameBuffer::Attachment::Color0, refract_obj_depth_rtv_f_);
	refract_obj_fb_d_f_->Attach(refract_obj_ds_dsv_f);

	refract_obj_fb_d_b_ = rf.MakeFrameBuffer();
	refract_obj_fb_d_b_->Attach(FrameBuffer::Attachment::Color0, refract_obj_depth_rtv_b_);
	refract_obj_fb_d_b_->Attach(refract_obj_ds_dsv_b);

	refract_obj_fb_f_ = rf.MakeFrameBuffer();
	refract_obj_fb_f_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(refract_obj_N_texture_f_, 0, 1, 0));
	refract_obj_fb_f_->Attach(refract_obj_ds_dsv_f);
	
	refract_obj_fb_b_ = rf.MakeFrameBuffer();
	refract_obj_fb_b_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(refract_obj_N_texture_b_, 0, 1, 0));
	refract_obj_fb_b_->Attach(refract_obj_ds_dsv_b);

	auto const fmt = caps.BestMatchTextureRenderTargetFormat(
		caps.fp_color_support ? MakeSpan({EF_B10G11R11F, EF_ABGR16F}) : MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
	BOOST_ASSERT(fmt != EF_Unknown);
	caustics_texture_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	caustics_srv_ = rf.MakeTextureSrv(caustics_texture_);
	caustics_texture_filtered_ = rf.MakeTexture2D(CAUSTICS_GRID_SIZE, CAUSTICS_GRID_SIZE, 1, 1, caustics_texture_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	caustics_fb_ = rf.MakeFrameBuffer();
	caustics_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(caustics_texture_, 0, 1, 0));

	scene_fb_ = rf.MakeFrameBuffer();
	scene_fb_->Viewport()->Camera(re.DefaultFrameBuffer()->Viewport()->Camera());
}

void CausticsMapApp::InitEnvCube()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	auto const ds_fmt = caps.BestMatchRenderTargetFormat(MakeSpan({EF_D24S8, EF_D16}), 1, 0);
	auto depth_view = rf.Make2DDsv(ENV_CUBE_MAP_SIZE, ENV_CUBE_MAP_SIZE, ds_fmt, 1, 0);
	env_cube_buffer_ = rf.MakeFrameBuffer();
	auto const fmt = caps.BestMatchTextureRenderTargetFormat(
		caps.fp_color_support ? MakeSpan({EF_B10G11R11F, EF_ABGR16F}) : MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
	BOOST_ASSERT(fmt != EF_Unknown);
	auto env_tex = rf.MakeTexture2D(ENV_CUBE_MAP_SIZE, ENV_CUBE_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	auto env_srv = rf.MakeTextureSrv(env_tex);
	env_cube_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(env_tex, 0, 1, 0));
	env_cube_buffer_->Attach(depth_view);

	env_cube_tex_ = rf.MakeTextureCube(ENV_CUBE_MAP_SIZE, 1, 1, env_tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	for (int i = 0; i < 6; ++ i)
	{
		env_filter_pps_[i] = SyncLoadPostProcess("Copy.ppml", "Copy");

		env_filter_pps_[i]->InputPin(0, env_srv);
		env_filter_pps_[i]->OutputPin(0, rf.Make2DRtv(env_cube_tex_, 0, static_cast<Texture::CubeFaces>(i), 0));
	}
}

void CausticsMapApp::InitCubeSM()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	auto const ds_fmt = caps.BestMatchRenderTargetFormat(MakeSpan({EF_D24S8, EF_D16}), 1, 0);
	BOOST_ASSERT(ds_fmt != EF_Unknown);
	auto depth_view = rf.Make2DDsv(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, ds_fmt, 1, 0);
	shadow_cube_buffer_ = rf.MakeFrameBuffer();
	auto const fmt = caps.BestMatchTextureRenderTargetFormat(
		caps.pack_to_rgba_required ? MakeSpan({EF_ABGR8, EF_ARGB8}) : MakeSpan({EF_R16F, EF_R32F}), 1, 0);
	BOOST_ASSERT(fmt != EF_Unknown);
	auto shadow_tex = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	auto shadow_srv = rf.MakeTextureSrv(shadow_tex);
	shadow_cube_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(shadow_tex, 0, 1, 0));
	shadow_cube_buffer_->Attach(depth_view);

	shadow_cube_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	for (int i = 0; i < 6; ++ i)
	{
		sm_filter_pps_[i] = MakeSharedPtr<LogGaussianBlurPostProcess>(3, true);
		sm_filter_pps_[i]->InputPin(0, shadow_srv);
		sm_filter_pps_[i]->OutputPin(0, rf.Make2DRtv(shadow_cube_tex_, 0, static_cast<Texture::CubeFaces>(i), 0));
	}
}

void CausticsMapApp::InitUI()
{
	//UI Settings
	UIManager::Instance().Load(ResLoader::Instance().Open("Caustics.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	int ui_id = 0;
	ui_id = dialog_->IDFromName("Light_Density_Slider");
	dialog_->Control<UISlider>(ui_id)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->LightDensityHandler(sender);
		});
	LightDensityHandler(*(dialog_->Control<UISlider>(ui_id)));

	ui_id = dialog_->IDFromName("Refraction_Index_Slider");
	dialog_->Control<UISlider>(ui_id)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->RefractIndexHandler(sender);
		});
	RefractIndexHandler(*(dialog_->Control<UISlider>(ui_id)));

	ui_id = dialog_->IDFromName("Point_Size_Slider");
	dialog_->Control<UISlider>(ui_id)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->PointSizeHandler(sender);
		});
	PointSizeHandler(*(dialog_->Control<UISlider>(ui_id)));

	ui_id = dialog_->IDFromName("Dual_Caustics_Checkbox");
	dialog_->Control<UICheckBox>(ui_id)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->DualFaceCausticsCheckBoxHandler(sender);
		});
	DualFaceCausticsCheckBoxHandler(*(dialog_->Control<UICheckBox>(ui_id)));

	ui_id = dialog_->IDFromName("Model_Combobox");
	dialog_->Control<UIComboBox>(ui_id)->OnSelectionChangedEvent().Connect(
		[this](UIComboBox const & sender)
		{
			this->ModelSelectionComboBox(sender);
		});
	ModelSelectionComboBox(*(dialog_->Control<UIComboBox>(ui_id)));
}

void CausticsMapApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	auto fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_B10G11R11F, EF_ABGR8, EF_ARGB8}), 1, 0);
	BOOST_ASSERT(fmt != EF_Unknown);
	scene_texture_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	scene_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(scene_texture_, 0, 1, 0));
	fmt = caps.BestMatchRenderTargetFormat(MakeSpan({EF_D24S8, EF_D16}), 1, 0);
	BOOST_ASSERT(fmt != EF_Unknown);
	scene_fb_->Attach(rf.Make2DDsv(width, height, fmt, 1, 0));

	copy_pp_->InputPin(0, rf.MakeTextureSrv(scene_texture_));
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
		refract_obj_ = sphere_obj_;
		refract_obj_->Visible(true);
		refract_model_ = sphere_model_;
		break;

	default:
		refract_obj_->Visible(false);
		refract_obj_ = bunny_obj_;
		refract_obj_->Visible(true);
		refract_model_ = bunny_model_;
		break;
	}
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

	skybox_->Visible(false);
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
			checked_pointer_cast<ReceivePlane>(plane_renderable_)->CausticsPass(Position_Pass);
		}
		else
		{
			checked_pointer_cast<ReceivePlane>(plane_renderable_)->CausticsPass(Depth_WODT_Pass);
		}

		re.BindFrameBuffer(background_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
		re.CurFrameBuffer()->Viewport()->Camera(light_->SMCamera(0));

		if (depth_texture_support_)
		{
			depth_to_linear_pp_->SetParam(0, light_->SMCamera(0)->NearQFarParam());
		}

		return App3DFramework::URV_NeedFlush;
	}
	else if (1 == pass)
	{
		plane_obj_->Visible(false);
		refract_obj_->Visible(true);
		if (depth_texture_support_)
		{
			refract_model_->ForEachMesh([](Renderable& mesh)
				{
					checked_cast<RefractMesh&>(mesh).CausticsPass(Position_Normal_Front_Pass);
				});

			depth_to_linear_pp_->InputPin(0, background_ds_srv_);
			depth_to_linear_pp_->OutputPin(0, background_depth_rtv_);
			depth_to_linear_pp_->Apply();

			re.BindFrameBuffer(refract_obj_fb_f_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->Viewport()->Camera(light_->SMCamera(0));
		}
		else
		{
			refract_model_->ForEachMesh([](Renderable& mesh)
				{
					checked_cast<RefractMesh&>(mesh).CausticsPass(Depth_Front_WODT_Pass);
				});

			re.BindFrameBuffer(refract_obj_fb_d_f_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(light_->SMCamera(0)->FarPlane(), 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->Viewport()->Camera(light_->SMCamera(0));
		}

		return App3DFramework::URV_NeedFlush;
	}
	else if (2 == pass)
	{
		if (depth_texture_support_)
		{
			depth_to_linear_pp_->InputPin(0, refract_obj_ds_srv_f_);
			depth_to_linear_pp_->OutputPin(0, refract_obj_depth_rtv_f_);
			depth_to_linear_pp_->Apply();

			if (enable_dual_face_caustics_)
			{
				refract_obj_->Visible(true);
				refract_model_->ForEachMesh([](Renderable& mesh)
					{
						checked_cast<RefractMesh&>(mesh).CausticsPass(Position_Normal_Back_Pass);
					});

				re.BindFrameBuffer(refract_obj_fb_b_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
				re.CurFrameBuffer()->Viewport()->Camera(light_->SMCamera(0));
				return App3DFramework::URV_NeedFlush;
			}

			return 0;
		}
		else
		{
			plane_obj_->Visible(false);
			refract_obj_->Visible(true);

			refract_model_->ForEachMesh([](Renderable& mesh)
				{
					checked_cast<RefractMesh&>(mesh).CausticsPass(Normal_Front_WODT_Pass);
				});

			re.BindFrameBuffer(refract_obj_fb_f_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->Viewport()->Camera(light_->SMCamera(0));

			return App3DFramework::URV_NeedFlush;
		}
	}
	else if (3 == pass)
	{
		if (depth_texture_support_)
		{
			if (enable_dual_face_caustics_)
			{
				checked_pointer_cast<CausticsGrid>(caustics_grid_)->CausticsPass(Dual_Caustics_Pass);

				depth_to_linear_pp_->InputPin(0, refract_obj_ds_srv_b_);
				depth_to_linear_pp_->OutputPin(0, refract_obj_depth_rtv_b_);
				depth_to_linear_pp_->Apply();
			}
			else
			{
				checked_pointer_cast<CausticsGrid>(caustics_grid_)->CausticsPass(Single_Caustics_Pass);
			}

			re.BindFrameBuffer(caustics_fb_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->Viewport()->Camera(light_->SMCamera(0));

			caustics_grid_->Render();
			caustics_map_pps_->Apply();
		}
		else
		{
			if (enable_dual_face_caustics_)
			{
				refract_obj_->Visible(true);
				refract_model_->ForEachMesh([](Renderable& mesh)
					{
						checked_cast<RefractMesh&>(mesh).CausticsPass(Depth_Back_WODT_Pass);
					});

				re.BindFrameBuffer(refract_obj_fb_d_b_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(light_->SMCamera(0)->FarPlane(), 0.0f, 0.0f, 0.0f), 1.0f, 0);
				re.CurFrameBuffer()->Viewport()->Camera(light_->SMCamera(0));
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
			refract_model_->ForEachMesh([](Renderable& mesh)
				{
					checked_cast<RefractMesh&>(mesh).CausticsPass(Normal_Back_WODT_Pass);
				});

			re.BindFrameBuffer(refract_obj_fb_b_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
			re.CurFrameBuffer()->Viewport()->Camera(light_->SMCamera(0));
			return App3DFramework::URV_NeedFlush;
		}

		return 0;
	}
	else if (!depth_texture_support_ && (5 == pass))
	{
		if (enable_dual_face_caustics_)
		{
			checked_pointer_cast<CausticsGrid>(caustics_grid_)->CausticsPass(Dual_Caustics_Pass);
		}
		else
		{
			checked_pointer_cast<CausticsGrid>(caustics_grid_)->CausticsPass(Single_Caustics_Pass);
		}

		re.BindFrameBuffer(caustics_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
		re.CurFrameBuffer()->Viewport()->Camera(light_->SMCamera(0));

		caustics_grid_->Render();
		caustics_map_pps_->Apply();

		return 0;
	}
	else if (pass - sm_start_pass < 6)
	{
		//Shadow Cube Map Passes
	
		if (pass > sm_start_pass)
		{
			checked_pointer_cast<LogGaussianBlurPostProcess>(sm_filter_pps_[pass - sm_start_pass - 1])->ESMScaleFactor(8.0f,
				*light_->SMCamera(0));
			sm_filter_pps_[pass - sm_start_pass - 1]->Apply();
		}

		refract_obj_->Visible(true);
		plane_obj_->Visible(true);

		checked_pointer_cast<ReceivePlane>(plane_renderable_)->CausticsPass(Gen_Shadow_Pass);
		checked_pointer_cast<ReceivePlane>(plane_renderable_)->BindLight(dummy_light_);
		refract_model_->ForEachMesh([this](Renderable& mesh)
			{
				auto& refract_mesh = checked_cast<RefractMesh&>(mesh);

				refract_mesh.CausticsPass(Gen_Shadow_Pass);
				refract_mesh.BindLight(dummy_light_);
			});

		re.BindFrameBuffer(shadow_cube_buffer_);
		re.CurFrameBuffer()->AttachedDsv()->ClearDepth(1.0f);
		shadow_cube_buffer_->Viewport()->Camera(dummy_light_->SMCamera(pass - sm_start_pass));

		return App3DFramework::URV_NeedFlush;
	}
	else if (pass - env_start_pass < 6)
	{
		if (6 == pass - sm_start_pass)
		{
			checked_pointer_cast<LogGaussianBlurPostProcess>(sm_filter_pps_[pass - sm_start_pass - 1])->ESMScaleFactor(8.0f,
				*light_->SMCamera(0));
			sm_filter_pps_[pass - sm_start_pass - 1]->Apply();
		}

		// env map for reflect

		if (pass > env_start_pass)
		{
			env_filter_pps_[pass - env_start_pass - 1]->Apply();
		}

		refract_obj_->Visible(false);
		plane_obj_->Visible(true);
		skybox_->Visible(true);

		checked_pointer_cast<ReceivePlane>(plane_renderable_)->CausticsPass(Default_Pass);
		checked_pointer_cast<ReceivePlane>(plane_renderable_)->BindLight(dummy_light_);
		checked_pointer_cast<ReceivePlane>(plane_renderable_)->CausticsLight(light_);
		checked_pointer_cast<ReceivePlane>(plane_renderable_)->SetSMTexture(shadow_cube_tex_);
		checked_pointer_cast<ReceivePlane>(plane_renderable_)->SetCausticsMap(caustics_texture_filtered_);

		re.BindFrameBuffer(env_cube_buffer_);
		re.CurFrameBuffer()->AttachedDsv()->ClearDepth(1.0f);
		env_cube_buffer_->Viewport()->Camera(dummy_light_env_->SMCamera(pass - env_start_pass));
		
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
		plane_obj_->Visible(true);
		skybox_->Visible(true);

		checked_pointer_cast<ReceivePlane>(plane_renderable_)->CausticsPass(Default_Pass);
		checked_pointer_cast<ReceivePlane>(plane_renderable_)->BindLight(dummy_light_);
		checked_pointer_cast<ReceivePlane>(plane_renderable_)->CausticsLight(light_);
		checked_pointer_cast<ReceivePlane>(plane_renderable_)->SetSMTexture(shadow_cube_tex_);
		checked_pointer_cast<ReceivePlane>(plane_renderable_)->SetCausticsMap(caustics_texture_filtered_);

		re.BindFrameBuffer(scene_fb_);
		re.CurFrameBuffer()->AttachedDsv()->ClearDepth(1.0f);

		return App3DFramework::URV_NeedFlush;
	}
	else
	{
		//Scene

		skybox_->Visible(false);
		refract_obj_->Visible(true);
		plane_obj_->Visible(true);

		checked_pointer_cast<ReceivePlane>(plane_renderable_)->CausticsPass(Default_Pass);
		refract_model_->ForEachMesh([this](Renderable& mesh)
			{
				auto& refract_mesh = checked_cast<RefractMesh&>(mesh);

				refract_mesh.CausticsPass(Refract_Pass);
				refract_mesh.SceneTexture(scene_texture_);
				refract_mesh.SetEnvCube(env_cube_tex_);
			});

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->AttachedDsv()->ClearDepth(1.0f);

		copy_pp_->Apply();

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}
