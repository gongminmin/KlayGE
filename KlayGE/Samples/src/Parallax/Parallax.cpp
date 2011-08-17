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
#include <KlayGE/Light.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/JudaTexture.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <boost/bind.hpp>

#include "Parallax.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public StaticMesh
	{
	public:
		RenderPolygon(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("Parallax.fxml")->TechniqueByName("Parallax");

			tile_bb_[0] = int4(1, 1, 4, 4);
			tile_bb_[1] = int4(7, 1, 4, 4);
			tile_bb_[2] = int4(1, 7, 4, 4);

			*(technique_->Effect().ParameterByName("diffuse_tex_bb")) = tile_bb_[0];
			*(technique_->Effect().ParameterByName("normal_tex_bb")) = tile_bb_[1];
			*(technique_->Effect().ParameterByName("height_tex_bb")) = tile_bb_[2];
			*(technique_->Effect().ParameterByName("tex_size")) = int2(512, 512);
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("mvp")) = model * view * proj;
			*(technique_->Effect().ParameterByName("world")) = model;
			*(technique_->Effect().ParameterByName("eye_pos")) = app.ActiveCamera().EyePos();
		}

		void LightPos(float3 const & light_pos)
		{
			*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
		}

		void LightColor(float3 const & light_color)
		{
			*(technique_->Effect().ParameterByName("light_color")) = light_color;
		}

		void LightFalloff(float3 const & light_falloff)
		{
			*(technique_->Effect().ParameterByName("light_falloff")) = light_falloff;
		}

		void ParallaxScale(float scale)
		{
			*(technique_->Effect().ParameterByName("parallax_scale")) = scale;
		}

		void ParallaxBias(float bias)
		{
			*(technique_->Effect().ParameterByName("parallax_bias")) = bias;
		}

		void BindJudaTexture(JudaTexturePtr const & juda_tex)
		{
			juda_tex->SetParams(technique_);

			tile_ids_.clear();
			uint32_t level = juda_tex->TreeLevels() - 1;
			for (size_t i = 0; i < sizeof(tile_bb_) / sizeof(tile_bb_[0]); ++ i)
			{
				for (int32_t y = 0; y < tile_bb_[i].w(); ++ y)
				{
					for (int32_t x = 0; x < tile_bb_[i].z(); ++ x)
					{
						tile_ids_.push_back(juda_tex->EncodeTileID(level, tile_bb_[i].x() + x, tile_bb_[i].y() + y));
					}
				}
			}
		}

		std::vector<uint32_t> const & JudaTexTileIDs() const
		{
			return tile_ids_;
		}

		void DetailType(uint32_t dt)
		{
			switch (dt)
			{
			case 0:
				technique_ = technique_->Effect().TechniqueByName("None");
				break;

			case 1:
				technique_ = technique_->Effect().TechniqueByName("Bump");
				break;

			case 2:
				technique_ = technique_->Effect().TechniqueByName("Parallax");
				break;

			default:
				technique_ = technique_->Effect().TechniqueByName("ParallaxOcclusion");
				break;
			}
		}

	private:
		int4 tile_bb_[3];
		std::vector<uint32_t> tile_ids_;
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("teapot.meshml", EAH_GPU_Read, CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderPolygon>())();
		}

		void LightPos(float3 const & light_pos)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->LightPos(light_pos);
			}
		}

		void LightColor(float3 const & light_color)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->LightColor(light_color);
			}
		}

		void LightFalloff(float3 const & light_falloff)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->LightFalloff(light_falloff);
			}
		}

		void ParallaxScale(float scale)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->ParallaxScale(scale);
			}
		}

		void ParallaxBias(float bias)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->ParallaxBias(bias);
			}
		}

		void BindJudaTexture(JudaTexturePtr const & juda_tex)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->BindJudaTexture(juda_tex);
			}
		}

		std::vector<uint32_t> const & JudaTexTileIDs(uint32_t index) const
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			return checked_pointer_cast<RenderPolygon>(model->Mesh(index))->JudaTexTileIDs();
		}

		void DetailType(uint32_t dt)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->DetailType(dt);
			}
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

	Parallax app;
	app.Create();
	app.Run();

	return 0;
}

Parallax::Parallax()
			: App3DFramework("Parallax"),
				parallax_scale_(0.06f), parallax_bias_(0.02f)
{
	ResLoader::Instance().AddPath("../../Samples/media/Parallax");
}

bool Parallax::ConfirmDevice() const
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void Parallax::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");
	UIManager::Instance().Load(ResLoader::Instance().Load("Parallax.uiml"));

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	juda_tex_ = LoadJudaTexture("Parallax.jdt");

	ElementFormat fmt;
	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
	{
		fmt = EF_ABGR8;
	}
	else
	{
		BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8));

		fmt = EF_ARGB8;
	}
	juda_tex_->CacheProperty(1024, fmt, 4);

	loading_percentage_ = 0;
}

void Parallax::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void Parallax::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void Parallax::ScaleChangedHandler(KlayGE::UISlider const & sender)
{
	parallax_scale_ = sender.GetValue() / 100.0f;
	checked_pointer_cast<PolygonObject>(polygon_)->ParallaxScale(parallax_scale_);

	std::wostringstream stream;
	stream << L"Scale: " << parallax_scale_;
	dialog_->Control<UIStatic>(id_scale_static_)->SetText(stream.str());
}

void Parallax::BiasChangedHandler(KlayGE::UISlider const & sender)
{
	parallax_bias_ = sender.GetValue() / 100.0f;
	checked_pointer_cast<PolygonObject>(polygon_)->ParallaxBias(parallax_bias_);

	std::wostringstream stream;
	stream << L"Bias: " << parallax_bias_;
	dialog_->Control<UIStatic>(id_bias_static_)->SetText(stream.str());
}

