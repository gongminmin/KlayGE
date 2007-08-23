#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

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

	bool ConfirmDevice(RenderDeviceCaps const & caps)
	{
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
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.depth_stencil_fmt = EF_D24S8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	SkinnedMeshApp app("SkinnedMesh", settings);
	app.Create();
	app.Run();

	return 0;
}

SkinnedMeshApp::SkinnedMeshApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/SkinnedMesh");
}

void SkinnedMeshApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.TTF", 16);

	this->LookAt(float3(250.0f, 0.0f, 48.0f), float3(0.0f, 0.0f, 48.0f), float3(0.0f, 0.0f, 1.0f));
	this->Proj(0.1f, 1000);

	fpsController_.AttachCamera(this->ActiveCamera());
	fpsController_.Scalers(0.1f, 10);

	model_ = checked_pointer_cast<MD5SkinnedModel>(LoadKModel("pinky.kmodel", CreateMD5ModelFactory(), CreateKMeshFactory<MD5SkinnedMesh>()));
	model_->SetTime(0);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&SkinnedMeshApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);
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

uint32_t SkinnedMeshApp::DoUpdate(KlayGE::uint32_t /*pass*/)
{
	fpsController_.Update();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->Clear(Color(0.2f, 0.4f, 0.6f, 1));
	renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);

	model_->SetTime(std::clock() / 1000.0f);

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
