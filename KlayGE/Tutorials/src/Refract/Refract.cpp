#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
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
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneNodeHelper.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/PostProcess.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "Refract.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RefractorRenderable : public StaticMesh
	{
	public:
		explicit RefractorRenderable(std::wstring_view name)
			: StaticMesh(name)
		{
			effect_ = SyncLoadRenderEffect("Refract.fxml");
			front_face_tech_ = effect_->TechniqueByName("Refract");
			back_face_tech_ = effect_->TechniqueByName("RefractBackFace");
			back_face_depth_tech_ = effect_->TechniqueByName("RefractBackFaceDepth");

			technique_ = back_face_tech_;
			*(effect_->ParameterByName("eta_ratio")) = float3(1 / 1.1f, 1 / 1.11f, 1 / 1.12f);
		}

		void DoBuildMeshInfo(RenderModel const & model) override
		{
			KFL_UNUSED(model);

			AABBox const & pos_bb = this->PosBound();
			*(effect_->ParameterByName("pos_center")) = pos_bb.Center();
			*(effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();

			AABBox const & tc_bb = this->TexcoordBound();
			*(effect_->ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(effect_->ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
		}

		virtual void Pass(PassType pass)
		{
			switch (pass)
			{
			case PT_GenShadowMap:
				technique_ = back_face_depth_tech_;
				break;

			case PT_TransparencyBackGBufferMRT:
				technique_ = back_face_tech_;
				break;

			case PT_TransparencyFrontShading:
				technique_ = front_face_tech_;
				break;

			default:
				KFL_UNREACHABLE("Invalid pass type");
			}
		}

		void BackFaceTexture(TexturePtr const & bf_tex)
		{
			*(effect_->ParameterByName("BackFace_tex")) = bf_tex;
		}
		void BackFaceDepthTexture(TexturePtr const & bf_tex)
		{
			*(effect_->ParameterByName("BackFaceDepth_tex")) = bf_tex;
		}

		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			*(effect_->ParameterByName("skybox_Ycube_tex")) = y_cube;
			*(effect_->ParameterByName("skybox_Ccube_tex")) = c_cube;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & model = float4x4::Identity();

			float4x4 const mv = model * camera.ViewMatrix();
			float4x4 const mvp = model * camera.ViewProjMatrix();

			*(effect_->ParameterByName("model")) = model;
			*(effect_->ParameterByName("mvp")) = mvp;
			*(effect_->ParameterByName("mv")) = mv;
			*(effect_->ParameterByName("proj")) = camera.ProjMatrix();
			*(effect_->ParameterByName("eye_pos")) = camera.EyePos();

			*(effect_->ParameterByName("inv_view")) = camera.InverseViewMatrix();
			*(effect_->ParameterByName("inv_vp")) = camera.InverseViewProjMatrix();

			*(effect_->ParameterByName("far_plane")) = float2(camera.FarPlane(), 1 / camera.FarPlane());
		}

	private:
		RenderTechnique* back_face_depth_tech_;
		RenderTechnique* back_face_tech_;
		RenderTechnique* front_face_tech_;
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


int SampleMain()
{
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

void Refract::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	y_cube_map_ = SyncLoadTexture("uffizi_cross_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	c_cube_map_ = SyncLoadTexture("uffizi_cross_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);

	refractor_model_ = SyncLoadModel("teapot.meshml", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, nullptr,
		CreateModelFactory<RenderModel>, CreateMeshFactory<RefractorRenderable>);

	refractor_ = MakeSharedPtr<SceneNode>(refractor_model_->Mesh(0), SceneNode::SOA_Cullable);
	Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(refractor_);

	checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->CompressedCubeMap(y_cube_map_, c_cube_map_);

	sky_box_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableSkyBox>(), SceneNode::SOA_NotCastShadow);
	checked_pointer_cast<RenderableSkyBox>(sky_box_->GetRenderable())->CompressedCubeMap(y_cube_map_, c_cube_map_);
	Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(sky_box_);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	this->LookAt(float3(0.36f, 0.11f, -0.39f), float3(0, 0.11f, 0));
	this->Proj(0.05f, 100);

	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.05f, 0.005f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	depth_texture_support_ = rf.RenderEngineInstance().DeviceCaps().depth_texture_support;

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
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

	RenderViewPtr backface_ds_view;
	if (depth_texture_support_)
	{
		auto const ds_fmt = caps.BestMatchTextureRenderTargetFormat({ cfg.graphics_cfg.depth_stencil_fmt, EF_D16 }, 1, 0);
		BOOST_ASSERT(ds_fmt != EF_Unknown);
		backface_ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		backface_ds_view = rf.Make2DDepthStencilRenderView(*backface_ds_tex_, 0, 1, 0);
	}
	else
	{
		backface_ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);
	}

	auto const depth_fmt = caps.BestMatchTextureRenderTargetFormat(
		caps.pack_to_rgba_required ? MakeArrayRef({ EF_ABGR8, EF_ARGB8 }) : MakeArrayRef({ EF_R16F, EF_R32F }), 1, 0);
	BOOST_ASSERT(depth_fmt != EF_Unknown);
	backface_depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	auto const normal_fmt = caps.BestMatchTextureRenderTargetFormat({ EF_GR8, EF_ABGR8, EF_ARGB8 }, 1, 0);
	BOOST_ASSERT(normal_fmt != EF_Unknown);
	backface_tex_ = rf.MakeTexture2D(width, height, 1, 1, normal_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	backface_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*backface_tex_, 0, 1, 0));
	backface_buffer_->Attach(FrameBuffer::ATT_DepthStencil, backface_ds_view);

	if (depth_texture_support_)
	{
		depth_to_linear_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToLinear");
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

			checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->Pass(PT_TransparencyBackGBufferMRT);
			sky_box_->Visible(false);
			return App3DFramework::URV_NeedFlush;
		}
		else
		{
			// Pass 0: Render backface's depth
			re.BindFrameBuffer(backface_depth_buffer_);
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(0.0f);

			checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->Pass(PT_GenShadowMap);
			sky_box_->Visible(false);
			return App3DFramework::URV_NeedFlush;
		}

	case 1:
		if (depth_texture_support_)
		{
			Camera& camera = this->ActiveCamera();
			float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
			float4 near_q_far(camera.NearPlane() * q, q, camera.FarPlane(), 1 / camera.FarPlane());
			depth_to_linear_pp_->SetParam(0, near_q_far);
			depth_to_linear_pp_->Apply();
		
			// Pass 1: Render front face
			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

			checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->Pass(PT_TransparencyFrontShading);
			checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->BackFaceTexture(backface_tex_);
			checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->BackFaceDepthTexture(backface_depth_tex_);

			sky_box_->Visible(true);
			return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
		}
		else
		{
			// Pass 1: Render backface's normal and depth
			re.BindFrameBuffer(backface_buffer_);

			checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->Pass(PT_TransparencyBackGBufferMRT);
			sky_box_->Visible(false);
			return App3DFramework::URV_NeedFlush;
		}

	default:
		BOOST_ASSERT(!depth_texture_support_);
		
		// Pass 2: Render front face
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

		checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->Pass(PT_TransparencyFrontShading);
		checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->BackFaceTexture(backface_tex_);
		checked_pointer_cast<RefractorRenderable>(refractor_->GetRenderable())->BackFaceDepthTexture(backface_depth_tex_);

		sky_box_->Visible(true);
		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}
