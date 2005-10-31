#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>

#include "Refract.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RefractorRenderable : public KMesh
	{
	public:
		RefractorRenderable(std::wstring const & /*name*/, TexturePtr tex)
			: KMesh(L"Refractor", tex),
				cubemap_sampler_(new Sampler)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("Refract.fx");
			effect_->ActiveTechnique("Refract");

			cubemap_sampler_->Filtering(Sampler::TFO_Bilinear);
			cubemap_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			cubemap_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			cubemap_sampler_->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(effect_->ParameterByName("cubeMapSampler")) = cubemap_sampler_;
		}

		void CubeMap(TexturePtr const & texture)
		{
			cubemap_sampler_->SetTexture(texture);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			Matrix4 const & model = Matrix4::Identity();
			Matrix4 const & view = app.ActiveCamera().ViewMatrix();
			Matrix4 const & proj = app.ActiveCamera().ProjMatrix();

			*(effect_->ParameterByName("model")) = model;
			*(effect_->ParameterByName("modelit")) = MathLib::Transpose(MathLib::Inverse(model));
			*(effect_->ParameterByName("mvp")) = model * view * proj;
		}

	private:
		SamplerPtr cubemap_sampler_;
	};

	class RefractorObject : public SceneObjectHelper
	{
	public:
		RefractorObject(TexturePtr cube_map)
			: SceneObjectHelper(true, false)
		{
			renderable_ = LoadKMesh("teapot.kmesh", CreateFactory<RefractorRenderable>)->Mesh(0);
			checked_cast<RefractorRenderable*>(renderable_.get())->CubeMap(cube_map);	
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

	bool ConfirmDevice(RenderDeviceCaps const & caps)
	{
		if (caps.max_shader_model < 2)
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

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;
	settings.ConfirmDevice = ConfirmDevice;

	Refract app;
	app.Create("Refract", settings);
	app.Run();

	return 0;
}

Refract::Refract()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/Refract");
}

void Refract::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	cube_map_ = LoadTexture("Glacier2.dds");

	refractor_.reset(new RefractorObject(cube_map_));
	refractor_->AddToSceneManager();

	sky_box_.reset(new SceneObjectSkyBox);
	static_cast<SceneObjectSkyBox*>(sky_box_.get())->CubeMap(cube_map_);
	sky_box_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(-0.05f, -0.01f, -0.5f), Vector3(0, 0.05f, 0));
	this->Proj(0.05f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&Refract::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void Refract::InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void Refract::DoUpdate(uint32_t pass)
{
	fpcController_.Update();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	*(refractor_->GetRenderable()->GetRenderEffect()->ParameterByName("eyePos"))
		= Vector4(this->ActiveCamera().EyePos().x(), this->ActiveCamera().EyePos().y(),
			this->ActiveCamera().EyePos().z(), 1);

	std::wostringstream stream;
	stream << renderEngine.ActiveRenderTarget(0)->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Refract");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
}
