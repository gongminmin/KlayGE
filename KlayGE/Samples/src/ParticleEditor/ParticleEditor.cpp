#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>

#include "SampleCommon.hpp"
#include "ParticleEditor.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class TerrainRenderable : public RenderableHelper
	{
	public:
		TerrainRenderable()
			: RenderableHelper(L"Terrain")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("ParticleEditor.fxml");
			depth_tech_ = effect_->TechniqueByName("TerrainDepth");
			color_tech_ = effect_->TechniqueByName("Terrain");
			technique_ = color_tech_;

			*(effect_->ParameterByName("grass_tex")) = ASyncLoadTexture("grass.dds", EAH_GPU_Read | EAH_Immutable);

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

			float3 vertices[] =
			{
				float3(-10, 0, +10),
				float3(+10, 0, +10),
				float3(-10, 0, -10),
				float3(+10, 0, -10),
			};

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vertices), vertices);
			rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_BGR32F));

			pos_aabb_ = MathLib::compute_aabbox(vertices, vertices + std::size(vertices));
			pos_aabb_.Min().y() = -0.1f;
			pos_aabb_.Max().y() = +0.1f;
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();

			*(effect_->ParameterByName("view")) = view;
			*(effect_->ParameterByName("proj")) = proj;

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*(effect_->ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1.0f / camera.FarPlane());
		}

		virtual void Pass(PassType type) override
		{
			switch (type)
			{
			case PT_OpaqueGBufferMRT:
				technique_ = depth_tech_;
				break;

			default:
				technique_ = color_tech_;
				break;
			}
		}

	private:
		RenderTechnique* depth_tech_;
		RenderTechnique* color_tech_;
	};

	class TerrainObject : public SceneObjectHelper
	{
	public:
		TerrainObject()
			: SceneObjectHelper(MakeSharedPtr<TerrainRenderable>(), SOA_Cullable)
		{
		}
	};

	enum
	{
		Exit
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape)
	};
}


int SampleMain()
{
	ParticleEditorApp app;
	app.Create();
	app.Run();

	return 0;
}

ParticleEditorApp::ParticleEditorApp()
					: App3DFramework("Particle Editor")
{
	ResLoader::Instance().AddPath("../../Samples/media/ParticleEditor");
}

