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
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>

#include "Parallax.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public KMesh
	{
	public:
		RenderPolygon(RenderModelPtr model, std::wstring const & name)
			: KMesh(model, name)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("parallax.fx")->TechniqueByName("Parallax");

			SamplerPtr diffuse_sampler(new Sampler);
			diffuse_sampler->SetTexture(LoadTexture("diffuse.dds"));
			diffuse_sampler->Filtering(Sampler::TFO_Bilinear);
			diffuse_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			diffuse_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(technique_->Effect().ParameterByName("diffuseMapSampler")) = diffuse_sampler;

			SamplerPtr normal_sampler(new Sampler);
			normal_sampler->SetTexture(LoadTexture("normal.dds"));
			normal_sampler->Filtering(Sampler::TFO_Bilinear);
			normal_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			normal_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(technique_->Effect().ParameterByName("normalMapSampler")) = normal_sampler;

			SamplerPtr height_sampler(new Sampler);
			height_sampler->SetTexture(LoadTexture("height.dds"));
			height_sampler->Filtering(Sampler::TFO_Bilinear);
			height_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			height_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(technique_->Effect().ParameterByName("heightMapSampler")) = height_sampler;
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("mvp")) = model * view * proj;
			*(technique_->Effect().ParameterByName("eyePos")) = app.ActiveCamera().EyePos();
		}

		void LightPos(float3 const & light_pos)
		{
			*(technique_->Effect().ParameterByName("lightPos")) = light_pos;
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKModel("teapot.kmodel", CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderPolygon>());
		}

		void LightPos(float3 const & light_pos)
		{
			RenderModelPtr model = checked_pointer_cast<RenderModel>(renderable_);
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<RenderPolygon>(model->Mesh(i))->LightPos(light_pos);
			}
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
	OCTree sceneMgr(Box(float3(-5, -5, -5), float3(5, 5, 5)), 3);
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	Parallax app;
	app.Create("Parallax", settings);
	app.Run();

	return 0;
}

Parallax::Parallax()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/Parallax");
}

void Parallax::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	polygon_.reset(new PolygonObject);
	polygon_->AddToSceneManager();

	this->LookAt(float3(-0.3f, 0.4f, -0.3f), float3(0, 0, 0));
	this->Proj(0.01f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.01f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&Parallax::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void Parallax::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void Parallax::DoUpdate(uint32_t /*pass*/)
{
	fpcController_.Update();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

	float degree(std::clock() / 700.0f);
	float3 lightPos(2, 0, 1);
	float4x4 matRot(MathLib::rotation_y(degree));
	lightPos = MathLib::transform_coord(lightPos, matRot);
	checked_pointer_cast<PolygonObject>(polygon_)->LightPos(lightPos);

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Parallax²âÊÔ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str());
}
