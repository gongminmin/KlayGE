#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <boost/bind.hpp>
#include <sstream>
#include <ctime>

#include "DepthOfField.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const NUM_INSTANCE = 400;

	class Teapot : public SceneObjectHelper
	{
	private:
		struct InstData
		{
			float4 col[3];
			Color clr;
		};

	public:
		Teapot()
			: SceneObjectHelper(SOA_Cullable)
		{
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 1, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 2, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 3, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_Diffuse, 0, EF_ABGR32F));
		}

		void Instance(float4x4 const & mat, Color const & clr)
		{
			float4x4 matT = MathLib::transpose(mat);

			inst_.col[0] = matT.Row(0);
			inst_.col[1] = matT.Row(1);
			inst_.col[2] = matT.Row(2);
			inst_.clr = clr;
		}

		void const * InstanceData() const
		{
			return &inst_;
		}

		void SetRenderable(RenderablePtr ra)
		{
			renderable_ = ra;
		}

	private:
		InstData inst_;
	};

	class RenderInstance : public KMesh
	{
	public:
		RenderInstance(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Instance")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("DepthOfField.kfx")->TechniqueByName("ColorDepth");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("view")) = view;
			*(technique_->Effect().ParameterByName("view_proj")) = view * proj;

			*(technique_->Effect().ParameterByName("light_in_world")) = float3(2, 2, -3);

			*(technique_->Effect().ParameterByName("depth_min")) = app.ActiveCamera().NearPlane();
			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / (app.ActiveCamera().FarPlane() - app.ActiveCamera().NearPlane());
		}
	};


	class DepthOfField : public PostProcess
	{
	public:
		DepthOfField()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DepthOfField.kfx")->TechniqueByName("DepthOfField")),
				blur_(8, 1)
		{
		}

		void FocusPlane(float focus_plane)
		{
			focus_plane_ = focus_plane;
		}
		float FocusPlane() const
		{
			return focus_plane_;
		}

		void FocusRange(float focus_range)
		{
			focus_range_ = focus_range;
		}
		float FocusRange() const
		{
			return focus_range_;
		}

		void Source(TexturePtr const & tex, bool flipping)
		{
			PostProcess::Source(tex, flipping);

			uint32_t const width = tex->Width(0);
			uint32_t const height = tex->Height(0);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			downsample_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, EF_ABGR16F);
			blur_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, EF_ABGR16F);

			bool tmp_flipping;
			{
				FrameBufferPtr fb = rf.MakeFrameBuffer();
				fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*downsample_tex_, 0));
				downsampler_.Source(src_texture_, flipping);
				downsampler_.Destinate(fb);
				tmp_flipping = fb->RequiresFlipping();
			}

			{
				FrameBufferPtr fb = rf.MakeFrameBuffer();
				fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blur_tex_, 0));
				blur_.Source(downsample_tex_, tmp_flipping);
				blur_.Destinate(fb);
			}

			*(technique_->Effect().ParameterByName("blur_sampler")) = blur_tex_;
		}

		void Apply()
		{
			downsampler_.Apply();
			blur_.Apply();

			PostProcess::Apply();
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			float const depth_range = app.ActiveCamera().FarPlane() - app.ActiveCamera().NearPlane();

			*(technique_->Effect().ParameterByName("focus_plane")) = (focus_plane_ - app.ActiveCamera().NearPlane()) / depth_range;
			*(technique_->Effect().ParameterByName("inv_focus_range")) = depth_range / focus_range_;
		}

	private:
		TexturePtr downsample_tex_;
		TexturePtr blur_tex_;

		Downsampler2x2PostProcess downsampler_;
		BlurPostProcess blur_;

		float focus_plane_;
		float focus_range_;
	};


	enum
	{
		FocusPlaneSlider,
		FocusPlaneStatic,
		FocusRangeSlider,
		FocusRangeStatic,
		CtrlCamera,
	};

	enum
	{
		Exit,
	};

	InputActionDefine actions[] = 
	{
		InputActionDefine(CtrlCamera, KS_LeftCtrl),
		InputActionDefine(CtrlCamera, KS_RightCtrl),
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
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	DepthOfFieldApp app("Depth of field", settings);
	app.Create();
	app.Run();

	return 0;
}

DepthOfFieldApp::DepthOfFieldApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/DepthOfField");
}

void DepthOfFieldApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	renderInstance_ = LoadKModel("teapot.kmodel", CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderInstance>())->Mesh(0);
	for (int i = 0; i < 10; ++ i)
	{
		for (int j = 0; j < NUM_INSTANCE / 10; ++ j)
		{
			float const s = sin(2 * PI * j / (NUM_INSTANCE / 10));
			float const c = cos(2 * PI * j / (NUM_INSTANCE / 10));

			SceneObjectPtr so(new Teapot);
			checked_pointer_cast<Teapot>(so)->Instance(
				MathLib::translation(s, i / 10.0f, c), Color(s, c, 0, 1));

			checked_pointer_cast<Teapot>(so)->SetRenderable(renderInstance_);
			so->AddToSceneManager();
			scene_objs_.push_back(so);
		}
	}

	this->LookAt(float3(-1.8f, 1.9f, -1.8f), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	clr_depth_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	clr_depth_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&DepthOfFieldApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	depth_of_field_.reset(new DepthOfField);
	depth_of_field_->Destinate(FrameBufferPtr());

	dialog_ = UIManager::Instance().MakeDialog();
	
	dialog_->AddControl(UIControlPtr(new UIStatic(dialog_, FocusPlaneStatic, L"Focus plane:", 60, 280, 100, 24, false)));
	dialog_->AddControl(UIControlPtr(new UISlider(dialog_, FocusPlaneSlider, 60, 300, 100, 24, 0, 200, 100, false)));
	dialog_->Control<UISlider>(FocusPlaneSlider)->OnValueChangedEvent().connect(boost::bind(&DepthOfFieldApp::FocusPlaneChangedHandler, this, _1));
	
	dialog_->AddControl(UIControlPtr(new UIStatic(dialog_, FocusRangeStatic, L"Focus range:", 60, 348, 100, 24, false)));
	dialog_->AddControl(UIControlPtr(new UISlider(dialog_, FocusRangeSlider, 60, 368, 100, 24, 0, 200, 100, false)));
	dialog_->Control<UISlider>(FocusRangeSlider)->OnValueChangedEvent().connect(boost::bind(&DepthOfFieldApp::FocusRangeChangedHandler, this, _1));

	dialog_->AddControl(UIControlPtr(new UICheckBox(dialog_, CtrlCamera, L"Control camera",
                            60, 550, 350, 24, false, 0, false)));
	dialog_->Control<UICheckBox>(CtrlCamera)->SetChecked(false);
	dialog_->Control<UICheckBox>(CtrlCamera)->OnChangedEvent().connect(boost::bind(&DepthOfFieldApp::CtrlCameraHandler, this, _1));

	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusPlane(dialog_->Control<UISlider>(FocusPlaneSlider)->GetValue() / 50.0f);
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusRange(dialog_->Control<UISlider>(FocusRangeSlider)->GetValue() / 50.0f);
}

void DepthOfFieldApp::OnResize(uint32_t width, uint32_t height)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	clr_depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F);
	clr_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*clr_depth_tex_, 0));
	clr_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 0));

	depth_of_field_->Source(clr_depth_tex_, clr_depth_buffer_->RequiresFlipping());
	depth_of_field_->Destinate(FrameBufferPtr());

	dialog_->GetControl(FocusPlaneStatic)->SetLocation(width - 120, 280);
	dialog_->GetControl(FocusPlaneSlider)->SetLocation(width - 120, 300);
	dialog_->GetControl(FocusRangeStatic)->SetLocation(width - 120, 348);
	dialog_->GetControl(FocusRangeSlider)->SetLocation(width - 120, 368);
	dialog_->GetControl(CtrlCamera)->SetLocation(width - 120, 416);
}

void DepthOfFieldApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DepthOfFieldApp::FocusPlaneChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusPlane(sender.GetValue() / 50.0f);
}

void DepthOfFieldApp::FocusRangeChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusRange(sender.GetValue() / 50.0f);
}

void DepthOfFieldApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpcController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpcController_.DetachCamera();
	}
}

uint32_t DepthOfFieldApp::DoUpdate(uint32_t pass)
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		fpcController_.Update();
		UIManager::Instance().HandleInput();

		renderEngine.BindFrameBuffer(clr_depth_buffer_);
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->Clear(Color(0.2f, 0.4f, 0.6f, 1));
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);

		sceneMgr.Clear();
		for (size_t i = 0; i < scene_objs_.size(); ++ i)
		{
			scene_objs_[i]->AddToSceneManager();
		}
		return App3DFramework::URV_Need_Flush;
	
	default:
		sceneMgr.Clear();

		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);

		depth_of_field_->Apply();

		UIManager::Instance().Render();

		FrameBuffer& rw = *renderEngine.CurFrameBuffer();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Depth of field");
		font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description());

		std::wostringstream stream;
		stream << this->FPS() << " FPS";
		font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str());
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