void ParticleEditorApp::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(-1.2f, 0.5f, -1.2f), float3(0, 0.5f, 0));
	this->Proj(0.01f, 100);

	fpsController_.Scalers(0.05f, 0.1f);

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

	terrain_ = MakeSharedPtr<TerrainObject>();
	terrain_->AddToSceneManager();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	depth_texture_support_ = caps.depth_texture_support;

	scene_buffer_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	scene_buffer_->GetViewport()->camera = screen_buffer->GetViewport()->camera;
	if (depth_texture_support_)
	{
		depth_to_linear_pp_ = SyncLoadPostProcess("Depth.ppml", "DepthToLinear");
	}
	else
	{
		scene_depth_buffer_ = rf.MakeFrameBuffer();
		scene_depth_buffer_->GetViewport()->camera = screen_buffer->GetViewport()->camera;
	}

	copy_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");

	UIManager::Instance().Load(ResLoader::Instance().Open("ParticleEditor.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_open_ = dialog_->IDFromName("Open");
	id_save_as_ = dialog_->IDFromName("SaveAs");
	id_gravity_static_ = dialog_->IDFromName("GravityStatic");
	id_gravity_slider_ = dialog_->IDFromName("GravitySlider");
	id_density_static_ = dialog_->IDFromName("DensityStatic");
	id_density_slider_ = dialog_->IDFromName("DensitySlider");
	id_freq_static_ = dialog_->IDFromName("FreqStatic");
	id_freq_slider_ = dialog_->IDFromName("FreqSlider");
	id_angle_static_ = dialog_->IDFromName("AngleStatic");
	id_angle_slider_ = dialog_->IDFromName("AngleSlider");
	id_detail_x_static_ = dialog_->IDFromName("DetailXStatic");
	id_detail_x_slider_ = dialog_->IDFromName("DetailXSlider");
	id_detail_y_static_ = dialog_->IDFromName("DetailYStatic");
	id_detail_y_slider_ = dialog_->IDFromName("DetailYSlider");
	id_detail_z_static_ = dialog_->IDFromName("DetailZStatic");
	id_detail_z_slider_ = dialog_->IDFromName("DetailZSlider");
	id_min_velocity_static_ = dialog_->IDFromName("MinVelocityStatic");
	id_min_velocity_slider_ = dialog_->IDFromName("MinVelocitySlider");
	id_max_velocity_static_ = dialog_->IDFromName("MaxVelocityStatic");
	id_max_velocity_slider_ = dialog_->IDFromName("MaxVelocitySlider");
	id_min_life_static_ = dialog_->IDFromName("MinLifeStatic");
	id_min_life_slider_ = dialog_->IDFromName("MinLifeSlider");
	id_max_life_static_ = dialog_->IDFromName("MaxLifeStatic");
	id_max_life_slider_ = dialog_->IDFromName("MaxLifeSlider");
	id_fps_camera_ = dialog_->IDFromName("FPSCamera");
	id_particle_alpha_from_button_ = dialog_->IDFromName("ParticleAlphaFromButton");
	id_particle_alpha_to_button_ = dialog_->IDFromName("ParticleAlphaToButton");
	id_particle_color_from_button_ = dialog_->IDFromName("ParticleColorFromButton");
	id_particle_color_to_button_ = dialog_->IDFromName("ParticleColorToButton");
	id_curve_type_ = dialog_->IDFromName("CurveTypeCombo");
	id_size_over_life_ = dialog_->IDFromName("SizeOverLifePolyline");
	id_mass_over_life_ = dialog_->IDFromName("MassOverLifePolyline");
	id_opacity_over_life_ = dialog_->IDFromName("OpacityOverLifePolyline");

	this->LoadParticleSystem(ResLoader::Instance().Locate("Fire.psml"));

	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().connect(
		[this](UIButton const & sender)
		{
			this->OpenHandler(sender);
		});
	dialog_->Control<UIButton>(id_save_as_)->OnClickedEvent().connect(
		[this](UIButton const & sender)
		{
			this->SaveAsHandler(sender);
		});

	dialog_->Control<UISlider>(id_gravity_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->GravityChangedHandler(sender);
		});
	this->GravityChangedHandler(*dialog_->Control<UISlider>(id_gravity_slider_));
	dialog_->Control<UISlider>(id_density_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->DensityChangedHandler(sender);
		});
	this->DensityChangedHandler(*dialog_->Control<UISlider>(id_density_slider_));
	dialog_->Control<UISlider>(id_freq_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->FreqChangedHandler(sender);
		});
	this->FreqChangedHandler(*dialog_->Control<UISlider>(id_freq_slider_));
	dialog_->Control<UISlider>(id_angle_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->AngleChangedHandler(sender);
		});
	this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));
	dialog_->Control<UISlider>(id_detail_x_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->DetailXChangedHandler(sender);
		});
	this->DetailXChangedHandler(*dialog_->Control<UISlider>(id_detail_x_slider_));
	dialog_->Control<UISlider>(id_detail_y_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->DetailYChangedHandler(sender);
		});
	this->DetailYChangedHandler(*dialog_->Control<UISlider>(id_detail_y_slider_));
	dialog_->Control<UISlider>(id_detail_z_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->DetailZChangedHandler(sender);
		});
	this->DetailZChangedHandler(*dialog_->Control<UISlider>(id_detail_z_slider_));
	dialog_->Control<UISlider>(id_min_velocity_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->MinVelocityChangedHandler(sender);
		});
	this->MinVelocityChangedHandler(*dialog_->Control<UISlider>(id_min_velocity_slider_));
	dialog_->Control<UISlider>(id_max_velocity_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->MaxVelocityChangedHandler(sender);
		});
	this->MaxVelocityChangedHandler(*dialog_->Control<UISlider>(id_max_velocity_slider_));
	dialog_->Control<UISlider>(id_min_life_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->MinLifeChangedHandler(sender);
		});
	this->MinLifeChangedHandler(*dialog_->Control<UISlider>(id_min_life_slider_));
	dialog_->Control<UISlider>(id_max_life_slider_)->OnValueChangedEvent().connect(
		[this](UISlider const & sender)
		{
			this->MaxLifeChangedHandler(sender);
		});
	this->MaxLifeChangedHandler(*dialog_->Control<UISlider>(id_max_life_slider_));

	dialog_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(
		[this](UICheckBox const & sender)
		{
			this->FPSCameraHandler(sender);
		});

	dialog_->Control<UITexButton>(id_particle_alpha_from_button_)->OnClickedEvent().connect(
		[this](UITexButton const & sender)
		{
			this->ChangeParticleAlphaFromHandler(sender);
		});
	dialog_->Control<UITexButton>(id_particle_alpha_to_button_)->OnClickedEvent().connect(
		[this](UITexButton const & sender)
		{
			this->ChangeParticleAlphaToHandler(sender);
		});
	dialog_->Control<UITexButton>(id_particle_color_from_button_)->OnClickedEvent().connect(
		[this](UITexButton const & sender)
		{
			this->ChangeParticleColorFromHandler(sender);
		});
	dialog_->Control<UITexButton>(id_particle_color_to_button_)->OnClickedEvent().connect(
		[this](UITexButton const & sender)
		{
			this->ChangeParticleColorToHandler(sender);
		});

	dialog_->Control<UIComboBox>(id_curve_type_)->OnSelectionChangedEvent().connect(
		[this](UIComboBox const & sender)
		{
			this->CurveTypeChangedHandler(sender);
		});
}

