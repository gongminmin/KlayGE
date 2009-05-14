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

#include "DepthOfField.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int32_t const NUM_INSTANCE = 400;

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
			mat_ = mat;
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

		float4x4 const & GetModelMatrix() const
		{
			return mat_;
		}

	private:
		InstData inst_;
		float4x4 mat_;
	};

	class RenderInstance : public KMesh
	{
	public:
		RenderInstance(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"Instance")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("DepthOfField.fxml")->TechniqueByName("ColorDepth");
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

	class RenderNormalMesh : public KMesh
	{
	private:
		struct InstData
		{
			float4 col[3];
			Color clr;
		};

	public:
		RenderNormalMesh(RenderModelPtr model, std::wstring const & /*name*/)
			: KMesh(model, L"NormalMesh")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("DepthOfField.fxml")->TechniqueByName("NormalMesh");
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

		void OnInstanceBegin(uint32_t id)
		{
			InstData const * data = static_cast<InstData const *>(instances_[id].lock()->InstanceData());

			float4x4 model;
			model.Col(0, data->col[0]);
			model.Col(1, data->col[1]);
			model.Col(2, data->col[2]);
			model.Col(3, float4(0, 0, 0, 1));

			*(technique_->Effect().ParameterByName("modelmat")) = model;
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
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DepthOfField.fxml")->TechniqueByName("ClearFloat"))
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
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("DepthOfField.fxml")->TechniqueByName("DepthOfField"))
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

		void ShowBlurFactor(bool show)
		{
			show_blur_factor_ = show;
			*(technique_->Effect().ParameterByName("show_blur_factor")) = show_blur_factor_;
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
			if (!show_blur_factor_)
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
			float const depth_range = app.ActiveCamera().FarPlane() - app.ActiveCamera().NearPlane();

			*(technique_->Effect().ParameterByName("focus_plane")) = (focus_plane_ - app.ActiveCamera().NearPlane()) / depth_range;
			*(technique_->Effect().ParameterByName("inv_focus_range")) = depth_range / focus_range_;
		}

	private:
		SummedAreaTablePostProcess sat_;

		float focus_plane_;
		float focus_range_;
		bool show_blur_factor_;
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
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0);
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
	ResLoader::Instance().AddPath("../Samples/media/DepthOfField");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	DepthOfFieldApp app("Depth of field", settings);
	app.Create();
	app.Run();

	return 0;
}

DepthOfFieldApp::DepthOfFieldApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings),
						num_objs_rendered_(0), num_renderable_rendered_(0), num_primitives_rendered_(0), num_vertices_rendered_(0)
{
}

void DepthOfFieldApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	ScriptEngine scriptEng;
	ScriptModule module("DepthOfField_init");

	renderInstance_ = LoadModel("teapot.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderInstance>())->Mesh(0);
	for (int32_t i = 0; i < 10; ++ i)
	{
		for (int32_t j = 0; j < NUM_INSTANCE / 10; ++ j)
		{
			PyObjectPtr py_pos = module.Call("get_pos", boost::make_tuple(i, j, NUM_INSTANCE));

			float3 pos;
			pos.x() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_pos.get(), 0)));
			pos.y() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_pos.get(), 1)));
			pos.z() = static_cast<float>(PyFloat_AsDouble(PyTuple_GetItem(py_pos.get(), 2)));

			PyObjectPtr py_clr = module.Call("get_clr", boost::make_tuple(i, j, NUM_INSTANCE));

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

	renderMesh_ = LoadModel("teapot.meshml", EAH_GPU_Read, CreateKModelFactory<RenderModel>(), CreateKMeshFactory<RenderNormalMesh>())->Mesh(0);

	this->LookAt(float3(-1.8f, 1.9f, -1.8f), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	clr_depth_buffer_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	clr_depth_buffer_->GetViewport().camera = renderEngine.CurFrameBuffer()->GetViewport().camera;

	fpcController_.Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&DepthOfFieldApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	depth_of_field_ = MakeSharedPtr<DepthOfField>();
	depth_of_field_->Destinate(FrameBufferPtr());

	clear_float_ = MakeSharedPtr<ClearFloatPostProcess>();
	checked_pointer_cast<ClearFloatPostProcess>(clear_float_)->ClearColor(float4(0.2f - 0.5f, 0.4f - 0.5f, 0.6f - 0.5f, 1 - 0.5f));

	UIManager::Instance().Load(ResLoader::Instance().Load("DepthOfField.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_focus_plane_static_ = dialog_->IDFromName("FocusPlaneStatic");
	id_focus_plane_slider_ = dialog_->IDFromName("FocusPlaneSlider");
	id_focus_range_static_ = dialog_->IDFromName("FocusRangeStatic");
	id_focus_range_slider_ = dialog_->IDFromName("FocusRangeSlider");
	id_blur_factor_ = dialog_->IDFromName("BlurFactor");
	id_use_instancing_ = dialog_->IDFromName("UseInstancing");
	id_ctrl_camera_ = dialog_->IDFromName("CtrlCamera");

	dialog_->Control<UISlider>(id_focus_plane_slider_)->OnValueChangedEvent().connect(boost::bind(&DepthOfFieldApp::FocusPlaneChangedHandler, this, _1));
	dialog_->Control<UISlider>(id_focus_range_slider_)->OnValueChangedEvent().connect(boost::bind(&DepthOfFieldApp::FocusRangeChangedHandler, this, _1));

	dialog_->Control<UICheckBox>(id_blur_factor_)->OnChangedEvent().connect(boost::bind(&DepthOfFieldApp::BlurFactorHandler, this, _1));
	dialog_->Control<UICheckBox>(id_ctrl_camera_)->OnChangedEvent().connect(boost::bind(&DepthOfFieldApp::CtrlCameraHandler, this, _1));

	this->FocusPlaneChangedHandler(*dialog_->Control<UISlider>(id_focus_plane_slider_));
	this->FocusRangeChangedHandler(*dialog_->Control<UISlider>(id_focus_range_slider_));
	this->BlurFactorHandler(*dialog_->Control<UICheckBox>(id_blur_factor_));

	dialog_->Control<UICheckBox>(id_use_instancing_)->OnChangedEvent().connect(boost::bind(&DepthOfFieldApp::UseInstancingHandler, this, _1));
}

void DepthOfFieldApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	clr_depth_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	clr_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*clr_depth_tex_, 0));
	clr_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0));

	depth_of_field_->Source(clr_depth_tex_, clr_depth_buffer_->RequiresFlipping());
	depth_of_field_->Destinate(FrameBufferPtr());

	clear_float_->Destinate(clr_depth_buffer_);

	UIManager::Instance().SettleCtrls(width, height);
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

void DepthOfFieldApp::BlurFactorHandler(KlayGE::UICheckBox const & sender)
{
	checked_pointer_cast<DepthOfField>(depth_of_field_)->ShowBlurFactor(sender.GetChecked());
}

void DepthOfFieldApp::UseInstancingHandler(UICheckBox const & /*sender*/)
{
	use_instance_ = dialog_->Control<UICheckBox>(id_use_instancing_)->GetChecked();

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

void DepthOfFieldApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	FrameBuffer& rw = *renderEngine.CurFrameBuffer();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Depth of field", 16);
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

uint32_t DepthOfFieldApp::DoUpdate(uint32_t pass)
{
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	switch (pass)
	{
	case 0:
		renderEngine.BindFrameBuffer(clr_depth_buffer_);
		clear_float_->Apply();
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
		return App3DFramework::URV_Need_Flush;

	default:
		num_objs_rendered_ = sceneMgr.NumObjectsRendered();
		num_renderable_rendered_ = sceneMgr.NumRenderablesRendered();
		num_primitives_rendered_ = sceneMgr.NumPrimitivesRendered();
		num_vertices_rendered_ = sceneMgr.NumVerticesRendered();

		depth_of_field_->Apply();

		renderEngine.BindFrameBuffer(FrameBufferPtr());
		renderEngine.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->Clear(1.0f);
		return App3DFramework::URV_Finished;
	}
}
