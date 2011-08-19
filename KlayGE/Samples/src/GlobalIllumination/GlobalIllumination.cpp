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
#include <KlayGE/Mesh.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>

#include "GlobalIllumination.hpp"

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
			effect_ = rf.LoadEffect("GBufferAdv.fxml");
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
				DeferredRenderable(checked_pointer_cast<RenderModelTorus>(model)->Effect()),
				special_shading_(false)
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

			if ((mtl.emit.x() > 0) || (mtl.emit.y() > 0) || (mtl.emit.z() > 0))
			{
				special_shading_ = true;
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
			case PT_MRTGBuffer:
			case PT_GenReflectiveShadowMap:
				*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());
				*diffuse_map_enabled_param_ = static_cast<int32_t>(!!diffuse_tex_);
				*diffuse_tex_param_ = diffuse_tex_;
				*diffuse_clr_param_ = float4(mtl.diffuse.x(), mtl.diffuse.y(), mtl.diffuse.z(), 1);
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

			case PT_SpecialShading:
				*emit_map_enabled_param_ = static_cast<int32_t>(!!emit_tex_);
				*emit_tex_param_ = emit_tex_;
				*emit_clr_param_ = float4(mtl.emit.x(), mtl.emit.y(), mtl.emit.z(), 1);
				break;

			default:
				break;
			}
		}

		bool SpecialShading() const
		{
			return special_shading_;
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

		bool special_shading_;
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

			if (PT_SpecialShading == type)
			{
				this->Visible(checked_pointer_cast<RenderTorus>(renderable_)->SpecialShading());
			}
			else
			{
				this->Visible(true);
			}
		}
	};


	class RenderPointSpotLightProxy : public StaticMesh, public DeferredRenderable
	{
	public:
		RenderPointSpotLightProxy(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name),
				DeferredRenderable(Context::Instance().RenderFactoryInstance().LoadEffect("GBufferAdv.fxml"))
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
		SpotLightProxyObject(float cone_radius, float cone_height, float3 const & clr)
			: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_Deferred)
		{
			renderable_ = LoadModel("spot_light_proxy.meshml", EAH_GPU_Read, CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderPointSpotLightProxy>())()->Mesh(0);
			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->EmitClr(clr);
			model_org_ = MathLib::scaling(cone_radius, cone_radius, cone_height);

			this->AttachRenderable(checked_cast<RenderPointSpotLightProxy*>(renderable_.get()));
		}

		void Update()
		{
			model_ = MathLib::scaling(0.1f, 0.1f, 0.1f) * model_org_
				* MathLib::inverse(light_->SMCamera(0)->ViewMatrix());

			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->SetModelMatrix(model_);
			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->Update();

			light_->ModelMatrix(model_);
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_;
		}

		void Pass(PassType type)
		{
			checked_pointer_cast<RenderPointSpotLightProxy>(renderable_)->Pass(type);
			this->Visible((PT_GenShadowMap != type) && (PT_GenReflectiveShadowMap != type));
		}

		void AttachLightSrc(LightSourcePtr const & light)
		{
			light_ = light;
		}

	private:
		float4x4 model_;
		float4x4 model_org_;

		LightSourcePtr light_;

		Timer timer_;
	};

	class RenderableDeferredHDRSkyBox : public RenderableHDRSkyBox, public DeferredRenderable
	{
	public:
		RenderableDeferredHDRSkyBox()
			: DeferredRenderable(Context::Instance().RenderFactoryInstance().LoadEffect("GBufferAdv.fxml"))
		{
			gbuffer_tech_ = effect_->TechniqueByName("GBufferSkyBoxTech");
			gbuffer_mrt_tech_ = effect_->TechniqueByName("GBufferSkyBoxMRTTech");
			shading_tech_ = effect_->TechniqueByName("ShadingSkyBox");
			special_shading_tech_ = shading_tech_;
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

			case PT_MRTGBuffer:
				technique_ = gbuffer_mrt_tech_;
				break;

			case PT_Shading:
				technique_ = shading_tech_;
				break;

			case PT_SpecialShading:
				technique_ = special_shading_tech_;
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
			this->Visible((PT_GenShadowMap != type) && (PT_GenReflectiveShadowMap != type));
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

	ContextCfg cfg = Context::Instance().Config();
	cfg.graphics_cfg.hdr = false;
	Context::Instance().Config(cfg);

	GlobalIlluminationApp app;
	app.Create();
	app.Run();

	return 0;
}

GlobalIlluminationApp::GlobalIlluminationApp()
			: App3DFramework("GlobalIllumination"),
				il_scale_(1.0f)
{
	ResLoader::Instance().AddPath("../../Samples/media/GlobalIllumination");
}

bool GlobalIlluminationApp::ConfirmDevice() const
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

void GlobalIlluminationApp::InitObjects()
{
	this->LookAt(float3(-14.5f, 18, -3), float3(-13.6f, 17.55f, -2.8f));
	this->Proj(0.1f, 500.0f);

	boost::function<RenderModelPtr()> model_ml = LoadModel("sponza_crytek.7z//sponza_crytek.meshml", EAH_GPU_Read, CreateModelFactory<RenderModelTorus>(), CreateMeshFactory<RenderTorus>());
	boost::function<TexturePtr()> y_cube_tl = LoadTexture("Lake_CraterLake03_y.dds", EAH_GPU_Read);
	boost::function<TexturePtr()> c_cube_tl = LoadTexture("Lake_CraterLake03_c.dds", EAH_GPU_Read);

	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	deferred_rendering_ = MakeSharedPtr<DeferredRenderingLayer>();

	ambient_light_ = MakeSharedPtr<AmbientLightSource>();
	ambient_light_->Color(float3(0.0f, 0.0f, 0.0f));
	ambient_light_->AddToSceneManager();

	spot_light_ = MakeSharedPtr<SpotLightSource>();
	spot_light_->Attrib(LSA_IndirectLighting);
	spot_light_->Position(float3(0, 12, -4.8f));
	spot_light_->Direction(float3(0, 0, 1));
	spot_light_->Color(float3(6.0f, 5.88f, 4.38f));
	spot_light_->Falloff(float3(0, 0.1f, 0));
	spot_light_->OuterAngle(PI / 4);
	spot_light_->InnerAngle(PI / 6);
	spot_light_->AddToSceneManager();

	spot_light_src_ = MakeSharedPtr<SpotLightProxyObject>(sqrt(3.0f) / 3, 1.0f, spot_light_->Color());
	spot_light_src_->AddToSceneManager();

	checked_pointer_cast<SpotLightProxyObject>(spot_light_src_)->AttachLightSrc(spot_light_);

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&GlobalIlluminationApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	copy_pp_ = LoadPostProcess(ResLoader::Instance().Load("Copy.ppml"), "copy");

	UIManager::Instance().Load(ResLoader::Instance().Load("GlobalIllumination.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_illum_combo_ = dialog_->IDFromName("IllumCombo");
	id_il_scale_static_ = dialog_->IDFromName("ILScaleStatic");
	id_il_scale_slider_ = dialog_->IDFromName("ILScaleSlider");
	id_ssvo_ = dialog_->IDFromName("SSVO");
	id_hdr_ = dialog_->IDFromName("HDR");
	id_aa_ = dialog_->IDFromName("AA");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().connect(boost::bind(&GlobalIlluminationApp::IllumChangedHandler, this, _1));
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().connect(boost::bind(&GlobalIlluminationApp::ILScaleChangedHandler, this, _1));
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::SSVOHandler, this, _1));
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));

	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::HDRHandler, this, _1));
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));

	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::AAHandler, this, _1));
	this->AAHandler(*dialog_->Control<UICheckBox>(id_aa_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&GlobalIlluminationApp::CtrlCameraHandler, this, _1));

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

void GlobalIlluminationApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);
	deferred_rendering_->OnResize(width, height);

	copy_pp_->InputPin(0, deferred_rendering_->ShadingTex());
	
	UIManager::Instance().SettleCtrls(width, height);
}

void GlobalIlluminationApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void GlobalIlluminationApp::IllumChangedHandler(UIComboBox const & sender)
{
	deferred_rendering_->DisplayIllum(sender.GetSelectedIndex());
}

void GlobalIlluminationApp::ILScaleChangedHandler(KlayGE::UISlider const & sender)
{
	il_scale_ = sender.GetValue() / 10.0f;
	deferred_rendering_->IndirectScale(il_scale_);

	std::wostringstream stream;
	stream << L"Scale: " << il_scale_ << " x";
	dialog_->Control<UIStatic>(id_il_scale_static_)->SetText(stream.str());
}

void GlobalIlluminationApp::SSVOHandler(UICheckBox const & sender)
{
	deferred_rendering_->SSVOEnabled(sender.GetChecked());
}

void GlobalIlluminationApp::HDRHandler(UICheckBox const & sender)
{
	deferred_rendering_->HDREnabled(sender.GetChecked());
}

void GlobalIlluminationApp::AAHandler(UICheckBox const & sender)
{
	deferred_rendering_->AAEnabled(sender.GetChecked());
}

void GlobalIlluminationApp::CtrlCameraHandler(UICheckBox const & sender)
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

void GlobalIlluminationApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Global Illumination", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t GlobalIlluminationApp::DoUpdate(uint32_t pass)
{
	return deferred_rendering_->Update(pass);
}
