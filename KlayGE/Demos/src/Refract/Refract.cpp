#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
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
	class Refractor : public KMesh
	{
	public:
		Refractor(std::wstring const & /*name*/, TexturePtr tex)
			: KMesh(L"Refractor", tex)
		{
			effect_ = LoadRenderEffect("Refract.fx");
			effect_->SetTechnique("Refract");
		}

		void CubeMap(TexturePtr const & texture)
		{
			*(effect_->ParameterByName("cubemap")) = texture;	
		}

		void OnRenderBegin()
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			Matrix4 const & model = renderEngine.WorldMatrix();
			Matrix4 const & view = renderEngine.ViewMatrix();
			Matrix4 const & proj = renderEngine.ProjectionMatrix();

			*(effect_->ParameterByName("model")) = model;
			*(effect_->ParameterByName("modelit")) = MathLib::Transpose(MathLib::Inverse(model));
			*(effect_->ParameterByName("mvp")) = model * view * proj;
		}
	};


	enum
	{
		Exit,
	};

	InputAction actions[] = 
	{
		InputAction(Exit, KS_Escape),
	};

	struct TheRenderSettings : public RenderSettings
	{
		bool ConfirmDevice(RenderDeviceCaps const & caps) const
		{
			if (caps.max_shader_model < 2)
			{
				return false;
			}
			return true;
		}
	};
}

int main()
{
	SceneManager sceneMgr;
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	TheRenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

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

	refractor_ = LoadKMesh("bunny.kmesh", CreateFactory<Refractor>);
	static_cast<Refractor*>(refractor_->Children(0).get())->CubeMap(cube_map_);
	refractor_->AddToSceneManager();

	renderSkyBox_.reset(new RenderableSkyBox);
	static_cast<RenderableSkyBox*>(renderSkyBox_.get())->CubeMap(cube_map_);
	renderSkyBox_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(-0.05f, -0.01f, -0.5f), Vector3(0, 0.05f, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.01f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));
	action_map_id_ = inputEngine.ActionMap(actionMap, true);
}

void Refract::Update()
{
	fpcController_.Update();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionsType actions(inputEngine.Update(action_map_id_));
	for (InputActionsType::iterator iter = actions.begin(); iter != actions.end(); ++ iter)
	{
		switch (iter->first)
		{
		case Exit:
			this->Quit();
			break;
		}
	}

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.ViewMatrix(this->ActiveCamera().ViewMatrix());
	renderEngine.ProjectionMatrix(this->ActiveCamera().ProjMatrix());

	*(refractor_->Children(0)->GetRenderEffect()->ParameterByName("eyePos"))
		= Vector4(this->ActiveCamera().EyePos().x(), this->ActiveCamera().EyePos().y(),
			this->ActiveCamera().EyePos().z(), 1);

	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Refract");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
}
