#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/Renderable.hpp>
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
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <iterator>
#include <sstream>
#include <vector>

#include "SampleCommon.hpp"
#include "ShadowCubeMap.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	uint32_t const SHADOW_MAP_SIZE = 512;

	class ShadowMapped
	{
	public:
		explicit ShadowMapped(uint32_t shadow_map_size)
			: shadow_map_size_(shadow_map_size), pass_index_(0)
		{
		}

		virtual void GenShadowMapPass(bool gen_sm, SM_TYPE sm_type, int pass_index)
		{
			gen_sm_pass_ = gen_sm;
			sm_type_ = sm_type;
			pass_index_ = pass_index;
		}

		void LightSrc(LightSourcePtr const& light_src)
		{
			light_pos_ = light_src->Position();

			float4x4 const light_model = light_src->BoundSceneNode()->TransformToParent();
			inv_light_model_ = MathLib::inverse(light_model);

			App3DFramework const & app = Context::Instance().AppInstance();
			switch (sm_type_)
			{
			case SMT_CubeOne:
			case SMT_CubeOneInstance:
			case SMT_CubeOneInstanceGS:
			case SMT_CubeOneInstanceVpRt:
				light_view_ = light_src->SMCamera(0)->ViewMatrix();
				break;

			case SMT_Cube:
			default:
				light_view_ = app.ActiveCamera().ViewMatrix();
				break;
			}

			light_color_ = light_src->Color();
			light_falloff_ = light_src->Falloff();

			light_inv_range_ = 1.0f / (light_src->SMCamera(0)->FarPlane() - light_src->SMCamera(0)->NearPlane());
		}

		void CubeSMTexture(TexturePtr const & cube_tex)
		{
			sm_cube_tex_ = cube_tex;
		}

		void LampTexture(TexturePtr const & tex)
		{
			lamp_tex_ = tex;
		}

	protected:
		void OnRenderBegin(float4x4 const & model, RenderEffectPtr const & effect)
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			*(effect->ParameterByName("obj_model_to_light_model")) = model * inv_light_model_;
			*(effect->ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
			*(effect->ParameterByName("esm_scale_factor")) = esm_scale_factor_ * light_inv_range_;

			if (!gen_sm_pass_)
			{
				*(effect->ParameterByName("light_pos")) = light_pos_;

				*(effect->ParameterByName("light_projective_tex")) = lamp_tex_;
				*(effect->ParameterByName("shadow_cube_tex")) = sm_cube_tex_;

				*(effect->ParameterByName("light_color")) = light_color_;
				*(effect->ParameterByName("light_falloff")) = light_falloff_;
			}
		}

	protected:
		uint32_t shadow_map_size_;

		bool gen_sm_pass_;
		SM_TYPE sm_type_;
		int pass_index_;
		TexturePtr sm_cube_tex_;

		float3 light_pos_;
		float4x4 inv_light_model_;
		float4x4 light_view_;
		float3 light_color_;
		float3 light_falloff_;

		float esm_scale_factor_;
		float light_inv_range_;

		TexturePtr lamp_tex_;
	};

	class OccluderMesh : public StaticMesh, public ShadowMapped
	{
	public:
		explicit OccluderMesh(std::wstring_view name)
			: StaticMesh(name),
				ShadowMapped(SHADOW_MAP_SIZE)
		{
			effect_ = SyncLoadRenderEffect("ShadowCubeMap.fxml");
		}

		void ScaleFactor(float esm_scale_factor)
		{
			esm_scale_factor_ = esm_scale_factor;
		}

		void GenShadowMapPass(bool gen_sm, SM_TYPE sm_type, int pass_index)
		{
			ShadowMapped::GenShadowMapPass(gen_sm, sm_type, pass_index);


			if (gen_sm)
			{
				switch (sm_type_)
				{
				case SMT_Cube:
					technique_ = effect_->TechniqueByName("GenCubeShadowMap");
					break;

				case SMT_CubeOne:
					technique_ = effect_->TechniqueByName("GenCubeOneShadowMap");
					break;

				case SMT_CubeOneInstance:
					technique_ = effect_->TechniqueByName("GenCubeOneInstanceShadowMap");
					break;

				case SMT_CubeOneInstanceGS:
					technique_ = effect_->TechniqueByName("GenCubeOneInstanceGSShadowMap");
					break;

				case SMT_CubeOneInstanceVpRt:
				default:
					{
						RenderDeviceCaps const& caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
						BOOST_ASSERT(caps.vp_rt_index_at_every_stage_support);
						KFL_UNUSED(caps);
					}
					technique_ = effect_->TechniqueByName("GenCubeOneInstanceVpRtShadowMap");
					break;
				}
			}
			else
			{
				technique_ = effect_->TechniqueByName("RenderScene");
			}
		}

		void OnRenderBegin()
		{
			ShadowMapped::OnRenderBegin(model_mat_, effect_);
			StaticMesh::OnRenderBegin();
		}
	};


	class PointLightNodeUpdate
	{
	public:
		void operator()(SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(elapsed_time);

			node.TransformToParent(
				MathLib::rotation_z(0.4f) * MathLib::rotation_y(app_time / 1.4f) * MathLib::translation(2.0f, 12.0f, 4.0f));
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


int SampleMain()
{
	ShadowCubeMap app;
	app.Create();
	app.Run();

	return 0;
}

ShadowCubeMap::ShadowCubeMap()
				: App3DFramework("ShadowCubeMap")
{
	ResLoader::Instance().AddPath("../../Samples/media/ShadowCubeMap");
}

void ShadowCubeMap::OnCreate()
{
	loading_percentage_ = 0;
	lamp_tex_ = ASyncLoadTexture("lamp.dds", EAH_GPU_Read | EAH_Immutable);
	scene_model_ = ASyncLoadModel("ScifiRoom/Scifi.3DS", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, AddToSceneRootHelper,
		CreateModelFactory<RenderModel>, CreateMeshFactory<OccluderMesh>);
	teapot_model_ = ASyncLoadModel("teapot.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, nullptr,
		CreateModelFactory<RenderModel>, CreateMeshFactory<OccluderMesh>);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(0.0f, 10.0f, -25.0f), float3(0, 10.0f, 0));
	this->Proj(0.1f, 200);

	auto fmt = caps.BestMatchRenderTargetFormat(MakeSpan({EF_D24S8, EF_D16}), 1, 0);
	BOOST_ASSERT(fmt != EF_Unknown);
	DepthStencilViewPtr depth_view = rf.Make2DDsv(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, fmt, 1, 0);
	if (caps.pack_to_rgba_required)
	{
		fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);
	}
	else
	{
		fmt = EF_R16F;
	}
	shadow_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	shadow_srv_ = rf.MakeTextureSrv(shadow_tex_);
	shadow_cube_buffer_ = rf.MakeFrameBuffer();
	shadow_cube_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(shadow_tex_, 0, 1, 0));
	shadow_cube_buffer_->Attach(depth_view);

	shadow_cube_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(20, 20, 20));
	light_->Falloff(float3(1, 1, 0));

	auto light_proxy = LoadLightSourceProxyModel(light_);
	light_proxy->RootNode()->TransformToParent(MathLib::scaling(0.5f, 0.5f, 0.5f) * light_proxy->RootNode()->TransformToParent());

	auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	light_node->AddComponent(light_);
	light_node->AddChild(light_proxy->RootNode());
	light_node->OnMainThreadUpdate().Connect(PointLightNodeUpdate());
	root_node.AddChild(light_node);

	if (caps.render_to_texture_array_support)
	{
		shadow_cube_one_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		shadow_cube_one_buffer_ = rf.MakeFrameBuffer();
		if (caps.flexible_srvs_support)
		{
			flexible_srvs_support_ = true;
			for (uint32_t i = 0; i < 6; ++i)
			{
				shadow_cube_one_srvs_[i] = rf.MakeTexture2DSrv(shadow_cube_one_tex_, 0, static_cast<Texture::CubeFaces>(i), 0, 1);
			}
		}

		auto& viewport = *shadow_cube_one_buffer_->Viewport();

		viewport.NumCameras(6);
		for (uint32_t i = 0; i < 6; ++i)
		{
			viewport.Camera(i, light_->SMCamera(i));
		}

		shadow_cube_one_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.MakeCubeRtv(shadow_cube_one_tex_, 0, 0));
		TexturePtr shadow_one_depth_tex = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Write);
		shadow_cube_one_buffer_->Attach(rf.MakeCubeDsv(shadow_one_depth_tex, 0, 0));
	}

	for (int i = 0; i < 6; ++ i)
	{
		sm_filter_pps_[i] = MakeSharedPtr<LogGaussianBlurPostProcess>(3, true);
		sm_filter_pps_[i]->OutputPin(0, rf.Make2DRtv(shadow_cube_tex_, 0, static_cast<Texture::CubeFaces>(i), 0));
	}

	fpcController_.Scalers(0.05f, 1.0f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(*ResLoader::Instance().Open("ShadowCubeMap.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_scale_factor_static_ = dialog_->IDFromName("ScaleFactorStatic");
	id_scale_factor_slider_ = dialog_->IDFromName("ScaleFactorSlider");
	id_sm_type_static_ = dialog_->IDFromName("SMStatic");
	id_sm_type_combo_ = dialog_->IDFromName("SMCombo");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	if (!caps.vp_rt_index_at_every_stage_support)
	{
		dialog_->Control<UIComboBox>(id_sm_type_combo_)->RemoveItem(4);
	}
	if (caps.max_shader_model < ShaderModel(5, 0))
	{
		dialog_->Control<UIComboBox>(id_sm_type_combo_)->RemoveItem(3);
	}

	dialog_->Control<UISlider>(id_scale_factor_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->ScaleFactorChangedHandler(sender);
		});
	dialog_->Control<UIComboBox>(id_sm_type_combo_)->OnSelectionChangedEvent().Connect(
		[this](UIComboBox const & sender)
		{
			this->SMTypeChangedHandler(sender);
		});
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});

	this->ScaleFactorChangedHandler(*dialog_->Control<UISlider>(id_scale_factor_slider_));
	this->SMTypeChangedHandler(*dialog_->Control<UIComboBox>(id_sm_type_combo_));

	if (caps.max_shader_model < ShaderModel(5, 0))
	{
		dialog_->Control<UIComboBox>(id_sm_type_combo_)->RemoveItem(3);
	}
}

