#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
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
#include <KlayGE/PostProcess.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "Refract.hpp"

using namespace std;
using namespace KlayGE;

#ifdef KLAYGE_COMPILER_MSVC
extern "C"
{
	_declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
}
#endif

namespace
{
	class RefractorRenderable : public StaticMesh
	{
	public:
		RefractorRenderable(RenderModelPtr model, std::wstring const & /*name*/)
			: StaticMesh(model, L"Refractor")
		{
			RenderEffectPtr effect = SyncLoadRenderEffect("Refract.fxml");
			front_face_tech_ = effect->TechniqueByName("Refract");
			back_face_tech_ = effect->TechniqueByName("RefractBackFace");
			back_face_depth_tech_ = effect->TechniqueByName("RefractBackFaceDepth");

			technique_ = back_face_tech_;
			*(technique_->Effect().ParameterByName("eta_ratio")) = float3(1 / 1.1f, 1 / 1.11f, 1 / 1.12f);
		}

		void BuildMeshInfo()
		{
			AABBox const & pos_bb = this->PosBound();
			*(technique_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
			*(technique_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();

			AABBox const & tc_bb = this->TexcoordBound();
			*(technique_->Effect().ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(technique_->Effect().ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
		}

		virtual void Pass(PassType pass)
		{
			switch (pass)
			{
			case PT_TransparencyBackDepth:
				technique_ = back_face_depth_tech_;
				break;

			case PT_TransparencyBackGBufferRT0:
				technique_ = back_face_tech_;
				break;

			case PT_TransparencyFrontShading:
				technique_ = front_face_tech_;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		void BackFaceTexture(TexturePtr const & bf_tex)
		{
			*(technique_->Effect().ParameterByName("BackFace_tex")) = bf_tex;
		}
		void BackFaceDepthTexture(TexturePtr const & bf_tex)
		{
			*(technique_->Effect().ParameterByName("BackFaceDepth_tex")) = bf_tex;
		}

		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			*(technique_->Effect().ParameterByName("skybox_Ycube_tex")) = y_cube;
			*(technique_->Effect().ParameterByName("skybox_Ccube_tex")) = c_cube;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & model = float4x4::Identity();

			float4x4 const mv = model * camera.ViewMatrix();
			float4x4 const mvp = model * camera.ViewProjMatrix();

			*(technique_->Effect().ParameterByName("model")) = model;
			*(technique_->Effect().ParameterByName("mvp")) = mvp;
			*(technique_->Effect().ParameterByName("mv")) = mv;
			*(technique_->Effect().ParameterByName("proj")) = camera.ProjMatrix();
			*(technique_->Effect().ParameterByName("eye_pos")) = camera.EyePos();

			*(technique_->Effect().ParameterByName("inv_view")) = camera.InverseViewMatrix();
			*(technique_->Effect().ParameterByName("inv_vp")) = camera.InverseViewProjMatrix();
		}

	private:
		RenderTechniquePtr back_face_depth_tech_;
		RenderTechniquePtr back_face_tech_;
		RenderTechniquePtr front_face_tech_;
	};

	class RefractorObject : public SceneObjectHelper
	{
	public:
		RefractorObject(TexturePtr const & y_cube, TexturePtr const & c_cube)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = SyncLoadModel("teapot.meshml", EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<RefractorRenderable>())->Mesh(0);
			checked_pointer_cast<RefractorRenderable>(renderable_)->CompressedCubeMap(y_cube, c_cube);
		}

		void BackFaceTexture(TexturePtr const & bf_tex)
		{
			checked_pointer_cast<RefractorRenderable>(renderable_)->BackFaceTexture(bf_tex);
		}
		void BackFaceDepthTexture(TexturePtr const & bf_tex)
		{
			checked_pointer_cast<RefractorRenderable>(renderable_)->BackFaceDepthTexture(bf_tex);
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
	ResLoader::Instance().AddPath("../../Tutorials/media/Refract");
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
	font_ = SyncLoadFont("gkai00mp.kfont");

	y_cube_map_ = SyncLoadTexture("uffizi_cross_y.dds", EAH_GPU_Read | EAH_Immutable);
	c_cube_map_ = SyncLoadTexture("uffizi_cross_c.dds", EAH_GPU_Read | EAH_Immutable);

	refractor_ = MakeSharedPtr<RefractorObject>(y_cube_map_, c_cube_map_);
	refractor_->AddToSceneManager();

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>(0);
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map_, c_cube_map_);
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
	input_handler->connect(KlayGE::bind(&Refract::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_D16))
	{
		depth_texture_support_ = true;
	}
	else
	{
		depth_texture_support_ = false;
	}

	backface_buffer_ = rf.MakeFrameBuffer();
	FrameBufferPtr const & screen_buffer = re.CurFrameBuffer();
	backface_buffer_->GetViewport()->camera = screen_buffer->GetViewport()->camera;
	if (!depth_texture_support_)
	{
		backface_depth_buffer_ = rf.MakeFrameBuffer();
		backface_depth_buffer_->GetViewport()->camera = screen_buffer->GetViewport()->camera;
	}

	UIManager::Instance().Load(ResLoader::Instance().Open("Refract.uiml"));
}

void Refract::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	ContextCfg const & cfg = Context::Instance().Config();

	RenderViewPtr backface_ds_view;
	if (depth_texture_support_)
	{
		ElementFormat ds_fmt;
		if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(cfg.graphics_cfg.depth_stencil_fmt))
		{
			ds_fmt = cfg.graphics_cfg.depth_stencil_fmt;
		}
		else
		{
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_D16));

			ds_fmt = EF_D16;
		}
		backface_ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		backface_ds_view = rf.Make2DDepthStencilRenderView(*backface_ds_tex_, 0, 1, 0);
	}
	else
	{
		backface_ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);
	}

