#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>

#include "SampleCommon.hpp"
#include "ModelViewer.hpp"

using namespace KlayGE;
using namespace std;

namespace
{
	class RenderAxis : public RenderableHelper
	{
	public:
		RenderAxis()
			: RenderableHelper(L"Axis")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = SyncLoadRenderEffect("MVUtil.fxml");
			simple_forward_tech_ = effect->TechniqueByName("AxisTech");
			mvp_param_ = effect->ParameterByName("mvp");

			float4 xyzs[] =
			{
				float4(0, 0, 0, 0),
				float4(1, 0, 0, 0),
				float4(0, 0, 0, 1),
				float4(0, 1, 0, 1),
				float4(0, 0, 0, 2),
				float4(0, 0, 1, 2),
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_LineList);

			ElementInitData init_data;
			init_data.row_pitch = sizeof(xyzs);
			init_data.slice_pitch = 0;
			init_data.data = xyzs;
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

			rl_->BindVertexStream(pos_vb, KlayGE::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));

			pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			effect_attrs_ |= EA_SimpleForward;
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*mvp_param_ = model_mat_ * camera.ViewProjMatrix();
		}
	};

	class RenderGrid : public RenderableHelper
	{
	public:
		RenderGrid()
			: RenderableHelper(L"Grid")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = SyncLoadRenderEffect("MVUtil.fxml");
			simple_forward_tech_ = effect->TechniqueByName("GridTech");
			mvp_param_ = effect->ParameterByName("mvp");

			float3 xyzs[(21 + 21) * 2];
			for (int i = 0; i < 21; ++ i)
			{
				xyzs[i * 2 + 0] = float3(-10.0f + i, 0, -10);
				xyzs[i * 2 + 1] = float3(-10.0f + i, 0, +10);

				xyzs[(i + 21) * 2 + 0] = float3(-10, 0, -10.0f + i);
				xyzs[(i + 21) * 2 + 1] = float3(+10, 0, -10.0f + i);
			}

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_LineList);

			ElementInitData init_data;
			init_data.row_pitch = sizeof(xyzs);
			init_data.slice_pitch = 0;
			init_data.data = xyzs;
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

			rl_->BindVertexStream(pos_vb, KlayGE::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			effect_attrs_ |= EA_SimpleForward;
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*mvp_param_ = model_mat_ * camera.ViewProjMatrix();
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

	class ModelObject : public SceneObjectHelper
	{
	public:
		explicit ModelObject(std::string const & name)
			: SceneObjectHelper(0)
		{
			renderable_ = SyncLoadModel(name, EAH_GPU_Read | EAH_Immutable,
				CreateModelFactory<DetailedSkinnedModel>(), CreateMeshFactory<DetailedSkinnedMesh>());
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SetTime(0);
		}

		uint32_t NumFrames() const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->NumFrames();
		}

		uint32_t FrameRate() const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->FrameRate();
		}

		StaticMeshPtr const & Mesh(size_t id) const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->Mesh(id);
		}

		uint32_t NumMeshes() const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->NumMeshes();
		}

		void RebindJoints()
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->RebindJoints();
		}

		void UnbindJoints()
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->UnbindJoints();
		}

		RenderMaterialPtr const & GetMaterial(int32_t i) const
		{
			return checked_pointer_cast<DetailedSkinnedModel>(renderable_)->GetMaterial(i);
		}

		void SetTime(float time)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SetTime(time);
		}

		void SetFrame(float frame)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SetFrame(frame);
		}

		void VisualizeLighting()
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->VisualizeLighting();
		}

		void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->VisualizeVertex(usage, usage_index);
		}

		void VisualizeTexture(int slot)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->VisualizeTexture(slot);
		}

		void LineMode(bool line_mode)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->LineMode(line_mode);
		}

		void SmoothMesh(bool smooth)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SmoothMesh(smooth);
		}

		void SetTessFactor(KlayGE::int32_t tess_factor)
		{
			checked_pointer_cast<DetailedSkinnedModel>(renderable_)->SetTessFactor(tess_factor);
		}
	};

	class PointLightSourceUpdate
	{
	public:
		PointLightSourceUpdate()
		{
		}

		void operator()(LightSource& light, float /*app_time*/, float /*elapsed_time*/)
		{
			float4x4 inv_view = Context::Instance().AppInstance().ActiveCamera().InverseViewMatrix();
			light.Position(MathLib::transform_coord(float3(0, 2.0f, 0), inv_view));
		}
	};
}

