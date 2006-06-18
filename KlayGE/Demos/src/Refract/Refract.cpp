#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
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
	class HDRSkyBox : public RenderableSkyBox
	{
	public:
		HDRSkyBox()
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			technique_ = rf.LoadEffect("HDRSkyBox.fx")->Technique("HDRSkyBoxTec");

			cube_sampler_->Filtering(Sampler::TFO_Bilinear);
			cube_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			cube_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			cube_sampler_->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("skybox_cubeMapSampler")) = cube_sampler_;
		}

		void ExposureLevel(float exposure_level)
		{
			*(technique_->Effect().ParameterByName("exposure_level")) = exposure_level;
		}

	private:
	};

	class HDRSceneObjectSkyBox : public SceneObjectSkyBox
	{
	public:
		HDRSceneObjectSkyBox()
		{
			renderable_.reset(new HDRSkyBox);
		}
	};

	class RefractorRenderable : public KMesh
	{
	public:
		RefractorRenderable(std::wstring const & /*name*/, TexturePtr tex)
			: KMesh(L"Refractor", tex),
				cubemap_sampler_(new Sampler)
		{
			technique_ = Context::Instance().RenderFactoryInstance().LoadEffect("Refract.fx")->Technique("Refract");

			cubemap_sampler_->Filtering(Sampler::TFO_Bilinear);
			cubemap_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			cubemap_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			cubemap_sampler_->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("cubeMapSampler")) = cubemap_sampler_;
		}

		void CubeMap(TexturePtr const & texture)
		{
			cubemap_sampler_->SetTexture(texture);
		}

		void ExposureLevel(float exposure_level)
		{
			*(technique_->Effect().ParameterByName("exposure_level")) = exposure_level;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("model")) = model;
			*(technique_->Effect().ParameterByName("modelit")) = MathLib::transpose(MathLib::inverse(model));
			*(technique_->Effect().ParameterByName("mvp")) = model * view * proj;

			*(technique_->Effect().ParameterByName("eta_ratio")) = float3(1 / 1.1f, 1 / 1.1f - 0.003f, 1 / 1.1f - 0.006f);
			*(technique_->Effect().ParameterByName("fresnel_values")) = float3(2.0f, 2.0f, 0.1f);

			*(technique_->Effect().ParameterByName("eyePos")) = Context::Instance().AppInstance().ActiveCamera().EyePos();
		}

	private:
		SamplerPtr cubemap_sampler_;
	};

	class RefractorObject : public SceneObjectHelper
	{
	public:
		RefractorObject(TexturePtr cube_map)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKMesh("teapot.kmesh", CreateKMeshFactory<RefractorRenderable>())->Mesh(0);
			checked_cast<RefractorRenderable*>(renderable_.get())->CubeMap(cube_map);	
		}
	};


	enum
	{
		AddExposure,
		SubExposure,

		Exit,
	};

	InputActionDefine actions[] = 
	{
		InputActionDefine(AddExposure, KS_1),
		InputActionDefine(SubExposure, KS_2),

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
			: exposure_level_(1)
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/Refract");
}

void Refract::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	cube_map_ = LoadTexture("uffizi_cross.dds");

	refractor_.reset(new RefractorObject(cube_map_));
	refractor_->AddToSceneManager();

	sky_box_.reset(new HDRSceneObjectSkyBox);
	checked_cast<SceneObjectSkyBox*>(sky_box_.get())->CubeMap(cube_map_);
	sky_box_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(float3(-0.05f, -0.01f, -0.5f), float3(0, 0.05f, 0));
	this->Proj(0.05f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&Refract::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void Refract::InputHandler(InputEngine const & sender, InputAction const & action)
{
	switch (action.first)
	{
	case AddExposure:
		exposure_level_ += 0.01f;
		break;

	case SubExposure:
		exposure_level_ -= 0.01f;
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void Refract::DoUpdate(uint32_t pass)
{
	fpcController_.Update();

	checked_cast<HDRSkyBox*>(sky_box_->GetRenderable().get())->ExposureLevel(exposure_level_);
	checked_cast<RefractorRenderable*>(refractor_->GetRenderable().get())->ExposureLevel(exposure_level_);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

	std::wostringstream stream;
	stream << renderEngine.CurRenderTarget()->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"HDR Refract");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
	font_->RenderText(0, 54, Color(1, 1, 0, 1), L"Press '1' to turn up exposure, '2' to turn down exposure");
}
