#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/KMesh.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <boost/bind.hpp>
#include <sstream>
#include <ctime>

#include "Cartoon.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTorus : public KMesh
	{
	public:
		RenderTorus(TexturePtr const & toonTex, TexturePtr const & edgeTex)
			: KMesh(L"Torus", TexturePtr()),
				toon_sampler_(new Sampler), edge_sampler_(new Sampler)
		{
			toon_sampler_->SetTexture(toonTex);
			toon_sampler_->Filtering(Sampler::TFO_Point);
			toon_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			edge_sampler_->SetTexture(edgeTex);
			edge_sampler_->Filtering(Sampler::TFO_Point);
			edge_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);

			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("Cartoon.fx");
			*(effect_->ParameterByName("toonMapSampler")) = toon_sampler_;
			*(effect_->ParameterByName("edgeMapSampler")) = edge_sampler_;
			effect_->SetTechnique("cartoonTec");
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			Matrix4 view = app.ActiveCamera().ViewMatrix();
			Matrix4 proj = app.ActiveCamera().ProjMatrix();
			Vector3 eyePos = app.ActiveCamera().EyePos();

			*(effect_->ParameterByName("proj")) = proj;
			*(effect_->ParameterByName("lightPos")) = Vector4(2, 2, -3, 1);
			*(effect_->ParameterByName("eyePos")) = Vector4(eyePos.x(), eyePos.y(), eyePos.z(), 1);

			float rotX(std::clock() / 700.0f);
			float rotY(std::clock() / 700.0f);

			Matrix4 mat(MathLib::RotationX(rotX));
			Matrix4 matY(MathLib::RotationY(rotY));
			mat *= matY;
			mat *= view;

			*(effect_->ParameterByName("worldview")) = mat;
			*(effect_->ParameterByName("worldviewIT")) = MathLib::Transpose(MathLib::Inverse(mat));
		}

	private:
		SamplerPtr toon_sampler_;
		SamplerPtr edge_sampler_;
	};

	KMeshPtr CreateRenderTorusFactory(std::wstring const & name, TexturePtr tex, TexturePtr const & toonTex, TexturePtr const & edgeTex)
	{
		return KMeshPtr(new RenderTorus(toonTex, edgeTex));
	}


	enum
	{
		Exit,
	};

	InputAction actions[] = 
	{
		InputAction(Exit, KS_Escape),
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
	OCTree sceneMgr(Box(Vector3(-10, -10, -10), Vector3(10, 10, 10)), 3);

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;
	settings.ConfirmDevice = ConfirmDevice;

	Cartoon app;
	app.Create("¿¨Í¨äÖÈ¾²âÊÔ", settings);
	app.Run();

	return 0;
}

Cartoon::Cartoon()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/Cartoon");
}

void Cartoon::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	TexturePtr toonTex = LoadTexture("Toon.dds");
	TexturePtr edgeTex = LoadTexture("Edge.dds");

	renderTorus_ = LoadKMesh("torus.kmesh", boost::bind(CreateRenderTorusFactory, _1, _2, toonTex, edgeTex));
	renderTorus_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(0, 0, -6), Vector3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));
	action_map_id_ = inputEngine.ActionMap(actionMap, true);
}

void Cartoon::Update(uint32_t pass)
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

	RenderWindow* rw = static_cast<RenderWindow*>(renderEngine.ActiveRenderTarget(0).get());

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"¿¨Í¨äÖÈ¾²âÊÔ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw->Description());

	std::wostringstream stream;
	stream << rw->DepthBits() << " bits depth " << rw->StencilBits() << " bits stencil";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str().c_str());

	stream.str(L"");
	stream << renderEngine.ActiveRenderTarget(0)->FPS() << " FPS";
	font_->RenderText(0, 54, Color(1, 1, 0, 1), stream.str().c_str());
}
