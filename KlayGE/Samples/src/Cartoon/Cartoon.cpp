#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "Cartoon.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderTorus : public KMesh
	{
	public:
		RenderTorus(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Torus")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("Cartoon.kfx")->TechniqueByName("NormalDepth");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*(technique_->Effect().ParameterByName("model_view_proj")) = view * proj;
			*(technique_->Effect().ParameterByName("model_view")) = view;

			*(technique_->Effect().ParameterByName("depth_min")) = camera.NearPlane();
			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / (camera.FarPlane() - camera.NearPlane());
			*(technique_->Effect().ParameterByName("light_in_model")) = float3(2, 2, -3);
			*(technique_->Effect().ParameterByName("eye_in_model")) = MathLib::transform_coord(float3(0, 0, 0), MathLib::inverse(view));
		}
	};

	class TorusObject : public SceneObjectHelper
	{
	public:
		TorusObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadKModel("dino50.kmodel", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderTorus>())->Mesh(0);
		}
	};


	class CartoonPostProcess : public PostProcess
	{
	public:
		CartoonPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("Cartoon.kfx")->TechniqueByName("Cartoon"))
		{
			*(technique_->Effect().ParameterByName("toonmap_sampler")) = LoadTexture("toon.dds", EAH_GPU_Read);
		}

		void Source(TexturePtr const & tex, bool flipping)
		{
			PostProcess::Source(tex, flipping);
			if (tex)
			{
				*(technique_->Effect().ParameterByName("inv_width")) = 1.0f / tex->Width(0);
				*(technique_->Effect().ParameterByName("inv_height")) = 1.0f / tex->Height(0);
			}
		}

		void ColorTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("color_sampler")) = tex;
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float depth_range = camera.FarPlane() - camera.NearPlane();
			*(technique_->Effect().ParameterByName("e_barrier")) = float2(0.8f, 0.1f / depth_range);
		}
	};


	enum
	{
		Switch_Cartoon
	};


	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};

	bool ConfirmDevice()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		if (caps.max_simultaneous_rts < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_ABGR16F, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0);
			rf.MakeDepthStencilRenderView(800, 600, EF_D16, 0);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}

int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/Cartoon");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	Cartoon app("¿¨Í¨äÖÈ¾", settings);
	app.Create();
	app.Run();

	return 0;
}

Cartoon::Cartoon(std::string const & name, RenderSettings const & settings)
			: App3DFramework(name, settings),
				cartoon_style_(true)
{
}

void Cartoon::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont", 16);

	torus_.reset(new TorusObject);
	torus_->AddToSceneManager();

	this->LookAt(float3(0, 0, -2), float3(0, 0, 0));
	this->Proj(0.1f, 10.0f);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	g_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	g_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&Cartoon::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	cartoon_.reset(new CartoonPostProcess);

	dialog_ = UIManager::Instance().MakeDialog();
	dialog_->AddControl(UIControlPtr(new UICheckBox(dialog_, Switch_Cartoon, L"Cartoon style",
                            60, 550, 350, 24, false, 0, false)));
	dialog_->Control<UICheckBox>(Switch_Cartoon)->SetChecked(true);
	dialog_->Control<UICheckBox>(Switch_Cartoon)->OnChangedEvent().connect(boost::bind(&Cartoon::CheckBoxHandler, this, _1));
}

void Cartoon::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	color_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, EAH_GPU_Read | EAH_GPU_Write, NULL);
	normal_depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, EAH_GPU_Read | EAH_GPU_Write, NULL);
	g_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*color_tex_, 0));
	g_buffer_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*normal_depth_tex_, 0));
	g_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

	cartoon_->Source(normal_depth_tex_, g_buffer_->RequiresFlipping());
	checked_pointer_cast<CartoonPostProcess>(cartoon_)->ColorTex(color_tex_);
	cartoon_->Destinate(FrameBufferPtr());

	dialog_->GetControl(Switch_Cartoon)->SetLocation(60, height - 50);
}

void Cartoon::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void Cartoon::CheckBoxHandler(UICheckBox const & /*sender*/)
{
	cartoon_style_ = dialog_->Control<UICheckBox>(Switch_Cartoon)->GetChecked();
}

uint32_t Cartoon::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		UIManager::Instance().HandleInput();
	}

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	if (cartoon_style_)
	{
		switch (pass)
		{
		case 0:
			renderEngine.BindFrameBuffer(g_buffer_);
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
			return App3DFramework::URV_Need_Flush;

		case 1:
			renderEngine.BindFrameBuffer(FrameBufferPtr());
			renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
			cartoon_->Apply();
			break;
		}
	}
	else
	{
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);
	}

	UIManager::Instance().Render();

	FrameBuffer& rw = *checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer());

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Cartoon Rendering");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description());

	std::wostringstream stream;
	stream << rw.DepthBits() << " bits depth " << rw.StencilBits() << " bits stencil";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str());

	stream.str(L"");
	stream.precision(2);
	stream << fixed << this->FPS() << " FPS";
	font_->RenderText(0, 54, Color(1, 1, 0, 1), stream.str());

	if (!cartoon_style_ && (0 == pass))
	{
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
	else
	{
		return App3DFramework::URV_Only_New_Objs | App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
