#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderLayout.hpp>
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
#include <KlayGE/Timer.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>

#include "DeferredShadingLayer.hpp"
#include "DeferredShading.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderModelTorus : public RenderModel
	{
	public:
		RenderModelTorus(std::wstring const & name)
			: RenderModel(name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			effect_ = rf.LoadEffect("GBuffer.fxml");
		}

		RenderEffectPtr const & Effect() const
		{
			return effect_;
		}

	private:
		RenderEffectPtr effect_;
	};

	class RenderTorus : public KMesh, public DeferredRenderable
	{
	public:
		RenderTorus(RenderModelPtr const & model, std::wstring const & name)
			: KMesh(model, name),
				DeferredRenderable(checked_pointer_cast<RenderModelTorus>(model)->Effect())
		{
			gbuffer_technique_ = effect_->TechniqueByName("GBufferTech");
			gen_sm_technique_ = effect_->TechniqueByName("GenShadowMap");
			shading_technique_ = effect_->TechniqueByName("Shading");

			mvp_param_ = effect_->ParameterByName("mvp");
			model_view_param_ = effect_->ParameterByName("model_view");
			depth_near_far_invfar_param_ = effect_->ParameterByName("depth_near_far_invfar");
			shininess_param_ = effect_->ParameterByName("shininess");
			bump_map_enabled_param_ = effect_->ParameterByName("bump_map_enabled");
			bump_tex_param_ = effect_->ParameterByName("bump_tex");
			diffuse_map_enabled_param_ = effect_->ParameterByName("diffuse_map_enabled");
			diffuse_tex_param_ = effect_->ParameterByName("diffuse_tex");
			diffuse_clr_param_ = effect_->ParameterByName("diffuse_clr");
			specular_map_enabled_param_ = effect_->ParameterByName("specular_map_enabled");
			specular_tex_param_ = effect_->ParameterByName("specular_tex");
			emit_clr_param_ = effect_->ParameterByName("emit_clr");
			specular_level_param_ = effect_->ParameterByName("specular_level");
			flipping_param_ = effect_->ParameterByName("flipping");
		}

		void BuildMeshInfo()
		{
			std::map<std::string, TexturePtr> tex_pool;

			RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());
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

				if (("Diffuse Color" == iter->first) || ("Diffuse Color Map" == iter->first))
				{
					diffuse_tex_ = tex;
				}
				else if (("Specular Level" == iter->first) || ("Reflection Glossiness Map" == iter->first))
				{
					specular_tex_ = tex;
				}
				else if (("Bump" == iter->first) || ("Bump Map" == iter->first))
				{
					bump_tex_ = tex;
				}
			}
		}

		void Pass(PassType type)
		{
			type_ = type;
			technique_ = DeferredRenderable::Pass(type);
		}

		void LightingTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("lighting_tex")) = tex;
		}

		void SSAOTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("ssao_tex")) = tex;
		}

		void SSAOEnabled(bool ssao)
		{
			*(effect_->ParameterByName("ssao_enabled")) = static_cast<int32_t>(ssao);
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*mvp_param_ = view * proj;
			*model_view_param_ = view;

			RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());
			switch (type_)
			{
			case PT_GBuffer:
				*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
				*shininess_param_ = max(1e-6f, mtl.shininess);
				*diffuse_map_enabled_param_ = static_cast<int32_t>(!!diffuse_tex_);
				*diffuse_tex_param_ = diffuse_tex_;
				*bump_map_enabled_param_ = static_cast<int32_t>(!!bump_tex_);
				*bump_tex_param_ = bump_tex_;
				break;

			case PT_GenShadowMap:
				*diffuse_map_enabled_param_ = static_cast<int32_t>(!!diffuse_tex_);
				*diffuse_tex_param_ = diffuse_tex_;
				break;

			case PT_Shading:
				*shininess_param_ = max(1e-6f, mtl.shininess);
				*diffuse_map_enabled_param_ = static_cast<int32_t>(!!diffuse_tex_);
				*diffuse_tex_param_ = diffuse_tex_;
				*diffuse_clr_param_ = float4(mtl.diffuse.x(), mtl.diffuse.y(), mtl.diffuse.z(), 1);
				*emit_clr_param_ = float4(mtl.emit.x(), mtl.emit.y(), mtl.emit.z(), 1);
				*specular_map_enabled_param_ = static_cast<int32_t>(!!specular_tex_);
				*specular_tex_param_ = specular_tex_;
				*specular_level_param_ = mtl.specular_level;
				{
					RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
					*flipping_param_ = static_cast<int32_t>(re.CurFrameBuffer()->RequiresFlipping() ? -1 : +1);
				}
				break;

			default:
				break;
			}
		}

	private:
		PassType type_;

		RenderEffectParameterPtr mvp_param_;
		RenderEffectParameterPtr model_view_param_;
		RenderEffectParameterPtr depth_near_far_invfar_param_;
		RenderEffectParameterPtr shininess_param_;
		RenderEffectParameterPtr specular_map_enabled_param_;
		RenderEffectParameterPtr specular_tex_param_;
		RenderEffectParameterPtr bump_map_enabled_param_;
		RenderEffectParameterPtr bump_tex_param_;
		RenderEffectParameterPtr diffuse_map_enabled_param_;
		RenderEffectParameterPtr diffuse_tex_param_;
		RenderEffectParameterPtr diffuse_clr_param_;
		RenderEffectParameterPtr emit_clr_param_;
		RenderEffectParameterPtr specular_level_param_;
		RenderEffectParameterPtr flipping_param_;

		TexturePtr diffuse_tex_;
		TexturePtr specular_tex_;
		TexturePtr bump_tex_;
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
			: RenderableHelper(L"Cone"),
				DeferredRenderable(Context::Instance().RenderFactoryInstance().LoadEffect("GBuffer.fxml"))
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = gbuffer_technique_;

			*(effect_->ParameterByName("bump_map_enabled")) = static_cast<int32_t>(0);
			*(effect_->ParameterByName("diffuse_map_enabled")) = static_cast<int32_t>(0);

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

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

			init_data.row_pitch = static_cast<uint32_t>(normal.size() * sizeof(normal[0]));
			init_data.slice_pitch = 0;
			init_data.data = &normal[0];
			GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(float2));
			GraphicsBufferPtr texcoord_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);
			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			rl_->BindVertexStream(texcoord_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
			rl_->BindVertexStream(normal_vb, boost::make_tuple(vertex_element(VEU_Normal, 0, EF_BGR32F)));
			rl_->BindVertexStream(normal_vb, boost::make_tuple(vertex_element(VEU_Tangent, 0, EF_BGR32F)));

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
		}

		void OnRenderBegin()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			*(technique_->Effect().ParameterByName("flipping")) = static_cast<int32_t>(re.CurFrameBuffer()->RequiresFlipping() ? -1 : +1);
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
			model_ = MathLib::scaling(0.1f, 0.1f, 0.1f) * model_org_
				* MathLib::rotation_y(static_cast<float>(timer_.current_time()) * 1000 * rot_speed_)
				* MathLib::translation(0.0f, height_, 0.0f);
			checked_pointer_cast<RenderCone>(renderable_)->SetModelMatrix(model_);
			checked_pointer_cast<RenderCone>(renderable_)->Update();

			float3 p = MathLib::transform_coord(float3(0, 0, 0), model_);
			float3 d = MathLib::normalize(MathLib::transform_normal(float3(0, 0, 1), model_));
			light_->Position(p);
			light_->Direction(d);
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

		void AttachLightSrc(DeferredLightSourcePtr const & light)
		{
			light_ = checked_pointer_cast<DeferredSpotLightSource>(light);
		}

	private:
		float4x4 model_;
		float4x4 model_org_;
		float rot_speed_, height_;

		DeferredSpotLightSourcePtr light_;

		Timer timer_;
	};

	class RenderSphere : public KMesh, public DeferredRenderable
	{
	public:
		RenderSphere(RenderModelPtr const & model, std::wstring const & name)
			: KMesh(model, name),
				DeferredRenderable(Context::Instance().RenderFactoryInstance().LoadEffect("GBuffer.fxml"))
		{
			technique_ = gbuffer_technique_;

			*(effect_->ParameterByName("bump_map_enabled")) = static_cast<int32_t>(0);
			*(effect_->ParameterByName("diffuse_map_enabled")) = static_cast<int32_t>(0);

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
		}

		void OnRenderBegin()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			*(technique_->Effect().ParameterByName("flipping")) = static_cast<int32_t>(re.CurFrameBuffer()->RequiresFlipping() ? -1 : +1);
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
			model_ = MathLib::scaling(0.1f, 0.1f, 0.1f)
				* MathLib::translation(sin(static_cast<float>(timer_.current_time()) * 1000 * move_speed_), 0.0f, 0.0f)
				* MathLib::translation(pos_);
			checked_pointer_cast<RenderSphere>(renderable_)->SetModelMatrix(model_);
			checked_pointer_cast<RenderSphere>(renderable_)->Update();

			float3 p = MathLib::transform_coord(float3(0, 0, 0), model_);
			light_->Position(p);
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

		void AttachLightSrc(DeferredLightSourcePtr const & light)
		{
			light_ = checked_pointer_cast<DeferredPointLightSource>(light);
		}

	private:
		float4x4 model_;
		float move_speed_;
		float3 pos_;

		DeferredPointLightSourcePtr light_;

		Timer timer_;
	};

	class RenderableDeferredHDRSkyBox : public RenderableHDRSkyBox, public DeferredRenderable
	{
	public:
		RenderableDeferredHDRSkyBox()
			: DeferredRenderable(Context::Instance().RenderFactoryInstance().LoadEffect("GBuffer.fxml"))
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

			output_pins_.push_back(std::make_pair("out_tex", TexturePtr()));

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEffectPtr effect = rf.LoadEffect("AdaptiveAntiAliasPP.fxml");

			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if (caps.cs_support && (5 == caps.max_shader_model))
			{
				adaptive_aa_tech_ = effect->TechniqueByName("AdaptiveAntiAliasCS");
				cs_pp_ = true;
			}
			else
			{
				adaptive_aa_tech_ = effect->TechniqueByName("AdaptiveAntiAlias");
				cs_pp_ = false;
			}
			show_edge_tech_ = effect->TechniqueByName("AdaptiveAntiAliasShowEdge");

			this->Technique(adaptive_aa_tech_);
		}

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			PostProcess::InputPin(index, tex);
			if ((0 == index) && tex)
			{
				*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
			}
		}

		using PostProcess::InputPin;

		void ShowEdge(bool se)
		{
			if (se)
			{
				technique_ = show_edge_tech_;
			}
			else
			{
				technique_ = adaptive_aa_tech_;
			}
		}

		void Apply()
		{
			if (cs_pp_)
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
			else
			{
				PostProcess::Apply();
			}
		}

	protected:
		bool cs_pp_;
		RenderTechniquePtr adaptive_aa_tech_;
		RenderTechniquePtr show_edge_tech_;
	};

	class SSAOPostProcess : public PostProcess
	{
	public:
		SSAOPostProcess()
			: PostProcess(L"SSAO",
					std::vector<std::string>(),
					std::vector<std::string>(1, "src_tex"),
					std::vector<std::string>(1, "out_tex"),
					Context::Instance().RenderFactoryInstance().LoadEffect("SSAOPP.fxml")->TechniqueByName("SSVO"))
		{
			depth_near_far_invfar_param_ = technique_->Effect().ParameterByName("depth_near_far_invfar");
			upper_left_param_ = technique_->Effect().ParameterByName("upper_left");
			upper_right_param_ = technique_->Effect().ParameterByName("upper_right");
			lower_left_param_ = technique_->Effect().ParameterByName("lower_left");
			lower_right_param_ = technique_->Effect().ParameterByName("lower_right");
			proj_param_ = technique_->Effect().ParameterByName("proj");
		}

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			PostProcess::InputPin(index, tex);
			if ((0 == index) && tex)
			{
				*(technique_->Effect().ParameterByName("inv_tex_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
			}
		}

		using PostProcess::InputPin;

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

			float4x4 const & proj = camera.ProjMatrix();
			float4x4 const inv_proj = MathLib::inverse(proj);
			*upper_left_param_ = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
			*upper_right_param_ = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
			*lower_left_param_ = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
			*lower_right_param_ = MathLib::transform_coord(float3(1, -1, 1), inv_proj);
			*proj_param_ = proj;
		}

	protected:
		RenderEffectParameterPtr depth_near_far_invfar_param_;
		RenderEffectParameterPtr upper_left_param_;
		RenderEffectParameterPtr upper_right_param_;
		RenderEffectParameterPtr lower_left_param_;
		RenderEffectParameterPtr lower_right_param_;
		RenderEffectParameterPtr proj_param_;
	};

	class DeferredShadingDebug : public PostProcess
	{
	public:
		DeferredShadingDebug()
			: PostProcess(L"DeferredShadingDebug")
		{
			input_pins_.push_back(std::make_pair("g_buffer_tex", TexturePtr()));
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
				technique_ = technique_->Effect().TechniqueByName("ShowDiffuseLighting");
				break;

			case 7:
				technique_ = technique_->Effect().TechniqueByName("ShowSpecularLighting");
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
}

int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	DeferredShadingApp app;
	app.Create();
	app.Run();

	return 0;
}

