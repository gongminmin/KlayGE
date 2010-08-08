#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "SubSurface.hpp"

using namespace KlayGE;
using namespace std;

namespace
{
	class ModelObject : public SceneObjectHelper
	{
	public:
		ModelObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("Dragon.meshml", EAH_GPU_Read, CreateKModelFactory<DetailedModel>(), CreateKMeshFactory<DetailedMesh>())();
		}

		void SetLightPos(float3 const & light_pos)
		{
			checked_pointer_cast<DetailedModel>(renderable_)->SetLightPos(light_pos);
		}

		void SetEyePos(float3 const & eye_pos)
		{
			checked_pointer_cast<DetailedModel>(renderable_)->SetEyePos(eye_pos);
		}

		void BackFaceDepthPass(bool dfdp)
		{
			checked_pointer_cast<DetailedModel>(renderable_)->BackFaceDepthPass(dfdp);
		}

		void BackFaceDepthTex(TexturePtr const & tex, bool flipping)
		{
			checked_pointer_cast<DetailedModel>(renderable_)->BackFaceDepthTex(tex, flipping);
		}

		void SigmaT(float sigma_t)
		{
			checked_pointer_cast<DetailedModel>(renderable_)->SigmaT(sigma_t);
		}

		void MtlThickness(float thickness)
		{
			checked_pointer_cast<DetailedModel>(renderable_)->MtlThickness(thickness);
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
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		return true;
	}
}

int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	ContextCfg context_cfg = Context::Instance().LoadCfg("KlayGE.cfg");
	context_cfg.graphics_cfg.ConfirmDevice = ConfirmDevice;

	Context::Instance().Config(context_cfg);

	SubSurfaceApp app;
	app.Create();
	app.Run();

	return 0;
}

SubSurfaceApp::SubSurfaceApp()
					: App3DFramework("SubSurface")
{
	ResLoader::Instance().AddPath("../Samples/media/SubSurface");
}

void SubSurfaceApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	model_ = MakeSharedPtr<ModelObject>();
	model_->AddToSceneManager();

	this->LookAt(float3(-0.4f, 1, 3.9f), float3(0, 1, 0), float3(0.0f, 1.0f, 0.0f));
	this->Proj(0.1f, 200.0f);

	tbController_.AttachCamera(this->ActiveCamera());
	tbController_.Scalers(0.01f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&SubSurfaceApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Load("SubSurface.uiml"));
	dialog_params_ = UIManager::Instance().GetDialog("Parameters");
	id_sigma_static_ = dialog_params_->IDFromName("SigmaStatic");
	id_sigma_slider_ = dialog_params_->IDFromName("SigmaSlider");
	id_mtl_thickness_static_ = dialog_params_->IDFromName("MtlThicknessStatic");
	id_mtl_thickness_slider_ = dialog_params_->IDFromName("MtlThicknessSlider");

	dialog_params_->Control<UISlider>(id_sigma_slider_)->OnValueChangedEvent().connect(boost::bind(&SubSurfaceApp::SigmaChangedHandler, this, _1));
	this->SigmaChangedHandler(*dialog_params_->Control<UISlider>(id_sigma_slider_));

	dialog_params_->Control<UISlider>(id_mtl_thickness_slider_)->OnValueChangedEvent().connect(boost::bind(&SubSurfaceApp::MtlThicknessChangedHandler, this, _1));
	this->MtlThicknessChangedHandler(*dialog_params_->Control<UISlider>(id_mtl_thickness_slider_));
}

void SubSurfaceApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	back_face_depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	back_face_depth_fb_ = rf.MakeFrameBuffer();
	back_face_depth_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*back_face_depth_tex_, 0, 0));
	back_face_depth_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0));
	FrameBufferPtr screen_buffer = rf.RenderEngineInstance().CurFrameBuffer();
	back_face_depth_fb_->GetViewport().camera = screen_buffer->GetViewport().camera;

	checked_pointer_cast<ModelObject>(model_)->BackFaceDepthTex(back_face_depth_tex_, back_face_depth_fb_->RequiresFlipping());

	UIManager::Instance().SettleCtrls(width, height);
}

void SubSurfaceApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void SubSurfaceApp::SigmaChangedHandler(KlayGE::UISlider const & sender)
{
	float sigma_t = sender.GetValue() * 0.2f;
	checked_pointer_cast<ModelObject>(model_)->SigmaT(sigma_t);

	std::wostringstream stream;
	stream << L"Sigma_t: " << sigma_t;
	dialog_params_->Control<UIStatic>(id_sigma_static_)->SetText(stream.str());
}

void SubSurfaceApp::MtlThicknessChangedHandler(KlayGE::UISlider const & sender)
{
	float mtl_thickness = sender.GetValue() * 0.1f;
	checked_pointer_cast<ModelObject>(model_)->MtlThickness(mtl_thickness);

	std::wostringstream stream;
	stream << L"Material thickness: " << mtl_thickness;
	dialog_params_->Control<UIStatic>(id_mtl_thickness_static_)->SetText(stream.str());
}

void SubSurfaceApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), renderEngine.Name(), 16);

	FrameBuffer& rw(*checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer()));
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description(), 16);

	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t SubSurfaceApp::DoUpdate(KlayGE::uint32_t pass)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		renderEngine.BindFrameBuffer(back_face_depth_fb_);
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 0.0f, 0);
		checked_pointer_cast<ModelObject>(model_)->BackFaceDepthPass(true);
		return App3DFramework::URV_Need_Flush;

	default:
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

		checked_pointer_cast<ModelObject>(model_)->SetLightPos(float3(0, 2, -3));
		checked_pointer_cast<ModelObject>(model_)->SetEyePos(this->ActiveCamera().EyePos());
		checked_pointer_cast<ModelObject>(model_)->BackFaceDepthPass(false);

		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
