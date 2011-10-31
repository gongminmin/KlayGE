#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
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

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <boost/bind.hpp>

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

			RenderEffectPtr effect = rf.LoadEffect("MVUtil.fxml");
			technique_ = effect->TechniqueByName("AxisTech");

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

			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();
			float4x4 vp = view * proj;

			float4x4 scaling = MathLib::scaling(0.006f, 0.006f, 0.006f);
			float4x4 trans = MathLib::translation(MathLib::transform_coord(float3(0.8f, -0.8f, 0.1f), MathLib::inverse(vp)));
			*(technique_->Effect().ParameterByName("worldviewproj")) = scaling * trans * vp;
		}
	};

	class AxisObject : public SceneObjectHelper
	{
	public:
		AxisObject()
			: SceneObjectHelper(MakeSharedPtr<RenderAxis>(), 0)
		{
		}
	};


	class RenderGrid : public RenderableHelper
	{
	public:
		RenderGrid()
			: RenderableHelper(L"Grid")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = rf.LoadEffect("MVUtil.fxml");
			technique_ = effect->TechniqueByName("GridTech");

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

			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();
			float4x4 mvp = view * proj;

			*(technique_->Effect().ParameterByName("worldviewproj")) = mvp;
		}
	};

	class GridObject : public SceneObjectHelper
	{
	public:
		GridObject()
			: SceneObjectHelper(MakeSharedPtr<RenderGrid>(), 0)
		{
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

	struct CreateDetailedModelFactory
	{
		RenderModelPtr operator()(std::wstring const & name)
		{
			return MakeSharedPtr<DetailedSkinnedModel>(name);
		}
	};
}

int main()
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

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

	font_ = rf.MakeFont("gkai00mp.kfont");
	
	axis_ = MakeSharedPtr<AxisObject>();
	axis_->AddToSceneManager();

	grid_ = MakeSharedPtr<GridObject>();
	grid_->AddToSceneManager();

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
	
	this->OpenModel("archer_attacking.meshml");

	tbController_.AttachCamera(this->ActiveCamera());

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&ModelViewerApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	dialog_animation_->Control<UICheckBox>(id_skinned_)->OnChangedEvent().connect(boost::bind(&ModelViewerApp::SkinnedHandler, this, _1));

	dialog_animation_->Control<UISlider>(id_frame_slider_)->OnValueChangedEvent().connect(boost::bind(&ModelViewerApp::FrameChangedHandler, this, _1));
	this->FrameChangedHandler(*dialog_animation_->Control<UISlider>(id_frame_slider_));

	dialog_animation_->Control<UICheckBox>(id_play_)->OnChangedEvent().connect(boost::bind(&ModelViewerApp::PlayHandler, this, _1));
	dialog_animation_->Control<UICheckBox>(id_smooth_mesh_)->OnChangedEvent().connect(boost::bind(&ModelViewerApp::SmoothMeshHandler, this, _1));
	dialog_animation_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(boost::bind(&ModelViewerApp::FPSCameraHandler, this, _1));

	dialog_model_->Control<UIButton>(id_open_)->OnClickedEvent().connect(boost::bind(&ModelViewerApp::OpenHandler, this, _1));
	dialog_model_->Control<UIButton>(id_save_as_)->OnClickedEvent().connect(boost::bind(&ModelViewerApp::SaveAsHandler, this, _1));

	dialog_model_->Control<UIComboBox>(id_mesh_)->OnSelectionChangedEvent().connect(boost::bind(&ModelViewerApp::MeshChangedHandler, this, _1));
	dialog_model_->Control<UIComboBox>(id_visualize_)->OnSelectionChangedEvent().connect(boost::bind(&ModelViewerApp::VisualizeChangedHandler, this, _1));
	dialog_model_->Control<UICheckBox>(id_line_mode_)->OnChangedEvent().connect(boost::bind(&ModelViewerApp::LineModeChangedHandler, this, _1));
}

void ModelViewerApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void ModelViewerApp::OpenModel(std::string const & name)
{
	model_ = checked_pointer_cast<DetailedSkinnedModel>(SyncLoadModel(name, EAH_GPU_Read | EAH_Immutable, CreateDetailedModelFactory(), CreateMeshFactory<DetailedSkinnedMesh>()));
	model_->SetTime(0);

	frame_ = 0;
	dialog_animation_->Control<UISlider>(id_frame_slider_)->SetRange(model_->StartFrame() * 10, model_->EndFrame() * 10 - 1);
	dialog_animation_->Control<UISlider>(id_frame_slider_)->SetValue(static_cast<int>(frame_ * 10 + 0.5f));

	dialog_model_->Control<UIComboBox>(id_mesh_)->RemoveAllItems();
	for (uint32_t i = 0; i < model_->NumMeshes(); ++ i)
	{
		dialog_model_->Control<UIComboBox>(id_mesh_)->AddItem(model_->Mesh(i)->Name());
	}

	Box const & bb = model_->GetBound();
	float3 center = bb.Center();
	float3 half_size = bb.HalfSize();
	this->LookAt(center + float3(half_size.x() * 2, half_size.y() * 2.5f, half_size.z() * 3), float3(0, half_size.y() * 0.5f, 0), float3(0.0f, 1.0f, 0.0f));
	this->Proj(0.1f, std::max(200.0f, MathLib::length(half_size) * 5));
	this->FPSCameraHandler(*dialog_animation_->Control<UICheckBox>(id_fps_camera_));

	tbController_.Scalers(0.01f, MathLib::length(half_size) * 0.001f);
	fpsController_.Scalers(0.01f, MathLib::length(half_size) * 0.001f);

	this->MeshChangedHandler(*dialog_model_->Control<UIComboBox>(id_mesh_));
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
#if defined KLAYGE_PLATFORM_WINDOWS
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
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		HCURSOR cur = GetCursor();
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		
		this->OpenModel(fn);
		
		SetCursor(cur);
	}
