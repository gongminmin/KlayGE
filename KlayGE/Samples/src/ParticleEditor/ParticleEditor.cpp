#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#ifdef Bool
#undef Bool		// foreach
#endif

#include <vector>
#include <sstream>
#include <fstream>

#include "SampleCommon.hpp"
#include "ParticleEditor.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	bool use_gs = false;

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
	struct ParticleInstance
	{
		float3 pos;
		float life;
		float spin;
		float size;
		float life_factor;
		float alpha;
	};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

	class TerrainRenderable : public RenderableHelper
	{
	public:
		TerrainRenderable()
			: RenderableHelper(L"Terrain")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = SyncLoadRenderEffect("ParticleEditor.fxml")->TechniqueByName("Terrain");
			*(technique_->Effect().ParameterByName("grass_tex")) = ASyncLoadTexture("grass.dds", EAH_GPU_Read | EAH_Immutable);

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

			float3 vertices[] =
			{
				float3(-10, 0, +10),
				float3(+10, 0, +10),
				float3(-10, 0, -10),
				float3(+10, 0, -10),
			};

			ElementInitData init_data;
			init_data.row_pitch = sizeof(vertices);
			init_data.slice_pitch = 0;
			init_data.data = &vertices[0];
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindVertexStream(pos_vb, KlayGE::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			pos_aabb_ = MathLib::compute_aabbox(vertices, vertices + sizeof(vertices) / sizeof(vertices[0]));
			pos_aabb_.Min().y() = -0.1f;
			pos_aabb_.Max().y() = +0.1f;
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1.0f / camera.FarPlane());
		}
	};

	class TerrainObject : public SceneObjectHelper
	{
	public:
		TerrainObject()
			: SceneObjectHelper(MakeSharedPtr<TerrainRenderable>(), SOA_Cullable)
		{
		}
	};

	int const NUM_PARTICLE = 4096;

	class RenderParticles : public RenderableHelper
	{
	public:
		RenderParticles()
			: RenderableHelper(L"Particles")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			float2 texs[] =
			{
				float2(-1.0f, 1.0f),
				float2(1.0f, 1.0f),
				float2(-1.0f, -1.0f),
				float2(1.0f, -1.0f)
			};

			uint16_t indices[] =
			{
				0, 1, 2, 3
			};

			rl_ = rf.MakeRenderLayout();
			if (use_gs)
			{
				rl_->TopologyType(RenderLayout::TT_PointList);

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, nullptr);
				rl_->BindVertexStream(pos_vb, KlayGE::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F),
					vertex_element(VEU_TextureCoord, 0, EF_ABGR32F)));

				technique_ = SyncLoadRenderEffect("ParticleEditor.fxml")->TechniqueByName("ParticleWithGS");
			}
			else
			{
				rl_->TopologyType(RenderLayout::TT_TriangleStrip);

				ElementInitData init_data;
				init_data.row_pitch = sizeof(texs);
				init_data.slice_pitch = 0;
				init_data.data = texs;
				GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
				rl_->BindVertexStream(tex_vb, KlayGE::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, nullptr);
				rl_->BindVertexStream(pos_vb,
					KlayGE::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_ABGR32F),
						vertex_element(VEU_TextureCoord, 1, EF_ABGR32F)),
					RenderLayout::ST_Instance);

				init_data.row_pitch = sizeof(indices);
				init_data.slice_pitch = 0;
				init_data.data = indices;
				GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
				rl_->BindIndexStream(ib, EF_R16UI);

				technique_ = SyncLoadRenderEffect("ParticleEditor.fxml")->TechniqueByName("Particle");
			}

			*(technique_->Effect().ParameterByName("point_radius")) = 0.08f;
		}

		void SceneTexture(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("scene_tex")) = tex;
		}

		void ParticleColorFrom(float3 const & clr)
		{
			*(technique_->Effect().ParameterByName("particle_color_from")) = clr;
		}

		void ParticleColorTo(float3 const & clr)
		{
			*(technique_->Effect().ParameterByName("particle_color_to")) = clr;
		}

		void ParticleAlphaFrom(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("particle_alpha_from_tex")) = tex;
		}

		void ParticleAlphaTo(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("particle_alpha_to_tex")) = tex;
		}

		void InitLife(float life)
		{
			*(technique_->Effect().ParameterByName("init_life")) = life;
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;

			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(), camera.FarPlane(), 1.0f / camera.FarPlane());
		}
	};

	class ParticlesObject : public SceneObjectHelper
	{
	public:
		ParticlesObject()
			: SceneObjectHelper(SOA_Moveable)
		{
			renderable_ = MakeSharedPtr<RenderParticles>();
		}

		void ParticleColorFrom(float3 const & clr)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleColorFrom(clr);
		}

		void ParticleColorTo(float3 const & clr)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleColorTo(clr);
		}

		void ParticleAlphaFrom(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleAlphaFrom(tex);
		}

		void ParticleAlphaTo(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleAlphaTo(tex);
		}

		void InitLife(float life)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->InitLife(life);
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

