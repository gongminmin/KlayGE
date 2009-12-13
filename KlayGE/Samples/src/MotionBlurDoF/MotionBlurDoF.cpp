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
#include <KlayGE/KMesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/SATPostProcess.hpp>
#include <KlayGE/Script.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

#include "MotionBlurDoF.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int32_t const NUM_LINE = 10;
	int32_t const NUM_INSTANCE = 400;

	class Teapot : public SceneObjectHelper
	{
	private:
		struct InstData
		{
			float4 mat[3];
			float4 last_mat[3];
			Color clr;
		};

	public:
		Teapot()
			: SceneObjectHelper(SOA_Cullable)
		{
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 1, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 2, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 3, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 4, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 5, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_TextureCoord, 6, EF_ABGR32F));
			instance_format_.push_back(vertex_element(VEU_Diffuse, 0, EF_ABGR32F));
		}

		void Instance(float4x4 const & mat, Color const & clr)
		{
			mat_ = mat;
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

		float4x4 const & GetModelMatrix() const
		{
			return mat_;
		}

		void Update()
		{
			last_mat_ = mat_;
			mat_ *= MathLib::rotation_y(0.001f);

			float4x4 matT = MathLib::transpose(mat_);
			inst_.mat[0] = matT.Row(0);
			inst_.mat[1] = matT.Row(1);
			inst_.mat[2] = matT.Row(2);
			matT = MathLib::transpose(last_mat_);
			inst_.last_mat[0] = matT.Row(0);
			inst_.last_mat[1] = matT.Row(1);
			inst_.last_mat[2] = matT.Row(2);
		}

	private:
		InstData inst_;
		float4x4 mat_;
		float4x4 last_mat_;
	};

	class RenderInstance : public KMesh
	{
	public:
		RenderInstance(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Instance")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("MotionBlurDoF.fxml")->TechniqueByName("ColorDepth");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & last_view = app.ActiveCamera().LastViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("view_proj")) = view * proj;
			*(technique_->Effect().ParameterByName("last_view_proj")) = last_view * proj;
			*(technique_->Effect().ParameterByName("light_in_world")) = float3(2, 2, -3);

			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / app.ActiveCamera().FarPlane();
		}
	};

	class RenderNormalMesh : public KMesh
	{
	private:
		struct InstData
		{
			float4 mat[3];
			float4 last_mat[3];
			Color clr;
		};

	public:
		RenderNormalMesh(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"NormalMesh")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("MotionBlurDoF.fxml")->TechniqueByName("NormalMesh");
		}

		void BuildMeshInfo()
		{
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & last_view = app.ActiveCamera().LastViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("view_proj")) = view * proj;
			*(technique_->Effect().ParameterByName("last_view_proj")) = last_view * proj;
			*(technique_->Effect().ParameterByName("light_in_world")) = float3(2, 2, -3);

			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / app.ActiveCamera().FarPlane();
		}

		void OnInstanceBegin(uint32_t id)
		{
			InstData const * data = static_cast<InstData const *>(instances_[id].lock()->InstanceData());

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

			*(technique_->Effect().ParameterByName("modelmat")) = model;
			*(technique_->Effect().ParameterByName("last_modelmat")) = last_model;
			*(technique_->Effect().ParameterByName("color")) = float4(data->clr.r(), data->clr.g(), data->clr.b(), data->clr.a());
		}

	private:
		void UpdateInstanceStream()
		{
		}
	};

	class ClearFloatPostProcess : public PostProcess
	{
	public:
		ClearFloatPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("MotionBlurDoF.fxml")->TechniqueByName("ClearFloat"))
		{
		}

		void ClearColor(float4 const & clr)
		{
			*(technique_->Effect().ParameterByName("clear_clr")) = clr;
		}

		void OnRenderBegin()
		{
		}
	};

	class DepthOfField : public PostProcess
	{
	public:
		DepthOfField()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DepthOfFieldPP.fxml")->TechniqueByName("DepthOfField")),
				dof_on_(true), show_blur_factor_(false)
		{
		}

		void DoFOn(bool on)
		{
			dof_on_ = on;
			if (dof_on_)
			{
				if (show_blur_factor_)
				{
					technique_ = technique_->Effect().TechniqueByName("DepthOfFieldBlurFactor");
				}
				else
				{
					technique_ = technique_->Effect().TechniqueByName("DepthOfField");
				}
			}
			else
			{
				technique_ = technique_->Effect().TechniqueByName("DepthOfFieldPassThrough");
			}
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

		void ShowBlurFactor(bool show)
		{
			show_blur_factor_ = show;
			if (show_blur_factor_)
			{
				technique_ = technique_->Effect().TechniqueByName("DepthOfFieldBlurFactor");
			}
			else
			{
				if (dof_on_)
				{
					technique_ = technique_->Effect().TechniqueByName("DepthOfField");
				}
				else
				{
					technique_ = technique_->Effect().TechniqueByName("DepthOfFieldPassThrough");
				}
			}
		}
		bool ShowBlurFactor() const
		{
			return show_blur_factor_;
		}

		void Source(TexturePtr const & tex, bool flipping)
		{
			PostProcess::Source(tex, flipping);

			uint32_t const width = tex->Width(0);
			uint32_t const height = tex->Height(0);

			sat_.Source(tex, flipping);

			*(technique_->Effect().ParameterByName("sat_size")) = float4(static_cast<float>(width),
				static_cast<float>(height), 1.0f / width, 1.0f / height);
		}

		void Apply()
		{
			if (!show_blur_factor_ && dof_on_)
			{
				sat_.Apply();
				*(technique_->Effect().ParameterByName("sat_tex")) = sat_.SATTexture();
			}

			PostProcess::Apply();
		}

		void OnRenderBegin()
		{
			PostProcess::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			float const depth_range = app.ActiveCamera().FarPlane();

			*(technique_->Effect().ParameterByName("focus_plane_inv_range")) = float2(focus_plane_ / depth_range, depth_range / focus_range_);
		}

	private:
		bool dof_on_;

		SummedAreaTablePostProcess sat_;

		float focus_plane_;
		float focus_range_;
		bool show_blur_factor_;
	};

	class MotionBlur : public PostProcess
	{
	public:
		MotionBlur()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("MotionBlurPP.fxml")->TechniqueByName("MotionBlur")),
				mb_on_(true), show_motion_vec_(false)
		{
		}

		void MBOn(bool on)
		{
			mb_on_ = on;
			if (mb_on_)
			{
				if (show_motion_vec_)
				{
					technique_ = technique_->Effect().TechniqueByName("MotionBlurMotionVec");
				}
				else
				{
					technique_ = technique_->Effect().TechniqueByName("MotionBlur");
				}
			}
			else
			{
				technique_ = technique_->Effect().TechniqueByName("MotionBlurPassThrough");
			}
		}

		void ShowMotionVector(bool show)
		{
			show_motion_vec_ = show;
			if (show_motion_vec_)
			{
				technique_ = technique_->Effect().TechniqueByName("MotionBlurMotionVec");
			}
			else
			{
				if (mb_on_)
				{
					technique_ = technique_->Effect().TechniqueByName("MotionBlur");
				}
				else
				{
					technique_ = technique_->Effect().TechniqueByName("MotionBlurPassThrough");
				}
			}
		}
		bool ShowMotionVector() const
		{
			return show_motion_vec_;
		}

		void MotionVecTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("motion_vec_tex")) = tex;
		}

	private:
		bool mb_on_;
		bool show_motion_vec_;
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

	bool ConfirmDevice()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0, 0);
			rf.MakeDepthStencilRenderView(800, 600, EF_D16, 1, 0);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}