#endif
}

void ModelViewerApp::SaveAsHandler(KlayGE::UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
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
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if (GetSaveFileNameA(&ofn))
	{
		HCURSOR cur = GetCursor();
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		
		SaveModel(model_, fn);

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
		model_->RebindJoints();
		this->FrameChangedHandler(*dialog_animation_->Control<UISlider>(id_frame_slider_));
	}
	else
	{
		model_->UnbindJoints();
		dialog_animation_->Control<UICheckBox>(id_play_)->SetChecked(false);
		dialog_animation_->Control<UICheckBox>(id_play_)->SetEnabled(false);
	}
}

void ModelViewerApp::FrameChangedHandler(KlayGE::UISlider const & sender)
{
	frame_ = sender.GetValue() * 0.1f;
	if (skinned_)
	{
		model_->SetFrame(frame_);
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
	model_->SmoothMesh(sender.GetChecked());
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
	RenderLayoutPtr const & rl = model_->Mesh(mi)->GetRenderLayout();
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

			dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Vertex " + oss_one.str(), rl->VertexStreamFormat(i)[j]);
		}

		dialog_model_->Control<UIListBox>(id_vertex_streams_)->AddItem(oss.str());
	}

	dialog_model_->Control<UIListBox>(id_textures_)->RemoveAllItems();
	TextureSlotsType const & texture_slots = model_->GetMaterial(model_->Mesh(mi)->MaterialID())->texture_slots;
	for (size_t i = 0; i < texture_slots.size(); ++ i)
	{
		std::wostringstream oss;
		std::wstring type, name;
		Convert(type, texture_slots[i].first);
		Convert(name, texture_slots[i].second);
		oss << type << L": " << name;
		dialog_model_->Control<UIListBox>(id_textures_)->AddItem(oss.str());

		dialog_model_->Control<UIComboBox>(id_visualize_)->AddItem(L"Texture " + type, static_cast<int>(i));
	}
}

void ModelViewerApp::VisualizeChangedHandler(KlayGE::UIComboBox const & sender)
{
	if (0 == sender.GetSelectedIndex())
	{
		model_->VisualizeLighting();
	}
	else
	{
		boost::any data = sender.GetSelectedData();
		try
		{
			int slot = boost::any_cast<int>(data);
			model_->VisualizeTexture(slot);
		}
		catch (boost::bad_any_cast&)
		{
			vertex_element ve = boost::any_cast<vertex_element>(data);
			model_->VisualizeVertex(ve.usage, ve.usage_index);
		}
	}
}

void ModelViewerApp::LineModeChangedHandler(KlayGE::UICheckBox const & sender)
{
	model_->LineMode(sender.GetChecked());
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

uint32_t ModelViewerApp::DoUpdate(KlayGE::uint32_t /*pass*/)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	
	/*Box const & bb = model_->GetBound();
	float near_plane = 1e10f;
	float far_plane = 1e-10f;
	for (int i = 0; i < 8; ++ i)
	{
		App3DFramework& app = Context::Instance().AppInstance();
		float4x4 const & view = app.ActiveCamera().ViewMatrix();

		float3 v = MathLib::transform_coord(bb[i], view);
		near_plane = std::min(near_plane, v.z() * 0.8f);
		near_plane = std::max(0.01f, near_plane);
		far_plane = std::max(near_plane + 0.1f, std::max(far_plane, v.z() * 1.2f));
	}
	this->Proj(near_plane, far_plane);*/

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

	if (play_)
	{
		float this_time = static_cast<float>(ani_timer_.elapsed());
		if (this_time - last_time_ > 0.1f / model_->FrameRate())
		{
			frame_ += 0.1f;
			frame_ = fmod(frame_, static_cast<float>(model_->EndFrame() - model_->StartFrame())) + model_->StartFrame();

			last_time_ = this_time;
		}

		dialog_animation_->Control<UISlider>(id_frame_slider_)->SetValue(static_cast<int>(frame_ * 10 + 0.5f));
	}

	model_->SetLightPos(float3(0, 2, 0));
	model_->SetEyePos(this->ActiveCamera().EyePos());

	model_->AddToRenderQueue();

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
