#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
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

#include <vector>
#include <sstream>

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

		float4x4 LightViewProj() const
		{
			return light_views_[pass_index_] * light_proj_;
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
			if ((SMT_CubeOne == sm_type_) || (SMT_CubeOneInstance == sm_type_) || (SMT_CubeOneInstanceGS == sm_type_))
			{
				for (int i = 0; i < 6; ++ i)
				{
					light_views_[i] = light_src->SMCamera(i)->ViewMatrix();
				}
			}
			else
			{
				light_views_[pass_index_] = app.ActiveCamera().ViewMatrix();
			}
			light_proj_ = app.ActiveCamera().ProjMatrix();

			light_color_ = light_src->Color();
			light_falloff_ = light_src->Falloff();

			light_inv_range_ = 1.0f / (light_src->SMCamera(0)->FarPlane() - light_src->SMCamera(0)->NearPlane());
		}

		void CubeSMTexture(TexturePtr const & cube_tex)
		{
			sm_cube_tex_ = cube_tex;
		}

		void DPSMTexture(TexturePtr const & dual_tex)
		{
			sm_dual_tex_ = dual_tex;
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

			*(effect->ParameterByName("model")) = model;
			*(effect->ParameterByName("obj_model_to_light_model")) = model * inv_light_model_;
			*(effect->ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
			*(effect->ParameterByName("esm_scale_factor")) = esm_scale_factor_ * light_inv_range_;

			if (gen_sm_pass_)
			{
				switch (sm_type_)
				{
				case SMT_DP:
					{
						*(effect->ParameterByName("mvp")) = model * this->LightViewProj();

						float4x4 mv = model * light_views_[pass_index_];
						*(effect->ParameterByName("mv")) = mv;
						*(effect->ParameterByName("far")) = camera.FarPlane();

						FrameBufferPtr const & cur_fb = Context::Instance().RenderFactoryInstance().RenderEngineInstance().CurFrameBuffer();
						*(effect->ParameterByName("tess_edge_length_scale")) = float2(static_cast<float>(cur_fb->Width()), static_cast<float>(cur_fb->Height())) / 12.0f;
					}
					break;

				case SMT_Cube:
					*(effect->ParameterByName("mvp")) = model * this->LightViewProj();
					break;

				default:
					{
						std::vector<float4x4> mvps(6);
						for (int i = 0; i < 6; ++ i)
						{
							mvps[i] = model * light_views_[i] * light_proj_;
						}
						*(effect->ParameterByName("mvps")) = mvps;
					}
					break;
				}
			}
			else
			{
				*(effect->ParameterByName("mvp")) = model * camera.ViewProjMatrix();
				*(effect->ParameterByName("light_pos")) = light_pos_;

				*(effect->ParameterByName("light_projective_tex")) = lamp_tex_;
				*(effect->ParameterByName("shadow_cube_tex")) = sm_cube_tex_;
				*(effect->ParameterByName("shadow_dual_tex")) = sm_dual_tex_;

				*(effect->ParameterByName("light_color")) = light_color_;
				*(effect->ParameterByName("light_falloff")) = light_falloff_;

				if (SMT_DP == sm_type_)
				{
					*(effect->ParameterByName("obj_model_to_light_view")) = model * light_views_[0];
				}
			}
		}

	protected:
		uint32_t shadow_map_size_;

		bool gen_sm_pass_;
		SM_TYPE sm_type_;
		int pass_index_;
		TexturePtr sm_cube_tex_;
		TexturePtr sm_dual_tex_;

		float3 light_pos_;
		float4x4 inv_light_model_;
		float4x4 light_views_[6];
		float4x4 light_proj_;
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
				ShadowMapped(SHADOW_MAP_SIZE),
				smooth_mesh_(false), tess_factor_(5)
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

			for (auto const & rl : rls_)
			{
				rl->TopologyType(RenderLayout::TT_TriangleList);
			}

			if (gen_sm)
			{
				switch (sm_type_)
				{
				case SMT_DP:
					{
						RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
						if (TM_Hardware == caps.tess_method)
						{
							technique_ = effect_->TechniqueByName("GenDPShadowMapTessTech");
							for (auto const & rl : rls_)
							{
								rl->TopologyType(RenderLayout::TT_3_Ctrl_Pt_PatchList);
							}
							smooth_mesh_ = true;
						}
						else
						{
							technique_ = effect_->TechniqueByName("GenDPShadowMap");
							smooth_mesh_ = false;
						}
					}
					for (auto const & rl : rls_)
					{
						rl->NumInstances(1);
					}
					break;
				
				case SMT_Cube:
					technique_ = effect_->TechniqueByName("GenCubeShadowMap");
					smooth_mesh_ = false;
					for (auto const & rl : rls_)
					{
						rl->NumInstances(1);
					}
					break;

				case SMT_CubeOne:
					technique_ = effect_->TechniqueByName("GenCubeOneShadowMap");
					smooth_mesh_ = false;
					for (auto const & rl : rls_)
					{
						rl->NumInstances(1);
					}
					break;

				case SMT_CubeOneInstance:
					technique_ = effect_->TechniqueByName("GenCubeOneInstanceShadowMap");
					smooth_mesh_ = false;
					for (auto const & rl : rls_)
					{
						rl->NumInstances(6);
					}
					break;

				default:
					technique_ = effect_->TechniqueByName("GenCubeOneInstanceGSShadowMap");
					smooth_mesh_ = false;
					for (auto const & rl : rls_)
					{
						rl->NumInstances(1);
					}
					break;
				}
			}
			else
			{
				if (SMT_DP == sm_type_)
				{
					technique_ = effect_->TechniqueByName("RenderSceneDPSM");
				}
				else
				{
					technique_ = effect_->TechniqueByName("RenderScene");
				}
				smooth_mesh_ = false;
				for (auto const & rl : rls_)
				{
					rl->NumInstances(1);
				}
			}
		}

		void OnRenderBegin()
		{
			ShadowMapped::OnRenderBegin(model_mat_, effect_);
			StaticMesh::OnRenderBegin();

			if (smooth_mesh_)
			{
				RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
				if (caps.tess_method != TM_No)
				{
					*(effect_->ParameterByName("adaptive_tess")) = true;
					*(effect_->ParameterByName("tess_factors")) = float4(tess_factor_, tess_factor_, 1.0f, 32.0f);
				}
			}
		}
		
		void SetTessFactor(int32_t tess_factor)
		{
			tess_factor_ = static_cast<float>(tess_factor);
		}

	private:	
		bool smooth_mesh_;
		float tess_factor_;
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
				: App3DFramework("ShadowCubeMap"),
					sm_type_(SMT_DP)
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
	auto shadow_srv = rf.MakeTextureSrv(shadow_tex_);
	shadow_cube_buffer_ = rf.MakeFrameBuffer();
	shadow_cube_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(shadow_tex_, 0, 1, 0));
	shadow_cube_buffer_->Attach(depth_view);

	shadow_cube_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	if (caps.render_to_texture_array_support)
	{
		shadow_cube_one_tex_ = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, shadow_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		shadow_cube_one_buffer_ = rf.MakeFrameBuffer();
		shadow_cube_one_buffer_->Viewport()->Camera()->OmniDirectionalMode(true);
		shadow_cube_one_buffer_->Viewport()->Camera()->ProjParams(PI / 2, 1, 0.1f, 500.0f);
		shadow_cube_one_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.MakeCubeRtv(shadow_cube_one_tex_, 0, 0));
		TexturePtr shadow_one_depth_tex = rf.MakeTextureCube(SHADOW_MAP_SIZE, 1, 1, EF_D24S8, 1, 0, EAH_GPU_Write);
		shadow_cube_one_buffer_->Attach(rf.MakeCubeDsv(shadow_one_depth_tex, 0, 0));
	}

	for (int i = 0; i < 2; ++ i)
	{
		shadow_dual_texs_[i] = rf.MakeTexture2D(SHADOW_MAP_SIZE,  SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		shadow_dual_view_[i] = rf.Make2DRtv(shadow_dual_texs_[i], 0, 1, 0);

		shadow_dual_buffers_[i] = rf.MakeFrameBuffer();
		shadow_dual_buffers_[i]->Viewport()->Camera()->ProjParams(PI, 1, 0.1f, 500.0f);
		shadow_dual_buffers_[i]->Attach(FrameBuffer::Attachment::Color0, shadow_dual_view_[i]);
		shadow_dual_buffers_[i]->Attach(depth_view);
	}
	shadow_dual_tex_ = rf.MakeTexture2D(SHADOW_MAP_SIZE * 2,  SHADOW_MAP_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read);

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

	for (int i = 0; i < 6; ++ i)
	{
		sm_filter_pps_[i] = MakeSharedPtr<LogGaussianBlurPostProcess>(3, true);
		sm_filter_pps_[i]->InputPin(0, shadow_srv);
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

	UIManager::Instance().Load(ResLoader::Instance().Open("ShadowCubeMap.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_scale_factor_static_ = dialog_->IDFromName("ScaleFactorStatic");
	id_scale_factor_slider_ = dialog_->IDFromName("ScaleFactorSlider");
	id_sm_type_static_ = dialog_->IDFromName("SMStatic");
	id_sm_type_combo_ = dialog_->IDFromName("SMCombo");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

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
		dialog_->Control<UIComboBox>(id_sm_type_combo_)->RemoveItem(4);
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
						checked_pointer_cast<OccluderMesh>(scene_model_->Mesh(i))->DPSMTexture(shadow_dual_tex_);
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
				checked_pointer_cast<OccluderMesh>(teapot)->DPSMTexture(shadow_dual_tex_);
				checked_pointer_cast<OccluderMesh>(teapot)->ScaleFactor(esm_scale_factor_);
				scene_meshes_.push_back(teapot);

				loading_percentage_ = 100;
			}
		}
	}

	RenderEngine& renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (sm_type_)
	{
	case SMT_DP:
		switch (pass)
		{
		case 0:
		case 1:
			{
				float3 const pos = light_->Position();
				float3 const lookat = light_->Position() + ((0 == pass) ? 1.0f : -1.0f) * light_->Direction();

				auto& camera = *shadow_dual_buffers_[pass]->Viewport()->Camera();
				camera.LookAtDist(MathLib::length(lookat - pos));
				camera.BoundSceneNode()->TransformToWorld(MathLib::inverse(MathLib::look_at_lh(pos, lookat)));

				renderEngine.BindFrameBuffer(shadow_dual_buffers_[pass]);
				renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);

				for (auto const & mesh : scene_meshes_)
				{
					checked_pointer_cast<OccluderMesh>(mesh)->GenShadowMapPass(true, sm_type_, pass);
					checked_pointer_cast<OccluderMesh>(mesh)->LightSrc(light_);
				}
			}
			return App3DFramework::URV_NeedFlush;

		default:
			{
				renderEngine.BindFrameBuffer(FrameBufferPtr());
				
				Color clear_clr(0.2f, 0.4f, 0.6f, 1);
				if (Context::Instance().Config().graphics_cfg.gamma)
				{
					clear_clr.r() = 0.029f;
					clear_clr.g() = 0.133f;
					clear_clr.b() = 0.325f;
				}
				renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

				shadow_dual_texs_[0]->CopyToSubTexture2D(*shadow_dual_tex_, 0, 0, 0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, 0, 0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
				shadow_dual_texs_[1]->CopyToSubTexture2D(*shadow_dual_tex_, 0, 0, SHADOW_MAP_SIZE, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, 0, 0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

				for (auto const & mesh : scene_meshes_)
				{
					checked_pointer_cast<OccluderMesh>(mesh)->GenShadowMapPass(false, sm_type_, pass);
				}
			}
			return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
		}
		break;
	
	case SMT_Cube:
		if (pass > 0)
		{
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
				renderEngine.BindFrameBuffer(shadow_cube_buffer_);
				renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);

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
				renderEngine.BindFrameBuffer(FrameBufferPtr());

				Color clear_clr(0.2f, 0.4f, 0.6f, 1);
				if (Context::Instance().Config().graphics_cfg.gamma)
				{
					clear_clr.r() = 0.029f;
					clear_clr.g() = 0.133f;
					clear_clr.b() = 0.325f;
				}
				renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

				for (auto const & mesh : scene_meshes_)
				{
					checked_pointer_cast<OccluderMesh>(mesh)->GenShadowMapPass(false, sm_type_, pass);
				}
			}
			return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
		}
		break;

	default:
		if (renderEngine.DeviceCaps().render_to_texture_array_support)
		{
			switch (pass)
			{
			case 0:
				{
					float3 const& pos = light_->Position();
					float3 const lookat = pos + light_->Direction();

					auto& camera = *shadow_cube_one_buffer_->Viewport()->Camera();
					camera.LookAtDist(MathLib::length(lookat - pos));
					camera.BoundSceneNode()->TransformToWorld(MathLib::inverse(MathLib::look_at_lh(pos, lookat)));

					renderEngine.BindFrameBuffer(shadow_cube_one_buffer_);
					renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1), 1.0f, 0);

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
						shadow_cube_one_tex_->CopyToSubTexture2D(*shadow_tex_, 0, 0, 0, 0, shadow_tex_->Width(0), shadow_tex_->Height(0), 
							p, 0, 0, 0, shadow_cube_one_tex_->Width(0), shadow_cube_one_tex_->Height(0));
						checked_pointer_cast<LogGaussianBlurPostProcess>(sm_filter_pps_[p])->ESMScaleFactor(esm_scale_factor_,
							*light_->SMCamera(p));
						sm_filter_pps_[p]->Apply();
					}

					renderEngine.BindFrameBuffer(FrameBufferPtr());

					Color clear_clr(0.2f, 0.4f, 0.6f, 1);
					if (Context::Instance().Config().graphics_cfg.gamma)
					{
						clear_clr.r() = 0.029f;
						clear_clr.g() = 0.133f;
						clear_clr.b() = 0.325f;
					}
					renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

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
