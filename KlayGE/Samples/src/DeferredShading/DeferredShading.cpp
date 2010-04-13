#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "DeferredShadingLayer.hpp"
#include "DeferredShading.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTorus : public KMesh, public DeferredRenderable
	{
	public:
		RenderTorus(RenderModelPtr const & model, std::wstring const & name)
			: KMesh(model, name)
		{
			mvp_param_ = effect_->ParameterByName("mvp");
			model_view_param_ = effect_->ParameterByName("model_view");
			depth_near_far_invfar_param_ = effect_->ParameterByName("depth_near_far_invfar");
		}

		void BuildMeshInfo()
		{
			std::map<std::string, TexturePtr> tex_pool;

			RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());

			bool diffuse_map_enabled = false;
			bool bump_map_enabled = false;
			RenderModel::TextureSlotsType const & texture_slots = mtl.texture_slots;
			for (RenderModel::TextureSlotsType::const_iterator iter = texture_slots.begin();
				iter != texture_slots.end(); ++ iter)
			{
				TexturePtr tex;
				BOOST_AUTO(titer, tex_pool.find(iter->second));
				if (titer != tex_pool.end())
				{
					tex = titer->second;
				}
				else
				{
					tex = LoadTexture(iter->second, EAH_GPU_Read)();
					tex_pool.insert(std::make_pair(iter->second, tex));
				}
				diffuse_map_enabled = tex;

				if ("Diffuse Color" == iter->first)
				{
					*(effect_->ParameterByName("diffuse_tex")) = tex;
				}
				if ("Bump" == iter->first)
				{
					*(effect_->ParameterByName("bump_tex")) = tex;
					bump_map_enabled = tex;
				}
			}

			*(effect_->ParameterByName("bump_map_enabled")) = bump_map_enabled;
			*(effect_->ParameterByName("diffuse_map_enabled")) = diffuse_map_enabled;

			*(effect_->ParameterByName("diffuse_clr")) = float4(mtl.diffuse.x(), mtl.diffuse.y(), mtl.diffuse.z(), 1);
			*(effect_->ParameterByName("emit_clr")) = float4(mtl.emit.x(), mtl.emit.y(), mtl.emit.z(), 1);
			*(effect_->ParameterByName("specular_level")) = mtl.specular_level;
			*(effect_->ParameterByName("shininess")) = MathLib::clamp(mtl.shininess / 256.0f, 0.0f, 1.0f);
		}

		void Pass(PassType type)
		{
			technique_ = DeferredRenderable::Pass(type);
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*mvp_param_ = view * proj;
			*model_view_param_ = view;

			*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset() * 2;
			float const x_offset = texel_to_pixel.x() / re.CurFrameBuffer()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurFrameBuffer()->Height();
			*(technique_->Effect().ParameterByName("texel_to_pixel_offset")) = float4(x_offset, y_offset, 0, 0);
			*(technique_->Effect().ParameterByName("flipping")) = static_cast<int32_t>(re.CurFrameBuffer()->RequiresFlipping() ? -1 : +1);
		}

	private:
		RenderEffectParameterPtr mvp_param_;
		RenderEffectParameterPtr model_view_param_;
		RenderEffectParameterPtr depth_near_far_invfar_param_;
	};

	class TorusObject : public SceneObjectHelper, public DeferredSceneObject
	{
	public:
		TorusObject(RenderablePtr const & mesh)
			: SceneObjectHelper(mesh, SOA_Cullable)
		{
		}

		void Pass(PassType type)
		{
			checked_pointer_cast<RenderTorus>(renderable_)->Pass(type);
		}

		void LightingTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderTorus>(renderable_)->LightingTex(tex);
		}

		void SSAOTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderTorus>(renderable_)->SSAOTex(tex);
		}

		void SSAOEnabled(bool ssao)
		{
			checked_pointer_cast<RenderTorus>(renderable_)->SSAOEnabled(ssao);
		}
	};


	class RenderCone : public RenderableHelper, public DeferredRenderable
	{
	public:
		RenderCone(float cone_radius, float cone_height, float3 const & clr)
			: RenderableHelper(L"Cone")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = gbuffer_technique_;

			*(effect_->ParameterByName("bump_map_enabled")) = false;
			*(effect_->ParameterByName("diffuse_map_enabled")) = false;

			*(effect_->ParameterByName("diffuse_clr")) = float4(1, 1, 1, 1);
			*(effect_->ParameterByName("emit_clr")) = float4(clr.x(), clr.y(), clr.z(), 1);

			mvp_param_ = effect_->ParameterByName("mvp");
			model_view_param_ = effect_->ParameterByName("model_view");
			depth_near_far_invfar_param_ = effect_->ParameterByName("depth_near_far_invfar");

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateConeMesh(pos, index, 0, cone_radius, cone_height, 12);

			std::vector<float3> normal(pos.size());
			MathLib::compute_normal<float>(normal.begin(), index.begin(), index.end(), pos.begin(), pos.end());

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];

			rl_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data),
				boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(normal.size() * sizeof(normal[0]));
			init_data.slice_pitch = 0;
			init_data.data = &normal[0];
			rl_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data),
				boost::make_tuple(vertex_element(VEU_Normal, 0, EF_BGR32F)));
			rl_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data),
				boost::make_tuple(vertex_element(VEU_Tangent, 0, EF_BGR32F)));
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(float2));
			rl_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data),
				boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.slice_pitch = 0;
			init_data.data = &index[0];

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindIndexStream(ib, EF_R16UI);

			box_ = MathLib::compute_bounding_box<float>(pos.begin(), pos.end());
		}

		void SetModelMatrix(float4x4 const & mat)
		{
			model_ = mat;
		}

		void Pass(PassType type)
		{
			technique_ = DeferredRenderable::Pass(type);
		}

		void Update()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			float4x4 mv = model_ * view;
			*mvp_param_ = mv * proj;
			*model_view_param_ = mv;

			*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset() * 2;
			float const x_offset = texel_to_pixel.x() / re.CurFrameBuffer()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurFrameBuffer()->Height();
			*(technique_->Effect().ParameterByName("texel_to_pixel_offset")) = float4(x_offset, y_offset, 0, 0);
			*(technique_->Effect().ParameterByName("flipping")) = static_cast<int32_t>(re.CurFrameBuffer()->RequiresFlipping() ? -1 : +1);
		}

		void OnRenderBegin()
		{
			this->Update();
		}

	private:
		float4x4 model_;

		RenderEffectParameterPtr mvp_param_;
		RenderEffectParameterPtr model_view_param_;
		RenderEffectParameterPtr depth_near_far_invfar_param_;
	};

	class ConeObject : public SceneObjectHelper, public DeferredSceneObject
	{
	public:
		ConeObject(float cone_radius, float cone_height, float org_angle, float rot_speed, float height, float3 const & clr)
			: SceneObjectHelper(SOA_Cullable | SOA_Moveable), rot_speed_(rot_speed), height_(height)
		{
			renderable_ = MakeSharedPtr<RenderCone>(cone_radius, cone_height, clr);
			model_org_ = MathLib::rotation_x(org_angle);
		}

		void Update()
		{
			model_ = MathLib::scaling(0.1f, 0.1f, 0.1f) * model_org_ * MathLib::rotation_y(std::clock() * rot_speed_) * MathLib::translation(0.0f, height_, 0.0f);
			checked_pointer_cast<RenderCone>(renderable_)->SetModelMatrix(model_);
			checked_pointer_cast<RenderCone>(renderable_)->Update();
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_;
		}

		void Pass(PassType type)
		{
			checked_pointer_cast<RenderCone>(renderable_)->Pass(type);
			this->Visible(PT_GenShadowMap != type);
		}

		void LightingTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderCone>(renderable_)->LightingTex(tex);
		}

		void SSAOTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderCone>(renderable_)->SSAOTex(tex);
		}

		void SSAOEnabled(bool ssao)
		{
			checked_pointer_cast<RenderCone>(renderable_)->SSAOEnabled(ssao);
		}

	private:
		float4x4 model_;
		float4x4 model_org_;
		float rot_speed_, height_;
	};

	class RenderSphere : public KMesh, public DeferredRenderable
	{
	public:
		RenderSphere(RenderModelPtr const & model, std::wstring const & name)
			: KMesh(model, name)
		{
			technique_ = gbuffer_technique_;

			*(effect_->ParameterByName("bump_map_enabled")) = false;
			*(effect_->ParameterByName("diffuse_map_enabled")) = false;

			*(effect_->ParameterByName("diffuse_clr")) = float4(1, 1, 1, 1);

			mvp_param_ = effect_->ParameterByName("mvp");
			model_view_param_ = effect_->ParameterByName("model_view");
			depth_near_far_invfar_param_ = effect_->ParameterByName("depth_near_far_invfar");
		}

		void BuildMeshInfo()
		{
		}

		void SetModelMatrix(float4x4 const & mat)
		{
			model_ = mat;
		}

		void EmitClr(float3 const & clr)
		{
			*(effect_->ParameterByName("emit_clr")) = float4(clr.x(), clr.y(), clr.z(), 1);
		}

		void Pass(PassType type)
		{
			technique_ = DeferredRenderable::Pass(type);
		}

		void Update()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			float4x4 mv = model_ * view;
			*mvp_param_ = mv * proj;
			*model_view_param_ = mv;

			*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset() * 2;
			float const x_offset = texel_to_pixel.x() / re.CurFrameBuffer()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurFrameBuffer()->Height();
			*(technique_->Effect().ParameterByName("texel_to_pixel_offset")) = float4(x_offset, y_offset, 0, 0);
			*(technique_->Effect().ParameterByName("flipping")) = static_cast<int32_t>(re.CurFrameBuffer()->RequiresFlipping() ? -1 : +1);
		}

		void OnRenderBegin()
		{
			this->Update();
		}

	private:
		float4x4 model_;

		RenderEffectParameterPtr mvp_param_;
		RenderEffectParameterPtr model_view_param_;
		RenderEffectParameterPtr depth_near_far_invfar_param_;
	};

	class SphereObject : public SceneObjectHelper, public DeferredSceneObject
	{
	public:
		SphereObject(std::string const & model_name, float move_speed, float3 const & pos, float3 const & clr)
			: SceneObjectHelper(SOA_Cullable | SOA_Moveable), move_speed_(move_speed), pos_(pos)
		{
			renderable_ = LoadModel(model_name, EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderSphere>())()->Mesh(0);
			checked_pointer_cast<RenderSphere>(renderable_)->EmitClr(clr);
		}

		void Update()
		{
			model_ = MathLib::scaling(0.1f, 0.1f, 0.1f) * MathLib::translation(sin(std::clock() * move_speed_), 0.0f, 0.0f) * MathLib::translation(pos_);
			checked_pointer_cast<RenderSphere>(renderable_)->SetModelMatrix(model_);
			checked_pointer_cast<RenderSphere>(renderable_)->Update();
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_;
		}

		void Pass(PassType type)
		{
			checked_pointer_cast<RenderSphere>(renderable_)->Pass(type);
			this->Visible(PT_GenShadowMap != type);
		}

		void LightingTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderSphere>(renderable_)->LightingTex(tex);
		}

		void SSAOTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderSphere>(renderable_)->SSAOTex(tex);
		}

		void SSAOEnabled(bool ssao)
		{
			checked_pointer_cast<RenderSphere>(renderable_)->SSAOEnabled(ssao);
		}

	private:
		float4x4 model_;
		float move_speed_;
		float3 pos_;
	};

	class RenderableDeferredHDRSkyBox : public RenderableHDRSkyBox, public DeferredRenderable
	{
	public:
		RenderableDeferredHDRSkyBox()
		{
			gbuffer_technique_ = effect_->TechniqueByName("GBufferSkyBoxTech");
			shading_technique_ = effect_->TechniqueByName("ShadingSkyBox");
			this->Technique(gbuffer_technique_);

			skybox_cube_tex_ep_ = technique_->Effect().ParameterByName("skybox_tex");
			skybox_Ccube_tex_ep_ = technique_->Effect().ParameterByName("skybox_C_tex");
			inv_mvp_ep_ = technique_->Effect().ParameterByName("inv_mvp");
		}

		void Pass(PassType type)
		{
			switch (type)
			{
			case PT_GBuffer:
				technique_ = gbuffer_technique_;
				break;

			case PT_Shading:
				technique_ = shading_technique_;
				break;

			default:
				break;
			}
		}
	};

	class SceneObjectDeferredHDRSkyBox : public SceneObjectHDRSkyBox, public DeferredSceneObject
	{
	public:
		SceneObjectDeferredHDRSkyBox()
		{
			renderable_ = MakeSharedPtr<RenderableDeferredHDRSkyBox>();
		}

		void Pass(PassType type)
		{
			checked_pointer_cast<RenderableDeferredHDRSkyBox>(renderable_)->Pass(type);
			this->Visible(PT_GenShadowMap != type);
		}

		void LightingTex(TexturePtr const & /*tex*/)
		{
		}

		void SSAOTex(TexturePtr const & /*tex*/)
		{
		}

		void SSAOEnabled(bool /*ssao*/)
		{
		}
	};

	class AdaptiveAntiAliasPostProcess : public PostProcess
	{
	public:
		AdaptiveAntiAliasPostProcess()
			: PostProcess(L"AdaptiveAntiAlias")
		{
			input_pins_.push_back(std::make_pair("src_tex", TexturePtr()));
			input_pins_.push_back(std::make_pair("color_tex", TexturePtr()));

			output_pins_.push_back(std::make_pair("output", TexturePtr()));

			this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("AdaptiveAntiAliasPP.fxml")->TechniqueByName("AdaptiveAntiAlias"));
		}

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			PostProcess::InputPin(index, tex);
			if ((0 == index) && tex)
			{
				*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
			}
		}

		void ShowEdge(bool se)
		{
			if (se)
			{
				technique_ = technique_->Effect().TechniqueByName("AdaptiveAntiAliasShowEdge");
			}
			else
			{
				technique_ = technique_->Effect().TechniqueByName("AdaptiveAntiAlias");
			}
		}
	};

	class SSAOPostProcess : public PostProcess
	{
	public:
		SSAOPostProcess()
			: PostProcess(L"SSAO",
					std::vector<std::string>(1, "src_tex"),
					std::vector<std::string>(1, "out_tex"),
					Context::Instance().RenderFactoryInstance().LoadEffect("SSAOPP.fxml")->TechniqueByName("SSAOHigh")),
				ssao_level_(4)
		{
			depth_near_far_invfar_param_ = technique_->Effect().ParameterByName("depth_near_far_invfar");
			rt_size_inv_size_param_ = technique_->Effect().ParameterByName("rt_size_inv_size");

			ssao_techs_[0] = technique_->Effect().TechniqueByName("SSAOLow");
			ssao_techs_[1] = technique_->Effect().TechniqueByName("SSAOMiddle");
			ssao_techs_[2] = technique_->Effect().TechniqueByName("SSAOHigh");
		}

		void SSAOLevel(int level)
		{
			ssao_level_ = level;
			if (level > 0)
			{
				technique_ = ssao_techs_[level - 1];
			}
		}

		void Apply()
		{
			if (ssao_level_ > 0)
			{
				PostProcess::Apply();
			}
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			*rt_size_inv_size_param_ = float4(1.0f * re.CurFrameBuffer()->Width(), 1.0f * re.CurFrameBuffer()->Height(),
				1.0f / re.CurFrameBuffer()->Width(), 1.0f / re.CurFrameBuffer()->Height());
		}

	protected:
		int ssao_level_;

		RenderTechniquePtr ssao_techs_[4];

		RenderEffectParameterPtr depth_near_far_invfar_param_;
		RenderEffectParameterPtr rt_size_inv_size_param_;
	};

	class SSAOPostProcessCS : public SSAOPostProcess
	{
	public:
		SSAOPostProcessCS()
		{
			ssao_techs_[0] = technique_->Effect().TechniqueByName("SSAOLowCS");
			ssao_techs_[1] = technique_->Effect().TechniqueByName("SSAOMiddleCS");
			ssao_techs_[2] = technique_->Effect().TechniqueByName("SSAOHighCS");

			technique_ = ssao_techs_[2];
		}

		void Apply()
		{
			if (ssao_level_ > 0)
			{
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				re.BindFrameBuffer(re.DefaultFrameBuffer());

				TexturePtr const & tex = this->InputPin(0);

				int const BLOCK_SIZE_X = 16;
				int const BLOCK_SIZE_Y = 16;

				this->OnRenderBegin();
				re.Dispatch(*technique_, (tex->Width(0) + (BLOCK_SIZE_X - 1)) / BLOCK_SIZE_X, (tex->Height(0) + (BLOCK_SIZE_Y - 1)) / BLOCK_SIZE_Y, 1);
				this->OnRenderEnd();
			}
		}
	};

	class DeferredShadingDebug : public PostProcess
	{
	public:
		DeferredShadingDebug()
			: PostProcess(L"DeferredShadingDebug")
		{
			input_pins_.push_back(std::make_pair("nd_tex", TexturePtr()));
			input_pins_.push_back(std::make_pair("lighting_tex", TexturePtr()));
			input_pins_.push_back(std::make_pair("ssao_tex", TexturePtr()));

			this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredShadingDebug.fxml")->TechniqueByName("ShowPosition"));
		}

		void ShowType(int show_type)
		{
			switch (show_type)
			{
			case 0:
				break;

			case 1:
				technique_ = technique_->Effect().TechniqueByName("ShowPosition");
				break;

			case 2:
				technique_ = technique_->Effect().TechniqueByName("ShowNormal");
				break;

			case 3:
				technique_ = technique_->Effect().TechniqueByName("ShowDepth");
				break;

			case 4:
				break;

			case 5:
				technique_ = technique_->Effect().TechniqueByName("ShowSSAO");
				break;

			case 6:
				technique_ = technique_->Effect().TechniqueByName("ShowLighting");
				break;

			default:
				break;
			}
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			float4x4 const inv_proj = MathLib::inverse(camera.ProjMatrix());

			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

			*(technique_->Effect().ParameterByName("upper_left")) = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("upper_right")) = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_left")) = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_right")) = MathLib::transform_coord(float3(1, -1, 1), inv_proj);
		}
	};


	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};

	bool ConfirmDevice()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		if (caps.max_simultaneous_rts < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0, 0);
			rf.MakeDepthStencilRenderView(800, 600, EF_D16, 1, 0);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}