bool ParticleEditorApp::ConfirmDevice() const
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	if (!caps.rendertarget_format_support(EF_ABGR16F, 1, 0))
	{
		return false;
	}

	return true;
}

void ParticleEditorApp::InitObjects()
{
	use_gs = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps().gs_support;

	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(-1.2f, 0.5f, -1.2f), float3(0, 0.5f, 0));
	this->Proj(0.01f, 100);

	fpsController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(KlayGE::bind(&ParticleEditorApp::InputHandler, this, KlayGE::placeholders::_1, KlayGE::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	particles_ = MakeSharedPtr<ParticlesObject>();
	particles_->AddToSceneManager();

	terrain_ = MakeSharedPtr<TerrainObject>();
	terrain_->AddToSceneManager();

	ps_ = MakeSharedPtr<ParticleSystem>(NUM_PARTICLE);
	ps_->Gravity(0.5f);
	ps_->MediaDensity(0.5f);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	scene_buffer_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	scene_buffer_->GetViewport()->camera = screen_buffer->GetViewport()->camera;

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
	id_transparency_over_life_ = dialog_->IDFromName("TransparencyOverLifePolyline");

	this->LoadParticleSystem(ResLoader::Instance().Locate("Fire.psml"));

	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::OpenHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UIButton>(id_save_as_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::SaveAsHandler, this, KlayGE::placeholders::_1));

	dialog_->Control<UISlider>(id_gravity_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::GravityChangedHandler, this, KlayGE::placeholders::_1));
	this->GravityChangedHandler(*dialog_->Control<UISlider>(id_gravity_slider_));
	dialog_->Control<UISlider>(id_density_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::DensityChangedHandler, this, KlayGE::placeholders::_1));
	this->DensityChangedHandler(*dialog_->Control<UISlider>(id_density_slider_));
	dialog_->Control<UISlider>(id_freq_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::FreqChangedHandler, this, KlayGE::placeholders::_1));
	this->FreqChangedHandler(*dialog_->Control<UISlider>(id_freq_slider_));
	dialog_->Control<UISlider>(id_angle_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::AngleChangedHandler, this, KlayGE::placeholders::_1));
	this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));
	dialog_->Control<UISlider>(id_detail_x_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::DetailXChangedHandler, this, KlayGE::placeholders::_1));
	this->DetailXChangedHandler(*dialog_->Control<UISlider>(id_detail_x_slider_));
	dialog_->Control<UISlider>(id_detail_y_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::DetailYChangedHandler, this, KlayGE::placeholders::_1));
	this->DetailYChangedHandler(*dialog_->Control<UISlider>(id_detail_y_slider_));
	dialog_->Control<UISlider>(id_detail_z_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::DetailZChangedHandler, this, KlayGE::placeholders::_1));
	this->DetailZChangedHandler(*dialog_->Control<UISlider>(id_detail_z_slider_));
	dialog_->Control<UISlider>(id_min_velocity_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::MinVelocityChangedHandler, this, KlayGE::placeholders::_1));
	this->MinVelocityChangedHandler(*dialog_->Control<UISlider>(id_min_velocity_slider_));
	dialog_->Control<UISlider>(id_max_velocity_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::MaxVelocityChangedHandler, this, KlayGE::placeholders::_1));
	this->MaxVelocityChangedHandler(*dialog_->Control<UISlider>(id_max_velocity_slider_));
	dialog_->Control<UISlider>(id_min_life_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::MinLifeChangedHandler, this, KlayGE::placeholders::_1));
	this->MinLifeChangedHandler(*dialog_->Control<UISlider>(id_min_life_slider_));
	dialog_->Control<UISlider>(id_max_life_slider_)->OnValueChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::MaxLifeChangedHandler, this, KlayGE::placeholders::_1));
	this->MaxLifeChangedHandler(*dialog_->Control<UISlider>(id_max_life_slider_));

	dialog_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::FPSCameraHandler, this, KlayGE::placeholders::_1));

	dialog_->Control<UITexButton>(id_particle_alpha_from_button_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::ChangeParticleAlphaFromHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UITexButton>(id_particle_alpha_to_button_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::ChangeParticleAlphaToHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UITexButton>(id_particle_color_from_button_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::ChangeParticleColorFromHandler, this, KlayGE::placeholders::_1));
	dialog_->Control<UITexButton>(id_particle_color_to_button_)->OnClickedEvent().connect(KlayGE::bind(&ParticleEditorApp::ChangeParticleColorToHandler, this, KlayGE::placeholders::_1));

	dialog_->Control<UIComboBox>(id_curve_type_)->OnSelectionChangedEvent().connect(KlayGE::bind(&ParticleEditorApp::CurveTypeChangedHandler, this, KlayGE::placeholders::_1));
}

void ParticleEditorApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	RenderViewPtr ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);

	scene_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
	scene_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_tex_, 0, 1, 0));
	scene_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	checked_pointer_cast<RenderParticles>(particles_->GetRenderable())->SceneTexture(scene_tex_);

	copy_pp_->InputPin(0, scene_tex_);

	UIManager::Instance().SettleCtrls(width, height);
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
	ofn.lpstrFilter = "PSML File\0*.psml\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		this->LoadParticleSystem(fn);
	}
#endif
}

void ParticleEditorApp::SaveAsHandler(KlayGE::UIButton const & /*sender*/)
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
	checked_pointer_cast<ParticlesObject>(particles_)->InitLife(max_life);

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
	ofn.lpstrFilter = "DDS File\0*.dds\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		particle_alpha_from_tex_ = fn;
		this->LoadParticleAlpha(id_particle_alpha_from_button_, particle_alpha_from_tex_);
	}
#endif
}

void ParticleEditorApp::ChangeParticleAlphaToHandler(KlayGE::UITexButton const & /*sender*/)
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
	ofn.lpstrFilter = "DDS File\0*.dds\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		particle_alpha_to_tex_ = fn;
		this->LoadParticleAlpha(id_particle_alpha_to_button_, particle_alpha_to_tex_);
	}
#endif
}

void ParticleEditorApp::ChangeParticleColorFromHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	CHOOSECOLORA occ;
	HWND hwnd = this->MainWnd()->HWnd();

	static COLORREF cust_clrs[16] = { RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF) };

	Color clr_srgb;
	clr_srgb.r() = MathLib::linear_to_srgb(particle_color_from_.r());
	clr_srgb.g() = MathLib::linear_to_srgb(particle_color_from_.g());
	clr_srgb.b() = MathLib::linear_to_srgb(particle_color_from_.b());
	clr_srgb.a() = particle_color_from_.a();

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
		particle_color_from_.r() = MathLib::srgb_to_linear(clr_srgb.b());
		particle_color_from_.g() = MathLib::srgb_to_linear(clr_srgb.g());
		particle_color_from_.b() = MathLib::srgb_to_linear(clr_srgb.r());
		particle_color_from_.a() = 1;
		this->LoadParticleColor(id_particle_color_from_button_, particle_color_from_);
	}
#endif
}

