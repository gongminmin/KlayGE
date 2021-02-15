#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/RenderMaterial.hpp>

#include <KlayGE/SSRPostProcess.hpp>

#include <iterator>
#include <sstream>

#include "SampleCommon.hpp"
#include "Reflection.hpp"

using namespace KlayGE;

namespace
{
	class DualSideSSRMesh : public StaticMesh
	{
	public:
		explicit DualSideSSRMesh(std::wstring_view name) : StaticMesh(name)
		{
		}

		void DoBuildMeshInfo(RenderModel const & model) override
		{
			StaticMesh::DoBuildMeshInfo(model);

			mtl_ = SyncLoadRenderMaterial("ReflectMesh.mtlml");

			effect_attrs_ |= EA_Reflection;

			this->BindDeferredEffect(SyncLoadRenderEffect("Reflection.fxml"));
			technique_ = special_shading_tech_;

			auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			if (re.DeviceCaps().vp_rt_index_at_every_stage_support)
			{
				reflection_tech_ = effect_->TechniqueByName("DualSideSSRReflectionTech");
				special_shading_tech_ = effect_->TechniqueByName("DualSideSSRSpecialShadingTech");
			}
			else
			{
				reflection_tech_ = effect_->TechniqueByName("DualSideSSRReflectionNoVpRtTech");
				special_shading_tech_ = effect_->TechniqueByName("DualSideSSRSpecialShadingNoVpRtTech");
			}

			reflection_alpha_blend_back_tech_ = reflection_tech_;
			reflection_alpha_blend_front_tech_ = reflection_tech_;

			special_shading_alpha_blend_back_tech_ = special_shading_tech_;
			special_shading_alpha_blend_front_tech_ = special_shading_tech_;

			reflection_tex_param_ = effect_->ParameterByName("reflection_tex");
		}

		void OnRenderBegin()
		{
			StaticMesh::OnRenderBegin();

			switch (type_)
			{
			case PT_OpaqueReflection:
			case PT_TransparencyBackReflection:
			case PT_TransparencyFrontReflection:
				{
					App3DFramework const & app = Context::Instance().AppInstance();
					Camera const & camera = app.ActiveCamera();
					*(effect_->ParameterByName("proj")) = camera.ProjMatrix();
					*(effect_->ParameterByName("inv_proj")) = camera.InverseProjMatrix();
					float4 const near_q_far = camera.NearQFarParam();
					*(effect_->ParameterByName("near_q_far")) = float3(near_q_far.x(), near_q_far.y(), near_q_far.z());
					*(effect_->ParameterByName("ray_length")) = camera.FarPlane() - camera.NearPlane();
					*(effect_->ParameterByName("inv_view")) = camera.InverseViewMatrix();
				}
				break;

			default:
				break;
			}
		}

