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
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/half.hpp>
#include <KlayGE/FXAAPostProcess.hpp>
#include <KlayGE/SSVOPostProcess.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>

#include "DeferredRenderingLayer.hpp"
#include "DeferredRendering.hpp"

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

		std::map<std::string, TexturePtr>& TexPool()
		{
			return tex_pool_;
		}

	private:
		RenderEffectPtr effect_;
		std::map<std::string, TexturePtr> tex_pool_;
	};

	class RenderTorus : public StaticMesh, public DeferredRenderable
	{
	public:
		RenderTorus(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name),
				DeferredRenderable(checked_pointer_cast<RenderModelTorus>(model)->Effect())
		{
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
			emit_map_enabled_param_ = effect_->ParameterByName("emit_map_enabled");
			emit_tex_param_ = effect_->ParameterByName("emit_tex");
			emit_clr_param_ = effect_->ParameterByName("emit_clr");
			specular_level_param_ = effect_->ParameterByName("specular_level");
			flipping_param_ = effect_->ParameterByName("flipping");
		}

		void BuildMeshInfo()
		{
			alpha_ = false;

			boost::shared_ptr<RenderModelTorus> model = checked_pointer_cast<RenderModelTorus>(model_.lock());

			std::map<std::string, TexturePtr>& tex_pool = model->TexPool();

			RenderModel::Material const & mtl = model->GetMaterial(this->MaterialID());
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
				else if ("Self-Illumination" == iter->first)
				{
					emit_tex_ = tex;
				}
				else if ("Opacity" == iter->first)
				{
					alpha_ = true;
				}
			}
		}

		void Pass(PassType type)
		{
			type_ = type;
			technique_ = DeferredRenderable::Pass(type, alpha_);
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
				*diffuse_map_enabled_param_ = static_cast<int32_t>(!!diffuse_tex_);
				*diffuse_tex_param_ = diffuse_tex_;
				*bump_map_enabled_param_ = static_cast<int32_t>(!!bump_tex_);
				*bump_tex_param_ = bump_tex_;
				*specular_map_enabled_param_ = static_cast<int32_t>(!!specular_tex_);
				*specular_tex_param_ = specular_tex_;
				*specular_level_param_ = mtl.specular_level;
				*shininess_param_ = MathLib::clamp(mtl.shininess / 256.0f, 1e-6f, 0.999f);
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
				*emit_map_enabled_param_ = static_cast<int32_t>(!!emit_tex_);
				*emit_tex_param_ = emit_tex_;
				*emit_clr_param_ = float4(mtl.emit.x(), mtl.emit.y(), mtl.emit.z(), 1);
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
		bool alpha_;

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
		RenderEffectParameterPtr emit_map_enabled_param_;
		RenderEffectParameterPtr emit_tex_param_;
		RenderEffectParameterPtr emit_clr_param_;
		RenderEffectParameterPtr specular_level_param_;
		RenderEffectParameterPtr flipping_param_;

		TexturePtr diffuse_tex_;
		TexturePtr specular_tex_;
		TexturePtr bump_tex_;
		TexturePtr emit_tex_;
	};

	class TorusObject : public SceneObjectHelper, public DeferredSceneObject
	{
	public:
		TorusObject(RenderablePtr const & mesh)
			: SceneObjectHelper(mesh, SOA_Cullable | SOA_Deferred)
		{
			this->AttachRenderable(checked_cast<RenderTorus*>(renderable_.get()));
		}

		void Pass(PassType type)
		{
			checked_pointer_cast<RenderTorus>(renderable_)->Pass(type);
		}
	};


	class RenderPointSpotLightProxy : public StaticMesh, public DeferredRenderable
	{
	public:
		RenderPointSpotLightProxy(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name),
				DeferredRenderable(Context::Instance().RenderFactoryInstance().LoadEffect("GBuffer.fxml"))
		{
			technique_ = gbuffer_tech_;

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
			technique_ = DeferredRenderable::Pass(type, false);
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

	class SpotLightProxyObject : public SceneObjectHelper, public DeferredSceneObject
	{
	public:
		SpotLightProxyObject(LightSourcePtr const & light)
			: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_Deferred),
				light_(light)
		{
			renderable_ = LoadModel("spot_light_proxy.meshml", EAH_GPU_Read, CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderPointSpotLightProxy>())()->Mesh(0);
			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->EmitClr(light_->Color());
			model_org_ = MathLib::scaling(0.1f, 0.1f, 0.1f);

			this->AttachRenderable(checked_cast<RenderPointSpotLightProxy*>(renderable_.get()));
		}

		void Update()
		{
			model_ = model_org_ * MathLib::inverse(light_->SMCamera(0)->ViewMatrix());

			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->SetModelMatrix(model_);
			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->Update();
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_;
		}

		void Pass(PassType type)
		{
			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->Pass(type);
			this->Visible(PT_GenShadowMap != type);
		}

	private:
		float4x4 model_;
		float4x4 model_org_;

		LightSourcePtr light_;
	};

	class PointLightProxyObject : public SceneObjectHelper, public DeferredSceneObject
	{
	public:
		PointLightProxyObject(LightSourcePtr const & light)
			: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_Deferred),
				light_(light)
		{
			renderable_ = LoadModel("point_light_proxy.meshml", EAH_GPU_Read, CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderPointSpotLightProxy>())()->Mesh(0);
			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->EmitClr(light_->Color());
			model_org_ = MathLib::scaling(0.1f, 0.1f, 0.1f);

			this->AttachRenderable(checked_cast<RenderPointSpotLightProxy*>(renderable_.get()));
		}

		void Update()
		{
			model_ = model_org_ * MathLib::inverse(light_->SMCamera(0)->ViewMatrix());

			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->SetModelMatrix(model_);
			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->Update();
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_;
		}

		void Pass(PassType type)
		{
			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->Pass(type);
			this->Visible(PT_GenShadowMap != type);
		}

	private:
		float4x4 model_;
		float4x4 model_org_;

		LightSourcePtr light_;
	};

	class SpotLightSourceUpdate
	{
	public:
		SpotLightSourceUpdate(float cone_radius, float cone_height, float org_angle, float rot_speed, float3 const & pos)
			: rot_speed_(rot_speed), pos_(pos)
		{
			model_org_ = MathLib::scaling(cone_radius, cone_radius, cone_height) * MathLib::rotation_x(org_angle);
		}

		void operator()(LightSource& light)
		{
			light.ModelMatrix(MathLib::rotation_y(static_cast<float>(timer_.elapsed()) * 1000 * rot_speed_)
				* MathLib::translation(pos_));
		}

	private:
		float4x4 model_org_;
		float rot_speed_;
		float3 pos_;

		Timer timer_;
	};

	class PointLightSourceUpdate
	{
	public:
		PointLightSourceUpdate(float move_speed, float3 const & pos)
			: move_speed_(move_speed), pos_(pos)
		{
		}

		void operator()(LightSource& light)
		{
			light.ModelMatrix(MathLib::translation(sin(static_cast<float>(timer_.elapsed()) * 1000 * move_speed_), 0.0f, 0.0f)
				* MathLib::translation(pos_));
		}

	private:
		float move_speed_;
		float3 pos_;

		Timer timer_;
	};

	class RenderableDeferredHDRSkyBox : public RenderableHDRSkyBox, public DeferredRenderable
	{
	public:
		RenderableDeferredHDRSkyBox()
			: DeferredRenderable(Context::Instance().RenderFactoryInstance().LoadEffect("GBuffer.fxml"))
		{
			gbuffer_tech_ = effect_->TechniqueByName("GBufferSkyBoxTech");
			shading_tech_ = effect_->TechniqueByName("ShadingSkyBox");
			this->Technique(gbuffer_tech_);

			skybox_cube_tex_ep_ = technique_->Effect().ParameterByName("skybox_tex");
			skybox_Ccube_tex_ep_ = technique_->Effect().ParameterByName("skybox_C_tex");
			inv_mvp_ep_ = technique_->Effect().ParameterByName("inv_mvp");
		}

		void Pass(PassType type)
		{
			switch (type)
			{
			case PT_GBuffer:
				technique_ = gbuffer_tech_;
				break;

			case PT_Shading:
				technique_ = shading_tech_;
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
			: SceneObjectHDRSkyBox(SOA_Deferred)
		{
			renderable_ = MakeSharedPtr<RenderableDeferredHDRSkyBox>();
			this->AttachRenderable(checked_cast<RenderableDeferredHDRSkyBox*>(renderable_.get()));
		}

		void Pass(PassType type)
		{
			checked_pointer_cast<RenderableDeferredHDRSkyBox>(renderable_)->Pass(type);
			this->Visible(PT_GenShadowMap != type);
		}
	};

	class DeferredRenderingDebug : public PostProcess
	{
	public:
		DeferredRenderingDebug()
			: PostProcess(L"DeferredRenderingDebug")
		{
			input_pins_.push_back(std::make_pair("g_buffer_tex", TexturePtr()));
			input_pins_.push_back(std::make_pair("lighting_tex", TexturePtr()));
			input_pins_.push_back(std::make_pair("ssvo_tex", TexturePtr()));

			this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("DeferredRenderingDebug.fxml")->TechniqueByName("ShowPosition"));
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
				technique_ = technique_->Effect().TechniqueByName("ShowSSVO");
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
			*(technique_->Effect().ParameterByName("inv_proj")) = MathLib::inverse(camera.ProjMatrix());
			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
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
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	DeferredRenderingApp app;
	app.Create();
	app.Run();

	return 0;
}

DeferredRenderingApp::DeferredRenderingApp()
			: App3DFramework("DeferredRendering"),
				anti_alias_enabled_(true),
				num_objs_rendered_(0), num_renderable_rendered_(0),
				num_primitives_rendered_(0), num_vertices_rendered_(0)
{
	ResLoader::Instance().AddPath("../../Samples/media/DeferredRendering");
}

bool DeferredRenderingApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	if (!caps.rendertarget_format_support(EF_ABGR16F, 1, 0))
	{
		return false;
	}

	return true;
}

void DeferredRenderingApp::InitObjects()
{
	this->LookAt(float3(-14.5f, 15, -4), float3(-13.6f, 14.8f, -3.7f));
	this->Proj(0.1f, 500.0f);

	boost::function<RenderModelPtr()> model_ml = LoadModel("sponza_crytek.7z//sponza_crytek.meshml", EAH_GPU_Read, CreateModelFactory<RenderModelTorus>(), CreateMeshFactory<RenderTorus>());
	boost::function<TexturePtr()> y_cube_tl = LoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read);
	boost::function<TexturePtr()> c_cube_tl = LoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read);

	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	deferred_rendering_ = MakeSharedPtr<DeferredRenderingLayer>();
	
	ambient_light_ = MakeSharedPtr<AmbientLightSource>();
	ambient_light_->Color(float3(1, 1, 1));
	ambient_light_->AddToSceneManager();

	point_light_ = MakeSharedPtr<PointLightSource>();
	point_light_->Attrib(0);
	point_light_->Color(float3(6.0f, 5.88f, 4.38f));
	point_light_->Position(float3(0, 0, 0));
	point_light_->Falloff(float3(0, 0.5f, 0));
	point_light_->BindUpdateFunc(PointLightSourceUpdate(1 / 1000.0f, float3(2, 10, 0)));
	point_light_->AddToSceneManager();

	spot_light_[0] = MakeSharedPtr<SpotLightSource>();
	spot_light_[0]->Attrib(0);
	spot_light_[0]->Color(float3(2, 0, 0));
	spot_light_[0]->Falloff(float3(0, 0.5f, 0));
	spot_light_[0]->OuterAngle(PI / 6);
	spot_light_[0]->InnerAngle(PI / 8);
	spot_light_[0]->BindUpdateFunc(SpotLightSourceUpdate(sqrt(3.0f) / 3, 1.0f, PI, 1 / 1400.0f, float3(0.0f, 4.0f, 0.0f)));
	spot_light_[0]->AddToSceneManager();

	spot_light_[1] = MakeSharedPtr<SpotLightSource>();
	spot_light_[1]->Attrib(0);
	spot_light_[1]->Color(float3(0, 2, 0));
	spot_light_[1]->Falloff(float3(0, 0.5f, 0));
	spot_light_[1]->OuterAngle(PI / 4);
	spot_light_[1]->InnerAngle(PI / 6);
	spot_light_[1]->BindUpdateFunc(SpotLightSourceUpdate(1.0f, 1.0f, 0.0f, -1 / 700.0f, float3(0.0f, 3.4f, 0.0f)));
	spot_light_[1]->AddToSceneManager();

	point_light_src_ = MakeSharedPtr<PointLightProxyObject>(point_light_);
	spot_light_src_[0] = MakeSharedPtr<SpotLightProxyObject>(spot_light_[0]);
	spot_light_src_[1] = MakeSharedPtr<SpotLightProxyObject>(spot_light_[1]);
	point_light_src_->AddToSceneManager();
	spot_light_src_[0]->AddToSceneManager();
	spot_light_src_[1]->AddToSceneManager();

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&DeferredRenderingApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	//edge_anti_alias_ = MakeSharedPtr<AdaptiveAntiAliasPostProcess>();
	edge_anti_alias_ = MakeSharedPtr<FXAAPostProcess>();
	ssvo_pp_ = MakeSharedPtr<SSVOPostProcess>();
	blur_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBilateralFilterPostProcess> >(8, 1.0f);
	copy_pp_ = LoadPostProcess(ResLoader::Instance().Load("Copy.ppml"), "copy");

	debug_pp_ = MakeSharedPtr<DeferredRenderingDebug>();

	UIManager::Instance().Load(ResLoader::Instance().Load("DeferredRendering.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_buffer_combo_ = dialog_->IDFromName("BufferCombo");
	id_anti_alias_ = dialog_->IDFromName("AntiAlias");
	id_ssvo_ = dialog_->IDFromName("SSVO");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_buffer_combo_)->OnSelectionChangedEvent().connect(boost::bind(&DeferredRenderingApp::BufferChangedHandler, this, _1));
	this->BufferChangedHandler(*dialog_->Control<UIComboBox>(id_buffer_combo_));

	dialog_->Control<UICheckBox>(id_anti_alias_)->OnChangedEvent().connect(boost::bind(&DeferredRenderingApp::AntiAliasHandler, this, _1));
	this->AntiAliasHandler(*dialog_->Control<UICheckBox>(id_anti_alias_));
	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().connect(boost::bind(&DeferredRenderingApp::SSVOHandler, this, _1));
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&DeferredRenderingApp::CtrlCameraHandler, this, _1));
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

void DeferredRenderingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);
	deferred_rendering_->OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	ElementFormat fmt;
	if (rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_GR16F, 1, 0))
	{
		fmt = EF_GR16F;
	}
	else
	{
		BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_ABGR16F, 1, 0));

		fmt = EF_ABGR16F;
	}
	ssvo_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	blur_ssvo_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

	deferred_rendering_->SSVOTex(blur_ssvo_tex_);

	edge_anti_alias_->InputPin(0, deferred_rendering_->ShadingTex());

	copy_pp_->InputPin(0, deferred_rendering_->ShadingTex());

	ssvo_pp_->InputPin(0, deferred_rendering_->GBufferTex());
	ssvo_pp_->OutputPin(0, ssvo_tex_);

	blur_pp_->InputPin(0, ssvo_tex_);
	blur_pp_->OutputPin(0, blur_ssvo_tex_);

	debug_pp_->InputPin(0, deferred_rendering_->GBufferTex());
	debug_pp_->InputPin(1, deferred_rendering_->LightingTex());
	debug_pp_->InputPin(2, blur_ssvo_tex_);

	UIManager::Instance().SettleCtrls(width, height);
}

void DeferredRenderingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DeferredRenderingApp::BufferChangedHandler(UIComboBox const & sender)
{
	buffer_type_ = sender.GetSelectedIndex();
	checked_pointer_cast<DeferredRenderingDebug>(debug_pp_)->ShowType(buffer_type_);

	if (buffer_type_ != 0)
	{
		anti_alias_enabled_ = false;
	}
	else
	{
		anti_alias_enabled_ = true;
	}
	dialog_->Control<UICheckBox>(id_anti_alias_)->SetChecked(anti_alias_enabled_);

	checked_pointer_cast<FXAAPostProcess>(edge_anti_alias_)->ShowEdge(4 == buffer_type_);
}

