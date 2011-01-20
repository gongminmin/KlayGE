#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
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
#include <KlayGE/UI.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "PNTriangles.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class PNTrianglesSkinnedModel;

	class PNTrianglesSkinnedMesh : public SkinnedMesh
	{
	public:
		PNTrianglesSkinnedMesh(RenderModelPtr model, std::wstring const & name)
			: SkinnedMesh(model, name),
				tess_factor_(5), line_mode_(false)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			RenderEffectPtr effect = rf.LoadEffect("PNTriangles.fxml");
			if (caps.max_shader_model < 5)
			{
				pn_enabled_ = false;
				technique_ = effect->TechniqueByName("NoPNTriangles");
				rl_->TopologyType(RenderLayout::TT_TriangleList);
			}
			else
			{
				pn_enabled_ = true;
				technique_ = effect->TechniqueByName("PNTriangles");
				rl_->TopologyType(RenderLayout::TT_3_Ctrl_Pt_PatchList);
			}
		}

		void BuildMeshInfo()
		{
			TexturePtr dm;
			RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());
			RenderModel::TextureSlotsType const & texture_slots = mtl.texture_slots;
			for (RenderModel::TextureSlotsType::const_iterator iter = texture_slots.begin();
				iter != texture_slots.end(); ++ iter)
			{
				if (("DiffuseMap" == iter->first) || ("Diffuse Color" == iter->first))
				{
					if (!ResLoader::Instance().Locate(iter->second).empty())
					{
						dm = LoadTexture(iter->second, EAH_GPU_Read)();
					}
				}
			}
			*(technique_->Effect().ParameterByName("diffuse_tex")) = dm;
		}

		void SetModelMatrix(float4x4 model_matrix)
		{
			model_matrix_ = model_matrix;
		}

		void LineMode(bool line)
		{
			line_mode_ = line;
		}

		void AdaptiveTess(bool adaptive)
		{
			*(technique_->Effect().ParameterByName("adaptive_tess")) = adaptive;
		}

		void SetTessFactor(int32_t tess_factor)
		{
			tess_factor_ = static_cast<float>(tess_factor);
		}

		void EnablePNTriangles(bool pn)
		{
			pn_enabled_ = pn;
		}

		void OnRenderBegin()
		{
			if (pn_enabled_)
			{
				if (line_mode_)
				{
					technique_ = technique_->Effect().TechniqueByName("PNTrianglesLine");
				}
				else
				{
					technique_ = technique_->Effect().TechniqueByName("PNTriangles");
				}
				rl_->TopologyType(RenderLayout::TT_3_Ctrl_Pt_PatchList);
			}
			else
			{
				if (line_mode_)
				{
					technique_ = technique_->Effect().TechniqueByName("NoPNTrianglesLine");
				}
				else
				{
					technique_ = technique_->Effect().TechniqueByName("NoPNTriangles");
				}
				rl_->TopologyType(RenderLayout::TT_TriangleList);
			}

			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("world")) = model_matrix_;
			*(technique_->Effect().ParameterByName("view")) = view;
			*(technique_->Effect().ParameterByName("worldview")) = model_matrix_ * view;
			*(technique_->Effect().ParameterByName("viewproj")) = view * proj;
			*(technique_->Effect().ParameterByName("worldviewproj")) = model_matrix_ * view * proj;
			*(technique_->Effect().ParameterByName("eye_pos")) = app.ActiveCamera().EyePos();

			*(technique_->Effect().ParameterByName("tess_factors")) = float4(tess_factor_, tess_factor_, 1.0f, 9.0f);

			RenderModelPtr model = model_.lock();
			if (model)
			{
				*(technique_->Effect().ParameterByName("joint_rots")) = checked_pointer_cast<SkinnedModel>(model)->GetBindRotations();
				*(technique_->Effect().ParameterByName("joint_poss")) = checked_pointer_cast<SkinnedModel>(model)->GetBindPositions();
			}
		}

	private:
		float4x4 model_matrix_;
		float tess_factor_;
		bool line_mode_;
		bool pn_enabled_;
	};

	class PNTrianglesSkinnedModel : public SkinnedModel
	{
	public:
		PNTrianglesSkinnedModel(std::wstring const & name)
			: SkinnedModel(name)
		{
		}

		void SetModelMatrix(float4x4 model_matrix)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->SetModelMatrix(model_matrix);
			}
		}

		void LineMode(bool line)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->LineMode(line);
			}
		}

		void SetTessFactor(int32_t tess_factor)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->SetTessFactor(tess_factor);
			}
		}

		void AdaptiveTess(bool adaptive)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->AdaptiveTess(adaptive);
			}
		}

		void EnablePNTriangles(bool pn)
		{
			for (uint32_t i = 0; i < this->NumMeshes(); ++ i)
			{
				checked_pointer_cast<PNTrianglesSkinnedMesh>(Mesh(i))->EnablePNTriangles(pn);
			}
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("archer_attacking.meshml", EAH_GPU_Read, CreateModelFactory<PNTrianglesSkinnedModel>(), CreateMeshFactory<PNTrianglesSkinnedMesh>())();
			model_matrix_ = float4x4::Identity();
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->SetModelMatrix(model_matrix_);
		}

		float4x4 const & GetModelMatrix() const
		{
			return model_matrix_;
		}

		void SetTessFactor(int32_t tess_factor)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->SetTessFactor(tess_factor);
		}

		void LineMode(bool line)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->LineMode(line);
		}

		void AdaptiveTess(bool adaptive)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->AdaptiveTess(adaptive);
		}

		void EnablePNTriangles(bool pn)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->EnablePNTriangles(pn);
		}

		void SetFrame(float frame)
		{
			checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->SetFrame(frame);
		}

		uint32_t FrameRate() const
		{
			return checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->FrameRate();
		}

		uint32_t StartFrame() const
		{
			return checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->StartFrame();
		}

		uint32_t EndFrame() const
		{
			return checked_pointer_cast<PNTrianglesSkinnedModel>(renderable_)->EndFrame();
		}

	private:
		float4x4 model_matrix_;
	};


	enum
	{
		Exit,
		FullScreen,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
		InputActionDefine(FullScreen, KS_Enter),
	};
}