void Parallax::DetailTypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	checked_pointer_cast<PolygonObject>(polygon_)->DetailType(sender.GetSelectedIndex());
}

void Parallax::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

void Parallax::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Parallax Mapping", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);
}

uint32_t Parallax::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	if (loading_percentage_ < 100)
	{
		UIDialogPtr const & dialog_loading = UIManager::Instance().GetDialog("Loading");
		UIStaticPtr const & msg = dialog_loading->Control<UIStatic>(dialog_loading->IDFromName("Msg"));
		UIProgressBarPtr const & progress_bar = dialog_loading->Control<UIProgressBar>(dialog_loading->IDFromName("Progress"));

		if (loading_percentage_ < 20)
		{
			dialog_ = UIManager::Instance().GetDialog("Parallax");
			dialog_->SetVisible(false);

			dialog_loading->SetVisible(true);

			loading_percentage_ = 20;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Loading Geometry");
		}
		else if (loading_percentage_ < 60)
		{
			polygon_ = MakeSharedPtr<PolygonObject>();
			checked_pointer_cast<PolygonObject>(polygon_)->BindJudaTexture(juda_tex_);
			juda_tex_->UpdateCache(checked_pointer_cast<PolygonObject>(polygon_)->JudaTexTileIDs(0));
			polygon_->AddToSceneManager();

			this->LookAt(float3(-0.3f, 0.4f, -0.3f), float3(0, 0, 0));
			this->Proj(0.01f, 100);

			fpcController_.Scalers(0.05f, 0.01f);

			loading_percentage_ = 60;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Loading Light");
		}
		else if (loading_percentage_ < 80)
		{
			light_ = MakeSharedPtr<PointLightSource>();
			light_->Attrib(0);
			light_->Color(float3(2, 2, 2));
			light_->Falloff(float3(0, 0, 1.0f));
			light_->Position(float3(0.25f, 0.5f, -1.0f));
			light_->AddToSceneManager();

			light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
			checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Scaling(0.01f, 0.01f, 0.01f);
			light_proxy_->AddToSceneManager();

			loading_percentage_ = 80;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Initalizing Input System");
		}
		else if (loading_percentage_ < 90)
		{
			InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
			InputActionMap actionMap;
			actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

			action_handler_t input_handler = MakeSharedPtr<input_signal>();
			input_handler->connect(boost::bind(&Parallax::InputHandler, this, _1, _2));
			inputEngine.ActionMap(actionMap, input_handler, true);

			loading_percentage_ = 90;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"Initalizing UI");
		}
		else
		{
			id_scale_static_ = dialog_->IDFromName("ScaleStatic");
			id_scale_slider_ = dialog_->IDFromName("ScaleSlider");
			id_bias_static_ = dialog_->IDFromName("BiasStatic");
			id_bias_slider_ = dialog_->IDFromName("BiasSlider");
			id_detail_type_static_ = dialog_->IDFromName("DetailTypeStatic");
			id_detail_type_combo_ = dialog_->IDFromName("DetailTypeCombo");
			id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

			dialog_->Control<UISlider>(id_scale_slider_)->SetValue(static_cast<int>(parallax_scale_ * 100));
			dialog_->Control<UISlider>(id_scale_slider_)->OnValueChangedEvent().connect(boost::bind(&Parallax::ScaleChangedHandler, this, _1));
			this->ScaleChangedHandler(*dialog_->Control<UISlider>(id_scale_slider_));

			dialog_->Control<UISlider>(id_bias_slider_)->SetValue(static_cast<int>(parallax_bias_ * 100));
			dialog_->Control<UISlider>(id_bias_slider_)->OnValueChangedEvent().connect(boost::bind(&Parallax::BiasChangedHandler, this, _1));
			this->BiasChangedHandler(*dialog_->Control<UISlider>(id_bias_slider_));

			dialog_->Control<UIComboBox>(id_detail_type_combo_)->SetSelectedByIndex(2);
			dialog_->Control<UIComboBox>(id_detail_type_combo_)->OnSelectionChangedEvent().connect(boost::bind(&Parallax::DetailTypeChangedHandler, this, _1));
			this->DetailTypeChangedHandler(*dialog_->Control<UIComboBox>(id_detail_type_combo_));

			dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&Parallax::CtrlCameraHandler, this, _1));

			loading_percentage_ = 100;
			progress_bar->SetValue(loading_percentage_);
			msg->SetText(L"DONE");

			dialog_->SetVisible(true);
			dialog_loading->SetVisible(false);
		}

		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);
		return App3DFramework::URV_Skip_Postprocess | App3DFramework::URV_Flushed | App3DFramework::URV_Finished;
	}
	else
	{
		Color clear_clr(0.2f, 0.4f, 0.6f, 1);
		if (Context::Instance().Config().graphics_cfg.gamma)
		{
			clear_clr.r() = 0.029f;
			clear_clr.g() = 0.133f;
			clear_clr.b() = 0.325f;
		}
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

		/*float degree(std::clock() / 700.0f);
		float3 lightPos(2, 0, 1);
		float4x4 matRot(MathLib::rotation_y(degree));
		lightPos = MathLib::transform_coord(lightPos, matRot);*/

		checked_pointer_cast<PolygonObject>(polygon_)->LightPos(light_->Position());
		checked_pointer_cast<PolygonObject>(polygon_)->LightColor(light_->Color());
		checked_pointer_cast<PolygonObject>(polygon_)->LightFalloff(light_->Falloff());

		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