		void FrontReflectionTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("front_side_tex")) = tex;
		}
		void FrontReflectionDepthTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("front_side_depth_tex")) = tex;
		}

		void BackReflectionTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("back_side_tex")) = tex;
		}
		void BackReflectionDepthTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("back_side_depth_tex")) = tex;
		}

		void BackCamera(CameraPtr const & camera)
		{
			float4x4 const & view = camera->ViewMatrix();
			float4x4 const & proj = camera->ProjMatrix();

			float4x4 const mv = model_mat_ * view;

			*(effect_->ParameterByName("back_model_view")) = mv;
			*(effect_->ParameterByName("back_proj")) = proj;
			*(effect_->ParameterByName("back_mvp")) = mv * proj;
			float4 const near_q_far = camera->NearQFarParam();
			*(effect_->ParameterByName("back_near_q_far")) = float3(near_q_far.x(), near_q_far.y(), near_q_far.z());

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & scene_camera = app.ActiveCamera();
			*(effect_->ParameterByName("eye_in_back_camera")) = MathLib::transform_coord(scene_camera.EyePos(), view);
		}

		void MinSamples(int32_t samples)
		{
			*(effect_->ParameterByName("min_samples")) = samples;
		}
		void MaxSamples(int32_t samples)
		{
			*(effect_->ParameterByName("max_samples")) = samples;
		}

		void EnbleReflection(bool enable)
		{
			if (enable)
			{
				effect_attrs_ |= EA_SpecialShading;
			}
			else
			{
				effect_attrs_ &= ~EA_SpecialShading;
			}
		}

		void SkyBox(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			*(effect_->ParameterByName("skybox_tex")) = y_cube;
			*(effect_->ParameterByName("skybox_C_tex")) = c_cube;
		}
	};

	class SingleSideSSRPlane : public RenderablePlane
	{
	public:
		explicit SingleSideSSRPlane() : RenderablePlane(20.0f, 20.0f, 1, 1, true, true)
		{
			mtl_ = SyncLoadRenderMaterial("ReflectPlane.mtlml");

			effect_attrs_ |= EA_Reflection;

			this->BindDeferredEffect(SyncLoadRenderEffect("Reflection.fxml"));
			technique_ = special_shading_tech_;

			auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			if (re.DeviceCaps().vp_rt_index_at_every_stage_support)
			{
				reflection_tech_ = effect_->TechniqueByName("SingleSideSSRReflectionTech");
				special_shading_tech_ = effect_->TechniqueByName("SingleSideSSRSpecialShadingTech");
			}
			else
			{
				reflection_tech_ = effect_->TechniqueByName("SingleSideSSRReflectionNoVpRtTech");
				special_shading_tech_ = effect_->TechniqueByName("SingleSideSSRSpecialShadingNoVpRtTech");
			}

			reflection_alpha_blend_back_tech_ = reflection_tech_;
			reflection_alpha_blend_front_tech_ = reflection_tech_;

			special_shading_alpha_blend_back_tech_ = special_shading_tech_;
			special_shading_alpha_blend_front_tech_ = special_shading_tech_;

			reflection_tex_param_ = effect_->ParameterByName("reflection_tex");
		}

		void OnRenderBegin() override
		{
			model_mat_ = MathLib::rotation_x(DEG90) * MathLib::translation(0.0f, -0.2f, 0.0f);

			RenderablePlane::OnRenderBegin();

			switch (type_)
			{
			case PT_OpaqueReflection:
			case PT_TransparencyBackReflection:
			case PT_TransparencyFrontReflection:
			{
				App3DFramework const& app = Context::Instance().AppInstance();
				Camera const& camera = app.ActiveCamera();
				*(effect_->ParameterByName("proj")) = camera.ProjMatrix();
				*(effect_->ParameterByName("inv_proj")) = camera.InverseProjMatrix();
				float4 const near_q_far = camera.NearQFarParam();
				*(effect_->ParameterByName("near_q_far")) = float3(near_q_far.x(), near_q_far.y(), near_q_far.z());
				*(effect_->ParameterByName("ray_length")) = camera.FarPlane() - camera.NearPlane();
				*(effect_->ParameterByName("inv_view")) = camera.InverseViewMatrix();
			}
			break;

			default:
				break;
			}
		}

		void FrontReflectionTex(TexturePtr const& tex)
		{
			*(effect_->ParameterByName("front_side_tex")) = tex;
		}
		void FrontReflectionDepthTex(TexturePtr const& tex)
		{
			*(effect_->ParameterByName("front_side_depth_tex")) = tex;
		}

		void BackReflectionTex(TexturePtr const& tex)
		{
			*(effect_->ParameterByName("back_side_tex")) = tex;
		}
		void BackReflectionDepthTex(TexturePtr const& tex)
		{
			*(effect_->ParameterByName("back_side_depth_tex")) = tex;
		}

		void MinSamples(int32_t samples)
		{
			*(effect_->ParameterByName("min_samples")) = samples;
		}
		void MaxSamples(int32_t samples)
		{
			*(effect_->ParameterByName("max_samples")) = samples;
		}

		void EnbleReflection(bool enable)
		{
			if (enable)
			{
				effect_attrs_ |= EA_SpecialShading;
			}
			else
			{
				effect_attrs_ &= ~EA_SpecialShading;
			}
		}

		void SkyBox(TexturePtr const& y_cube, TexturePtr const& c_cube)
		{
			*(effect_->ParameterByName("skybox_tex")) = y_cube;
			*(effect_->ParameterByName("skybox_C_tex")) = c_cube;
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
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	ScreenSpaceReflectionApp app;
	app.Create();
	app.Run();
	
	return 0;
}

ScreenSpaceReflectionApp::ScreenSpaceReflectionApp()
	: App3DFramework("Reflection")
{
	ResLoader::Instance().AddPath("../../Samples/media/CausticsMap");
	ResLoader::Instance().AddPath("../../Samples/media/Reflection");
}

void ScreenSpaceReflectionApp::OnCreate()
{
	UIManager::Instance().Load(*ResLoader::Instance().Open("Reflection.uiml"));
	parameter_dialog_ = UIManager::Instance().GetDialog("Reflection");

	id_min_sample_num_static_ = parameter_dialog_->IDFromName("min_sample_num_static");
	id_min_sample_num_slider_ = parameter_dialog_->IDFromName("min_sample_num_slider");
	parameter_dialog_->Control<UISlider>(id_min_sample_num_slider_)->OnValueChangedEvent().Connect([this](UISlider const& sender) {
		this->MinSampleNumHandler(sender);
	});

	id_max_sample_num_static_ = parameter_dialog_->IDFromName("max_sample_num_static");
	id_max_sample_num_slider_ = parameter_dialog_->IDFromName("max_sample_num_slider");
	parameter_dialog_->Control<UISlider>(id_max_sample_num_slider_)->OnValueChangedEvent().Connect([this](UISlider const& sender) {
		this->MaxSampleNumHandler(sender);
	});

	id_enable_reflection_ = parameter_dialog_->IDFromName("enable_reflection");
	parameter_dialog_->Control<UICheckBox>(id_enable_reflection_)->OnChangedEvent().Connect([this](UICheckBox const& sender) {
		this->EnbleReflectionHandler(sender);
	});

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	c_cube_ = ASyncLoadTexture("Lake_CraterLake03_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	y_cube_ = ASyncLoadTexture("Lake_CraterLake03_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	teapot_model_ = ASyncLoadModel(
		"teapot.glb", EAH_GPU_Read | EAH_Immutable, SceneNode::SOA_Cullable,
		[this](RenderModel& model) {
			teapot_node_ = model.RootNode();
			teapot_node_->TransformToParent(MathLib::scaling(float3(15, 15, 15)));
			teapot_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<DualSideSSRMesh>().SkyBox(y_cube_, c_cube_);
			AddToSceneRootHelper(model);

			this->MinSampleNumHandler(*(parameter_dialog_->Control<UISlider>(id_min_sample_num_slider_)));
			this->MaxSampleNumHandler(*(parameter_dialog_->Control<UISlider>(id_max_sample_num_slider_)));
			this->EnbleReflectionHandler(*(parameter_dialog_->Control<UICheckBox>(id_enable_reflection_)));
		},
		CreateModelFactory<RenderModel>, CreateMeshFactory<DualSideSSRMesh>);
	auto dino_model = ASyncLoadModel("dino50.glb", EAH_GPU_Read | EAH_Immutable, SceneNode::SOA_Cullable, [](RenderModel& model) {
		model.Mesh(0)->Material(SyncLoadRenderMaterial("DinoMesh.mtlml"));
		model.RootNode()->TransformToParent(MathLib::scaling(float3(2, 2, 2)) * MathLib::translation(0.0f, 1.0f, -2.5f));
		AddToSceneRootHelper(model);
	});

	this->LookAt(float3(3.0f, 3.0f, -6.75f), float3(0.0f, 1.0f, 0.0f), float3(0, 1, 0));
	this->Proj(0.1f, 500.0f);
	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.003f, 0.05f);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	plane_node_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(MakeSharedPtr<SingleSideSSRPlane>()), SceneNode::SOA_Cullable);
	plane_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<SingleSideSSRPlane>().SkyBox(y_cube_, c_cube_);
	root_node.AddChild(plane_node_);

	screen_camera_path_ = LoadCameraPath(*ResLoader::Instance().Open("Reflection.cam_path"));
	screen_camera_path_->AttachCamera(this->ActiveCamera());
	auto camera_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
	camera_node->AddComponent(this->ActiveCamera().shared_from_this());
	root_node.AddChild(camera_node);

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube_, c_cube_);
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	root_node.AddComponent(ambient_light);

	auto point_light = MakeSharedPtr<PointLightSource>();
	point_light->Attrib(LightSource::LSA_NoShadow);
	point_light->Color(float3(1, 1, 1));
	point_light->Falloff(float3(1, 0, 0.3f));

	auto point_light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	point_light_node->TransformToParent(MathLib::translation(0.0f, 3.0f, -2.0f));
	point_light_node->AddComponent(point_light);
	root_node.AddChild(point_light_node);

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();

	font_ = SyncLoadFont("gkai00mp.kfont");

	skybox_ = MakeSharedPtr<RenderableSkyBox>();
	checked_cast<RenderableSkyBox&>(*skybox_).CompressedCubeMap(y_cube_, c_cube_);			
	root_node.AddChild(MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox_), SceneNode::SOA_NotCastShadow));

	back_refl_fb_ = rf.MakeFrameBuffer();

	auto back_refl_camera_node = MakeSharedPtr<SceneNode>(
		L"BackReflectionCameraNode", SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
	back_refl_camera_node->AddComponent(back_refl_fb_->Viewport()->Camera());
	root_node.AddChild(back_refl_camera_node);

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
}

