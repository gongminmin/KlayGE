#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/UI.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <boost/bind.hpp>

#include "Refract.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RefractorRenderable : public StaticMesh
	{
	public:
		RefractorRenderable(RenderModelPtr model, std::wstring const & /*name*/)
			: StaticMesh(model, L"Refractor")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = rf.LoadEffect("Refract.fxml");
			front_face_tech_ = effect->TechniqueByName("Refract");
			back_face_tech_ = effect->TechniqueByName("RefractBackFace");

			technique_ = back_face_tech_;
			*(technique_->Effect().ParameterByName("eta_ratio")) = float3(1 / 1.1f, 1 / 1.11f, 1 / 1.12f);
		}

		void BuildMeshInfo()
		{
		}

		void Pass(int pass)
		{
			switch (pass)
			{
			case 0:
				technique_ = back_face_tech_;
				break;

			case 1:
				technique_ = front_face_tech_;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		void BackFaceTexture(TexturePtr const & bf_tex, bool flip)
		{
			*(technique_->Effect().ParameterByName("BackFace_tex")) = bf_tex;
			*(technique_->Effect().ParameterByName("flip")) = static_cast<int32_t>(flip ? -1 : 1);
		}

		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			*(technique_->Effect().ParameterByName("skybox_Ycube_tex")) = y_cube;
			*(technique_->Effect().ParameterByName("skybox_Ccube_tex")) = c_cube;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & model = float4x4::Identity();
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			float4x4 const mv = model * view;
			float4x4 const mvp = mv * proj;

			*(technique_->Effect().ParameterByName("model")) = model;
			*(technique_->Effect().ParameterByName("modelviewit")) = MathLib::transpose(MathLib::inverse(mv));
			*(technique_->Effect().ParameterByName("mvp")) = mvp;
			*(technique_->Effect().ParameterByName("mv")) = mv;
			*(technique_->Effect().ParameterByName("proj")) = proj;
			*(technique_->Effect().ParameterByName("eye_pos")) = app.ActiveCamera().EyePos();

			*(technique_->Effect().ParameterByName("inv_view")) = MathLib::inverse(view);
			*(technique_->Effect().ParameterByName("inv_vp")) = MathLib::inverse(view * proj);
		}

	private:
		RenderTechniquePtr back_face_tech_;
		RenderTechniquePtr front_face_tech_;
	};

	class RefractorObject : public SceneObjectHelper
	{
	public:
		RefractorObject(TexturePtr const & y_cube, TexturePtr const & c_cube)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = LoadModel("teapot.meshml", EAH_GPU_Read, CreateModelFactory<RenderModel>(), CreateMeshFactory<RefractorRenderable>())()->Mesh(0);
			checked_pointer_cast<RefractorRenderable>(renderable_)->CompressedCubeMap(y_cube, c_cube);
		}

		void Pass(int pass)
		{
			checked_pointer_cast<RefractorRenderable>(renderable_)->Pass(pass);
		}

		void BackFaceTexture(TexturePtr const & bf_tex, bool flip)
		{
			checked_pointer_cast<RefractorRenderable>(renderable_)->BackFaceTexture(bf_tex, flip);
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
}


int main()
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	Refract app;
	app.Create();
	app.Run();

	return 0;
}

Refract::Refract()
			: App3DFramework("Refract")
{
	ResLoader::Instance().AddPath("../../Samples/media/Refract");
}

bool Refract::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	if (!caps.rendertarget_format_support(EF_ABGR16F, 1, 0))
	{
		return false;
	}

	return true;
}

void Refract::InitObjects()
{
	// 建立字体
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	y_cube_map_ = LoadTexture("uffizi_cross_y.dds", EAH_GPU_Read)();
	c_cube_map_ = LoadTexture("uffizi_cross_c.dds", EAH_GPU_Read)();

	refractor_ = MakeSharedPtr<RefractorObject>(y_cube_map_, c_cube_map_);
	refractor_->AddToSceneManager();

	sky_box_ = MakeSharedPtr<SceneObjectHDRSkyBox>(0);
	checked_pointer_cast<SceneObjectHDRSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map_, c_cube_map_);
	sky_box_->AddToSceneManager();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	this->LookAt(float3(0.36f, 0.11f, -0.39f), float3(0, 0.11f, 0));
	this->Proj(0.05f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.05f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&Refract::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	render_buffer_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	render_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;

	UIManager::Instance().Load(ResLoader::Instance().Load("Refract.uiml"));
}

void Refract::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	ContextCfg const & cfg = Context::Instance().Config();

	RenderViewPtr ds_view = rf.Make2DDepthStencilRenderView(width, height, cfg.graphics_cfg.depth_stencil_fmt, 1, 0);

	render_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	render_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*render_tex_, 0, 1, 0));
	render_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	UIManager::Instance().SettleCtrls(width, height);
}

void Refract::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void Refract::DoUpdateOverlay()
{
	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"HDR Refract", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	UIManager::Instance().Render();
}

uint32_t Refract::DoUpdate(uint32_t pass)
{
	RenderEngine& re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		// 第一遍，渲染背面
		re.BindFrameBuffer(render_buffer_);
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(0.0f);

		checked_pointer_cast<RefractorObject>(refractor_)->Pass(0);
		sky_box_->Visible(false);
		return App3DFramework::URV_Need_Flush;

	default:
		// 第二遍，渲染正面
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

		checked_pointer_cast<RefractorObject>(refractor_)->Pass(1);
		checked_pointer_cast<RefractorObject>(refractor_)->BackFaceTexture(render_tex_, render_buffer_->RequiresFlipping());

		sky_box_->Visible(true);
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
