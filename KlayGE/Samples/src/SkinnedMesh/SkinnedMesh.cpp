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

	struct CreateDetailedModelFactory
	{
		RenderModelPtr operator()(std::wstring const & /*name*/)
		{
			return RenderModelPtr(new DetailedSkinnedModel());
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
						last_time_(0), frame_(0),
						skinned_(true), play_(false)
{
}

void SkinnedMeshApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	model_ = checked_pointer_cast<DetailedSkinnedModel>(LoadKModel("felhound.kmodel", EAH_GPU_Read, CreateDetailedModelFactory(), CreateKMeshFactory<DetailedSkinnedMesh>()));
	model_->SetTime(0);

	Box const & bb = model_->GetBound();
	float3 center = bb.Center();
	float3 half_size = bb.HalfSize();

	this->LookAt(center + float3(half_size.x() * 2, half_size.y() * 2.5f, half_size.z() * 3), center, float3(0.0f, 1.0f, 0.0f));

	fpsController_.Scalers(0.1f, 10);

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
	Box const & bb = model_->GetBound();
	float near_plane = 1e10f;
	float far_plane = 1e-10f;
	for (int i = 0; i < 8; ++ i)
	{
		App3DFramework& app = Context::Instance().AppInstance();
		float4x4 const & view = app.ActiveCamera().ViewMatrix();

		float3 v = MathLib::transform_coord(bb[i], view);
		near_plane = std::min(near_plane, v.z() * 0.8f);
		far_plane = std::max(far_plane, v.z() * 1.2f);
	}
	this->Proj(std::max(0.01f, near_plane), far_plane);

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

	model_->SetLightPos(float3(20, 100, 100));
	model_->SetEyePos(this->ActiveCamera().EyePos());

	model_->AddToRenderQueue();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), renderEngine.Name(), 16);

	FrameBuffer& rw(*checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer()));
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description(), 16);

	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