DeferredShadingApp::DeferredShadingApp()
			: App3DFramework("DeferredShading"),
				anti_alias_enabled_(true),
				num_objs_rendered_(0), num_renderable_rendered_(0),
				num_primitives_rendered_(0), num_vertices_rendered_(0)
{
	ResLoader::Instance().AddPath("../Samples/media/DeferredShading");

	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.graphics_cfg.sample_count = 1;
	context_cfg.graphics_cfg.sample_quality = 0;
	Context::Instance().Config(context_cfg);
}

bool DeferredShadingApp::ConfirmDevice() const
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}

	try
	{
		TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		rf.Make2DRenderView(*temp_tex, 0, 0);
		rf.Make2DDepthStencilRenderView(800, 600, EF_D16, 1, 0);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

void DeferredShadingApp::InitObjects()
{
	this->LookAt(float3(-14.5f, 15, -4), float3(-13.6f, 14.8f, -3.7f));
	this->Proj(0.1f, 500.0f);

	boost::function<RenderModelPtr()> model_ml = LoadModel("sponza_crytek.7z//sponza_crytek.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModelTorus>(), CreateKMeshFactory<RenderTorus>());
	boost::function<TexturePtr()> y_cube_tl = LoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read);
	boost::function<TexturePtr()> c_cube_tl = LoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read);

	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	deferred_shading_ = MakeSharedPtr<DeferredShadingLayer>();
	ambient_light_ = deferred_shading_->AddAmbientLight(float3(1, 1, 1));
	point_light_ = deferred_shading_->AddPointLight(0, float3(0, 0, 0), float3(3, 3, 3), float3(0, 0.2f, 0));
	spot_light_[0] = deferred_shading_->AddSpotLight(0, float3(0, 0, 0), float3(0, 0, 0), PI / 6, PI / 8, float3(2, 0, 0), float3(0, 0.2f, 0));
	spot_light_[1] = deferred_shading_->AddSpotLight(0, float3(0, 0, 0), float3(0, 0, 0), PI / 4, PI / 6, float3(0, 2, 0), float3(0, 0.2f, 0));

	point_light_src_ = MakeSharedPtr<SphereObject>("sphere.meshml", 1 / 1000.0f, float3(2, 10, 0), point_light_->Color());
	spot_light_src_[0] = MakeSharedPtr<ConeObject>(sqrt(3.0f) / 3, 1.0f, PI, 1 / 1400.0f, 4.0f, spot_light_[0]->Color());
	spot_light_src_[1] = MakeSharedPtr<ConeObject>(1.0f, 1.0f, 0.0f, -1 / 700.0f, 3.4f, spot_light_[1]->Color());
	point_light_src_->AddToSceneManager();
	spot_light_src_[0]->AddToSceneManager();
	spot_light_src_[1]->AddToSceneManager();

	checked_pointer_cast<SphereObject>(point_light_src_)->AttachLightSrc(point_light_);
	checked_pointer_cast<ConeObject>(spot_light_src_[0])->AttachLightSrc(spot_light_[0]);
	checked_pointer_cast<ConeObject>(spot_light_src_[1])->AttachLightSrc(spot_light_[1]);

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&DeferredShadingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	edge_anti_alias_ = MakeSharedPtr<AdaptiveAntiAliasPostProcess>();
	ssao_pp_ = MakeSharedPtr<SSAOPostProcess>();
	blur_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBilateralFilterPostProcess> >(8, 1.0f);
	hdr_pp_ = MakeSharedPtr<HDRPostProcess>();

	debug_pp_ = MakeSharedPtr<DeferredShadingDebug>();

	UIManager::Instance().Load(ResLoader::Instance().Load("DeferredShading.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_buffer_combo_ = dialog_->IDFromName("BufferCombo");
	id_anti_alias_ = dialog_->IDFromName("AntiAlias");
	id_ssao_ = dialog_->IDFromName("SSAO");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_buffer_combo_)->OnSelectionChangedEvent().connect(boost::bind(&DeferredShadingApp::BufferChangedHandler, this, _1));
	this->BufferChangedHandler(*dialog_->Control<UIComboBox>(id_buffer_combo_));

	dialog_->Control<UICheckBox>(id_anti_alias_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::AntiAliasHandler, this, _1));
	this->AntiAliasHandler(*dialog_->Control<UICheckBox>(id_anti_alias_));
	dialog_->Control<UICheckBox>(id_ssao_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::SSAOHandler, this, _1));
	this->SSAOHandler(*dialog_->Control<UICheckBox>(id_ssao_));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&DeferredShadingApp::CtrlCameraHandler, this, _1));
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	scene_model_ = model_ml();
	scene_objs_.resize(scene_model_->NumMeshes());
	for (size_t i = 0; i < scene_model_->NumMeshes(); ++ i)
	{
		scene_objs_[i] = MakeSharedPtr<TorusObject>(scene_model_->Mesh(i));
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
		ssao_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	}
	catch (...)
	{
		ssao_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	}
	blur_ssao_tex_ = rf.MakeTexture2D(width, height, 1, 1, ssao_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

	{
		uint32_t access_hint = EAH_GPU_Read | EAH_GPU_Write;
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		if (caps.cs_support && (5 == caps.max_shader_model))
		{
			access_hint |= EAH_GPU_Unordered;
		}
		hdr_tex_ = rf.MakeTexture2D(width, height, 1, 1, deferred_shading_->ShadingTex()->Format(), 1, 0, access_hint, NULL);
	}

	deferred_shading_->SSAOTex(blur_ssao_tex_);

	edge_anti_alias_->InputPin(0, deferred_shading_->GBufferTex());
	edge_anti_alias_->InputPin(1, deferred_shading_->ShadingTex());
	edge_anti_alias_->OutputPin(0, hdr_tex_);

	hdr_pp_->InputPin(0, hdr_tex_);

	ssao_pp_->InputPin(0, deferred_shading_->GBufferTex());
	ssao_pp_->OutputPin(0, ssao_tex_);

	blur_pp_->InputPin(0, ssao_tex_);
	blur_pp_->OutputPin(0, blur_ssao_tex_);

	debug_pp_->InputPin(0, deferred_shading_->GBufferTex());
	debug_pp_->InputPin(1, deferred_shading_->LightingTex());
	debug_pp_->InputPin(2, blur_ssao_tex_);

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

void DeferredShadingApp::SSAOHandler(UICheckBox const & sender)
{
	if ((0 == buffer_type_) || (5 == buffer_type_))
	{
		ssao_enabled_ = sender.GetChecked();
		deferred_shading_->SSAOEnabled(ssao_enabled_);
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

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Deferred Shading", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.CurFrameBuffer()->Description(), 16);

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

	if (1 == pass)
	{
		num_objs_rendered_ = sceneMgr.NumObjectsRendered();
		num_renderable_rendered_ = sceneMgr.NumRenderablesRendered();
		num_primitives_rendered_ = sceneMgr.NumPrimitivesRendered();
		num_vertices_rendered_ = sceneMgr.NumVerticesRendered();

		if ((1 == buffer_type_) || (2 == buffer_type_) || (3 == buffer_type_))
		{
			renderEngine.BindFrameBuffer(FrameBufferPtr());
			renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
			debug_pp_->Apply();
			return App3DFramework::URV_Finished;
		}
		else
		{
			if (4 == buffer_type_)
			{
				renderEngine.BindFrameBuffer(FrameBufferPtr());
				renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
				edge_anti_alias_->Apply();
				return App3DFramework::URV_Finished;
			}
		}
	}
	else if (2 == pass)
	{
		if ((0 == buffer_type_) || (5 == buffer_type_))
		{
			if (ssao_enabled_)
			{
				ssao_pp_->Apply();
				blur_pp_->Apply();

				if (5 == buffer_type_)
				{
					renderEngine.BindFrameBuffer(FrameBufferPtr());
					renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
					debug_pp_->Apply();
					return App3DFramework::URV_Finished;
				}
			}
		}
	}

	uint32_t ret = deferred_shading_->Update(pass);
	if (App3DFramework::URV_Finished == ret)
	{
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
		if (0 == buffer_type_)
		{
			if (anti_alias_enabled_)
			{
				edge_anti_alias_->Apply();
			}
			hdr_pp_->Apply();
		}
		else
		{
			if ((6 == buffer_type_) || (7 == buffer_type_))
			{
				debug_pp_->Apply();
			}
		}
	}

	return ret;
}