void ParticleEditorApp::OnResize(uint32_t width, uint32_t height)
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

	if (caps.pack_to_rgba_required)
	{
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt = EF_ABGR8;
		}
		else if (caps.rendertarget_format_support(EF_ARGB8, 1, 0))
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));
			fmt = EF_ARGB8;
		}
	}
	else
	{
		if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			fmt = EF_R16F;
		}
		else if (caps.rendertarget_format_support(EF_R32F, 1, 0))
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_R32F, 1, 0));
			fmt = EF_R32F;
		}
	}
	scene_depth_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	ElementFormat ds_fmt;
	if (caps.rendertarget_format_support(EF_D24S8, 1, 0))
	{
		ds_fmt = EF_D24S8;
	}
	else
	{
		BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

		ds_fmt = EF_D16;
	}

	RenderViewPtr ds_view;
	if (depth_texture_support_)
	{
		scene_ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		ds_view = rf.Make2DDepthStencilRenderView(*scene_ds_tex_, 0, 1, 0);

		depth_to_linear_pp_->InputPin(0, scene_ds_tex_);
		depth_to_linear_pp_->OutputPin(0, scene_depth_tex_);
	}
	else
	{
		ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);

		scene_depth_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_depth_tex_, 0, 1, 0));
		scene_depth_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
	}

	scene_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_tex_, 0, 1, 0));
	scene_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	copy_pp_->InputPin(0, scene_tex_);

	if (ps_)
	{
		ps_->SceneDepthTexture(scene_depth_tex_);
	}

	UIManager::Instance().SettleCtrls();
}

void ParticleEditorApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ParticleEditorApp::OpenHandler(KlayGE::UIButton const & /*sender*/)
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
	ofn.lpstrFilter = "PSML File\0*.psml\0All\0*.*\0";
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

		this->LoadParticleSystem(fn);

		SetCursor(cur);
	}
#endif
}

void ParticleEditorApp::SaveAsHandler(KlayGE::UIButton const & /*sender*/)
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
	ofn.lpstrFilter = "PSML File\0*.psml\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ".psml";

	if (GetSaveFileNameA(&ofn))
	{
		this->SaveParticleSystem(fn);
	}
#endif
}

void ParticleEditorApp::FreqChangedHandler(KlayGE::UISlider const & sender)
{
	float freq = static_cast<float>(sender.GetValue() * 10);
	particle_emitter_->Frequency(freq);

	std::wostringstream stream;
	stream << "Freq: " << freq;
	dialog_->Control<UIStatic>(id_freq_static_)->SetText(stream.str());
}

void ParticleEditorApp::AngleChangedHandler(KlayGE::UISlider const & sender)
{
	float angle = static_cast<float>(sender.GetValue());
	particle_emitter_->EmitAngle(angle * DEG2RAD);

	std::wostringstream stream;
	stream << "Emit Angle: " << angle;
	dialog_->Control<UIStatic>(id_angle_static_)->SetText(stream.str());
}