int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	PNTrianglesApp app;
	app.Create();
	app.Run();

	return 0;
}

PNTrianglesApp::PNTrianglesApp()
					: App3DFramework("PNTriangles"),
						tess_factor_(5)
{
	ResLoader::Instance().AddPath("../Samples/media/PNTriangles");
}

bool PNTrianglesApp::ConfirmDevice() const
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void PNTrianglesApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	polygon_ = MakeSharedPtr<PolygonObject>();
	checked_pointer_cast<PolygonObject>(polygon_)->SetFrame(0);
	polygon_->AddToSceneManager();

	this->LookAt(float3(-1.3f, 3, -2.2f), float3(0, 1.5f, 0));
	this->Proj(0.1f, 100);

	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&PNTrianglesApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("PNTriangles.uiml"));
	dialog_params_ = UIManager::Instance().GetDialog("Parameters");
	id_warning_static_ = dialog_params_->IDFromName("WarningStatic");
	id_tess_static_ = dialog_params_->IDFromName("TessStatic");
	id_tess_slider_ = dialog_params_->IDFromName("TessSlider");
	id_line_mode_ = dialog_params_->IDFromName("LineModeCheck");
	id_adaptive_tess_ = dialog_params_->IDFromName("AdaptiveTess");
	id_enable_pn_triangles_ = dialog_params_->IDFromName("EnablePNTriangles");
	id_animation_ = dialog_params_->IDFromName("Animation");
	id_fps_camera_ = dialog_params_->IDFromName("FPSCamera");

	dialog_params_->Control<UISlider>(id_tess_slider_)->OnValueChangedEvent().connect(boost::bind(&PNTrianglesApp::TessChangedHandler, this, _1));
	this->TessChangedHandler(*dialog_params_->Control<UISlider>(id_tess_slider_));

	dialog_params_->Control<UICheckBox>(id_line_mode_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::LineModeHandler, this, _1));
	dialog_params_->Control<UICheckBox>(id_adaptive_tess_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::AdaptiveTessHandler, this, _1));
	this->AdaptiveTessHandler(*dialog_params_->Control<UICheckBox>(id_adaptive_tess_));
	dialog_params_->Control<UICheckBox>(id_enable_pn_triangles_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::EnablePNTrianglesHandler, this, _1));
	dialog_params_->Control<UICheckBox>(id_animation_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::AnimationHandler, this, _1));
	this->AnimationHandler(*dialog_params_->Control<UICheckBox>(id_animation_));
	dialog_params_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(boost::bind(&PNTrianglesApp::FPSCameraHandler, this, _1));

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
	bool tess_support = (caps.max_shader_model >= 5);

	dialog_params_->Control<UIStatic>(id_warning_static_)->SetVisible(!tess_support);

	dialog_params_->Control<UISlider>(id_tess_slider_)->SetEnabled(tess_support);
	dialog_params_->Control<UIStatic>(id_tess_static_)->SetEnabled(tess_support);

	dialog_params_->Control<UICheckBox>(id_adaptive_tess_)->SetChecked(tess_support);
	dialog_params_->Control<UICheckBox>(id_adaptive_tess_)->SetEnabled(tess_support);

	dialog_params_->Control<UICheckBox>(id_enable_pn_triangles_)->SetChecked(tess_support);
	dialog_params_->Control<UICheckBox>(id_enable_pn_triangles_)->SetEnabled(tess_support);
}