int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	ModelViewerApp app;
	app.Create();
	app.Run();

	return 0;
}

ModelViewerApp::ModelViewerApp()
					: App3DFramework("Model Viewer"),
						last_time_(0), frame_(0),
						skinned_(true), play_(false)
{
	ResLoader::Instance().AddPath("../../Samples/media/ModelViewer");
}

bool ModelViewerApp::ConfirmDevice() const
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void ModelViewerApp::InitObjects()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(0, false);

	point_light_ = MakeSharedPtr<PointLightSource>();
	point_light_->Attrib(LightSource::LSA_NoShadow);
	point_light_->Color(float3(1.0f, 1.0f, 1.0f));
	point_light_->Position(float3(0, 2.0f, 0));
	point_light_->Falloff(float3(1, 0, 0));
	point_light_->BindUpdateFunc(PointLightSourceUpdate());
	point_light_->AddToSceneManager();
	
	axis_ = MakeSharedPtr<SceneObjectHelper>(MakeSharedPtr<RenderAxis>(), 0);
	axis_->AddToSceneManager();

	grid_ = MakeSharedPtr<SceneObjectHelper>(MakeSharedPtr<RenderGrid>(), 0);
	grid_->AddToSceneManager();

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	uint32_t texel;
	ElementFormat fmt;
	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
	{
		fmt = EF_ABGR8;
		texel = clear_clr.ABGR();
	}
	else
	{
		BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8));

		fmt = EF_ARGB8;
		texel = clear_clr.ARGB();
	}
	ElementInitData init_data[6];
	for (int i = 0; i < 6; ++ i)
	{
		init_data[i].data = &texel;
		init_data[i].row_pitch = sizeof(uint32_t);
		init_data[i].slice_pitch = init_data[i].row_pitch;	
	}
	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>();
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CubeMap(rf.MakeTextureCube(1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, init_data));
	sky_box_->AddToSceneManager();

	UIManager::Instance().Load(ResLoader::Instance().Open("ModelViewer.uiml"));
	dialog_animation_ = UIManager::Instance().GetDialog("Animation");
	dialog_model_ = UIManager::Instance().GetDialog("Model");

	id_skinned_ = dialog_animation_->IDFromName("Skinned");
	id_frame_static_ = dialog_animation_->IDFromName("FrameStatic");
	id_frame_slider_ = dialog_animation_->IDFromName("FrameSlider");
	id_play_ = dialog_animation_->IDFromName("Play");
	id_smooth_mesh_ = dialog_animation_->IDFromName("SmoothMesh");
	id_fps_camera_ = dialog_animation_->IDFromName("FPSCamera");
	id_hdr_ = dialog_animation_->IDFromName("HDR");
	id_open_ = dialog_model_->IDFromName("Open");
	id_save_as_ = dialog_model_->IDFromName("SaveAs");
	id_mesh_ = dialog_model_->IDFromName("MeshCombo");
	id_vertex_streams_ = dialog_model_->IDFromName("VertexStreamList");
	id_textures_ = dialog_model_->IDFromName("TextureList");
	id_visualize_ = dialog_model_->IDFromName("VisualizeCombo");
	id_line_mode_ = dialog_model_->IDFromName("LineModeCheck");

	tbController_.AttachCamera(this->ActiveCamera());

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&ModelViewerApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	dialog_animation_->Control<UICheckBox>(id_skinned_)->OnChangedEvent().connect(KlayGE::bind(&ModelViewerApp::SkinnedHandler, this, KlayGE::placeholders::_1));

	dialog_animation_->Control<UISlider>(id_frame_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ModelViewerApp::FrameChangedHandler, this, KlayGE::placeholders::_1));

	dialog_animation_->Control<UICheckBox>(id_play_)->OnChangedEvent().connect(KlayGE::bind(&ModelViewerApp::PlayHandler, this, KlayGE::placeholders::_1));
	dialog_animation_->Control<UICheckBox>(id_smooth_mesh_)->OnChangedEvent().connect(KlayGE::bind(&ModelViewerApp::SmoothMeshHandler, this, KlayGE::placeholders::_1));
	dialog_animation_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(KlayGE::bind(&ModelViewerApp::FPSCameraHandler, this, KlayGE::placeholders::_1));

	dialog_model_->Control<UIButton>(id_open_)->OnClickedEvent().connect(KlayGE::bind(&ModelViewerApp::OpenHandler, this, KlayGE::placeholders::_1));
	dialog_model_->Control<UIButton>(id_save_as_)->OnClickedEvent().connect(KlayGE::bind(&ModelViewerApp::SaveAsHandler, this, KlayGE::placeholders::_1));

	dialog_model_->Control<UIComboBox>(id_mesh_)->OnSelectionChangedEvent().connect(KlayGE::bind(&ModelViewerApp::MeshChangedHandler, this, KlayGE::placeholders::_1));
	dialog_model_->Control<UIComboBox>(id_visualize_)->OnSelectionChangedEvent().connect(KlayGE::bind(&ModelViewerApp::VisualizeChangedHandler, this, KlayGE::placeholders::_1));
	dialog_model_->Control<UICheckBox>(id_line_mode_)->OnChangedEvent().connect(KlayGE::bind(&ModelViewerApp::LineModeChangedHandler, this, KlayGE::placeholders::_1));

	this->OpenModel("archer_attacking.meshml");
}

void ModelViewerApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	deferred_rendering_->SetupViewport(0, re.CurFrameBuffer(), 0);

	UIManager::Instance().SettleCtrls();
}

void ModelViewerApp::OpenModel(std::string const & name)
{
	if (model_)
	{
		model_->DelFromSceneManager();
	}
	model_ = MakeSharedPtr<ModelObject>(name);
	model_->AddToSceneManager();

	KlayGE::shared_ptr<ModelObject> model = checked_pointer_cast<ModelObject>(model_);

	frame_ = 0;
	if (model->NumFrames() > 0)
	{
		dialog_animation_->Control<UICheckBox>(id_skinned_)->SetEnabled(true);
		dialog_animation_->Control<UICheckBox>(id_play_)->SetEnabled(true);
		dialog_animation_->Control<UISlider>(id_frame_slider_)->SetEnabled(true);
		dialog_animation_->Control<UISlider>(id_frame_slider_)->SetRange(0, model->NumFrames() * 10 - 1);
		dialog_animation_->Control<UISlider>(id_frame_slider_)->SetValue(static_cast<int>(frame_ * 10 + 0.5f));
	}
	else
	{
		dialog_animation_->Control<UICheckBox>(id_skinned_)->SetEnabled(false);
		dialog_animation_->Control<UICheckBox>(id_play_)->SetEnabled(false);
		dialog_animation_->Control<UISlider>(id_frame_slider_)->SetEnabled(false);
		dialog_animation_->Control<UISlider>(id_frame_slider_)->SetRange(0, 1);
		dialog_animation_->Control<UISlider>(id_frame_slider_)->SetValue(0);
	}

	dialog_model_->Control<UIComboBox>(id_mesh_)->RemoveAllItems();
	for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
	{
		dialog_model_->Control<UIComboBox>(id_mesh_)->AddItem(model->Mesh(i)->Name());
	}

	AABBox const & bb = model_->PosBound();
	float3 center = bb.Center();
	float3 half_size = bb.HalfSize();
	this->LookAt(center + float3(half_size.x() * 2, half_size.y() * 2.5f, half_size.z() * 3), float3(0, center.y(), 0), float3(0.0f, 1.0f, 0.0f));
	float far_plane = std::max(200.0f, MathLib::length(half_size) * 5);
	this->Proj(0.1f, far_plane);
	deferred_rendering_->LightDistance(far_plane);
	this->FPSCameraHandler(*dialog_animation_->Control<UICheckBox>(id_fps_camera_));

	tbController_.Scalers(0.01f, MathLib::length(half_size) * 0.001f);
	fpsController_.Scalers(0.01f, 0.2f);

	this->MeshChangedHandler(*dialog_model_->Control<UIComboBox>(id_mesh_));
	this->FrameChangedHandler(*dialog_animation_->Control<UISlider>(id_frame_slider_));
	this->LineModeChangedHandler(*dialog_model_->Control<UICheckBox>(id_line_mode_));
	this->SmoothMeshHandler(*dialog_animation_->Control<UICheckBox>(id_smooth_mesh_));
}

void ModelViewerApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		//this->Quit();
		break;
	}
}

void ModelViewerApp::OpenHandler(KlayGE::UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "All Model File (*.meshml, *.model_bin)\0*.meshml;*.model_bin\0MeshML File (*.meshml)\0*.meshml\0model_bin File (*.model_bin)\0*.model_bin\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn))
	{
		HCURSOR cur = GetCursor();
		SetCursor(LoadCursor(nullptr, IDC_WAIT));

		if (last_file_path_.empty())
		{
			ResLoader::Instance().DelPath(last_file_path_);
		}

		std::string file_name = fn;
		last_file_path_ = file_name.substr(0, file_name.find_last_of('\\'));
		ResLoader::Instance().AddPath(last_file_path_);
		
		this->OpenModel(file_name);
		
		SetCursor(cur);
	}
#endif
}