void ParticleEditorApp::DetailXChangedHandler(KlayGE::UISlider const & sender)
{
	float3 p = particle_emitter_->MaxPosition();

	p.x() = sender.GetValue() / 1000.0f;
	particle_emitter_->MinPosition(-p);
	particle_emitter_->MaxPosition(p);

	std::wostringstream stream;
	stream << "Detail X: " << p.x();
	dialog_->Control<UIStatic>(id_detail_x_static_)->SetText(stream.str());
}

void ParticleEditorApp::DetailYChangedHandler(KlayGE::UISlider const & sender)
{
	float3 p = particle_emitter_->MaxPosition();

	p.y() = sender.GetValue() / 1000.0f;
	particle_emitter_->MinPosition(-p);
	particle_emitter_->MaxPosition(p);

	std::wostringstream stream;
	stream << "Detail Y: " << p.y();
	dialog_->Control<UIStatic>(id_detail_y_static_)->SetText(stream.str());
}

void ParticleEditorApp::DetailZChangedHandler(KlayGE::UISlider const & sender)
{
	float3 p = particle_emitter_->MaxPosition();

	p.z() = sender.GetValue() / 1000.0f;
	particle_emitter_->MinPosition(-p);
	particle_emitter_->MaxPosition(p);

	std::wostringstream stream;
	stream << "Detail Z: " << p.z();
	dialog_->Control<UIStatic>(id_detail_z_static_)->SetText(stream.str());
}

void ParticleEditorApp::MinVelocityChangedHandler(KlayGE::UISlider const & sender)
{
	float velocity = sender.GetValue() / 100.0f;
	particle_emitter_->MinVelocity(velocity);

	std::wostringstream stream;
	stream << "Min Vel.: " << velocity;
	dialog_->Control<UIStatic>(id_min_velocity_static_)->SetText(stream.str());
}

void ParticleEditorApp::MaxVelocityChangedHandler(KlayGE::UISlider const & sender)
{
	float velocity = sender.GetValue() / 100.0f;
	particle_emitter_->MaxVelocity(velocity);

	std::wostringstream stream;
	stream << "Max Vel.: " << velocity;
	dialog_->Control<UIStatic>(id_max_velocity_static_)->SetText(stream.str());
}

void ParticleEditorApp::MinLifeChangedHandler(KlayGE::UISlider const & sender)
{
	float min_life = static_cast<float>(sender.GetValue());
	particle_emitter_->MinLife(min_life);

	std::wostringstream stream;
	stream << "Min Life: " << min_life;
	dialog_->Control<UIStatic>(id_min_life_static_)->SetText(stream.str());
}

void ParticleEditorApp::MaxLifeChangedHandler(KlayGE::UISlider const & sender)
{
	float max_life = static_cast<float>(sender.GetValue());
	particle_emitter_->MaxLife(max_life);

	std::wostringstream stream;
	stream << "Max Life: " << max_life;
	dialog_->Control<UIStatic>(id_max_life_static_)->SetText(stream.str());
}

void ParticleEditorApp::GravityChangedHandler(KlayGE::UISlider const & sender)
{
	float gravity = sender.GetValue() / 100.0f;
	ps_->Gravity(gravity);

	std::wostringstream stream;
	stream << "Gravity: " << gravity;
	dialog_->Control<UIStatic>(id_gravity_static_)->SetText(stream.str());
}

void ParticleEditorApp::DensityChangedHandler(KlayGE::UISlider const & sender)
{
	float density = sender.GetValue() / 100.0f;
	ps_->MediaDensity(density);

	std::wostringstream stream;
	stream << "Density: " << density;
	dialog_->Control<UIStatic>(id_density_static_)->SetText(stream.str());
}

void ParticleEditorApp::FPSCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpsController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpsController_.DetachCamera();
	}
}

void ParticleEditorApp::ChangeParticleAlphaFromHandler(KlayGE::UITexButton const & /*sender*/)
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
	ofn.lpstrFilter = "DDS File\0*.dds\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		ps_->ParticleAlphaFromTex(fn);
		this->LoadParticleAlpha(id_particle_alpha_from_button_, ps_->ParticleAlphaFromTex());
	}
#endif
}

