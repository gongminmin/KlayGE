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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

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
		RenderTorus(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Torus")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("Cartoon.kfx");

			*(effect_->ParameterByName("toonmap_sampler")) = LoadTexture("toon.dds");
		}

		void Pass(int i)
		{
			switch (i)
			{
			case 0:
				{
					float rotX(std::clock() / 700.0f);
					float rotY(std::clock() / 700.0f);

					model_mat_ = MathLib::rotation_x(rotX) * MathLib::rotation_y(rotY);
				}
				technique_ = effect_->TechniqueByName("NormalDepth");
				break;

			case 1:
				technique_ = effect_->TechniqueByName("Cartoon");
				break;
			}
		}

		void CartoonStyle(bool cs)
		{
			*(effect_->ParameterByName("cartoon_style")) = cs;
		}

		void UpdateTexture(TexturePtr const & normal_depth_tex, bool flipping)
		{
			*(effect_->ParameterByName("normal_depth_sampler")) = normal_depth_tex;
			if (normal_depth_tex)
			{
				*(effect_->ParameterByName("inv_width")) = 2.0f / normal_depth_tex->Width(0);
				*(effect_->ParameterByName("inv_height")) = 2.0f / normal_depth_tex->Height(0);
			}

			*(effect_->ParameterByName("flipping")) = flipping ? -1 : +1;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(effect_->ParameterByName("model_view")) = model_mat_ * view;
			*(effect_->ParameterByName("proj")) = proj;

			*(effect_->ParameterByName("light_in_model")) = MathLib::transform_coord(float3(2, 2, -3),
																	MathLib::inverse(model_mat_));
			*(effect_->ParameterByName("eye_in_model")) = MathLib::transform_coord(app.ActiveCamera().EyePos(),
																	MathLib::inverse(model_mat_));

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset() * 2;
			float const x_offset = texel_to_pixel.x() / re.CurFrameBuffer()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurFrameBuffer()->Height();
			*(effect_->ParameterByName("offset")) = float2(x_offset, y_offset);

			*(effect_->ParameterByName("depth_min")) = app.ActiveCamera().NearPlane();
			*(effect_->ParameterByName("inv_depth_range")) = 1 / (app.ActiveCamera().FarPlane() - app.ActiveCamera().NearPlane());
		}

	private:
		float4x4 model_mat_;

		RenderEffectPtr effect_;
	};

	class TorusObject : public SceneObjectHelper
	{
	public:
		TorusObject()
			: SceneObjectHelper(SOA_Cullable | SOA_ShortAge)
		{
			renderable_ = LoadKModel("torus.kmodel", CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderTorus>())->Mesh(0);
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
	OCTree sceneMgr(3);

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
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
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/Cartoon");
}

void Cartoon::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	torus_.reset(new TorusObject);

	this->LookAt(float3(0, 0, -6), float3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	normal_depth_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	normal_depth_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&Cartoon::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

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
	normal_depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F);
	normal_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*normal_depth_tex_, 0));
	normal_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

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
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		fpcController_.Update();

		UIManager::Instance().HandleInput();

		renderEngine.BindFrameBuffer(normal_depth_buffer_);
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->Clear(Color(0.2f, 0.4f, 0.6f, 1));
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);

		sceneMgr.Clear();
		checked_pointer_cast<RenderTorus>(torus_->GetRenderable())->Pass(0);
		torus_->AddToSceneManager();
		return App3DFramework::URV_Need_Flush;
	
	default:
		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->Clear(Color(0.2f, 0.4f, 0.6f, 1));
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);

		sceneMgr.Clear();
		checked_pointer_cast<RenderTorus>(torus_->GetRenderable())->UpdateTexture(normal_depth_tex_,
			normal_depth_buffer_->RequiresFlipping());
		checked_pointer_cast<RenderTorus>(torus_->GetRenderable())->Pass(1);
		checked_pointer_cast<RenderTorus>(torus_->GetRenderable())->CartoonStyle(cartoon_style_);
		torus_->AddToSceneManager();

		UIManager::Instance().Render();

		FrameBuffer& rw = *checked_pointer_cast<FrameBuffer>(renderEngine.CurFrameBuffer());

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"¿¨Í¨äÖÈ¾");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description());

		std::wostringstream stream;
		stream << rw.DepthBits() << " bits depth " << rw.StencilBits() << " bits stencil";
		font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str());

		stream.str(L"");
		stream << this->FPS() << " FPS";
		font_->RenderText(0, 54, Color(1, 1, 0, 1), stream.str());
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
