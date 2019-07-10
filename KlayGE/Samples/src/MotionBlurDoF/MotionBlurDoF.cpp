#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/SATPostProcess.hpp>
#include <KlayGE/Script.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/MotionBlur.hpp>
#include <KlayGE/DepthOfField.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/ScriptFactory.hpp>

#include <sstream>
#include <random>

#include "SampleCommon.hpp"
#include "MotionBlurDoF.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int32_t const NUM_LINE = 10;
	int32_t const NUM_INSTANCE = 400;

	class MotionBlurRenderMesh : public StaticMesh
	{
	public:
		explicit MotionBlurRenderMesh(std::wstring_view name)
			: StaticMesh(name),
				half_exposure_(1)
		{
		}

		virtual void VelocityPass(bool velocity) = 0;

		void BlurRadius(uint32_t blur_radius)
		{
			*(effect_->ParameterByName("blur_radius")) = static_cast<float>(blur_radius);
		}

		void Exposure(float exposure)
		{
			half_exposure_ = 0.5f * exposure;
		}

	protected:
		float half_exposure_;
	};

	class RenderInstanceMesh : public MotionBlurRenderMesh
	{
	public:
		explicit RenderInstanceMesh(std::wstring_view /*name*/)
			: MotionBlurRenderMesh(L"InstancedMesh")
		{
			effect_ = SyncLoadRenderEffect("MotionBlurDoF.fxml");
			technique_ = effect_->TechniqueByName("ColorDepthInstanced");
		}

		void OnRenderBegin()
		{
			MotionBlurRenderMesh::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & curr_view = camera.ViewMatrix();
			float4x4 const & curr_proj = camera.ProjMatrix();
			float4x4 const & prev_view = camera.PrevViewMatrix();
			float4x4 const & prev_proj = camera.PrevProjMatrix();

			*(effect_->ParameterByName("eye_in_world")) = camera.EyePos();
			*(effect_->ParameterByName("view")) = curr_view;
			*(effect_->ParameterByName("proj")) = curr_proj;

			*(effect_->ParameterByName("prev_view")) = prev_view;
			*(effect_->ParameterByName("prev_proj")) = prev_proj;
			*(effect_->ParameterByName("half_exposure_x_framerate")) = half_exposure_ / app.FrameTime();
		}

		void VelocityPass(bool velocity) override
		{
			if (velocity)
			{
				technique_ = effect_->TechniqueByName("VelocityInstanced");
			}
			else
			{
				technique_ = effect_->TechniqueByName("ColorDepthInstanced");
			}
		}
	};

	class RenderNonInstancedMesh : public MotionBlurRenderMesh
	{
	private:
		struct InstData
		{
			float4 mat[3];
			float4 last_mat[3];
			uint32_t clr;
		};

	public:
		explicit RenderNonInstancedMesh(std::wstring_view /*name*/)
			: MotionBlurRenderMesh(L"NonInstancedMesh")
		{
			effect_ = SyncLoadRenderEffect("MotionBlurDoF.fxml");
			technique_ = effect_->TechniqueByName("ColorDepthNonInstanced");
		}

		void OnRenderBegin()
		{
			MotionBlurRenderMesh::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const & curr_view = camera.ViewMatrix();
			float4x4 const & curr_proj = camera.ProjMatrix();
			float4x4 const & prev_view = camera.PrevViewMatrix();
			float4x4 const & prev_proj = camera.PrevProjMatrix();

			*(effect_->ParameterByName("eye_in_world")) = camera.EyePos();
			*(effect_->ParameterByName("view")) = curr_view;
			*(effect_->ParameterByName("proj")) = curr_proj;
			*(effect_->ParameterByName("prev_view")) = prev_view;
			*(effect_->ParameterByName("prev_proj")) = prev_proj;

			*(effect_->ParameterByName("half_exposure_x_framerate")) = half_exposure_ / app.FrameTime();

			InstData const * data = static_cast<InstData const *>(curr_node_->InstanceData());

			float4x4 model;
			model.Col(0, data->mat[0]);
			model.Col(1, data->mat[1]);
			model.Col(2, data->mat[2]);
			model.Col(3, float4(0, 0, 0, 1));

			float4x4 last_model;
			last_model.Col(0, data->last_mat[0]);
			last_model.Col(1, data->last_mat[1]);
			last_model.Col(2, data->last_mat[2]);
			last_model.Col(3, float4(0, 0, 0, 1));

			*(effect_->ParameterByName("modelmat")) = model;
			*(effect_->ParameterByName("last_modelmat")) = last_model;
			Color clr(data->clr);
			*(effect_->ParameterByName("color")) = float4(clr.b(), clr.g(), clr.r(), clr.a());	// swap b and r
		}

		void VelocityPass(bool velocity) override
		{
			if (velocity)
			{
				technique_ = effect_->TechniqueByName("VelocityNonInstanced");
			}
			else
			{
				technique_ = effect_->TechniqueByName("ColorDepthNonInstanced");
			}
		}

	private:
		void UpdateInstanceStream()
		{
		}
	};

	class Teapot
	{
	private:
		struct InstData
		{
			float4 mat[3];
			float4 last_mat[3];
			uint32_t clr;
		};

	public:
		Teapot() : node_(MakeSharedPtr<SceneNode>(L"TeapotNode", SceneNode::SOA_Moveable | SceneNode::SOA_Cullable))
		{
			auto& instance_format = node_->InstanceFormat();
			instance_format.assign({
				VertexElement(VEU_TextureCoord, 1, EF_ABGR32F),
				VertexElement(VEU_TextureCoord, 2, EF_ABGR32F),
				VertexElement(VEU_TextureCoord, 3, EF_ABGR32F),
				VertexElement(VEU_TextureCoord, 4, EF_ABGR32F),
				VertexElement(VEU_TextureCoord, 5, EF_ABGR32F),
				VertexElement(VEU_TextureCoord, 6, EF_ABGR32F),
				VertexElement(VEU_Diffuse, 0, EF_ABGR8)
			});

			node_->InstanceData(&inst_);

			node_->OnMainThreadUpdate().Connect([this](SceneNode& node, float app_time, float elapsed_time)
				{
					KFL_UNUSED(node);
					KFL_UNUSED(app_time);

					last_xform_to_parent_ = node.TransformToParent();

					float4x4 mat_t = MathLib::transpose(last_xform_to_parent_);
					inst_.last_mat[0] = mat_t.Row(0);
					inst_.last_mat[1] = mat_t.Row(1);
					inst_.last_mat[2] = mat_t.Row(2);

					float e = elapsed_time * 0.3f * -node.TransformToParent()(3, 1);
					node.TransformToParent(node.TransformToParent() * MathLib::rotation_y(e));

					mat_t = MathLib::transpose(node.TransformToParent());
					inst_.mat[0] = mat_t.Row(0);
					inst_.mat[1] = mat_t.Row(1);
					inst_.mat[2] = mat_t.Row(2);
				});
		}

		void Instance(float4x4 const & mat, Color const & clr)
		{
			node_->TransformToParent(mat);
			inst_.clr = clr.ABGR();
		}

		SceneNodePtr const& RootNode() const
		{
			return node_;
		}

	private:
		SceneNodePtr node_;
		InstData inst_;
		float4x4 last_xform_to_parent_;
	};


	enum
	{
		CtrlCamera,
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(CtrlCamera, KS_LeftCtrl),
		InputActionDefine(CtrlCamera, KS_RightCtrl),
		InputActionDefine(Exit, KS_Escape),
	};
}