void ParticleEditorApp::ChangeParticleAlphaToHandler(KlayGE::UITexButton const & /*sender*/)
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
	ofn.lpstrFilter = "DDS File\0*.dds\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		ps_->ParticleAlphaToTex(fn);
		this->LoadParticleAlpha(id_particle_alpha_to_button_, ps_->ParticleAlphaToTex());
	}
#endif
}

void ParticleEditorApp::ChangeParticleColorFromHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	CHOOSECOLORA occ;
	HWND hwnd = this->MainWnd()->HWnd();

	static COLORREF cust_clrs[16] = { RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF) };

	Color clr_srgb;
	clr_srgb.r() = MathLib::linear_to_srgb(ps_->ParticleColorFrom().r());
	clr_srgb.g() = MathLib::linear_to_srgb(ps_->ParticleColorFrom().g());
	clr_srgb.b() = MathLib::linear_to_srgb(ps_->ParticleColorFrom().b());

	ZeroMemory(&occ, sizeof(occ));
	occ.lStructSize = sizeof(occ);
	occ.hwndOwner = hwnd;
	occ.hInstance = nullptr;
	occ.rgbResult = clr_srgb.ABGR();
	occ.lpCustColors = cust_clrs;
	occ.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	occ.lCustData = 0;
	occ.lpfnHook = nullptr;
	occ.lpTemplateName = nullptr;

	if (ChooseColorA(&occ))
	{
		clr_srgb = Color(occ.rgbResult);
		Color clr_linear;
		clr_linear.r() = MathLib::srgb_to_linear(clr_srgb.b());
		clr_linear.g() = MathLib::srgb_to_linear(clr_srgb.g());
		clr_linear.b() = MathLib::srgb_to_linear(clr_srgb.r());
		ps_->ParticleColorFrom(clr_linear);
		this->LoadParticleColor(id_particle_color_from_button_, clr_linear);
	}
#endif
}

void ParticleEditorApp::ChangeParticleColorToHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	CHOOSECOLORA occ;
	HWND hwnd = this->MainWnd()->HWnd();

	static COLORREF cust_clrs[16] = { RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF) };

	Color clr_srgb;
	clr_srgb.r() = MathLib::linear_to_srgb(ps_->ParticleColorTo().r());
	clr_srgb.g() = MathLib::linear_to_srgb(ps_->ParticleColorTo().g());
	clr_srgb.b() = MathLib::linear_to_srgb(ps_->ParticleColorTo().b());

	ZeroMemory(&occ, sizeof(occ));
	occ.lStructSize = sizeof(occ);
	occ.hwndOwner = hwnd;
	occ.hInstance = nullptr;
	occ.rgbResult = clr_srgb.ABGR();
	occ.lpCustColors = cust_clrs;
	occ.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	occ.lCustData = 0;
	occ.lpfnHook = nullptr;
	occ.lpTemplateName = nullptr;

	if (ChooseColorA(&occ))
	{
		clr_srgb = Color(occ.rgbResult);
		Color clr_linear;
		clr_linear.r() = MathLib::srgb_to_linear(clr_srgb.b());
		clr_linear.g() = MathLib::srgb_to_linear(clr_srgb.g());
		clr_linear.b() = MathLib::srgb_to_linear(clr_srgb.r());
		ps_->ParticleColorTo(clr_linear);
		this->LoadParticleColor(id_particle_color_to_button_, clr_linear);
	}
#endif
}

void ParticleEditorApp::CurveTypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	uint32_t ct = sender.GetSelectedIndex();
	dialog_->GetControl(id_size_over_life_)->SetVisible(false);
	dialog_->GetControl(id_mass_over_life_)->SetVisible(false);
	dialog_->GetControl(id_opacity_over_life_)->SetVisible(false);
	switch (ct)
	{
	case 0:
		dialog_->GetControl(id_size_over_life_)->SetVisible(true);
		break;

	case 1:
		dialog_->GetControl(id_mass_over_life_)->SetVisible(true);
		break;

	default:
		dialog_->GetControl(id_opacity_over_life_)->SetVisible(true);
		break;
	}
}