void ScreenSpaceReflectionApp::OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	deferred_rendering_->SetupViewport(1, re.CurFrameBuffer(), VPAM_NoSSVO | VPAM_NoGI);
	
	back_refl_tex_ = rf.MakeTexture2D(width / 2, width / 2, 1, 1, 
		deferred_rendering_->ShadingTex(1)->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	back_refl_ds_tex_ = rf.MakeTexture2D(width / 2, width / 2, 1, 1, 
		EF_D16, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	back_refl_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(back_refl_tex_, 0, 1, 0));
	back_refl_fb_->Attach(rf.Make2DDsv(back_refl_ds_tex_, 0, 1, 0));

	deferred_rendering_->SetupViewport(0, back_refl_fb_, VPAM_NoTransparencyBack | VPAM_NoTransparencyFront | VPAM_NoSimpleForward | VPAM_NoGI | VPAM_NoSSVO);

	screen_camera_ = re.CurFrameBuffer()->Viewport()->Camera();
}

void ScreenSpaceReflectionApp::InputHandler(KlayGE::InputEngine const & /*sender*/, KlayGE::InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ScreenSpaceReflectionApp::MinSampleNumHandler(KlayGE::UISlider const & sender)
{
	int32_t sample_num = sender.GetValue();
	if (teapot_node_)
	{
		teapot_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<DualSideSSRMesh>().MinSamples(sample_num);
		plane_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<SingleSideSSRPlane>().MinSamples(sample_num);

		std::wostringstream oss;
		oss << "Min Samples: " << sample_num;
		parameter_dialog_->Control<UIStatic>(id_min_sample_num_static_)->SetText(oss.str());
	}
}

void ScreenSpaceReflectionApp::MaxSampleNumHandler(KlayGE::UISlider const & sender)
{
	int32_t sample_num = sender.GetValue();
	if (teapot_node_)
	{
		teapot_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<DualSideSSRMesh>().MaxSamples(sample_num);
		plane_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<SingleSideSSRPlane>().MaxSamples(sample_num);

		std::wostringstream oss;
		oss << "Max Samples: " << sample_num;
		parameter_dialog_->Control<UIStatic>(id_max_sample_num_static_)->SetText(oss.str());
	}
}

void ScreenSpaceReflectionApp::EnbleReflectionHandler(KlayGE::UICheckBox const & sender)
{
	bool enabled = sender.GetChecked();
	if (teapot_node_)
	{
		teapot_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<DualSideSSRMesh>().EnbleReflection(enabled);
		plane_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<SingleSideSSRPlane>().EnbleReflection(enabled);
	}
}

void ScreenSpaceReflectionApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Reflection", 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	uint32_t const num_loading_res = ResLoader::Instance().NumLoadingResources();
	if (num_loading_res > 0)
	{
		stream.str(L"");
		stream << "Loading " << num_loading_res << " resources...";
		font_->RenderText(100, 300, Color(1, 0, 0, 1), stream.str(), 48);
	}
}