void ParticleEditorApp::ChangeParticleColorToHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	CHOOSECOLORA occ;
	HWND hwnd = this->MainWnd()->HWnd();

	static COLORREF cust_clrs[16] = { RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF),
		RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF) };

	Color clr_srgb;
	clr_srgb.r() = MathLib::linear_to_srgb(particle_color_to_.r());
	clr_srgb.g() = MathLib::linear_to_srgb(particle_color_to_.g());
	clr_srgb.b() = MathLib::linear_to_srgb(particle_color_to_.b());
	clr_srgb.a() = particle_color_to_.a();

	ZeroMemory(&occ, sizeof(occ));
	occ.lStructSize = sizeof(occ);
	occ.hwndOwner = hwnd;
	occ.hInstance = nullptr;
	occ.rgbResult = particle_color_to_.ABGR();
	occ.lpCustColors = cust_clrs;
	occ.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	occ.lCustData = 0;
	occ.lpfnHook = nullptr;
	occ.lpTemplateName = nullptr;

	if (ChooseColorA(&occ))
	{
		clr_srgb = Color(occ.rgbResult);
		particle_color_to_.r() = MathLib::srgb_to_linear(clr_srgb.b());
		particle_color_to_.g() = MathLib::srgb_to_linear(clr_srgb.g());
		particle_color_to_.b() = MathLib::srgb_to_linear(clr_srgb.r());
		particle_color_to_.a() = 1;
		this->LoadParticleColor(id_particle_color_to_button_, particle_color_to_);
	}
#endif
}

void ParticleEditorApp::CurveTypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	uint32_t ct = sender.GetSelectedIndex();
	dialog_->GetControl(id_size_over_life_)->SetVisible(false);
	dialog_->GetControl(id_mass_over_life_)->SetVisible(false);
	dialog_->GetControl(id_transparency_over_life_)->SetVisible(false);
	switch (ct)
	{
	case 0:
		dialog_->GetControl(id_size_over_life_)->SetVisible(true);
		break;

	case 1:
		dialog_->GetControl(id_mass_over_life_)->SetVisible(true);
		break;

	default:
		dialog_->GetControl(id_transparency_over_life_)->SetVisible(true);
		break;
	}
}

void ParticleEditorApp::LoadParticleAlpha(int id, std::string const & name)
{
	TexturePtr tex = SyncLoadTexture(name, EAH_GPU_Read | EAH_Immutable);
	if (id_particle_alpha_from_button_ == id)
	{
		checked_pointer_cast<ParticlesObject>(particles_)->ParticleAlphaFrom(tex);
	}
	else
	{
		BOOST_ASSERT(id_particle_alpha_to_button_ == id);
		checked_pointer_cast<ParticlesObject>(particles_)->ParticleAlphaTo(tex);
	}

	TexturePtr tex_for_button;
	if (EF_R8 == tex->Format())
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		TexturePtr cpu_tex = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, EF_R8, 1, 0, EAH_CPU_Read, nullptr);
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
			rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8) ? EF_ABGR8 : EF_ARGB8, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data);
	}
	else
	{
		tex_for_button = tex;
	}
	dialog_->Control<UITexButton>(id)->SetTexture(tex_for_button);
}

void ParticleEditorApp::LoadParticleColor(int id, KlayGE::Color const & clr)
{
	if (id_particle_color_from_button_ == id)
	{
		checked_pointer_cast<ParticlesObject>(particles_)->ParticleColorFrom(float3(clr.r(), clr.g(), clr.b()));
	}
	else
	{
		BOOST_ASSERT(id_particle_color_to_button_ == id);
		checked_pointer_cast<ParticlesObject>(particles_)->ParticleColorTo(float3(clr.r(), clr.g(), clr.b()));
	}

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

	ElementFormat fmt = caps.texture_format_support(EF_ABGR8) ? EF_ABGR8 : EF_ARGB8;

	uint32_t data;
	data = 0xFF000000 | ((EF_ABGR8 == fmt) ? clr.ABGR() : clr.ARGB());
	ElementInitData init_data;
	init_data.data = &data;
	init_data.row_pitch = 4;
	TexturePtr tex_for_button = rf.MakeTexture2D(1, 1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data);

	dialog_->Control<UITexButton>(id)->SetTexture(tex_for_button);
}