void ParticleEditorApp::LoadParticleAlpha(int id, std::string const & name)
{
	TexturePtr tex = SyncLoadTexture(name, EAH_GPU_Read | EAH_Immutable);

	TexturePtr tex_for_button;
	if (EF_R8 == tex->Format())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		TexturePtr cpu_tex = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, EF_R8, 1, 0, EAH_CPU_Read);
		tex->CopyToTexture(*cpu_tex);

		std::vector<uint8_t> data(tex->Width(0) * tex->Height(0) * 4);
		{
			Texture::Mapper mapper(*cpu_tex, 0, 0, TMA_Read_Only, 0, 0, tex->Width(0), tex->Height(0));
			uint8_t const * p = mapper.Pointer<uint8_t>();
			for (uint32_t y = 0; y < tex->Height(0); ++ y)
			{
				for (uint32_t x = 0; x < tex->Width(0); ++ x)
				{
					uint8_t d = p[y * mapper.RowPitch() + x];
					data[(y * tex->Width(0) + x) * 4 + 0] = d;
					data[(y * tex->Width(0) + x) * 4 + 1] = d;
					data[(y * tex->Width(0) + x) * 4 + 2] = d;
					data[(y * tex->Width(0) + x) * 4 + 3] = 0xFF;
				}
			}
		}

		ElementInitData init_data;
		init_data.data = &data[0];
		init_data.row_pitch = tex->Width(0) * 4;
		tex_for_button = rf.MakeTexture2D(cpu_tex->Width(0), cpu_tex->Height(0), 1, 1,
			rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8) ? EF_ABGR8 : EF_ARGB8, 1, 0,
			EAH_GPU_Read | EAH_Immutable, init_data);
	}
	else
	{
		tex_for_button = tex;
	}
	dialog_->Control<UITexButton>(id)->SetTexture(tex_for_button);
}

void ParticleEditorApp::LoadParticleColor(int id, KlayGE::Color const & clr)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

	ElementFormat fmt = caps.texture_format_support(EF_ABGR8) ? EF_ABGR8 : EF_ARGB8;

	uint32_t data;
	data = 0xFF000000 | ((EF_ABGR8 == fmt) ? clr.ABGR() : clr.ARGB());
	ElementInitData init_data;
	init_data.data = &data;
	init_data.row_pitch = 4;
	TexturePtr tex_for_button = rf.MakeTexture2D(1, 1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, init_data);

	dialog_->Control<UITexButton>(id)->SetTexture(tex_for_button);
}

void ParticleEditorApp::LoadParticleSystem(std::string const & name)
{
	dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->ClearCtrlPoints();
	dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->ClearCtrlPoints();
	dialog_->Control<UIPolylineEditBox>(id_opacity_over_life_)->ClearCtrlPoints();

	if (ps_)
	{
		ps_->DelFromSceneManager();
	}
	ps_ = SyncLoadParticleSystem(name);
	ps_->Gravity(0.5f);
	ps_->MediaDensity(0.5f);
	ps_->AddToSceneManager();

	if (scene_depth_tex_)
	{
		ps_->SceneDepthTexture(scene_depth_tex_);
	}

	particle_emitter_ = ps_->Emitter(0);
	particle_updater_ = ps_->Updater(0);

	particle_emitter_->MinSpin(-PI / 2);
	particle_emitter_->MaxSpin(+PI / 2);
	particle_emitter_->ModelMatrix(MathLib::translation(0.0f, 0.1f, 0.0f));

	this->LoadParticleAlpha(id_particle_alpha_from_button_, ResLoader::Instance().Locate(ps_->ParticleAlphaFromTex()));
	this->LoadParticleAlpha(id_particle_alpha_to_button_, ResLoader::Instance().Locate(ps_->ParticleAlphaToTex()));

	this->LoadParticleColor(id_particle_color_from_button_, ps_->ParticleColorFrom());
	this->LoadParticleColor(id_particle_color_to_button_, ps_->ParticleColorTo());

	dialog_->Control<UISlider>(id_freq_slider_)->SetValue(static_cast<int>(particle_emitter_->Frequency() * 0.1f + 0.5f));
	this->FreqChangedHandler(*dialog_->Control<UISlider>(id_freq_slider_));

	dialog_->Control<UISlider>(id_angle_slider_)->SetValue(static_cast<int>(particle_emitter_->EmitAngle() * RAD2DEG + 0.5f));
	this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));

	dialog_->Control<UISlider>(id_detail_x_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxPosition().x() * 1000.0f + 0.5f));
	this->DetailXChangedHandler(*dialog_->Control<UISlider>(id_detail_x_slider_));

	dialog_->Control<UISlider>(id_detail_y_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxPosition().y() * 1000.0f + 0.5f));
	this->DetailYChangedHandler(*dialog_->Control<UISlider>(id_detail_y_slider_));

	dialog_->Control<UISlider>(id_detail_z_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxPosition().z() * 1000.0f + 0.5f));
	this->DetailZChangedHandler(*dialog_->Control<UISlider>(id_detail_z_slider_));

	dialog_->Control<UISlider>(id_min_velocity_slider_)->SetValue(static_cast<int>(particle_emitter_->MinVelocity() * 100.0f + 0.5f));
	this->MinVelocityChangedHandler(*dialog_->Control<UISlider>(id_min_velocity_slider_));

	dialog_->Control<UISlider>(id_max_velocity_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxVelocity() * 100.0f + 0.5f));
	this->MaxVelocityChangedHandler(*dialog_->Control<UISlider>(id_max_velocity_slider_));

	dialog_->Control<UISlider>(id_min_life_slider_)->SetValue(static_cast<int>(particle_emitter_->MinLife()));
	this->MinLifeChangedHandler(*dialog_->Control<UISlider>(id_min_life_slider_));

	dialog_->Control<UISlider>(id_max_life_slider_)->SetValue(static_cast<int>(particle_emitter_->MaxLife()));
	this->MaxLifeChangedHandler(*dialog_->Control<UISlider>(id_max_life_slider_));

	dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->SetCtrlPoints(checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->SizeOverLife());
	dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->SetCtrlPoints(checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->MassOverLife());
	dialog_->Control<UIPolylineEditBox>(id_opacity_over_life_)->SetCtrlPoints(checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->OpacityOverLife());
}