	ElementFormat depth_fmt;
	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_R16F)
		&& rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_R16F, 1, 0))
	{
		depth_fmt = EF_R16F;
	}
	else
	{
		if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_GR16F)
			&& rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_GR16F, 1, 0))
		{
			depth_fmt = EF_GR16F;
		}
		else
		{
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR16F)
				&& rf.RenderEngineInstance().DeviceCaps().rendertarget_format_support(EF_ABGR16F, 1, 0));

			depth_fmt = EF_ABGR16F;
		}
	}
	backface_depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	ElementFormat normal_fmt;
	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_GR8))
	{
		normal_fmt = EF_GR8;
	}
	else
	{
		if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
		{
			normal_fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8));

			normal_fmt = EF_ARGB8;
		}
	}
	backface_tex_ = rf.MakeTexture2D(width, height, 1, 1, normal_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

	backface_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*backface_tex_, 0, 1, 0));
	backface_buffer_->Attach(FrameBuffer::ATT_DepthStencil, backface_ds_view);

	if (depth_texture_support_)
	{
		depth_to_linear_pp_ = SyncLoadPostProcess("DepthToSM.ppml", "DepthToSM");
		depth_to_linear_pp_->InputPin(0, backface_ds_tex_);
		depth_to_linear_pp_->OutputPin(0, backface_depth_tex_);
	}
	else
	{
		backface_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*backface_depth_tex_, 0, 1, 0));
		backface_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, backface_ds_view);
	}

	UIManager::Instance().SettleCtrls();
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
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"HDR Refract", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t Refract::DoUpdate(uint32_t pass)
{
	RenderEngine& re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		if (depth_texture_support_)
		{
			// Pass 0: Render backface's normal and depth
			re.BindFrameBuffer(backface_buffer_);
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(0.0f);

			checked_pointer_cast<RefractorObject>(refractor_)->Pass(PT_TransparencyBackGBufferRT0);
			sky_box_->Visible(false);
			return App3DFramework::URV_Need_Flush;
		}
		else
		{
			// Pass 0: Render backface's depth
			re.BindFrameBuffer(backface_depth_buffer_);
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(0.0f);

			checked_pointer_cast<RefractorObject>(refractor_)->Pass(PT_TransparencyBackDepth);
			sky_box_->Visible(false);
			return App3DFramework::URV_Need_Flush;
		}

	case 1:
		if (depth_texture_support_)
		{
			Camera& camera = this->ActiveCamera();
			float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
			float2 near_q(camera.NearPlane() * q, q);
			depth_to_linear_pp_->SetParam(0, near_q);
			depth_to_linear_pp_->Apply();
		
			// Pass 1: Render front face
			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

			checked_pointer_cast<RefractorObject>(refractor_)->Pass(PT_TransparencyFrontShading);
			checked_pointer_cast<RefractorObject>(refractor_)->BackFaceTexture(backface_tex_);
			checked_pointer_cast<RefractorObject>(refractor_)->BackFaceDepthTexture(backface_depth_tex_);

			sky_box_->Visible(true);
			return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
		}
		else
		{
			// Pass 1: Render backface's normal and depth
			re.BindFrameBuffer(backface_buffer_);

			checked_pointer_cast<RefractorObject>(refractor_)->Pass(PT_TransparencyBackGBufferRT0);
			sky_box_->Visible(false);
			return App3DFramework::URV_Need_Flush;
		}

	default:
		BOOST_ASSERT(!depth_texture_support_);
		
		// Pass 2: Render front face
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

		checked_pointer_cast<RefractorObject>(refractor_)->Pass(PT_TransparencyFrontShading);
		checked_pointer_cast<RefractorObject>(refractor_)->BackFaceTexture(backface_tex_);
		checked_pointer_cast<RefractorObject>(refractor_)->BackFaceDepthTexture(backface_depth_tex_);

		sky_box_->Visible(true);
		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