void ModelViewerApp::SaveAsHandler(KlayGE::UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "MeshML File\0*.meshml\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if (GetSaveFileNameA(&ofn))
	{
		HCURSOR cur = GetCursor();
		SetCursor(LoadCursor(nullptr, IDC_WAIT));
		
		SaveModel(checked_pointer_cast<DetailedSkinnedModel>(model_->GetRenderable()), fn);

		SetCursor(cur);
	}
#endif
}

void ModelViewerApp::SkinnedHandler(UICheckBox const & /*sender*/)
{
	skinned_ = dialog_animation_->Control<UICheckBox>(id_skinned_)->GetChecked();
	if (skinned_)
	{
		dialog_animation_->Control<UICheckBox>(id_play_)->SetEnabled(true);
		checked_pointer_cast<ModelObject>(model_)->RebindJoints();
		this->FrameChangedHandler(*dialog_animation_->Control<UISlider>(id_frame_slider_));
	}
	else
	{
		checked_pointer_cast<ModelObject>(model_)->UnbindJoints();
		dialog_animation_->Control<UICheckBox>(id_play_)->SetChecked(false);
		dialog_animation_->Control<UICheckBox>(id_play_)->SetEnabled(false);
	}
}

void ModelViewerApp::FrameChangedHandler(KlayGE::UISlider const & sender)
{
	frame_ = sender.GetValue() * 0.1f;
	if (skinned_)
	{
		checked_pointer_cast<ModelObject>(model_)->SetFrame(frame_);
	}

	std::wostringstream stream;
	stream << frame_ << L":";
	dialog_animation_->Control<UIStatic>(id_frame_static_)->SetText(stream.str());
}

void ModelViewerApp::PlayHandler(KlayGE::UICheckBox const & sender)
{
	play_ = sender.GetChecked();
}

void ModelViewerApp::SmoothMeshHandler(KlayGE::UICheckBox const & sender)
{
	checked_pointer_cast<ModelObject>(model_)->SmoothMesh(sender.GetChecked());
}

void ModelViewerApp::FPSCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		tbController_.DetachCamera();
		fpsController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpsController_.DetachCamera();
		tbController_.AttachCamera(this->ActiveCamera());
	}
}

void ModelViewerApp::MeshChangedHandler(KlayGE::UIComboBox const & sender)
{
	uint32_t mi = sender.GetSelectedIndex();

	dialog_model_->Control<UIComboBox>(id_visualize_)->RemoveAllItems();
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Lighting");

	dialog_model_->Control<UIListBox>(id_vertex_streams_)->RemoveAllItems();
	RenderLayoutPtr const & rl = checked_pointer_cast<ModelObject>(model_)->Mesh(mi)->GetRenderLayout();
	for (uint32_t i = 0; i < rl->NumVertexStreams(); ++ i)
	{
		std::wostringstream oss;
		vertex_elements_type const & ves = rl->VertexStreamFormat(i);
		for (size_t j = 0; j < ves.size(); ++ j)
		{
			if (j != 0)
			{
				oss << L"|";
			}

			std::wostringstream oss_one;

			std::wstring str;
			switch (rl->VertexStreamFormat(i)[j].usage)
			{
			case VEU_Position:
				oss_one << L"Position";
				break;

			case VEU_Normal:
				oss_one << L"Normal";
				break;

			case VEU_Diffuse:
				oss_one << L"Diffuse";
				break;

			case VEU_Specular:
				oss_one << L"Specular";
				break;

			case VEU_BlendWeight:
				oss_one << L"Blend weight";
				break;

			case VEU_BlendIndex:
				oss_one << L"Blend index";
				break;

			case VEU_TextureCoord:
				oss_one << L"Texcoord";
				oss_one << rl->VertexStreamFormat(i)[j].usage_index;
				break;

			case VEU_Tangent:
				oss_one << L"Tangent";
				break;

			case VEU_Binormal:
				oss_one << L"Binormal";
				break;

			default:
				break;
			}

			oss << oss_one.str();
		}

		dialog_model_->Control<UIListBox>(id_vertex_streams_)->AddItem(oss.str());
	}

	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex Position", VEU_Position);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex Normal", VEU_Normal);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex Diffuse", VEU_Diffuse);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex Specular", VEU_Specular);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex Blend weight", VEU_BlendWeight);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex Blend index", VEU_BlendIndex);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex Texcoord", VEU_TextureCoord);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex Tangent", VEU_Tangent);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex Binormal", VEU_Binormal);

	dialog_model_->Control<UIListBox>(id_textures_)->RemoveAllItems();
	KlayGE::shared_ptr<ModelObject> model = checked_pointer_cast<ModelObject>(model_);
	TextureSlotsType const & texture_slots = model->GetMaterial(model->Mesh(mi)->MaterialID())->texture_slots;
	for (size_t i = 0; i < texture_slots.size(); ++ i)
	{
		std::wostringstream oss;
		std::wstring type, name;
		Convert(type, texture_slots[i].first);
		Convert(name, texture_slots[i].second);
		oss << type << L": " << name;
		dialog_model_->Control<UIListBox>(id_textures_)->AddItem(oss.str());
	}

	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Texture Diffuse", 0);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Texture Specular", 1);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Texture Shininess", 2);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Texture Bump", 3);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Texture Emit", 4);
	dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Texture Opacity", 5);
}

