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
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/XMLDom.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <boost/bind.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "ScenePlayer.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SpotLightSourceUpdate
	{
	public:
		void operator()(LightSource& light, float /*app_time*/, float /*elapsed_time*/)
		{
			light.Position(float3(0, 12, -4.8f));
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
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	ScenePlayerApp app;
	app.Create();
	app.Run();

	return 0;
}

ScenePlayerApp::ScenePlayerApp()
			: App3DFramework("ScenePlayer"),
				il_scale_(1.0f)
{
	ResLoader::Instance().AddPath("../../Samples/media/ScenePlayer");
}

bool ScenePlayerApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void ScenePlayerApp::LoadScene(std::string const & name)
{
	scene_models_.clear();
	scene_objs_.clear();
	sky_box_.reset();

	lights_.clear();
	light_proxies_.clear();

	using boost::lexical_cast;

	ResIdentifierPtr ifs = ResLoader::Instance().Open(name.c_str());

	KlayGE::XMLDocument doc;
	XMLNodePtr root = doc.Parse(ifs);

	for (XMLNodePtr light_node = root->FirstNode("light"); light_node; light_node = light_node->NextSibling("light"))
	{
		LightSourcePtr light;

		uint32_t light_attr = 0;
		float3 light_clr(0, 0, 0);
		float3 fall_off(1, 0, 0);

		XMLNodePtr attribute_node = light_node->FirstNode("attribute");
		if (attribute_node)
		{
			std::string attribute_str = attribute_node->Attrib("value")->ValueString();
			if ("indirect" == attribute_str)
			{
				light_attr |= LSA_IndirectLighting;
			}
		}

		XMLNodePtr color_node = light_node->FirstNode("color");
		if (color_node)
		{
			light_clr.x() = color_node->Attrib("x")->ValueFloat();
			light_clr.y() = color_node->Attrib("y")->ValueFloat();
			light_clr.z() = color_node->Attrib("z")->ValueFloat();
		}

		XMLNodePtr fall_off_node = light_node->FirstNode("fall_off");
		if (fall_off_node)
		{
			fall_off.x() = fall_off_node->Attrib("x")->ValueFloat();
			fall_off.y() = fall_off_node->Attrib("y")->ValueFloat();
			fall_off.z() = fall_off_node->Attrib("z")->ValueFloat();
		}

		XMLAttributePtr attr = light_node->Attrib("type");
		BOOST_ASSERT(attr);

		std::string type = attr->ValueString();
		if ("spot" == type)
		{
			float3 light_pos(0, 0, 0);
			float3 light_dir(0, 0, 1);
			float outer_angle = PI / 4;
			float inner_angle = PI / 6;

			XMLNodePtr pos_node = light_node->FirstNode("position");
			if (pos_node)
			{
				light_pos.x() = pos_node->Attrib("x")->ValueFloat();
				light_pos.y() = pos_node->Attrib("y")->ValueFloat();
				light_pos.z() = pos_node->Attrib("z")->ValueFloat();
			}

			XMLNodePtr dir_node = light_node->FirstNode("direction");
			if (dir_node)
			{
				light_dir.x() = dir_node->Attrib("x")->ValueFloat();
				light_dir.y() = dir_node->Attrib("y")->ValueFloat();
				light_dir.z() = dir_node->Attrib("z")->ValueFloat();
			}

			XMLNodePtr angle_node = light_node->FirstNode("angle");
			if (angle_node)
			{
				outer_angle = angle_node->Attrib("outer")->ValueFloat();
				inner_angle = angle_node->Attrib("inner")->ValueFloat();
			}

			light = MakeSharedPtr<SpotLightSource>();
			light->Attrib(light_attr);
			light->Position(light_pos);
			light->Direction(light_dir);
			light->Color(light_clr);
			light->Falloff(fall_off);
			light->OuterAngle(outer_angle);
			light->InnerAngle(inner_angle);
			light->BindUpdateFunc(SpotLightSourceUpdate());
		}

		light->AddToSceneManager();

		lights_.push_back(light);

		XMLNodePtr proxy_node = light_node->FirstNode("proxy");
		if (proxy_node)
		{
			float scale = 1;
			XMLAttributePtr attr = proxy_node->Attrib("scale");
			if (attr)
			{
				scale = attr->ValueFloat();
			}

			SceneObjectPtr light_proxy = MakeSharedPtr<SceneObjectLightSourceProxy>(light);
			checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy)->Scaling(scale, scale, scale);
			light_proxy->AddToSceneManager();

			light_proxies_.push_back(light_proxy);
		}
	}

	for (XMLNodePtr model_node = root->FirstNode("model"); model_node; model_node = model_node->NextSibling("model"))
	{
		XMLAttributePtr attr = model_node->Attrib("name");
		BOOST_ASSERT(attr);

		RenderModelPtr model = SyncLoadModel(attr->ValueString(), EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<StaticMesh>());
		scene_models_.push_back(model);
		for (size_t i = 0; i < model->NumMeshes(); ++ i)
		{
			SceneObjectPtr scene_obj = MakeSharedPtr<SceneObjectHelper>(model->Mesh(i), SceneObject::SOA_Cullable);
			scene_objs_.push_back(scene_obj);
			scene_obj->AddToSceneManager();
		}
	}

	{
		XMLNodePtr skybox_node = root->FirstNode("skybox");

		XMLAttributePtr y_cube_attr = skybox_node->Attrib("y_cube");
		BOOST_ASSERT(y_cube_attr);

		XMLAttributePtr c_cube_attr = skybox_node->Attrib("c_cube");
		BOOST_ASSERT(c_cube_attr);

		boost::function<TexturePtr()> y_cube_tl = ASyncLoadTexture(y_cube_attr->ValueString(), EAH_GPU_Read | EAH_Immutable);
		boost::function<TexturePtr()> c_cube_tl = ASyncLoadTexture(c_cube_attr->ValueString(), EAH_GPU_Read | EAH_Immutable);

		sky_box_ = MakeSharedPtr<SceneObjectHDRSkyBox>();
		checked_pointer_cast<SceneObjectHDRSkyBox>(sky_box_)->CompressedCubeMap(y_cube_tl(), c_cube_tl());
		sky_box_->AddToSceneManager();
	}

	{
		XMLNodePtr camera_node = root->FirstNode("camera");

		XMLAttributePtr x_attr = camera_node->Attrib("x");
		BOOST_ASSERT(x_attr);

		XMLAttributePtr y_attr = camera_node->Attrib("y");
		BOOST_ASSERT(y_attr);

		XMLAttributePtr z_attr = camera_node->Attrib("z");
		BOOST_ASSERT(z_attr);

		float3 eye_pos(x_attr->ValueFloat(), y_attr->ValueFloat(), z_attr->ValueFloat());
		float3 look_at(0, 0, 0);
		float3 up(0, 1, 0);
		float near_plane = 0.1f;
		float far_plane = 500;

		XMLNodePtr look_at_node = camera_node->FirstNode("look_at");
		if (look_at_node)
		{
			XMLAttributePtr x_attr = look_at_node->Attrib("x");
			BOOST_ASSERT(x_attr);

			XMLAttributePtr y_attr = look_at_node->Attrib("y");
			BOOST_ASSERT(y_attr);

			XMLAttributePtr z_attr = look_at_node->Attrib("z");
			BOOST_ASSERT(z_attr);

			look_at = float3(x_attr->ValueFloat(), y_attr->ValueFloat(), z_attr->ValueFloat());
		}

		XMLNodePtr up_node = camera_node->FirstNode("up");
		if (up_node)
		{
			XMLAttributePtr x_attr = up_node->Attrib("x");
			BOOST_ASSERT(x_attr);

			XMLAttributePtr y_attr = up_node->Attrib("y");
			BOOST_ASSERT(y_attr);

			XMLAttributePtr z_attr = up_node->Attrib("z");
			BOOST_ASSERT(z_attr);

			up = float3(x_attr->ValueFloat(), y_attr->ValueFloat(), z_attr->ValueFloat());
		}

		XMLNodePtr near_node = camera_node->FirstNode("near_plane");
		if (near_node)
		{
			XMLAttributePtr val_attr = near_node->Attrib("value");
			BOOST_ASSERT(val_attr);

			near_plane = val_attr->ValueFloat();
		}

		XMLNodePtr far_node = camera_node->FirstNode("far_plane");
		if (far_node)
		{
			XMLAttributePtr val_attr = far_node->Attrib("value");
			BOOST_ASSERT(val_attr);

			far_plane = val_attr->ValueFloat();
		}

		this->LookAt(eye_pos, look_at, up);
		this->Proj(near_plane, far_plane);
	}
}