void ParticleEditorApp::LoadParticleSystem(std::string const & name)
{
	dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->ClearCtrlPoints();
	dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->ClearCtrlPoints();
	dialog_->Control<UIPolylineEditBox>(id_transparency_over_life_)->ClearCtrlPoints();

	ps_->ClearEmitter();
	ps_->ClearUpdater();

	ResIdentifierPtr ifs = ResLoader::Instance().Open(name.c_str());

	KlayGE::XMLDocument doc;
	XMLNodePtr root = doc.Parse(ifs);

	{
		XMLNodePtr particle_node = root->FirstNode("particle");
		{
			XMLNodePtr alpha_node = particle_node->FirstNode("alpha");

			particle_alpha_from_tex_ = alpha_node->Attrib("from")->ValueString();
			this->LoadParticleAlpha(id_particle_alpha_from_button_, ResLoader::Instance().Locate(particle_alpha_from_tex_));

			particle_alpha_to_tex_ = alpha_node->Attrib("to")->ValueString();
			this->LoadParticleAlpha(id_particle_alpha_to_button_, ResLoader::Instance().Locate(particle_alpha_to_tex_));
		}
		{
			XMLNodePtr color_node = particle_node->FirstNode("color");
			{
				XMLAttributePtr attr = color_node->Attrib("from");
				if (attr)
				{
					std::vector<std::string> strs;
					boost::algorithm::split(strs, attr->ValueString(), boost::is_any_of(" "));
					for (size_t i = 0; i < 3; ++ i)
					{
						if (i < strs.size())
						{
							boost::algorithm::trim(strs[i]);
							particle_color_from_[i] = static_cast<float>(atof(strs[i].c_str()));
						}
						else
						{
							particle_color_from_[i] = 0;
						}
					}
				}
				particle_color_from_.a() = 1;
				this->LoadParticleColor(id_particle_color_from_button_, particle_color_from_);

				attr = color_node->Attrib("to");
				if (attr)
				{
					std::vector<std::string> strs;
					boost::algorithm::split(strs, attr->ValueString(), boost::is_any_of(" "));
					for (size_t i = 0; i < 3; ++ i)
					{
						if (i < strs.size())
						{
							boost::algorithm::trim(strs[i]);
							particle_color_to_[i] = static_cast<float>(atof(strs[i].c_str()));
						}
						else
						{
							particle_color_to_[i] = 0;
						}
					}
				}
				particle_color_to_.a() = 1;
				this->LoadParticleColor(id_particle_color_to_button_, particle_color_to_);
			}
		}
	}

	{
		XMLNodePtr emitter_node = root->FirstNode("emitter");

		std::string emitter_type;
		XMLAttributePtr type_attr = emitter_node->Attrib("type");
		if (type_attr)
		{
			emitter_type = type_attr->ValueString();
		}
		else
		{
			emitter_type = "point";
		}

		particle_emitter_ = ps_->MakeEmitter(emitter_type);
		particle_emitter_->MinSpin(-PI / 2);
		particle_emitter_->MaxSpin(+PI / 2);
		ps_->AddEmitter(particle_emitter_);

		XMLNodePtr freq_node = emitter_node->FirstNode("frequency");
		if (freq_node)
		{
			XMLAttributePtr attr = freq_node->Attrib("value");
			particle_emitter_->Frequency(attr->ValueFloat());
		}

		XMLNodePtr angle_node = emitter_node->FirstNode("angle");
		if (angle_node)
		{
			XMLAttributePtr attr = angle_node->Attrib("value");
			particle_emitter_->EmitAngle(attr->ValueInt() * DEG2RAD);
		}

		XMLNodePtr pos_node = emitter_node->FirstNode("pos");
		if (pos_node)
		{
			float3 min_pos(0, 0, 0);
			XMLAttributePtr attr = pos_node->Attrib("min");
			if (attr)
			{
				std::vector<std::string> strs;
				boost::algorithm::split(strs, attr->ValueString(), boost::is_any_of(" "));
				for (size_t i = 0; i < 3; ++ i)
				{
					if (i < strs.size())
					{
						boost::algorithm::trim(strs[i]);
						min_pos[i] = static_cast<float>(atof(strs[i].c_str()));
					}
					else
					{
						min_pos[i] = 0;
					}
				}
			}
			particle_emitter_->MinPosition(min_pos);
			
			float3 max_pos(0, 0, 0);
			attr = pos_node->Attrib("max");
			if (attr)
			{
				std::vector<std::string> strs;
				boost::algorithm::split(strs, attr->ValueString(), boost::is_any_of(" "));
				for (size_t i = 0; i < 3; ++ i)
				{
					if (i < strs.size())
					{
						boost::algorithm::trim(strs[i]);
						max_pos[i] = static_cast<float>(atof(strs[i].c_str()));
					}
					else
					{
						max_pos[i] = 0;
					}
				}
			}			
			particle_emitter_->MaxPosition(max_pos);
		}

		XMLNodePtr vel_node = emitter_node->FirstNode("vel");
		if (vel_node)
		{
			XMLAttributePtr attr = vel_node->Attrib("min");
			particle_emitter_->MinVelocity(attr->ValueFloat());

			attr = vel_node->Attrib("max");
			particle_emitter_->MaxVelocity(attr->ValueFloat());
		}

		XMLNodePtr life_node = emitter_node->FirstNode("life");
		if (life_node)
		{
			XMLAttributePtr attr = life_node->Attrib("min");
			particle_emitter_->MinLife(attr->ValueFloat());

			attr = life_node->Attrib("max");
			particle_emitter_->MaxLife(attr->ValueFloat());
		}
	}

	std::vector<KlayGE::float2> size_over_life_ctrl_pts;
	std::vector<KlayGE::float2> mass_over_life_ctrl_pts;
	std::vector<KlayGE::float2> transparency_over_life_ctrl_pts;
	{
		XMLNodePtr updater_node = root->FirstNode("updater");

		std::string updater_type;
		XMLAttributePtr type_attr = updater_node->Attrib("type");
		if (type_attr)
		{
			updater_type = type_attr->ValueString();
		}
		else
		{
			updater_type = "polyline";
		}

		particle_updater_ = ps_->MakeUpdater(updater_type);
		ps_->AddUpdater(particle_updater_);

		for (XMLNodePtr node = updater_node->FirstNode("curve"); node; node = node->NextSibling("curve"))
		{
			std::vector<float2> xys;
			for (XMLNodePtr ctrl_point_node = node->FirstNode("ctrl_point"); ctrl_point_node; ctrl_point_node = ctrl_point_node->NextSibling("ctrl_point"))
			{
				XMLAttributePtr attr_x = ctrl_point_node->Attrib("x");
				XMLAttributePtr attr_y = ctrl_point_node->Attrib("y");

				xys.push_back(float2(attr_x->ValueFloat(), attr_y->ValueFloat()));
			}

			XMLAttributePtr attr = node->Attrib("name");
			if ("size_over_life" == attr->ValueString())
			{
				size_over_life_ctrl_pts = xys;
			}
			else if ("mass_over_life" == attr->ValueString())
			{
				mass_over_life_ctrl_pts = xys;
			}
			else if ("transparency_over_life" == attr->ValueString())
			{
				transparency_over_life_ctrl_pts = xys;
			}
		}
	}

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

	dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->SetCtrlPoints(size_over_life_ctrl_pts);
	dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->SetCtrlPoints(mass_over_life_ctrl_pts);
	dialog_->Control<UIPolylineEditBox>(id_transparency_over_life_)->SetCtrlPoints(transparency_over_life_ctrl_pts);
}