void ModelViewerApp::VisualizeChangedHandler(KlayGE::UIComboBox const & sender)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	if (0 == sender.GetSelectedIndex())
	{
		checked_pointer_cast<ModelObject>(model_)->VisualizeLighting();

		deferred_rendering_->SSVOEnabled(0, false);
		re.HDREnabled(true);
		re.PPAAEnabled(1);
		re.ColorGradingEnabled(true);
	}
	else
	{
		boost::any data = sender.GetSelectedData();
		try
		{
			int slot = boost::any_cast<int>(data);
			checked_pointer_cast<ModelObject>(model_)->VisualizeTexture(slot);
		}
		catch (boost::bad_any_cast&)
		{
			VertexElementUsage veu = boost::any_cast<VertexElementUsage>(data);
			checked_pointer_cast<ModelObject>(model_)->VisualizeVertex(veu, 0);
		}

		deferred_rendering_->SSVOEnabled(0, false);
		re.HDREnabled(false);
		re.PPAAEnabled(0);
		re.ColorGradingEnabled(false);
	}
}

void ModelViewerApp::LineModeChangedHandler(KlayGE::UICheckBox const & sender)
{
	checked_pointer_cast<ModelObject>(model_)->LineMode(sender.GetChecked());
}

void ModelViewerApp::DoUpdateOverlay()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), renderEngine.Name(), 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), renderEngine.ScreenFrameBuffer()->Description(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ModelViewerApp::DoUpdate(KlayGE::uint32_t pass)
{
	if (0 == pass)
	{
		Camera const & camera = this->ActiveCamera();

		AABBox bb = MathLib::transform_aabb(model_->PosBound(), camera.ViewMatrix())
			| MathLib::transform_aabb(grid_->PosBound(), camera.ViewMatrix());
		float near_plane = std::max(0.01f, bb.Min().z() * 0.8f);
		float far_plane = std::max(near_plane + 0.1f, bb.Max().z() * 1.2f);
		this->Proj(near_plane, far_plane);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		uint32_t width = re.CurFrameBuffer()->Width();
		uint32_t height = re.CurFrameBuffer()->Height();
		float x = (width - 70.5f) / width * 2 - 1;
		float y = 1 - (height - 70.5f) / height * 2;
		float3 origin = MathLib::transform_coord(float3(x, y, 0.1f), camera.InverseViewProjMatrix());

		float x1 = (width - 20.5f) / width * 2 - 1;
		float3 x_dir = MathLib::transform_coord(float3(x1, y, 0.1f), camera.InverseViewProjMatrix());
		float len = MathLib::length(x_dir - origin);

		float4x4 scaling = MathLib::scaling(len, len, len);
		float4x4 trans = MathLib::translation(origin);
		axis_->ModelMatrix(scaling * trans);

		KlayGE::shared_ptr<ModelObject> model = checked_pointer_cast<ModelObject>(model_);
		if (play_)
		{
			float this_time = this->AppTime();
			if (this_time - last_time_ > 0.02f)
			{
				frame_ += 0.02f * model->FrameRate();
				frame_ = fmod(frame_, static_cast<float>(model->NumFrames()));

				last_time_ = this_time;
			}

			dialog_animation_->Control<UISlider>(id_frame_slider_)->SetValue(static_cast<int>(frame_ * 10 + 0.5f));
		}
	}

	return deferred_rendering_->Update(pass);
}