void ParticleEditorApp::SaveParticleSystem(std::string const & name)
{
	KlayGE::SaveParticleSystem(ps_, name);
}

void ParticleEditorApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Particle System", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t ParticleEditorApp::DoUpdate(uint32_t pass)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	if (depth_texture_support_)
	{
		switch (pass)
		{
		case 0:
			{
				re.BindFrameBuffer(scene_buffer_);

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

				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->SizeOverLife(dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->GetCtrlPoints());
				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->MassOverLife(dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->GetCtrlPoints());
				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->OpacityOverLife(dialog_->Control<UIPolylineEditBox>(id_opacity_over_life_)->GetCtrlPoints());

				terrain_->Visible(true);
				ps_->Visible(false);
			}
			return App3DFramework::URV_NeedFlush;

		default:
			depth_to_linear_pp_->Apply();

			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);

			copy_pp_->Apply();

			terrain_->Visible(false);
			ps_->Visible(true);

			return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
		}
	}
	else
	{
		switch (pass)
		{
		case 0:
			{
				re.BindFrameBuffer(scene_depth_buffer_);
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(100.0f, 100.0f, 100.0f, 1), 1.0f, 0);

				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->SizeOverLife(dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->GetCtrlPoints());
				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->MassOverLife(dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->GetCtrlPoints());
				checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->OpacityOverLife(dialog_->Control<UIPolylineEditBox>(id_opacity_over_life_)->GetCtrlPoints());

				terrain_->Pass(PT_OpaqueGBufferMRT);
				terrain_->Visible(true);
				ps_->Visible(false);
			}
			return App3DFramework::URV_NeedFlush;

		case 1:
			{
				re.BindFrameBuffer(scene_buffer_);
			
				Color clear_clr(0.2f, 0.4f, 0.6f, 1);
				if (Context::Instance().Config().graphics_cfg.gamma)
				{
					clear_clr.r() = 0.029f;
					clear_clr.g() = 0.133f;
					clear_clr.b() = 0.325f;
				}
				re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

				terrain_->Pass(PT_OpaqueShading);
				terrain_->Visible(true);
				ps_->Visible(false);
			}
			return App3DFramework::URV_NeedFlush;

		default:
			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);

			copy_pp_->Apply();

			terrain_->Visible(false);
			ps_->Visible(true);

			return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
		}
	}
}
