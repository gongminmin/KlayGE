#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
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
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "VertexDisplacement.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const LENGTH = 4;
	int const WIDTH = 3;

	class FlagRenderable : public RenderablePlane
	{
	public:
		FlagRenderable(int length_segs, int width_segs)
			: RenderablePlane(static_cast<float>(LENGTH), static_cast<float>(WIDTH), length_segs, width_segs, true)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("VertexDisplacement.kfx")->TechniqueByName("VertexDisplacement");

			flag_tex_ = LoadTexture("Flag.dds");
		}

		void OnRenderBegin()
		{
			float currentAngle(clock() / 400.0f);
			*(technique_->Effect().ParameterByName("currentAngle")) = currentAngle;

			*(technique_->Effect().ParameterByName("half_length")) = LENGTH / 2.0f;
			*(technique_->Effect().ParameterByName("half_width")) = WIDTH / 2.0f;

			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();
			float4x4 modelView = float4x4::Identity() * view;

			*(technique_->Effect().ParameterByName("modelview")) = modelView;
			*(technique_->Effect().ParameterByName("proj")) = proj;

			*(technique_->Effect().ParameterByName("modelviewIT")) = MathLib::transpose(MathLib::inverse(modelView));

			*(technique_->Effect().ParameterByName("flagSampler")) = flag_tex_;
		}

	private:
		TexturePtr flag_tex_;
	};

	class FlagObject : public SceneObjectHelper
	{
	public:
		FlagObject(int length_segs, int width_segs)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_.reset(new FlagRenderable(length_segs, width_segs));
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
	OCTree sceneMgr(Box(float3(-10, -10, -10), float3(10, 10, 10)), 3);

	Context::Instance().RenderFactoryInstance(OGLRenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	VertexDisplacement app;
	app.Create("VertexDisplacement", settings);
	app.Run();

	return 0;
}

VertexDisplacement::VertexDisplacement()
{
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/VertexDisplacement");
}

void VertexDisplacement::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	flag_.reset(new FlagObject(8 * 2, 6 * 2));
	flag_->AddToSceneManager();

	this->LookAt(float3(0, 0, -10), float3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&VertexDisplacement::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void VertexDisplacement::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void VertexDisplacement::DoUpdate(uint32_t /*pass*/)
{
	fpcController_.Update();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"¶¥µãÎ»ÒÆ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumRenderablesRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str());
	font_->RenderText(0, 54, Color(1, 1, 0, 1), renderEngine.Name());
}
