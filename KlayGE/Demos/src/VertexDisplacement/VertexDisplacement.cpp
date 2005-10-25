#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/VertexBuffer.hpp>
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

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>

#include "VertexDisplacement.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const LENGTH = 4;
	int const WIDTH = 3;

	class Flag : public RenderablePlane
	{
	public:
		Flag(int length_segs, int width_segs)
			: RenderablePlane(LENGTH, WIDTH, length_segs, width_segs, true, true, false)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("VertexDisplacement.fx");
			effect_->ActiveTechnique("VertexDisplacement");

			SamplerPtr flag_sampler(new Sampler);
			flag_sampler->SetTexture(LoadTexture("Flag.dds"));
			flag_sampler->Filtering(Sampler::TFO_Point);
			flag_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			flag_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			*(effect_->ParameterByName("flagSampler")) = flag_sampler;
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

	VertexDisplacement app;
	app.Create("VertexDisplacement", settings);
	app.Run();

	return 0;
}

VertexDisplacement::VertexDisplacement()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/VertexDisplacement");
}

void VertexDisplacement::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	flag_.reset(new Flag(8 * 2, 6 * 2));
	flag_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(0, 0, -10), Vector3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&VertexDisplacement::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void VertexDisplacement::InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void VertexDisplacement::Update(uint32_t pass)
{
	fpcController_.Update();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	inputEngine.Update();

	Matrix4 view = this->ActiveCamera().ViewMatrix();
	Matrix4 proj = this->ActiveCamera().ProjMatrix();
	Vector3 eyePos = this->ActiveCamera().EyePos();

	*(flag_->GetRenderEffect()->ParameterByName("half_length")) = LENGTH / 2.0f;
	*(flag_->GetRenderEffect()->ParameterByName("half_width")) = WIDTH / 2.0f;

	Matrix4 modelView = flag_->GetModelMatrix() * view;

	*(flag_->GetRenderEffect()->ParameterByName("modelview")) = modelView;
	*(flag_->GetRenderEffect()->ParameterByName("proj")) = proj;

	*(flag_->GetRenderEffect()->ParameterByName("modelviewIT")) = MathLib::Transpose(MathLib::Inverse(modelView));


	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	float currentAngle(clock() / 400.0f);
	*(flag_->GetRenderEffect()->ParameterByName("currentAngle")) = currentAngle;

	std::wostringstream stream;
	stream << renderEngine.ActiveRenderTarget(0)->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"¶¥µãÎ»ÒÆ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumObjectsRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str().c_str());
}
