#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Util.hpp>

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
		RenderTorus(std::wstring const & name, TexturePtr tex)
			: KMesh(L"Torus", TexturePtr()),
				toon_sampler_(new Sampler),
				normal_depth_sampler_(new Sampler)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("Cartoon.fx");

			toon_sampler_->SetTexture(LoadTexture("toon.dds"));
			toon_sampler_->Filtering(Sampler::TFO_Point);
			toon_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			normal_depth_sampler_->Filtering(Sampler::TFO_Point);
			normal_depth_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);

			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("Cartoon.fx");
			*(effect_->ParameterByName("toonmap_sampler")) = toon_sampler_;
			*(effect_->ParameterByName("normal_depth_sampler")) = normal_depth_sampler_;
		}

		void Pass(int i)
		{
			switch (i)
			{
			case 0:
				{
					float rotX(std::clock() / 700.0f);
					float rotY(std::clock() / 700.0f);

					model_mat_ = MathLib::RotationX(rotX) * MathLib::RotationY(rotY);
				}
				technique_ = effect_->Technique("NormalDepth");
				break;

			case 1:
				technique_ = effect_->Technique("Cartoon");
				break;
			}
		}

		void UpdateTexture(TexturePtr const & normal_depth_tex)
		{
			normal_depth_sampler_->SetTexture(normal_depth_tex);

			*(effect_->ParameterByName("inv_width")) = 2.0f / normal_depth_tex->Width(0);
			*(effect_->ParameterByName("inv_height")) = 2.0f / normal_depth_tex->Height(0);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			Matrix4 const & view = app.ActiveCamera().ViewMatrix();
			Matrix4 const & proj = app.ActiveCamera().ProjMatrix();

			*(effect_->ParameterByName("model_view")) = model_mat_ * view;
			*(effect_->ParameterByName("proj")) = proj;

			*(effect_->ParameterByName("light_in_model")) = MathLib::TransformCoord(Vector3(2, 2, -3),
																	MathLib::Inverse(model_mat_));
		}

	private:
		SamplerPtr toon_sampler_;
		SamplerPtr normal_depth_sampler_;

		Matrix4 model_mat_;

		RenderEffectPtr effect_;
	};

	class TorusObject : public SceneObjectHelper
	{
	public:
		TorusObject()
			: SceneObjectHelper(SOA_Cullable | SOA_ShortAge)
		{
			renderable_ = LoadKMesh("torus.kmesh", CreateKMeshFactory<RenderTorus>())->Mesh(0);
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

	torus_.reset(new TorusObject);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(0, 0, -6), Vector3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	normal_depth_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	normal_depth_buffer_->GetViewport().camera = renderEngine.CurRenderTarget()->GetViewport().camera;

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&Cartoon::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void Cartoon::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	normal_depth_tex_ = rf.MakeTexture2D(width, height, 1, PF_ABGR16F);
	normal_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*normal_depth_tex_, 0));
	normal_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, PF_D16, 0));
}

void Cartoon::InputHandler(InputEngine const & sender, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

uint32_t Cartoon::NumPasses() const
{
	return 2;
}

void Cartoon::DoUpdate(uint32_t pass)
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		fpcController_.Update();

		renderEngine.BindRenderTarget(normal_depth_buffer_);
		renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

		sceneMgr.Clear();
		checked_cast<RenderTorus*>(torus_->GetRenderable().get())->Pass(0);
		torus_->AddToSceneManager();
		break;
	
	case 1:
		renderEngine.BindRenderTarget(RenderTargetPtr());
		renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

		sceneMgr.Clear();
		checked_cast<RenderTorus*>(torus_->GetRenderable().get())->UpdateTexture(normal_depth_tex_);
		checked_cast<RenderTorus*>(torus_->GetRenderable().get())->Pass(1);
		torus_->AddToSceneManager();

		RenderWindow* rw = static_cast<RenderWindow*>(renderEngine.CurRenderTarget().get());

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"¿¨Í¨äÖÈ¾²âÊÔ");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), rw->Description());

		std::wostringstream stream;
		stream << rw->DepthBits() << " bits depth " << rw->StencilBits() << " bits stencil";
		font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str().c_str());

		stream.str(L"");
		stream << renderEngine.CurRenderTarget()->FPS() << " FPS";
		font_->RenderText(0, 54, Color(1, 1, 0, 1), stream.str().c_str());
		break;
	}
}
