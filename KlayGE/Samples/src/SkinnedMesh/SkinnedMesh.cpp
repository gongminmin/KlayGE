#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "SkinnedMesh.hpp"

using namespace KlayGE;
using namespace std;

namespace
{
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
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		return true;
	}

	struct CreateMD5ModelFactory
	{
		RenderModelPtr operator()(std::wstring const & /*name*/)
		{
			return RenderModelPtr(new MD5SkinnedModel());
		}
	};
}

int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/SkinnedMesh");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	SkinnedMeshApp app("SkinnedMesh", settings);
	app.Create();
	app.Run();

	return 0;
}

SkinnedMeshApp::SkinnedMeshApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings),
						skinned_(true), play_(false),
						last_time_(0), frame_(0)
{
}

void SkinnedMeshApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	this->LookAt(float3(250.0f, 48.0f, 0.0f), float3(0.0f, 48.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));
	this->Proj(10, 500);

	fpsController_.Scalers(0.1f, 10);

	model_ = checked_pointer_cast<MD5SkinnedModel>(LoadKModel("pinky.kmodel", EAH_GPU_Read, CreateMD5ModelFactory(), CreateKMeshFactory<MD5SkinnedMesh>()));
	model_->SetTime(0);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&SkinnedMeshApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("SkinnedMesh.kui"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_skinned_ = dialog_->IDFromName("Skinned");
	id_frame_static_ = dialog_->IDFromName("FrameStatic");
	id_frame_slider_ = dialog_->IDFromName("FrameSlider");
	id_play_ = dialog_->IDFromName("Play");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UICheckBox>(id_skinned_)->OnChangedEvent().connect(boost::bind(&SkinnedMeshApp::SkinnedHandler, this, _1));

	dialog_->Control<UISlider>(id_frame_slider_)->SetRange(model_->StartFrame(), model_->EndFrame() - 1);
	dialog_->Control<UISlider>(id_frame_slider_)->SetValue(frame_);
	dialog_->Control<UISlider>(id_frame_slider_)->OnValueChangedEvent().connect(boost::bind(&SkinnedMeshApp::FrameChangedHandler, this, _1));
	this->FrameChangedHandler(*dialog_->Control<UISlider>(id_frame_slider_));

	dialog_->Control<UICheckBox>(id_play_)->OnChangedEvent().connect(boost::bind(&SkinnedMeshApp::PlayHandler, this, _1));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&SkinnedMeshApp::CtrlCameraHandler, this, _1));
}

void SkinnedMeshApp::OnResize(uint32_t width, uint32_t height)
{
	UIManager::Instance().SettleCtrls(width, height);
}

void SkinnedMeshApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void SkinnedMeshApp::SkinnedHandler(UICheckBox const & /*sender*/)
{
	skinned_ = dialog_->Control<UICheckBox>(id_skinned_)->GetChecked();
	if (skinned_)
	{
		dialog_->Control<UICheckBox>(id_play_)->SetEnabled(true);
		model_->RebindJoints();
		this->FrameChangedHandler(*dialog_->Control<UISlider>(id_frame_slider_));
	}
	else
	{
		model_->UnbindJoints();
		dialog_->Control<UICheckBox>(id_play_)->SetChecked(false);
		dialog_->Control<UICheckBox>(id_play_)->SetEnabled(false);
	}
}

void SkinnedMeshApp::FrameChangedHandler(KlayGE::UISlider const & sender)
{
	frame_ = sender.GetValue();
	if (skinned_)
	{
		model_->SetFrame(frame_);
	}

	std::wostringstream stream;
	stream << frame_ << L":";
	dialog_->Control<UIStatic>(id_frame_static_)->SetText(stream.str());
}

void SkinnedMeshApp::PlayHandler(KlayGE::UICheckBox const & sender)
{
	play_ = sender.GetChecked();
}

void SkinnedMeshApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpsController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpsController_.DetachCamera();
	}
}

uint32_t SkinnedMeshApp::DoUpdate(KlayGE::uint32_t /*pass*/)
{
	UIManager::Instance().HandleInput();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	if (play_)
	{
		float this_time = clock() / 1000.0f;
		if (this_time - last_time_ > 1.0f / model_->FrameRate())
		{
			++ frame_;
			frame_ = frame_ % (model_->EndFrame() - model_->StartFrame()) + model_->StartFrame();

			last_time_ = this_time;
		}

		dialog_->Control<UISlider>(id_frame_slider_)->SetValue(frame_);
		this->FrameChangedHandler(*dialog_->Control<UISlider>(id_frame_slider_));
	}

	UIManager::Instance().Render();

	std::wostringstream stream;
	stream << this->FPS();

	model_->SetEyePos(this->ActiveCamera().EyePos());

	model_->AddToRenderQueue();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), renderEngine.Name());

	FrameBuffer& rw(*checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer()));
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description());

	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str());

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
