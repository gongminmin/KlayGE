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
	ResLoader::Instance().AddPath("../Samples/media/SubSurface");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	SubSurfaceApp app("SubSurface", settings);
	app.Create();
	app.Run();

	return 0;
}

SubSurfaceApp::SubSurfaceApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
}

void SubSurfaceApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	UIManager::Instance().Load(ResLoader::Instance().Load("SubSurface.uiml"));

	model_ = MakeSharedPtr<ModelObject>();
	model_->AddToSceneManager();

	this->LookAt(float3(-3, 3, -1.8f), float3(0, 1, 0), float3(0.0f, 1.0f, 0.0f));
	this->Proj(0.1f, 200.0f);

	tbController_.AttachCamera(this->ActiveCamera());
	tbController_.Scalers(0.01f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&SubSurfaceApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);
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
