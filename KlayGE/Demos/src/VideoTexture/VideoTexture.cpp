#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <KlayGE/Show.hpp>
#include <KlayGE/DShow/DShowFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>

#include "VideoTexture.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTeapot : public KMesh
	{
	public:
		RenderTeapot(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Teapot"),
				video_sampler_(new Sampler)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("VideoTexture.fx")->Technique("Object");

			video_sampler_->Filtering(Sampler::TFO_Bilinear);
			video_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			video_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("mvp")) = view * proj;
			*(technique_->Effect().ParameterByName("video_sampler")) = video_sampler_;
		}

		void VideoTexture(TexturePtr video_tex)
		{
			video_sampler_->SetTexture(video_tex);
		}

	private:
		SamplerPtr video_sampler_;
	};

	class TeapotObject : public SceneObjectHelper
	{
	public:
		TeapotObject()
			: SceneObjectHelper(SOA_Cullable | SOA_ShortAge)
		{
			renderable_ = LoadKModel("teapot.kmodel", CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderTeapot>())->Mesh(0);
		}

		void VideoTexture(TexturePtr video_tex)
		{
			checked_pointer_cast<RenderTeapot>(renderable_)->VideoTexture(video_tex);
		}
	};

	enum
	{
		Exit
	};

	InputActionDefine actions[] = 
	{
		InputActionDefine(Exit, KS_Escape)
	};

	bool ConfirmDevice(RenderDeviceCaps const & caps)
	{
		if (caps.max_shader_model < 1)
		{
			return false;
		}
		return true;
	}
}


int main()
{
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().InputFactoryInstance(DInputFactoryInstance());
	Context::Instance().ShowFactoryInstance(DShowFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	VideoTextureApp app;
	app.Create("Video Texture", settings);
	app.Run();

	return 0;
}

VideoTextureApp::VideoTextureApp()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/VideoTexture");
}

void VideoTextureApp::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(float3(-0.2f, 0.3f, -0.2f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&VideoTextureApp::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);

	ground_.reset(new TeapotObject);
	
	ShowEngine& se = Context::Instance().ShowFactoryInstance().ShowEngineInstance();
	se.Load(ResLoader::Instance().Locate("planete.avi"));
	se.Play();
}

void VideoTextureApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

uint32_t VideoTextureApp::NumPasses() const
{
	return 1;
}

void VideoTextureApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	ShowEngine& se = Context::Instance().ShowFactoryInstance().ShowEngineInstance();

	re.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

	TexturePtr tex = se.PresentTexture();
	checked_pointer_cast<TeapotObject>(ground_)->VideoTexture(tex);
	ground_->AddToSceneManager();

	fpcController_.Update();

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Video Texture");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
}