int SampleMain()
{
	MotionBlurDoFApp app;
	app.Create();
	app.Run();

	return 0;
}

MotionBlurDoFApp::MotionBlurDoFApp()
					: App3DFramework("MotionBlurDoF"),
						use_instance_(true), exposure_(2), blur_radius_(2),
						dof_on_(true), bokeh_on_(true), mb_on_(true),
						num_objs_rendered_(0), num_renderables_rendered_(0),
						num_primitives_rendered_(0), num_vertices_rendered_(0)
{
	ResLoader::Instance().AddPath("../../Samples/media/MotionBlurDoF");
}

void MotionBlurDoFApp::OnCreate()
{
	loading_percentage_ = 0;
	model_instance_ = ASyncLoadModel("teapot.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, nullptr, CreateModelFactory<RenderModel>, CreateMeshFactory<RenderInstanceMesh>);
	model_mesh_ = ASyncLoadModel("teapot.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, nullptr, CreateModelFactory<RenderModel>, CreateMeshFactory<RenderNonInstancedMesh>);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	font_ = SyncLoadFont("gkai00mp.kfont");

	ScriptEngine& scriptEngine = Context::Instance().ScriptFactoryInstance().ScriptEngineInstance();
	script_module_ = scriptEngine.CreateModule("MotionBlurDoF_init");

	this->LookAt(float3(-1.8f, 1.9f, -1.8f), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	clr_depth_fb_ = rf.MakeFrameBuffer();
	velocity_fb_ = rf.MakeFrameBuffer();
	clr_depth_fb_->Viewport()->Camera(re.CurFrameBuffer()->Viewport()->Camera());
	velocity_fb_->Viewport()->Camera(re.CurFrameBuffer()->Viewport()->Camera());

	fpcController_.Scalers(0.05f, 0.5f);

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

	if (caps.fp_color_support && caps.TextureRenderTargetFormatSupport(EF_ABGR32F, 1, 0))
	{
		depth_of_field_ = MakeSharedPtr<DepthOfField>();
		bokeh_filter_ = MakeSharedPtr<BokehFilter>();
	}
	depth_of_field_copy_pp_ = SyncLoadPostProcess("Copy.ppml", "Copy");
	
	motion_blur_ = MakeSharedPtr<MotionBlurPostProcess>();
	motion_blur_copy_pp_ = SyncLoadPostProcess("Copy.ppml", "Copy");

	UIManager::Instance().Load(ResLoader::Instance().Open("MotionBlurDoF.uiml"));
	dof_dialog_ = UIManager::Instance().GetDialogs()[0];
	mb_dialog_ = UIManager::Instance().GetDialogs()[1];
	app_dialog_ = UIManager::Instance().GetDialogs()[2];

	id_dof_on_ = dof_dialog_->IDFromName("DoFOn");
	id_bokeh_on_ = dof_dialog_->IDFromName("BokehOn");
	id_dof_focus_plane_static_ = dof_dialog_->IDFromName("FocusPlaneStatic");
	id_dof_focus_plane_slider_ = dof_dialog_->IDFromName("FocusPlaneSlider");
	id_dof_focus_range_static_ = dof_dialog_->IDFromName("FocusRangeStatic");
	id_dof_focus_range_slider_ = dof_dialog_->IDFromName("FocusRangeSlider");
	id_dof_blur_factor_ = dof_dialog_->IDFromName("BlurFactor");
	id_mb_on_ = mb_dialog_->IDFromName("MBOn");
	id_mb_exposure_static_ = mb_dialog_->IDFromName("ExposureStatic");
	id_mb_exposure_slider_ = mb_dialog_->IDFromName("ExposureSlider");
	id_mb_blur_radius_static_ = mb_dialog_->IDFromName("BlurRadiusStatic");
	id_mb_blur_radius_slider_ = mb_dialog_->IDFromName("BlurRadiusSlider");
	id_mb_reconstruction_samples_static_ = mb_dialog_->IDFromName("SamplesStatic");
	id_mb_reconstruction_samples_slider_ = mb_dialog_->IDFromName("SamplesSlider");
	id_motion_blur_type_ = mb_dialog_->IDFromName("MotionBlurTypeCombo");
	id_use_instancing_ = app_dialog_->IDFromName("UseInstancing");
	id_ctrl_camera_ = app_dialog_->IDFromName("CtrlCamera");

	if (depth_of_field_)
	{
		dof_dialog_->Control<UICheckBox>(id_dof_on_)->OnChangedEvent().Connect(
			[this](UICheckBox const & sender)
			{
				this->DoFOnHandler(sender);
			});
		this->DoFOnHandler(*dof_dialog_->Control<UICheckBox>(id_dof_on_));
		dof_dialog_->Control<UICheckBox>(id_bokeh_on_)->OnChangedEvent().Connect(
			[this](UICheckBox const & sender)
			{
				this->BokehOnHandler(sender);
			});
		this->BokehOnHandler(*dof_dialog_->Control<UICheckBox>(id_bokeh_on_));
		dof_dialog_->Control<UISlider>(id_dof_focus_plane_slider_)->OnValueChangedEvent().Connect(
			[this](UISlider const & sender)
			{
				this->DoFFocusPlaneChangedHandler(sender);
			});
		this->DoFFocusPlaneChangedHandler(*dof_dialog_->Control<UISlider>(id_dof_focus_plane_slider_));
		dof_dialog_->Control<UISlider>(id_dof_focus_range_slider_)->OnValueChangedEvent().Connect(
			[this](UISlider const & sender)
			{
				this->DoFFocusRangeChangedHandler(sender);
			});
		this->DoFFocusRangeChangedHandler(*dof_dialog_->Control<UISlider>(id_dof_focus_range_slider_));
		dof_dialog_->Control<UICheckBox>(id_dof_blur_factor_)->OnChangedEvent().Connect(
			[this](UICheckBox const & sender)
			{
				this->DoFBlurFactorHandler(sender);
			});
		this->DoFBlurFactorHandler(*dof_dialog_->Control<UICheckBox>(id_dof_blur_factor_));
	}
	else
	{
		dof_dialog_->Control<UICheckBox>(id_dof_on_)->SetEnabled(false);
		dof_dialog_->Control<UICheckBox>(id_bokeh_on_)->SetEnabled(false);
		dof_dialog_->Control<UISlider>(id_dof_focus_plane_slider_)->SetEnabled(false);
		dof_dialog_->Control<UISlider>(id_dof_focus_range_slider_)->SetEnabled(false);
		dof_dialog_->Control<UICheckBox>(id_dof_blur_factor_)->SetEnabled(false);
		dof_on_ = false;
	}

	mb_dialog_->Control<UICheckBox>(id_mb_on_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->MBOnHandler(sender);
		});
	mb_dialog_->Control<UISlider>(id_mb_exposure_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->MBExposureChangedHandler(sender);
		});
	this->MBExposureChangedHandler(*mb_dialog_->Control<UISlider>(id_mb_exposure_slider_));
	mb_dialog_->Control<UISlider>(id_mb_blur_radius_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->MBBlurRadiusChangedHandler(sender);
		});
	this->MBBlurRadiusChangedHandler(*mb_dialog_->Control<UISlider>(id_mb_blur_radius_slider_));
	mb_dialog_->Control<UISlider>(id_mb_reconstruction_samples_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->MBReconstructionSamplesChangedHandler(sender);
		});
	this->MBReconstructionSamplesChangedHandler(*mb_dialog_->Control<UISlider>(id_mb_reconstruction_samples_slider_));
	mb_dialog_->Control<UIComboBox>(id_motion_blur_type_)->OnSelectionChangedEvent().Connect(
		[this](UIComboBox const & sender)
		{
			this->MotionBlurChangedHandler(sender);
		});

	app_dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->CtrlCameraHandler(sender);
		});

	app_dialog_->Control<UICheckBox>(id_use_instancing_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->UseInstancingHandler(sender);
		});
}

void MotionBlurDoFApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	depth_texture_support_ = caps.depth_texture_support;

	DepthStencilViewPtr ds_view;
	if (depth_texture_support_)
	{
		ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_D16, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		ds_view = rf.Make2DDsv(ds_tex_, 0, 1, 0);
	}
	else
	{
		ds_view = rf.Make2DDsv(width, height, EF_D16, 1, 0);
	}

	auto const depth_fmt = caps.BestMatchTextureRenderTargetFormat(
		caps.pack_to_rgba_required ? MakeSpan({EF_ABGR8, EF_ARGB8}) : MakeSpan({EF_R16F, EF_R32F}), 1, 0);
	BOOST_ASSERT(depth_fmt != EF_Unknown);
	depth_tex_ = rf.MakeTexture2D(width, height, 2, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
	auto depth_srv = rf.MakeTextureSrv(depth_tex_);

	if (depth_texture_support_)
	{
		depth_to_linear_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToLinear");
		depth_to_linear_pp_->InputPin(0, rf.MakeTextureSrv(ds_tex_));
		depth_to_linear_pp_->OutputPin(0, rf.Make2DRtv(depth_tex_, 0, 1, 0));
	}

	auto const color_fmt = caps.BestMatchTextureRenderTargetFormat(caps.fp_color_support ? MakeSpan({EF_B10G11R11F, EF_ABGR16F})
		: MakeSpan({ EF_ABGR8, EF_ARGB8 }), 1, 0);
	BOOST_ASSERT(color_fmt != EF_Unknown);

	color_tex_ = rf.MakeTexture2D(width, height, 2, 1, color_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
	auto color_srv = rf.MakeTextureSrv(color_tex_);
	clr_depth_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(color_tex_, 0, 1, 0));
	clr_depth_fb_->Attach(ds_view);

	auto const motion_fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_GR8, EF_ABGR8, EF_ARGB8}), 1, 0);
	BOOST_ASSERT(motion_fmt != EF_Unknown);
	velocity_tex_ = rf.MakeTexture2D(width, height, 1, 1, motion_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	auto velocity_srv = rf.MakeTextureSrv(velocity_tex_);
	velocity_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(velocity_tex_, 0, 1, 0));
	velocity_fb_->Attach(ds_view);

	dof_tex_ = rf.MakeTexture2D(width, height, 1, 1, color_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	auto dof_srv = rf.MakeTextureSrv(dof_tex_);
	auto dof_rtv = rf.Make2DRtv(dof_tex_, 0, 1, 0);

	if (depth_of_field_)
	{
		depth_of_field_->InputPin(0, color_srv);
		depth_of_field_->InputPin(1, depth_srv);
		depth_of_field_->OutputPin(0, dof_rtv);
	}
	depth_of_field_copy_pp_->InputPin(0, color_srv);
	depth_of_field_copy_pp_->OutputPin(0, dof_rtv);

	if (bokeh_filter_)
	{
		bokeh_filter_->InputPin(0, color_srv);
		bokeh_filter_->InputPin(1, depth_srv);
		bokeh_filter_->OutputPin(0, dof_rtv);
	}

	motion_blur_->InputPin(0, dof_srv);
	motion_blur_->InputPin(1, depth_srv);
	motion_blur_->InputPin(2, velocity_srv);
	motion_blur_copy_pp_->InputPin(0, dof_srv);

	UIManager::Instance().SettleCtrls();
}

void MotionBlurDoFApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void MotionBlurDoFApp::DoFOnHandler(KlayGE::UICheckBox const & sender)
{
	dof_on_ = sender.GetChecked();

	dof_dialog_->Control<UICheckBox>(id_bokeh_on_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UIStatic>(id_dof_focus_plane_static_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UISlider>(id_dof_focus_plane_slider_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UIStatic>(id_dof_focus_range_static_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UISlider>(id_dof_focus_range_slider_)->SetEnabled(dof_on_);
	dof_dialog_->Control<UICheckBox>(id_dof_blur_factor_)->SetEnabled(dof_on_);
}

void MotionBlurDoFApp::BokehOnHandler(KlayGE::UICheckBox const & sender)
{
	bokeh_on_ = sender.GetChecked();
}

void MotionBlurDoFApp::DoFFocusPlaneChangedHandler(KlayGE::UISlider const & sender)
{
	float focus_plane = sender.GetValue() / 50.0f;
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusPlane(focus_plane);
	if (bokeh_filter_)
	{
		checked_pointer_cast<BokehFilter>(bokeh_filter_)->FocusPlane(focus_plane);
	}

	std::wostringstream stream;
	stream << "Focus Plane: " << focus_plane;
	dof_dialog_->Control<UIStatic>(id_dof_focus_plane_static_)->SetText(stream.str());
}

void MotionBlurDoFApp::DoFFocusRangeChangedHandler(KlayGE::UISlider const & sender)
{
	float focus_range = sender.GetValue() / 50.0f;
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusRange(focus_range);
	if (bokeh_filter_)
	{
		checked_pointer_cast<BokehFilter>(bokeh_filter_)->FocusRange(focus_range);
	}

	std::wostringstream stream;
	stream << "Focus Range: " << focus_range;
	dof_dialog_->Control<UIStatic>(id_dof_focus_range_static_)->SetText(stream.str());
}

void MotionBlurDoFApp::DoFBlurFactorHandler(KlayGE::UICheckBox const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->ShowBlurFactor(sender.GetChecked());
}

void MotionBlurDoFApp::MBOnHandler(KlayGE::UICheckBox const & sender)
{
	mb_on_ = sender.GetChecked();

	mb_dialog_->Control<UIComboBox>(id_motion_blur_type_)->SetEnabled(mb_on_);
}

void MotionBlurDoFApp::MBExposureChangedHandler(KlayGE::UISlider const & sender)
{
	exposure_ = sender.GetValue() / 10.0f;
	motion_blur_->SetParam(0, exposure_);

	std::wostringstream stream;
	stream << "Exposure: " << exposure_;
	mb_dialog_->Control<UIStatic>(id_mb_exposure_static_)->SetText(stream.str());

	for (size_t i = 0; i < teapots_.size(); ++i)
	{
		static_pointer_cast<Teapot>(teapots_[i])->RootNode()->FirstComponentOfType<RenderableComponent>()
			->BoundRenderableOfType<MotionBlurRenderMesh>().Exposure(exposure_);
	}
}

void MotionBlurDoFApp::MBBlurRadiusChangedHandler(KlayGE::UISlider const & sender)
{
	blur_radius_ = sender.GetValue();
	motion_blur_->SetParam(1, blur_radius_);

	std::wostringstream stream;
	stream << "Blur Radius: " << blur_radius_;
	mb_dialog_->Control<UIStatic>(id_mb_blur_radius_static_)->SetText(stream.str());

	for (size_t i = 0; i < teapots_.size(); ++i)
	{
		static_pointer_cast<Teapot>(teapots_[i])->RootNode()->FirstComponentOfType<RenderableComponent>()
			->BoundRenderableOfType<MotionBlurRenderMesh>().BlurRadius(blur_radius_);
	}
}

void MotionBlurDoFApp::MBReconstructionSamplesChangedHandler(KlayGE::UISlider const & sender)
{
	uint32_t reconstruction_samples = sender.GetValue();
	motion_blur_->SetParam(2, reconstruction_samples);

	std::wostringstream stream;
	stream << "Samples: " << reconstruction_samples;
	mb_dialog_->Control<UIStatic>(id_mb_reconstruction_samples_static_)->SetText(stream.str());
}

void MotionBlurDoFApp::MotionBlurChangedHandler(KlayGE::UIComboBox const & sender)
{
	motion_blur_->SetParam(3, static_cast<uint32_t>(sender.GetSelectedIndex()));
}

void MotionBlurDoFApp::UseInstancingHandler(UICheckBox const & /*sender*/)
{
	use_instance_ = app_dialog_->Control<UICheckBox>(id_use_instancing_)->GetChecked();

	if (use_instance_)
	{
		for (size_t i = 0; i < teapots_.size(); ++i)
		{
			auto& node = *static_pointer_cast<Teapot>(teapots_[i])->RootNode();
			node.ClearComponents();
			node.AddComponent(MakeSharedPtr<RenderableComponent>(renderInstance_));
		}
	}
	else
	{
		for (size_t i = 0; i < teapots_.size(); ++i)
		{
			auto& node = *static_pointer_cast<Teapot>(teapots_[i])->RootNode();
			node.ClearComponents();
			node.AddComponent(MakeSharedPtr<RenderableComponent>(renderMesh_));
		}
	}
}

void MotionBlurDoFApp::CtrlCameraHandler(KlayGE::UICheckBox const & sender)
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

void MotionBlurDoFApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Motion Blur and Depth of field", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << num_objs_rendered_ << " Scene objects "
		<< num_renderables_rendered_ << " Renderables "
		<< num_primitives_rendered_ << " Primitives "
		<< num_vertices_rendered_ << " Vertices";
	font_->RenderText(0, 54, Color(1, 1, 1, 1), stream.str(), 16);

	if (use_instance_)
	{
		font_->RenderText(0, 72, Color(1, 1, 1, 1), L"Instancing is enabled", 16);
	}
	else
	{
		font_->RenderText(0, 72, Color(1, 1, 1, 1), L"Instancing is disabled", 16);
	}
}

uint32_t MotionBlurDoFApp::DoUpdate(uint32_t pass)
{
	Context& context = Context::Instance();
	App3DFramework& app = context.AppInstance();
	SceneManager& scene_mgr = context.SceneManagerInstance();
	RenderEngine& re = context.RenderFactoryInstance().RenderEngineInstance();

	if (0 == pass)
	{
		if (loading_percentage_ < 100)
		{
			if (loading_percentage_ < 80 - NUM_LINE)
			{
				if (model_instance_->HWResourceReady())
				{
					renderInstance_ = model_instance_->Mesh(0);
					loading_percentage_ = 80 - NUM_LINE;
				}
			}
			else if (loading_percentage_ < 80)
			{
				int32_t i = loading_percentage_ - (80 - NUM_LINE);
				for (int32_t j = 0; j < NUM_INSTANCE / NUM_LINE; ++ j)
				{
					float3 pos(0, 0, 0);
					Color clr(0, 0, 0, 1);
					try
					{
						std::vector<std::any> scr_pos = std::any_cast<std::vector<std::any>>(script_module_->Call("get_pos",
							MakeSpan<std::any>({i, j, NUM_INSTANCE, NUM_LINE})));

						pos.x() = std::any_cast<float>(scr_pos[0]);
						pos.y() = std::any_cast<float>(scr_pos[1]);
						pos.z() = std::any_cast<float>(scr_pos[2]);

						std::vector<std::any> scr_clr = std::any_cast<std::vector<std::any>>(script_module_->Call("get_clr",
							MakeSpan<std::any>({i, j, NUM_INSTANCE, NUM_LINE})));

						clr.r() = std::any_cast<float>(scr_clr[0]);
						clr.g() = std::any_cast<float>(scr_clr[1]);
						clr.b() = std::any_cast<float>(scr_clr[2]);
						clr.a() = std::any_cast<float>(scr_clr[3]);
					}
					catch (...)
					{
						LogWarn() << "Wrong callings to script engine" << std::endl;
					}

					auto teapot = MakeSharedPtr<Teapot>();
					teapot->Instance(MathLib::translation(pos), clr);

					teapot->RootNode()->ClearComponents();
					teapot->RootNode()->AddComponent(MakeSharedPtr<RenderableComponent>(renderInstance_));
					checked_cast<MotionBlurRenderMesh&>(*renderInstance_).Exposure(exposure_);
					checked_cast<MotionBlurRenderMesh&>(*renderInstance_).BlurRadius(blur_radius_);
					teapots_.push_back(teapot);

					teapot->RootNode()->SubThreadUpdate(0, 0);
					teapot->RootNode()->MainThreadUpdate(0, 0);

					std::lock_guard<std::mutex> lock(scene_mgr.MutexForUpdate());
					scene_mgr.SceneRootNode().AddChild(teapot->RootNode());
				}

				++ loading_percentage_;
			}
			else
			{
				if (model_mesh_)
				{
					renderMesh_ = model_mesh_->Mesh(0);
					loading_percentage_ = 100;
				}
			}
		}
	}

	switch (pass)
	{
	case 0:
		{
			Camera& camera = this->ActiveCamera();

			camera.MainThreadUpdate(app.AppTime(), app.FrameTime());

			if (depth_texture_support_)
			{
				depth_to_linear_pp_->SetParam(0, camera.NearQFarParam());
			}
		}

		re.BindFrameBuffer(clr_depth_fb_);
		{
			Color clear_clr(0.2f, 0.4f, 0.6f, 1);
			if (Context::Instance().Config().graphics_cfg.gamma)
			{
				clear_clr.r() = 0.029f;
				clear_clr.g() = 0.133f;
				clear_clr.b() = 0.325f;
			}
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);
		}
		for (size_t i = 0; i < teapots_.size(); ++i)
		{
			static_pointer_cast<Teapot>(teapots_[i])->RootNode()->FirstComponentOfType<RenderableComponent>()
				->BoundRenderableOfType<MotionBlurRenderMesh>().VelocityPass(false);
		}
		return App3DFramework::URV_NeedFlush;

	case 1:
		if (depth_texture_support_)
		{
			depth_to_linear_pp_->Apply();
		}

		re.BindFrameBuffer(velocity_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.5f, 0.5f, 0.5f, 1), 1.0f, 0);
		for (size_t i = 0; i < teapots_.size(); ++i)
		{
			static_pointer_cast<Teapot>(teapots_[i])->RootNode()->FirstComponentOfType<RenderableComponent>()
				->BoundRenderableOfType<MotionBlurRenderMesh>().VelocityPass(true);
		}
		return App3DFramework::URV_NeedFlush;

	default:
		num_objs_rendered_ = scene_mgr.NumObjectsRendered();
		num_renderables_rendered_ = scene_mgr.NumRenderablesRendered();
		num_primitives_rendered_ = scene_mgr.NumPrimitivesRendered();
		num_vertices_rendered_ = scene_mgr.NumVerticesRendered();

		color_tex_->BuildMipSubLevels();
		depth_tex_->BuildMipSubLevels();

		if (dof_on_)
		{
			depth_of_field_->Apply();
			if (bokeh_on_ && bokeh_filter_)
			{
				bokeh_filter_->Apply();
			}
		}
		else
		{
			depth_of_field_copy_pp_->Apply();
		}
		if (mb_on_)
		{
			motion_blur_->Apply();
		}
		else
		{
			motion_blur_copy_pp_->Apply();
		}

		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->AttachedDsv()->ClearDepth(1.0f);
		return App3DFramework::URV_Finished;
	}
}