void ParticleEditorApp::SaveParticleSystem(std::string const & name)
{
	KlayGE::XMLDocument doc;

	XMLNodePtr root = doc.AllocNode(XNT_Element, "particle_system");
	doc.RootNode(root);

	{
		XMLNodePtr particle_node = doc.AllocNode(XNT_Element, "particle");
		{
			XMLNodePtr alpha_node = doc.AllocNode(XNT_Element, "alpha");
			alpha_node->AppendAttrib(doc.AllocAttribString("from", particle_alpha_from_tex_));
			alpha_node->AppendAttrib(doc.AllocAttribString("to", particle_alpha_to_tex_));
			particle_node->AppendNode(alpha_node);
		}
		{
			XMLNodePtr color_node = doc.AllocNode(XNT_Element, "color");
			{
				{
					std::ostringstream ss;
					ss << particle_color_from_.r() << ' ' << particle_color_from_.g() << ' ' << particle_color_from_.b();
					color_node->AppendAttrib(doc.AllocAttribString("from", ss.str()));
				}
				{
					std::ostringstream ss;
					ss << particle_color_to_.r() << ' ' << particle_color_to_.g() << ' ' << particle_color_to_.b();
					color_node->AppendAttrib(doc.AllocAttribString("to", ss.str()));
				}
			}
			particle_node->AppendNode(color_node);
		}
		root->AppendNode(particle_node);
	}

	{
		XMLNodePtr emitter_node = doc.AllocNode(XNT_Element, "emitter");
		emitter_node->AppendAttrib(doc.AllocAttribString("type", particle_emitter_->Type()));

		{
			XMLNodePtr freq_node = doc.AllocNode(XNT_Element, "frequency");
			freq_node->AppendAttrib(doc.AllocAttribFloat("value", particle_emitter_->Frequency()));
			emitter_node->AppendNode(freq_node);
		}
		{
			XMLNodePtr angle_node = doc.AllocNode(XNT_Element, "angle");
			angle_node->AppendAttrib(doc.AllocAttribInt("value", static_cast<int>(particle_emitter_->EmitAngle() * RAD2DEG + 0.5f)));
			emitter_node->AppendNode(angle_node);
		}
		{
			XMLNodePtr pos_node = doc.AllocNode(XNT_Element, "pos");
			{
				std::ostringstream ss;
				ss << particle_emitter_->MinPosition().x() << ' ' << particle_emitter_->MinPosition().y() << ' ' << particle_emitter_->MinPosition().z();
				pos_node->AppendAttrib(doc.AllocAttribString("min", ss.str()));
			}
			{
				std::ostringstream ss;
				ss << particle_emitter_->MaxPosition().x() << ' ' << particle_emitter_->MaxPosition().y() << ' ' << particle_emitter_->MaxPosition().z();
				pos_node->AppendAttrib(doc.AllocAttribString("max", ss.str()));
			}
			emitter_node->AppendNode(pos_node);
		}		
		{
			XMLNodePtr vel_node = doc.AllocNode(XNT_Element, "vel");
			vel_node->AppendAttrib(doc.AllocAttribFloat("min", particle_emitter_->MinVelocity()));
			vel_node->AppendAttrib(doc.AllocAttribFloat("max", particle_emitter_->MaxVelocity()));
			emitter_node->AppendNode(vel_node);
		}
		{
			XMLNodePtr life_node = doc.AllocNode(XNT_Element, "life");
			life_node->AppendAttrib(doc.AllocAttribFloat("min", particle_emitter_->MinLife()));
			life_node->AppendAttrib(doc.AllocAttribFloat("max", particle_emitter_->MaxLife()));
			emitter_node->AppendNode(life_node);
		}
		root->AppendNode(emitter_node);
	}

	{
		XMLNodePtr updater_node = doc.AllocNode(XNT_Element, "updater");
		updater_node->AppendAttrib(doc.AllocAttribString("type", particle_updater_->Type()));

		XMLNodePtr size_over_life_node = doc.AllocNode(XNT_Element, "curve");
		size_over_life_node->AppendAttrib(doc.AllocAttribString("name", "size_over_life"));
		for (size_t i = 0; i < dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->NumCtrlPoints(); ++ i)
		{
			float2 const & pt = dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->GetCtrlPoint(i);

			XMLNodePtr ctrl_point_node = doc.AllocNode(XNT_Element, "ctrl_point");
			ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("x", pt.x()));
			ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("y", pt.y()));

			size_over_life_node->AppendNode(ctrl_point_node);
		}
		updater_node->AppendNode(size_over_life_node);

		XMLNodePtr mass_over_life_node = doc.AllocNode(XNT_Element, "curve");
		mass_over_life_node->AppendAttrib(doc.AllocAttribString("name", "mass_over_life"));
		for (size_t i = 0; i < dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->NumCtrlPoints(); ++ i)
		{
			float2 const & pt = dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->GetCtrlPoint(i);

			XMLNodePtr ctrl_point_node = doc.AllocNode(XNT_Element, "ctrl_point");
			ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("x", pt.x()));
			ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("y", pt.y()));

			mass_over_life_node->AppendNode(ctrl_point_node);
		}
		updater_node->AppendNode(mass_over_life_node);

		XMLNodePtr transparency_over_life_node = doc.AllocNode(XNT_Element, "curve");
		transparency_over_life_node->AppendAttrib(doc.AllocAttribString("name", "transparency_over_life"));
		for (size_t i = 0; i < dialog_->Control<UIPolylineEditBox>(id_transparency_over_life_)->NumCtrlPoints(); ++ i)
		{
			float2 const & pt = dialog_->Control<UIPolylineEditBox>(id_transparency_over_life_)->GetCtrlPoint(i);

			XMLNodePtr ctrl_point_node = doc.AllocNode(XNT_Element, "ctrl_point");
			ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("x", pt.x()));
			ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("y", pt.y()));

			transparency_over_life_node->AppendNode(ctrl_point_node);
		}
		updater_node->AppendNode(transparency_over_life_node);

		root->AppendNode(updater_node);
	}

	std::ofstream ofs(name.c_str());
	doc.Print(ofs);
}