void PNTrianglesApp::OnResize(uint32_t width, uint32_t height)
{
	UIManager::Instance().SettleCtrls(width, height);
}

void PNTrianglesApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case FullScreen:
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			renderEngine.EndFrame();
			ContextCfg const & cfg = Context::Instance().Config();
			renderEngine.Resize(cfg.graphics_cfg.width, cfg.graphics_cfg.height);
			renderEngine.FullScreen(!renderEngine.FullScreen());
			renderEngine.BeginFrame();
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void PNTrianglesApp::TessChangedHandler(UISlider const & sender)
{
	tess_factor_ = sender.GetValue();
	checked_pointer_cast<PolygonObject>(polygon_)->SetTessFactor(tess_factor_);

	std::wostringstream stream;
	stream << L"Tessellation factor: " << tess_factor_;
	dialog_params_->Control<UIStatic>(id_tess_static_)->SetText(stream.str());
}

void PNTrianglesApp::LineModeHandler(UICheckBox const & sender)
{
	checked_pointer_cast<PolygonObject>(polygon_)->LineMode(sender.GetChecked());
}

void PNTrianglesApp::AdaptiveTessHandler(UICheckBox const & sender)
{
	checked_pointer_cast<PolygonObject>(polygon_)->AdaptiveTess(sender.GetChecked());
}

void PNTrianglesApp::EnablePNTrianglesHandler(UICheckBox const & sender)
{
	checked_pointer_cast<PolygonObject>(polygon_)->EnablePNTriangles(sender.GetChecked());
}

void PNTrianglesApp::AnimationHandler(KlayGE::UICheckBox const & sender)
{
	animation_ = sender.GetChecked();
}

void PNTrianglesApp::FPSCameraHandler(UICheckBox const & sender)
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

void PNTrianglesApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"PNTriangles", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), renderEngine.Name(), 16);
}

uint32_t PNTrianglesApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	boost::shared_ptr<PolygonObject> obj = checked_pointer_cast<PolygonObject>(polygon_);

	if (animation_)
	{
		float this_time = clock() / 1000.0f;
		obj->SetFrame(this_time * obj->FrameRate());
	}

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