int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/DeferredShading");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	DeferredShadingApp app("DeferredShading", settings);
	app.Create();
	app.Run();

	return 0;
}

DeferredShadingApp::DeferredShadingApp(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings),
				anti_alias_enabled_(true)
{
}

void DeferredShadingApp::InitObjects()
{
	boost::function<RenderModelPtr()> model_ml = LoadModel("sponza.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderTorus>());
	boost::function<TexturePtr()> y_cube_tl = LoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read);
	boost::function<TexturePtr()> c_cube_tl = LoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read);

	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	deferred_shading_ = MakeSharedPtr<DeferredShadingLayer>();
	ambient_light_ = deferred_shading_->AddAmbientLight(float3(1, 1, 1));
	point_light_ = deferred_shading_->AddPointLight(0, float3(0, 0, 0), float3(1, 1, 1), float3(0, 0.5f, 0));
	spot_light_[0] = deferred_shading_->AddSpotLight(0, float3(0, 0, 0), float3(0, 0, 0), PI / 6, PI / 8, float3(1, 0, 0), float3(0, 0.5f, 0));
	spot_light_[1] = deferred_shading_->AddSpotLight(0, float3(0, 0, 0), float3(0, 0, 0), PI / 4, PI / 6, float3(0, 1, 0), float3(0, 0.5f, 0));

	point_light_src_ = MakeSharedPtr<SphereObject>("sphere.meshml", 1 / 1000.0f, float3(2, 5, 0), point_light_->Color());
	spot_light_src_[0] = MakeSharedPtr<ConeObject>(sqrt(3.0f) / 3, 1.0f, PI, 1 / 1400.0f, 2.0f, spot_light_[0]->Color());
	spot_light_src_[1] = MakeSharedPtr<ConeObject>(1.0f, 1.0f, 0.0f, -1 / 700.0f, 1.7f, spot_light_[1]->Color());
	point_light_src_->AddToSceneManager();
	spot_light_src_[0]->AddToSceneManager();
	spot_light_src_[1]->AddToSceneManager();

	this->LookAt(float3(-2, 2, 0), float3(0, 2, 0));
	this->Proj(0.1f, 500.0f);

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&DeferredShadingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	edge_anti_alias_ = MakeSharedPtr<AdaptiveAntiAliasPostProcess>();

	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.cs_support && (5 == caps.max_shader_model))
	{
		ssao_pp_ = MakeSharedPtr<SSAOPostProcessCS>();
	}
	else
	{
		ssao_pp_ = MakeSharedPtr<SSAOPostProcess>();
	}
	hdr_pp_ = MakeSharedPtr<HDRPostProcess>(true, false);

	debug_pp_ = MakeSharedPtr<DeferredShadingDebug>();

	UIManager::Instance().Load(ResLoader::Instance().Load("DeferredShading.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_buffer_combo_ = dialog_->IDFromName("BufferCombo");
	id_anti_alias_ = dialog_->IDFromName("AntiAlias");
	id_ssao_combo_ = dialog_->IDFromName("SSAOCombo");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_buffer_combo_)->OnSelectionChangedEvent().connect(boost::bind(&DeferredShadingApp::BufferChangedHandler, this, _1));
	this->BufferChangedHandler(*dialog_->Control<UIComboBox>(id_buffer_combo_));

	dialog_->Control<UICheckBox>(id_anti_alias_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::AntiAliasHandler, this, _1));
	this->AntiAliasHandler(*dialog_->Control<UICheckBox>(id_anti_alias_));
	dialog_->Control<UIComboBox>(id_ssao_combo_)->OnSelectionChangedEvent().connect(boost::bind(&DeferredShadingApp::SSAOChangedHandler, this, _1));
	this->SSAOChangedHandler(*dialog_->Control<UIComboBox>(id_ssao_combo_));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::CtrlCameraHandler, this, _1));
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	RenderModelPtr model = model_ml();
	scene_objs_.resize(model->NumMeshes());
	for (size_t i = 0; i < model->NumMeshes(); ++ i)
	{
		scene_objs_[i] = MakeSharedPtr<TorusObject>(model->Mesh(i));
		scene_objs_[i]->AddToSceneManager();
	}

	sky_box_ = MakeSharedPtr<SceneObjectDeferredHDRSkyBox>();
	checked_pointer_cast<SceneObjectDeferredHDRSkyBox>(sky_box_)->CompressedCubeMap(y_cube_tl(), c_cube_tl());
	sky_box_->AddToSceneManager();
}

void DeferredShadingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);
	deferred_shading_->OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	try
	{
		uint32_t access_hint = EAH_GPU_Read | EAH_GPU_Write;
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		if (caps.cs_support && (5 == caps.max_shader_model))
		{
			access_hint |= EAH_GPU_Unordered;
		}
		ssao_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_R16F, 1, 0, access_hint, NULL);
	}
	catch (...)
	{
		ssao_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	}

	hdr_tex_ = rf.MakeTexture2D(width, height, 1, 1, deferred_shading_->ShadingTex()->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

	deferred_shading_->SSAOTex(ssao_tex_);

	edge_anti_alias_->InputPin(0, deferred_shading_->NormalDepthTex());
	edge_anti_alias_->InputPin(1, deferred_shading_->ShadingTex());
	edge_anti_alias_->OutputPin(0, hdr_tex_);

	hdr_pp_->InputPin(0, hdr_tex_);

	ssao_pp_->InputPin(0, deferred_shading_->NormalDepthTex());
	ssao_pp_->OutputPin(0, ssao_tex_);

	debug_pp_->InputPin(0, deferred_shading_->NormalDepthTex());
	debug_pp_->InputPin(1, deferred_shading_->LightingTex());
	debug_pp_->InputPin(2, ssao_tex_);

	UIManager::Instance().SettleCtrls(width, height);
}

void DeferredShadingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DeferredShadingApp::BufferChangedHandler(UIComboBox const & sender)
{
	buffer_type_ = sender.GetSelectedIndex();
	checked_pointer_cast<DeferredShadingDebug>(debug_pp_)->ShowType(buffer_type_);

	if (buffer_type_ != 0)
	{
		anti_alias_enabled_ = false;
	}
	else
	{
		anti_alias_enabled_ = true;
		edge_anti_alias_->OutputPin(0, hdr_tex_);
	}
	dialog_->Control<UICheckBox>(id_anti_alias_)->SetChecked(anti_alias_enabled_);

	checked_pointer_cast<AdaptiveAntiAliasPostProcess>(edge_anti_alias_)->ShowEdge(4 == buffer_type_);
	if (4 == buffer_type_)
	{
		edge_anti_alias_->OutputPin(0, TexturePtr());
	}
	else
	{
		edge_anti_alias_->OutputPin(0, hdr_tex_);
	}
}

void DeferredShadingApp::AntiAliasHandler(UICheckBox const & sender)
{
	if (0 == buffer_type_)
	{
		anti_alias_enabled_ = sender.GetChecked();
		if (anti_alias_enabled_)
		{
			edge_anti_alias_->OutputPin(0, hdr_tex_);

			if (hdr_tex_)
			{
				hdr_pp_->InputPin(0, hdr_tex_);
			}
		}
		else
		{
			hdr_pp_->InputPin(0, deferred_shading_->ShadingTex());
		}
	}
}