uint32_t ScreenSpaceReflectionApp::DoUpdate(KlayGE::uint32_t pass)
{
	uint32_t urt;

	if (0 == pass)
	{
		CameraPtr const& back_camera = back_refl_fb_->Viewport()->Camera();

		float3 eye = screen_camera_->EyePos();
		float3 at = screen_camera_->LookAt();

		auto& teapot_mesh = teapot_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<DualSideSSRMesh>();

		float3 center = MathLib::transform_coord(teapot_mesh.PosBound().Center(), teapot_node_->TransformToWorld());
		float3 direction = eye - at;

		back_camera->LookAtDist(MathLib::length(direction));
		back_camera->BoundSceneNode()->TransformToWorld(
			MathLib::inverse(MathLib::look_at_lh(center, center + direction, screen_camera_->UpVec())));
		back_camera->ProjParams(PI / 2, 1, screen_camera_->NearPlane(), screen_camera_->FarPlane());

		teapot_mesh.BackCamera(back_camera);

		teapot_mesh.FrontReflectionTex(deferred_rendering_->PrevFrameResolvedShadingTex(1));
		teapot_mesh.FrontReflectionDepthTex(deferred_rendering_->PrevFrameResolvedDepthTex(1));
		teapot_mesh.BackReflectionTex(deferred_rendering_->CurrFrameResolvedShadingTex(0));
		teapot_mesh.BackReflectionDepthTex(deferred_rendering_->CurrFrameResolvedDepthTex(0));

		auto& plane = plane_node_->FirstComponentOfType<RenderableComponent>()->BoundRenderableOfType<SingleSideSSRPlane>();
		plane.FrontReflectionTex(deferred_rendering_->PrevFrameResolvedShadingTex(1));
		plane.FrontReflectionDepthTex(deferred_rendering_->PrevFrameResolvedDepthTex(1));
	}

	urt = deferred_rendering_->Update(pass);

	if (teapot_node_)
	{
		if (0 == deferred_rendering_->ActiveViewport())
		{
			teapot_node_->Visible(false);
		}
		else
		{
			teapot_node_->Visible(true);
		}
	}
	if (plane_node_)
	{
		if (0 == deferred_rendering_->ActiveViewport())
		{
			plane_node_->Visible(false);
		}
		else
		{
			plane_node_->Visible(true);
		}
	}

	return urt;
}