class particle_cmp
{
public:
	bool operator()(std::pair<int, float> const & lhs, std::pair<int, float> const & rhs) const
	{
		return lhs.second > rhs.second;
	}
};

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

	switch (pass)
	{
	case 0:
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

			checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->SizeOverLife(dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->GetCtrlPoints());
			checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->MassOverLife(dialog_->Control<UIPolylineEditBox>(id_mass_over_life_)->GetCtrlPoints());
			checked_pointer_cast<PolylineParticleUpdater>(particle_updater_)->TransparencyOverLife(dialog_->Control<UIPolylineEditBox>(id_transparency_over_life_)->GetCtrlPoints());

			terrain_->Visible(true);
			particles_->Visible(false);
		}
		return App3DFramework::URV_Need_Flush;

	default:
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);

		copy_pp_->Apply();

		float4x4 mat = MathLib::translation(0.0f, 0.1f, 0.0f);
		particle_emitter_->ModelMatrix(mat);

		ps_->Update(this->AppTime(), this->FrameTime());

		float4x4 view_mat = Context::Instance().AppInstance().ActiveCamera().ViewMatrix();
		std::vector<std::pair<int, float> > active_particles;
		for (uint32_t i = 0; i < ps_->NumParticles(); ++ i)
		{
			if (ps_->GetParticle(i).life > 0)
			{
				float3 pos = ps_->GetParticle(i).pos;
				float p_to_v = (pos.x() * view_mat(0, 2) + pos.y() * view_mat(1, 2) + pos.z() * view_mat(2, 2) + view_mat(3, 2))
					/ (pos.x() * view_mat(0, 3) + pos.y() * view_mat(1, 3) + pos.z() * view_mat(2, 3) + view_mat(3, 3));

				active_particles.push_back(std::make_pair(i, p_to_v));
			}
		}
		if (!active_particles.empty())
		{
			std::sort(active_particles.begin(), active_particles.end(), particle_cmp());

			uint32_t const num_pars = static_cast<uint32_t>(active_particles.size());
			RenderLayoutPtr const & rl = particles_->GetRenderable()->GetRenderLayout();
			GraphicsBufferPtr instance_gb;
			if (use_gs)
			{
				instance_gb = rl->GetVertexStream(0);
			}
			else
			{
				instance_gb = rl->InstanceStream();

				for (uint32_t i = 0; i < rl->NumVertexStreams(); ++ i)
				{
					rl->VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, num_pars);
				}
			}

			instance_gb->Resize(sizeof(ParticleInstance) * num_pars);
			{
				GraphicsBuffer::Mapper mapper(*instance_gb, BA_Write_Only);
				ParticleInstance* instance_data = mapper.Pointer<ParticleInstance>();
				for (uint32_t i = 0; i < num_pars; ++ i, ++ instance_data)
				{
					Particle const & par = ps_->GetParticle(active_particles[i].first);
					instance_data->pos = par.pos;
					instance_data->life = par.life;
					instance_data->spin = par.spin;
					instance_data->size = par.size;
					instance_data->life_factor = (par.init_life - par.life) / par.init_life;
					instance_data->alpha = par.alpha;
				}
			}

			particles_->Visible(true);
		}
		terrain_->Visible(false);

		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