void DeferredShadingApp::SSAOChangedHandler(UIComboBox const & sender)
{
	if ((0 == buffer_type_) || (5 == buffer_type_))
	{
		int ssao_level = sender.GetNumItems() - 1 - sender.GetSelectedIndex();
		checked_pointer_cast<SSAOPostProcess>(ssao_pp_)->SSAOLevel(ssao_level);
		deferred_shading_->SSAOEnabled(ssao_level != 0);
	}
}

void DeferredShadingApp::CtrlCameraHandler(UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpcController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpcController_.DetachCamera();
	}
}

void DeferredShadingApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	FrameBuffer& rw = *checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer());

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Deferred Shading", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << num_objs_rendered_ << " Scene objects "
		<< num_renderable_rendered_ << " Renderables "
		<< num_primitives_rendered_ << " Primitives "
		<< num_vertices_rendered_ << " Vertices";
	font_->RenderText(0, 54, Color(1, 1, 1, 1), stream.str(), 16);
}

uint32_t DeferredShadingApp::DoUpdate(uint32_t pass)
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		{
			float4x4 model_mat = checked_pointer_cast<SphereObject>(point_light_src_)->GetModelMatrix();
			float3 p = MathLib::transform_coord(float3(0, 0, 0), model_mat);
			point_light_->Position(p);
		}
		for (int i = 0; i < 2; ++ i)
		{
			float4x4 model_mat = checked_pointer_cast<ConeObject>(spot_light_src_[i])->GetModelMatrix();
			float3 p = MathLib::transform_coord(float3(0, 0, 0), model_mat);
			float3 d = MathLib::normalize(MathLib::transform_normal(float3(0, 0, 1), model_mat));
			spot_light_[i]->Position(p);
			spot_light_[i]->Direction(d);
		}
		break;

	case 1:
		num_objs_rendered_ = sceneMgr.NumObjectsRendered();
		num_renderable_rendered_ = sceneMgr.NumRenderablesRendered();
		num_primitives_rendered_ = sceneMgr.NumPrimitivesRendered();
		num_vertices_rendered_ = sceneMgr.NumVerticesRendered();

		if ((0 == buffer_type_) || (5 == buffer_type_) || (6 == buffer_type_))
		{
			ssao_pp_->Apply();
		}

		break;
	}

	uint32_t ret = deferred_shading_->Update(pass);
	if (App3DFramework::URV_Finished == ret)
	{
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
		if ((buffer_type_ > 0) && (buffer_type_ != 4))
		{
			debug_pp_->Apply();
		}
		else
		{
			if (((0 == buffer_type_) && anti_alias_enabled_) || (4 == buffer_type_))
			{
				edge_anti_alias_->Apply();
			}
			if (0 == buffer_type_)
			{
				hdr_pp_->Apply();
			}
		}
	}

	return ret;
}