void DeferredRenderingApp::AntiAliasHandler(UICheckBox const & sender)
{
	if (0 == buffer_type_)
	{
		anti_alias_enabled_ = sender.GetChecked();
	}
}

void DeferredRenderingApp::SSVOHandler(UICheckBox const & sender)
{
	if ((0 == buffer_type_) || (5 == buffer_type_))
	{
		ssvo_enabled_ = sender.GetChecked();
		deferred_rendering_->SSVOEnabled(ssvo_enabled_);
	}
}

void DeferredRenderingApp::CtrlCameraHandler(UICheckBox const & sender)
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

void DeferredRenderingApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Deferred Rendering", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << num_objs_rendered_ << " Scene objects "
		<< num_renderable_rendered_ << " Renderables "
		<< num_primitives_rendered_ << " Primitives "
		<< num_vertices_rendered_ << " Vertices";
	font_->RenderText(0, 54, Color(1, 1, 1, 1), stream.str(), 16);
}

uint32_t DeferredRenderingApp::DoUpdate(uint32_t pass)
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
			return App3DFramework::URV_Skip_Postprocess | App3DFramework::URV_Finished;
		}
		else
		{
			if (4 == buffer_type_)
			{
				renderEngine.BindFrameBuffer(FrameBufferPtr());
				renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
				edge_anti_alias_->Apply();
				return App3DFramework::URV_Skip_Postprocess | App3DFramework::URV_Finished;
			}
		}
	}
	else if (2 == pass)
	{
		if ((0 == buffer_type_) || (5 == buffer_type_))
		{
			if (ssvo_enabled_)
			{
				ssvo_pp_->Apply();
				blur_pp_->Apply();

				if (5 == buffer_type_)
				{
					renderEngine.BindFrameBuffer(FrameBufferPtr());
					renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
					debug_pp_->Apply();
					return App3DFramework::URV_Skip_Postprocess | App3DFramework::URV_Finished;
				}
			}
		}
	}

	uint32_t ret = deferred_rendering_->Update(pass);
	if (ret & App3DFramework::URV_Finished)
	{
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
		if (0 == buffer_type_)
		{
			if (anti_alias_enabled_)
			{
				edge_anti_alias_->Apply();
			}
			else
			{
				copy_pp_->Apply();
			}
		}
		else
		{
			if ((6 == buffer_type_) || (7 == buffer_type_))
			{
				debug_pp_->Apply();
			}
			ret |= App3DFramework::URV_Skip_Postprocess;
		}
	}

	return ret;
}
