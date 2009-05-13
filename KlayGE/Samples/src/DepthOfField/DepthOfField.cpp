#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/SATPostProcess.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "DepthOfField.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const NUM_INSTANCE = 400;

	class Teapot : public SceneObjectHelper
	{
	private:
		struct InstData
		{
			float4 col[3];
			Color clr;
		};

	public:
		Teapot()
			: SceneObjectHelper(SOA_Cullable)
		{
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 1, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 2, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 3, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_Diffuse, 0, EF_ABGR32F));
		}

		void Instance(float4x4 const & mat, Color const & clr)
		{
			float4x4 matT = MathLib::transpose(mat);

			inst_.col[0] = matT.Row(0);
			inst_.col[1] = matT.Row(1);
			inst_.col[2] = matT.Row(2);
			inst_.clr = clr;
		}

		void const * InstanceData() const
		{
			return &inst_;
		}

		void SetRenderable(RenderablePtr ra)
		{
			renderable_ = ra;
		}

	private:
		InstData inst_;
	};

	class RenderInstance : public KMesh
	{
	public:
		RenderInstance(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Instance")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("DepthOfField.fxml")->TechniqueByName("ColorDepth");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("view")) = view;
			*(technique_->Effect().ParameterByName("view_proj")) = view * proj;

			*(technique_->Effect().ParameterByName("light_in_world")) = float3(2, 2, -3);

			*(technique_->Effect().ParameterByName("depth_min")) = app.ActiveCamera().NearPlane();
			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / (app.ActiveCamera().FarPlane() - app.ActiveCamera().NearPlane());
		}
	};

	class ClearFloatPostProcess : public PostProcess
	{
	public:
		ClearFloatPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DepthOfField.fxml")->TechniqueByName("ClearFloat"))
		{
		}

		void ClearColor(float4 const & clr)
		{
			*(technique_->Effect().ParameterByName("clear_clr")) = clr;
		}

		void OnRenderBegin()
		{
		}
	};

	class DepthOfField : public PostProcess
	{
	public:
		DepthOfField()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DepthOfField.fxml")->TechniqueByName("DepthOfField"))
		{
		}

		void FocusPlane(float focus_plane)
		{
			focus_plane_ = focus_plane;
		}
		float FocusPlane() const
		{
			return focus_plane_;
		}

		void FocusRange(float focus_range)
		{
			focus_range_ = focus_range;
		}
		float FocusRange() const
		{
			return focus_range_;
		}

		void ShowBlurFactor(bool show)
		{
			show_blur_factor_ = show;
			*(technique_->Effect().ParameterByName("show_blur_factor")) = show_blur_factor_;
		}
		bool ShowBlurFactor() const
		{
			return show_blur_factor_;
		}

		void Source(TexturePtr const & tex, bool flipping)
		{
			PostProcess::Source(tex, flipping);

			uint32_t const width = tex->Width(0);
			uint32_t const height = tex->Height(0);

			sat_.Source(tex, flipping);

			*(technique_->Effect().ParameterByName("sat_size")) = float4(static_cast<float>(width),
				static_cast<float>(height), 1.0f / width, 1.0f / height);
		}

		void Apply()
		{
			if (!show_blur_factor_)
			{
				sat_.Apply();
				*(technique_->Effect().ParameterByName("sat_tex")) = sat_.SATTexture();
			}

			PostProcess::Apply();
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			float const depth_range = app.ActiveCamera().FarPlane() - app.ActiveCamera().NearPlane();

			*(technique_->Effect().ParameterByName("focus_plane")) = (focus_plane_ - app.ActiveCamera().NearPlane()) / depth_range;
			*(technique_->Effect().ParameterByName("inv_focus_range")) = depth_range / focus_range_;
		}

	private:
		SummedAreaTablePostProcess sat_;

		float focus_plane_;
		float focus_range_;
		bool show_blur_factor_;
	};


	enum
	{
		CtrlCamera,
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(CtrlCamera, KS_LeftCtrl),
		InputActionDefine(CtrlCamera, KS_RightCtrl),
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

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0);
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
	ResLoader::Instance().AddPath("../Samples/media/DepthOfField");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	DepthOfFieldApp app("Depth of field", settings);
	app.Create();
	app.Run();

	return 0;
}

DepthOfFieldApp::DepthOfFieldApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
}

void DepthOfFieldApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	RenderablePtr renderInstance = LoadModel("teapot.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderInstance>())->Mesh(0);
	for (int i = 0; i < 10; ++ i)
	{
		for (int j = 0; j < NUM_INSTANCE / 10; ++ j)
		{
			float const s = sin(2 * PI * j / (NUM_INSTANCE / 10));
			float const c = cos(2 * PI * j / (NUM_INSTANCE / 10));

			SceneObjectPtr so = MakeSharedPtr<Teapot>();
			checked_pointer_cast<Teapot>(so)->Instance(
				MathLib::translation(s, i / 10.0f, c), Color(s, c, 0, 1));

			checked_pointer_cast<Teapot>(so)->SetRenderable(renderInstance);
			so->AddToSceneManager();
		}
	}

	this->LookAt(float3(-1.8f, 1.9f, -1.8f), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	clr_depth_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	clr_depth_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&DepthOfFieldApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	depth_of_field_ = MakeSharedPtr<DepthOfField>();
	depth_of_field_->Destinate(FrameBufferPtr());

	clear_float_ = MakeSharedPtr<ClearFloatPostProcess>();
	checked_pointer_cast<ClearFloatPostProcess>(clear_float_)->ClearColor(float4(0.2f - 0.5f, 0.4f - 0.5f, 0.6f - 0.5f, 1 - 0.5f));

	UIManager::Instance().Load(ResLoader::Instance().Load("DepthOfField.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_focus_plane_static_ = dialog_->IDFromName("FocusPlaneStatic");
	id_focus_plane_slider_ = dialog_->IDFromName("FocusPlaneSlider");
	id_focus_range_static_ = dialog_->IDFromName("FocusRangeStatic");
	id_focus_range_slider_ = dialog_->IDFromName("FocusRangeSlider");
	id_blur_factor_ = dialog_->IDFromName("BlurFactor");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UISlider>(id_focus_plane_slider_)->OnValueChangedEvent().connect(boost::bind(&DepthOfFieldApp::FocusPlaneChangedHandler, this, _1));
	dialog_->Control<UISlider>(id_focus_range_slider_)->OnValueChangedEvent().connect(boost::bind(&DepthOfFieldApp::FocusRangeChangedHandler, this, _1));

	dialog_->Control<UICheckBox>(id_blur_factor_)->OnChangedEvent().connect(boost::bind(&DepthOfFieldApp::BlurFactorHandler, this, _1));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&DepthOfFieldApp::CtrlCameraHandler, this, _1));

	this->FocusPlaneChangedHandler(*dialog_->Control<UISlider>(id_focus_plane_slider_));
	this->FocusRangeChangedHandler(*dialog_->Control<UISlider>(id_focus_range_slider_));
	this->BlurFactorHandler(*dialog_->Control<UICheckBox>(id_blur_factor_));
}

void DepthOfFieldApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	clr_depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	clr_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*clr_depth_tex_, 0));
	clr_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0));

	depth_of_field_->Source(clr_depth_tex_, clr_depth_buffer_->RequiresFlipping());
	depth_of_field_->Destinate(FrameBufferPtr());

	clear_float_->Destinate(clr_depth_buffer_);

	UIManager::Instance().SettleCtrls(width, height);
}

void DepthOfFieldApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DepthOfFieldApp::FocusPlaneChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusPlane(sender.GetValue() / 50.0f);
}

void DepthOfFieldApp::FocusRangeChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusRange(sender.GetValue() / 50.0f);
}

void DepthOfFieldApp::BlurFactorHandler(KlayGE::UICheckBox const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->ShowBlurFactor(sender.GetChecked());
}

void DepthOfFieldApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

uint32_t DepthOfFieldApp::DoUpdate(uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		renderEngine.BindFrameBuffer(clr_depth_buffer_);
		clear_float_->Apply();
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
		return App3DFramework::URV_Need_Flush;

	default:
		depth_of_field_->Apply();

		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);

		UIManager::Instance().Render();

		FrameBuffer& rw = *renderEngine.CurFrameBuffer();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Depth of field", 16);
		font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description(), 16);

		std::wostringstream stream;
		stream << this->FPS() << " FPS";
		font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
		return App3DFramework::URV_Only_New_Objs | App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