int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/MotionBlurDoF");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	MotionBlurDoFApp app("Motion Blur and Depth of field", settings);
	app.Create();
	app.Run();

	return 0;
}

MotionBlurDoFApp::MotionBlurDoFApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings),
						num_objs_rendered_(0), num_renderable_rendered_(0), num_primitives_rendered_(0), num_vertices_rendered_(0)
{
}

void MotionBlurDoFApp::InitObjects()
{
	boost::function<RenderModelPtr()> model_instance_ml = LoadModel("teapot.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderInstance>());
	boost::function<RenderModelPtr()> model_mesh_ml = LoadModel("teapot.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderNormalMesh>());
	
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	ScriptEngine scriptEng;
	ScriptModule module("MotionBlurDoF_init");

	this->LookAt(float3(-1.8f, 1.9f, -1.8f), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	clr_depth_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	clr_depth_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;
	mbed_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	mbed_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&MotionBlurDoFApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	depth_of_field_ = MakeSharedPtr<DepthOfField>();
	depth_of_field_->Destinate(FrameBufferPtr());

	motion_blur_ = MakeSharedPtr<MotionBlur>();

	clear_float_ = MakeSharedPtr<ClearFloatPostProcess>();
	checked_pointer_cast<ClearFloatPostProcess>(clear_float_)->ClearColor(float4(0.2f, 0.4f, 0.6f, 1));

	UIManager::Instance().Load(ResLoader::Instance().Load("MotionBlurDoF.uiml"));
	dof_dialog_ = UIManager::Instance().GetDialogs()[0];
	mb_dialog_ = UIManager::Instance().GetDialogs()[1];
	app_dialog_ = UIManager::Instance().GetDialogs()[2];

	id_dof_on_ = dof_dialog_->IDFromName("DoFOn");
	id_focus_plane_static_ = dof_dialog_->IDFromName("FocusPlaneStatic");
	id_focus_plane_slider_ = dof_dialog_->IDFromName("FocusPlaneSlider");
	id_focus_range_static_ = dof_dialog_->IDFromName("FocusRangeStatic");
	id_focus_range_slider_ = dof_dialog_->IDFromName("FocusRangeSlider");
	id_blur_factor_ = dof_dialog_->IDFromName("BlurFactor");
	id_mb_on_ = mb_dialog_->IDFromName("MBOn");
	id_motion_vec_ = mb_dialog_->IDFromName("MotionVec");
	id_use_instancing_ = app_dialog_->IDFromName("UseInstancing");
	id_ctrl_camera_ = app_dialog_->IDFromName("CtrlCamera");

	dof_dialog_->Control<UICheckBox>(id_dof_on_)->OnChangedEvent().connect(boost::bind(&MotionBlurDoFApp::DoFOnHandler, this, _1));
	dof_dialog_->Control<UISlider>(id_focus_plane_slider_)->OnValueChangedEvent().connect(boost::bind(&MotionBlurDoFApp::FocusPlaneChangedHandler, this, _1));
	dof_dialog_->Control<UISlider>(id_focus_range_slider_)->OnValueChangedEvent().connect(boost::bind(&MotionBlurDoFApp::FocusRangeChangedHandler, this, _1));
	dof_dialog_->Control<UICheckBox>(id_blur_factor_)->OnChangedEvent().connect(boost::bind(&MotionBlurDoFApp::BlurFactorHandler, this, _1));

	mb_dialog_->Control<UICheckBox>(id_mb_on_)->OnChangedEvent().connect(boost::bind(&MotionBlurDoFApp::MBOnHandler, this, _1));
	mb_dialog_->Control<UICheckBox>(id_motion_vec_)->OnChangedEvent().connect(boost::bind(&MotionBlurDoFApp::MotionVecHandler, this, _1));

	app_dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&MotionBlurDoFApp::CtrlCameraHandler, this, _1));

	this->DoFOnHandler(*dof_dialog_->Control<UICheckBox>(id_dof_on_));
	this->FocusPlaneChangedHandler(*dof_dialog_->Control<UISlider>(id_focus_plane_slider_));
	this->FocusRangeChangedHandler(*dof_dialog_->Control<UISlider>(id_focus_range_slider_));
	this->BlurFactorHandler(*dof_dialog_->Control<UICheckBox>(id_blur_factor_));

	app_dialog_->Control<UICheckBox>(id_use_instancing_)->OnChangedEvent().connect(boost::bind(&MotionBlurDoFApp::UseInstancingHandler, this, _1));

	renderInstance_ = model_instance_ml()->Mesh(0);
	for (int32_t i = 0; i < NUM_LINE; ++ i)
	{
		for (int32_t j = 0; j < NUM_INSTANCE / NUM_LINE; ++ j)
		{
			PyObjectPtr py_pos = module.Call("get_pos", boost::make_tuple(i, j, NUM_INSTANCE, NUM_LINE));

			float3 pos;
			pos.x() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_pos.get(), 0)));
			pos.y() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_pos.get(), 1)));
			pos.z() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_pos.get(), 2)));

			PyObjectPtr py_clr = module.Call("get_clr", boost::make_tuple(i, j, NUM_INSTANCE, NUM_LINE));

			Color clr;
			clr.r() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_clr.get(), 0)));
			clr.g() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_clr.get(), 1)));
			clr.b() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_clr.get(), 2)));
			clr.a() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_clr.get(), 3)));

			SceneObjectPtr so = MakeSharedPtr<Teapot>();
			checked_pointer_cast<Teapot>(so)->Instance(MathLib::translation(pos), clr);

			checked_pointer_cast<Teapot>(so)->SetRenderable(renderInstance_);
			so->AddToSceneManager();
			scene_objs_.push_back(so);
		}
	}
	use_instance_ = true;

	renderMesh_ = model_mesh_ml()->Mesh(0);
}

void MotionBlurDoFApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	clr_depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	motion_vec_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	clr_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*clr_depth_tex_, 0, 0));
	clr_depth_buffer_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*motion_vec_tex_, 0, 0));
	clr_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0));

	mbed_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	mbed_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*mbed_tex_, 0, 0));

	motion_blur_->Source(clr_depth_tex_, clr_depth_buffer_->RequiresFlipping());
	checked_pointer_cast<MotionBlur>(motion_blur_)->MotionVecTex(motion_vec_tex_);
	motion_blur_->Destinate(mbed_buffer_);

	depth_of_field_->Source(mbed_tex_, mbed_buffer_->RequiresFlipping());
	depth_of_field_->Destinate(FrameBufferPtr());

	clear_float_->Destinate(clr_depth_buffer_);

	UIManager::Instance().SettleCtrls(width, height);
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
	bool dof_on = sender.GetChecked();
	checked_pointer_cast<DepthOfField>(depth_of_field_)->DoFOn(dof_on);

	dof_dialog_->Control<UIStatic>(id_focus_plane_static_)->SetEnabled(dof_on);
	dof_dialog_->Control<UISlider>(id_focus_plane_slider_)->SetEnabled(dof_on);
	dof_dialog_->Control<UIStatic>(id_focus_range_static_)->SetEnabled(dof_on);
	dof_dialog_->Control<UISlider>(id_focus_range_slider_)->SetEnabled(dof_on);
	dof_dialog_->Control<UICheckBox>(id_blur_factor_)->SetEnabled(dof_on);
}