void ShadowCubeMap::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void ShadowCubeMap::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ShadowCubeMap::ScaleFactorChangedHandler(KlayGE::UISlider const & sender)
{
	esm_scale_factor_ = static_cast<float>(sender.GetValue());
	for (auto const & mesh : scene_meshes_)
	{
		checked_pointer_cast<OccluderMesh>(mesh)->ScaleFactor(esm_scale_factor_);
	}

	std::wostringstream stream;
	stream << L"Scale Factor: " << esm_scale_factor_;
	dialog_->Control<UIStatic>(id_scale_factor_static_)->SetText(stream.str());
}

void ShadowCubeMap::SMTypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	sm_type_ = static_cast<SM_TYPE>(sender.GetSelectedIndex());
}

void ShadowCubeMap::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

void ShadowCubeMap::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ShadowCubeMap", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ShadowCubeMap::DoUpdate(uint32_t pass)
{
	if (0 == pass)
	{
		if (loading_percentage_ < 100)
		{
			if (loading_percentage_ < 10)
			{
				if (lamp_tex_->HWResourceReady())
				{
					light_->ProjectiveTexture(lamp_tex_);
					loading_percentage_ = 10;
				}
			}
			else if (loading_percentage_ < 80)
			{
				if (scene_model_->HWResourceReady())
				{
					for (uint32_t i = 0; i < scene_model_->NumMeshes(); ++ i)
					{
						checked_pointer_cast<OccluderMesh>(scene_model_->Mesh(i))->LampTexture(lamp_tex_);
						checked_pointer_cast<OccluderMesh>(scene_model_->Mesh(i))->CubeSMTexture(shadow_cube_tex_);
						checked_pointer_cast<OccluderMesh>(scene_model_->Mesh(i))->ScaleFactor(esm_scale_factor_);
						scene_meshes_.push_back(scene_model_->Mesh(i));
					}

					loading_percentage_ = 90;
				}
			}
			else if (loading_percentage_ < 95)
			{
				if (teapot_model_->HWResourceReady())
				{
					loading_percentage_ = 95;
				}
			}
			else
			{
				auto const & teapot = teapot_model_->Mesh(0);
				auto so =
					MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(teapot), SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
				so->OnSubThreadUpdate().Connect(
					[](SceneNode& node, float app_time, float elapsed_time)
					{
						KFL_UNUSED(elapsed_time);

						node.TransformToParent(MathLib::scaling(5.0f, 5.0f, 5.0f) * MathLib::translation(5.0f, 5.0f, 0.0f)
							* MathLib::rotation_y(-app_time / 1.5f));
					});
				{
					auto& scene_mgr = Context::Instance().SceneManagerInstance();
					std::lock_guard<std::mutex> lock(scene_mgr.MutexForUpdate());
					scene_mgr.SceneRootNode().AddChild(so);
				}
				checked_pointer_cast<OccluderMesh>(teapot)->LampTexture(lamp_tex_);
				checked_pointer_cast<OccluderMesh>(teapot)->CubeSMTexture(shadow_cube_tex_);
				checked_pointer_cast<OccluderMesh>(teapot)->ScaleFactor(esm_scale_factor_);
				scene_meshes_.push_back(teapot);

				loading_percentage_ = 100;
			}
		}
	}

	auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	re.NumCameraInstances(0);

	switch (sm_type_)
	{
	case SMT_Cube:
		if (pass > 0)
		{
			sm_filter_pps_[pass - 1]->InputPin(0, shadow_srv_);
			checked_pointer_cast<LogGaussianBlurPostProcess>(sm_filter_pps_[pass - 1])->ESMScaleFactor(esm_scale_factor_,
				*light_->SMCamera(pass - 1));
			sm_filter_pps_[pass - 1]->Apply();
		}

		switch (pass)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			{
				re.BindFrameBuffer(shadow_cube_buffer_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);

				shadow_cube_buffer_->Viewport()->Camera(light_->SMCamera(pass));

				for (auto const & mesh : scene_meshes_)
				{
					checked_pointer_cast<OccluderMesh>(mesh)->GenShadowMapPass(true, sm_type_, pass);
					checked_pointer_cast<OccluderMesh>(mesh)->LightSrc(light_);
				}
			}
			return App3DFramework::URV_NeedFlush;

		default:
			{
				re.BindFrameBuffer(FrameBufferPtr());

				Color clear_clr(0.2f, 0.4f, 0.6f, 1);
				if (Context::Instance().Config().graphics_cfg.gamma)
				{
					clear_clr.r() = 0.029f;
					clear_clr.g() = 0.133f;
					clear_clr.b() = 0.325f;
				}
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

				for (auto const & mesh : scene_meshes_)
				{
					checked_pointer_cast<OccluderMesh>(mesh)->GenShadowMapPass(false, sm_type_, pass);
				}
			}
			return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
		}
		break;

	default:
		if (re.DeviceCaps().render_to_texture_array_support)
		{
			switch (pass)
			{
			case 0:
				{
					if ((sm_type_ == SMT_CubeOne) || (sm_type_ == SMT_CubeOneInstanceGS))
					{
						re.NumCameraInstances(1);
					}

					re.BindFrameBuffer(shadow_cube_one_buffer_);
					re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);

					for (auto const & mesh : scene_meshes_)
					{
						checked_pointer_cast<OccluderMesh>(mesh)->GenShadowMapPass(true, sm_type_, pass);
						checked_pointer_cast<OccluderMesh>(mesh)->LightSrc(light_);
					}
				}
				return App3DFramework::URV_NeedFlush;

			default:
				{
					for (int p = 0; p < 6; ++ p)
					{
						if (flexible_srvs_support_)
						{
							sm_filter_pps_[p]->InputPin(0, shadow_cube_one_srvs_[p]);
						}
						else
						{
							shadow_cube_one_tex_->CopyToSubTexture2D(*shadow_tex_, 0, 0, 0, 0, shadow_tex_->Width(0),
								shadow_tex_->Height(0), p, 0, 0, 0, shadow_cube_one_tex_->Width(0), shadow_cube_one_tex_->Height(0),
								TextureFilter::Point);
							sm_filter_pps_[p]->InputPin(0, shadow_srv_);
						}
						checked_pointer_cast<LogGaussianBlurPostProcess>(sm_filter_pps_[p])->ESMScaleFactor(esm_scale_factor_,
							*light_->SMCamera(p));
						sm_filter_pps_[p]->Apply();
					}

					re.BindFrameBuffer(FrameBufferPtr());

					Color clear_clr(0.2f, 0.4f, 0.6f, 1);
					if (Context::Instance().Config().graphics_cfg.gamma)
					{
						clear_clr.r() = 0.029f;
						clear_clr.g() = 0.133f;
						clear_clr.b() = 0.325f;
					}
					re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

					for (auto const & mesh : scene_meshes_)
					{
						checked_pointer_cast<OccluderMesh>(mesh)->GenShadowMapPass(false, sm_type_, pass);
					}
				}
				return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
			}
		}
		else
		{
			return App3DFramework::URV_Finished;
		}
		break;
	}
}