void ScenePlayerApp::InitObjects()
{
	this->LoadScene("GlobalIllumination.scene");

	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&ScenePlayerApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Open("ScenePlayer.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_illum_combo_ = dialog_->IDFromName("IllumCombo");
	id_il_scale_static_ = dialog_->IDFromName("ILScaleStatic");
	id_il_scale_slider_ = dialog_->IDFromName("ILScaleSlider");
	id_ssvo_ = dialog_->IDFromName("SSVO");
	id_hdr_ = dialog_->IDFromName("HDR");
	id_aa_ = dialog_->IDFromName("AA");
	id_cg_ = dialog_->IDFromName("CG");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_illum_combo_)->OnSelectionChangedEvent().connect(boost::bind(&ScenePlayerApp::IllumChangedHandler, this, _1));
	this->IllumChangedHandler(*dialog_->Control<UIComboBox>(id_illum_combo_));

	dialog_->Control<UISlider>(id_il_scale_slider_)->SetValue(static_cast<int>(il_scale_ * 10));
	dialog_->Control<UISlider>(id_il_scale_slider_)->OnValueChangedEvent().connect(boost::bind(&ScenePlayerApp::ILScaleChangedHandler, this, _1));
	this->ILScaleChangedHandler(*dialog_->Control<UISlider>(id_il_scale_slider_));

	dialog_->Control<UICheckBox>(id_ssvo_)->OnChangedEvent().connect(boost::bind(&ScenePlayerApp::SSVOHandler, this, _1));
	this->SSVOHandler(*dialog_->Control<UICheckBox>(id_ssvo_));

	dialog_->Control<UICheckBox>(id_hdr_)->OnChangedEvent().connect(boost::bind(&ScenePlayerApp::HDRHandler, this, _1));
	this->HDRHandler(*dialog_->Control<UICheckBox>(id_hdr_));

	dialog_->Control<UICheckBox>(id_aa_)->OnChangedEvent().connect(boost::bind(&ScenePlayerApp::AAHandler, this, _1));
	this->AAHandler(*dialog_->Control<UICheckBox>(id_aa_));

	dialog_->Control<UICheckBox>(id_cg_)->OnChangedEvent().connect(boost::bind(&ScenePlayerApp::ColorGradingHandler, this, _1));
	this->ColorGradingHandler(*dialog_->Control<UICheckBox>(id_cg_));

	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&ScenePlayerApp::CtrlCameraHandler, this, _1));
}

void ScenePlayerApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);
	deferred_rendering_->OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void ScenePlayerApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ScenePlayerApp::IllumChangedHandler(UIComboBox const & sender)
{
	deferred_rendering_->DisplayIllum(sender.GetSelectedIndex());
}

void ScenePlayerApp::ILScaleChangedHandler(KlayGE::UISlider const & sender)
{
	il_scale_ = sender.GetValue() / 10.0f;
	deferred_rendering_->IndirectScale(il_scale_);

	std::wostringstream stream;
	stream << L"Scale: " << il_scale_ << " x";
	dialog_->Control<UIStatic>(id_il_scale_static_)->SetText(stream.str());
}

void ScenePlayerApp::SSVOHandler(UICheckBox const & sender)
{
	deferred_rendering_->SSVOEnabled(sender.GetChecked());
}

void ScenePlayerApp::HDRHandler(UICheckBox const & sender)
{
	deferred_rendering_->HDREnabled(sender.GetChecked());
}

void ScenePlayerApp::AAHandler(UICheckBox const & sender)
{
	deferred_rendering_->AAEnabled(sender.GetChecked());
}

void ScenePlayerApp::ColorGradingHandler(UICheckBox const & sender)
{
	Context::Instance().RenderFactoryInstance().RenderEngineInstance().ColorGradingEnabled(sender.GetChecked());
}

void ScenePlayerApp::CtrlCameraHandler(UICheckBox const & sender)
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

void ScenePlayerApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Scene Player", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ScenePlayerApp::DoUpdate(uint32_t pass)
{
	return deferred_rendering_->Update(pass);
}