void MotionBlurDoFApp::FocusPlaneChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusPlane(sender.GetValue() / 50.0f);
}

void MotionBlurDoFApp::FocusRangeChangedHandler(KlayGE::UISlider const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->FocusRange(sender.GetValue() / 50.0f);
}

void MotionBlurDoFApp::BlurFactorHandler(KlayGE::UICheckBox const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->ShowBlurFactor(sender.GetChecked());
}

void MotionBlurDoFApp::MBOnHandler(KlayGE::UICheckBox const & sender)
{
	bool mb_on = sender.GetChecked();
	checked_pointer_cast<MotionBlur>(motion_blur_)->MBOn(mb_on);

	mb_dialog_->Control<UICheckBox>(id_motion_vec_)->SetEnabled(mb_on);
}

void MotionBlurDoFApp::MotionVecHandler(KlayGE::UICheckBox const & sender)
{
	checked_pointer_cast<MotionBlur>(motion_blur_)->ShowMotionVector(sender.GetChecked());
}

void MotionBlurDoFApp::UseInstancingHandler(UICheckBox const & /*sender*/)
{
	use_instance_ = app_dialog_->Control<UICheckBox>(id_use_instancing_)->GetChecked();

	if (use_instance_)
	{
		for (int i = 0; i < NUM_INSTANCE; ++ i)
		{
			checked_pointer_cast<Teapot>(scene_objs_[i])->SetRenderable(renderInstance_);
		}
	}
	else
	{
		for (int i = 0; i < NUM_INSTANCE; ++ i)
		{
			checked_pointer_cast<Teapot>(scene_objs_[i])->SetRenderable(renderMesh_);
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

	FrameBuffer& rw = *renderEngine.CurFrameBuffer();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Motion Blur and Depth of field", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw.Description(), 16);

	std::wostringstream stream;
	stream << this->FPS() << " FPS";
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << num_objs_rendered_ << " Scene objects "
		<< num_renderable_rendered_ << " Renderables "
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
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		this->ActiveCamera().Update();

		renderEngine.BindFrameBuffer(clr_depth_buffer_);
		clear_float_->Apply();
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
		return App3DFramework::URV_Need_Flush;

	default:
		num_objs_rendered_ = sceneMgr.NumObjectsRendered();
		num_renderable_rendered_ = sceneMgr.NumRenderablesRendered();
		num_primitives_rendered_ = sceneMgr.NumPrimitivesRendered();
		num_vertices_rendered_ = sceneMgr.NumVerticesRendered();

		motion_blur_->Apply();
		depth_of_field_->Apply();

		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
		return App3DFramework::URV_Finished;
	}
}
