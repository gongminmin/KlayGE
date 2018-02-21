#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/ParticleSystem.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "VDMParticle.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class ForwardMesh : public StaticMesh
	{
	public:
		ForwardMesh(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
		{
			effect_ = SyncLoadRenderEffect("VDMParticle.fxml");
			technique_ = effect_->TechniqueByName("Mesh");
		}

		void DoBuildMeshInfo() override
		{
			StaticMesh::DoBuildMeshInfo();

			*(effect_->ParameterByName("albedo_tex")) = textures_[RenderMaterial::TS_Albedo];
			*(effect_->ParameterByName("metalness_tex")) = textures_[RenderMaterial::TS_Metalness];
			*(effect_->ParameterByName("glossiness_tex")) = textures_[RenderMaterial::TS_Glossiness];
			*(effect_->ParameterByName("emissive_tex")) = textures_[RenderMaterial::TS_Emissive];
			*(effect_->ParameterByName("normal_tex")) = textures_[RenderMaterial::TS_Normal];

			*(effect_->ParameterByName("albedo_clr")) = mtl_->albedo;
			*(effect_->ParameterByName("metalness_clr")) = float2(mtl_->metalness, !!textures_[RenderMaterial::TS_Metalness]);
			*(effect_->ParameterByName("glossiness_clr")) = float2(mtl_->glossiness, !!textures_[RenderMaterial::TS_Glossiness]);
			*(effect_->ParameterByName("emissive_clr")) = float4(mtl_->emissive.x(), mtl_->emissive.y(), mtl_->emissive.z(),
				!!textures_[RenderMaterial::TS_Emissive]);
			*(effect_->ParameterByName("albedo_map_enabled")) = static_cast<int32_t>(!!textures_[RenderMaterial::TS_Albedo]);

			*(effect_->ParameterByName("normal_map_enabled")) = static_cast<int32_t>(!!textures_[RenderMaterial::TS_Normal]);

			AABBox const & pos_bb = this->PosBound();
			*(effect_->ParameterByName("pos_center")) = pos_bb.Center();
			*(effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();

			AABBox const & tc_bb = this->TexcoordBound();
			*(effect_->ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(effect_->ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
		}

		void ModelMatrix(float4x4 const & mat) override
		{
			StaticMesh::ModelMatrix(mat);

			inv_model_mat_ = MathLib::inverse(model_mat_);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			*(effect_->ParameterByName("mvp")) = model_mat_ * app.ActiveCamera().ViewProjMatrix();
			*(effect_->ParameterByName("eye_pos")) = MathLib::transform_coord(app.ActiveCamera().EyePos(), inv_model_mat_);

			auto const & light_src = Context::Instance().SceneManagerInstance().GetLight(0);

			*(effect_->ParameterByName("light_pos")) = MathLib::transform_coord(light_src->Position(), inv_model_mat_);
			*(effect_->ParameterByName("light_color")) = light_src->Color();
			*(effect_->ParameterByName("light_falloff")) = light_src->Falloff();
		}

	private:
		float4x4 inv_model_mat_;
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
	VDMParticleApp app;
	app.Create();
	app.Run();

	return 0;
}

VDMParticleApp::VDMParticleApp()
				: App3DFramework("VDMParticle"),
					particle_rendering_type_(PRT_FullRes)
{
	ResLoader::Instance().AddPath("../../Samples/media/VDMParticle");
}

bool VDMParticleApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if ((caps.max_simultaneous_rts < 3) || !caps.depth_texture_support || caps.pack_to_rgba_required)
	{
		return false;
	}
	return true;
}

void VDMParticleApp::OnCreate()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	RenderablePtr robot_model = ASyncLoadModel("attack_droid.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<ForwardMesh>());
	SceneObjectPtr robot = MakeSharedPtr<SceneObjectHelper>(robot_model, SceneObject::SOA_Cullable);
	robot->ModelMatrix(MathLib::translation(0.0f, 0.0f, -2.0f));
	scene_objs_.push_back(robot);

	RenderablePtr room_model = ASyncLoadModel("sponza_crytek.meshml", EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<ForwardMesh>());
	SceneObjectPtr room = MakeSharedPtr<SceneObjectHelper>(room_model, SceneObject::SOA_Cullable);
	scene_objs_.push_back(room);

	for (auto& so : scene_objs_)
	{
		so->AddToSceneManager();
	}

	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(1.47f, 2.35f, -5.75f), float3(-2.18f, 0.71f, 2.20f));
	this->Proj(0.1f, 200);

	light_ = MakeSharedPtr<SpotLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1.0f, 0.67f, 0.55f) * 20.0f);
	light_->Falloff(float3(1, 0.5f, 0));
	light_->Position(float3(0, 0, 0));
	light_->Direction(float3(0, 1, 0));
	light_->OuterAngle(PI / 2.5f);
	light_->InnerAngle(PI / 4);
	light_->AddToSceneManager();

	ps_ = SyncLoadParticleSystem(ResLoader::Instance().Locate("Fire.psml"));
	ps_->Gravity(0.5f);
	ps_->MediaDensity(0.5f);
	ps_->AddToSceneManager();

	float const SCALE = 6;
	ps_->ModelMatrix(MathLib::scaling(SCALE, SCALE, SCALE));
	ps_->Emitter(0)->ModelMatrix(MathLib::translation(light_->Position() / SCALE));

	scene_fb_ = rf.MakeFrameBuffer();
	scene_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;
	depth_to_linear_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToLinear");
	copy_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");
	add_copy_pp_ = SyncLoadPostProcess("Copy.ppml", "add_bilinear_copy");
	vdm_composition_pp_ = SyncLoadPostProcess("VarianceDepthMap.ppml", "VDMComposition");

	fpc_controller_.Scalers(0.05f, 0.1f);

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

	UIManager::Instance().Load(ResLoader::Instance().Open("VDMParticle.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_particle_rendering_type_static_ = dialog_->IDFromName("ParticleRenderingTypeStatic");
	id_particle_rendering_type_combo_ = dialog_->IDFromName("ParticleRenderingTypeCombo");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UIComboBox>(id_particle_rendering_type_combo_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->ParticleRenderingTypeChangedHandler(sender);
		});
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});

	this->ParticleRenderingTypeChangedHandler(*dialog_->Control<UIComboBox>(id_particle_rendering_type_combo_));
	this->CtrlCameraHandler(*dialog_->Control<UICheckBox>(id_ctrl_camera_));

	half_res_fb_ = rf.MakeFrameBuffer();
	half_res_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;

	quarter_res_fb_ = rf.MakeFrameBuffer();
	quarter_res_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;

	vdm_quarter_res_fb_ = rf.MakeFrameBuffer();
	vdm_quarter_res_fb_->GetViewport()->camera = re.CurFrameBuffer()->GetViewport()->camera;

	depth_to_max_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToMax");
	copy_to_depth_pp_ = SyncLoadPostProcess("Depth.ppml", "CopyToDepth");
	copy_to_depth_pp_->SetParam(0, 0);
}

void VDMParticleApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	ElementFormat fmt;
	if (caps.fp_color_support && caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
	{
		fmt = EF_B10G11R11F;
	}
	else if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
	{
		fmt = EF_ABGR8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
		fmt = EF_ARGB8;
	}
	scene_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	if (caps.rendertarget_format_support(EF_R16F, 1, 0))
	{
		fmt = EF_R16F;
	}
	else if (caps.rendertarget_format_support(EF_R32F, 1, 0))
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_R32F, 1, 0));
		fmt = EF_R32F;
	}
	scene_depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	scene_ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	depth_to_linear_pp_->InputPin(0, scene_ds_tex_);
	depth_to_linear_pp_->OutputPin(0, scene_depth_tex_);

	scene_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_tex_, 0, 1, 0));
	scene_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*scene_ds_tex_, 0, 1, 0));

	if (ps_)
	{
		ps_->SceneDepthTexture(scene_depth_tex_);
	}

	fmt = scene_tex_->Format();

	{
		uint32_t w = width / 2;
		uint32_t h = height / 2;
		for (uint32_t i = 0; i < 2; ++ i)
		{
			low_res_color_texs_.push_back(rf.MakeTexture2D(w, h, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write));
			low_res_max_ds_texs_.push_back(rf.MakeTexture2D(w, h, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Read | EAH_GPU_Write));
			low_res_max_ds_views_.push_back(rf.Make2DDepthStencilRenderView(*low_res_max_ds_texs_.back(), 0, 1, 0));
			w /= 2;
			h /= 2;
		}
	}

	
	half_res_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*low_res_color_texs_[0], 0, 1, 0));
	half_res_fb_->Attach(FrameBuffer::ATT_DepthStencil, low_res_max_ds_views_[0]);

	quarter_res_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*low_res_color_texs_[1], 0, 1, 0));
	quarter_res_fb_->Attach(FrameBuffer::ATT_DepthStencil, low_res_max_ds_views_[1]);

	vdm_transition_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	vdm_count_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, 1, EF_GR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	vdm_quarter_res_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*low_res_color_texs_[1], 0, 1, 0));
	vdm_quarter_res_fb_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*vdm_transition_tex_, 0, 1, 0));
	vdm_quarter_res_fb_->Attach(FrameBuffer::ATT_Color2, rf.Make2DRenderView(*vdm_count_tex_, 0, 1, 0));
	vdm_quarter_res_fb_->Attach(FrameBuffer::ATT_DepthStencil, low_res_max_ds_views_[1]);

	copy_pp_->InputPin(0, scene_tex_);
	copy_to_depth_pp_->InputPin(0, scene_ds_tex_);

	vdm_composition_pp_->InputPin(0, low_res_color_texs_[1]);
	vdm_composition_pp_->InputPin(1, vdm_transition_tex_);
	vdm_composition_pp_->InputPin(2, vdm_count_tex_);
	vdm_composition_pp_->InputPin(3, scene_depth_tex_);

	UIManager::Instance().SettleCtrls();
}

void VDMParticleApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void VDMParticleApp::ParticleRenderingTypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	particle_rendering_type_ = static_cast<ParticleRenderingType>(sender.GetSelectedIndex());
	if (PRT_VDMQuarterRes == particle_rendering_type_)
	{
		ps_->Pass(PT_VDM);
	}
	else
	{
		ps_->Pass(PT_SimpleForward);
	}
}

void VDMParticleApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpc_controller_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpc_controller_.DetachCamera();
	}
}

void VDMParticleApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"VDMParticle", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t VDMParticleApp::DoUpdate(uint32_t pass)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
		{
			re.BindFrameBuffer(scene_fb_);
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1, 0);

			Camera const & camera = this->ActiveCamera();
			float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
			float4 near_q_far(this->ActiveCamera().NearPlane() * q, q, camera.FarPlane(), 1 / camera.FarPlane());
			depth_to_linear_pp_->SetParam(0, near_q_far);

			Color clear_clr(0.2f, 0.4f, 0.6f, 1);
			if (Context::Instance().Config().graphics_cfg.gamma)
			{
				clear_clr.r() = 0.029f;
				clear_clr.g() = 0.133f;
				clear_clr.b() = 0.325f;
			}
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

			for (auto& so : scene_objs_)
			{
				so->Visible(true);
			}
			ps_->Visible(false);

			return App3DFramework::URV_NeedFlush;
		}

	case 1:
		{
			for (auto& so : scene_objs_)
			{
				so->Visible(false);
			}
			ps_->Visible(true);

			depth_to_linear_pp_->Apply();

			if (PRT_FullRes == particle_rendering_type_)
			{
				re.BindFrameBuffer(FrameBufferPtr());

				copy_to_depth_pp_->Apply();
				copy_pp_->Apply();
				return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
			}
			else
			{
				uint32_t t = (PRT_NaiveHalfRes == particle_rendering_type_) ? 0 : 1;
				for (uint32_t i = 0; i <= t; ++ i)
				{
					TexturePtr const & input_tex = (0 == i) ? scene_ds_tex_ : low_res_max_ds_texs_[i - 1];

					uint32_t const w = input_tex->Width(0);
					uint32_t const h = input_tex->Height(0);

					depth_to_max_pp_->SetParam(0, float2(0.5f / w, 0.5f / h));
					depth_to_max_pp_->SetParam(1, float2(static_cast<float>((w + 1) & ~1) / w,
						static_cast<float>((h + 1) & ~1) / h));
					depth_to_max_pp_->InputPin(0, input_tex);
					depth_to_max_pp_->OutputPin(0, low_res_color_texs_[i]);
					depth_to_max_pp_->OutputFrameBuffer()->Attach(FrameBuffer::ATT_DepthStencil, low_res_max_ds_views_[i]);
					depth_to_max_pp_->Apply();
				}

				switch (particle_rendering_type_)
				{
				case PRT_NaiveHalfRes:
					re.BindFrameBuffer(half_res_fb_);
					re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
					break;

				case PRT_NaiveQuarterRes:
					re.BindFrameBuffer(quarter_res_fb_);
					re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
					break;

				case PRT_VDMQuarterRes:
					re.BindFrameBuffer(vdm_quarter_res_fb_);
					re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
					re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color1)->ClearColor(Color(0, 0, 0, 0));
					re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color2)->ClearColor(Color(0, 0, 0, 0));
					break;

				default:
					KFL_UNREACHABLE("Invalid particle rendering type");
				}

				return App3DFramework::URV_NeedFlush;
			}
		}

	case 2:
	default:
		ps_->Visible(false);

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1, 0);

		copy_pp_->Apply();

		switch (particle_rendering_type_)
		{
		case PRT_NaiveHalfRes:
			add_copy_pp_->InputPin(0, low_res_color_texs_[0]);
			add_copy_pp_->Apply();
			break;

		case PRT_NaiveQuarterRes:
			add_copy_pp_->InputPin(0, low_res_color_texs_[1]);
			add_copy_pp_->Apply();
			break;

		case PRT_VDMQuarterRes:
			vdm_composition_pp_->Apply();
			break;

		default:
			KFL_UNREACHABLE("Invalid particle rendering type");
		}

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}
}
