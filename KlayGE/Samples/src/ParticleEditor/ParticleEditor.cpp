#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
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
#include <KlayGE/XMLDom.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#ifdef Bool
#undef Bool		// foreach
#endif

#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 4512)
#endif
#include <boost/random.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/typeof/typeof.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "ParticleEditor.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	bool use_gs = false;

	class TerrainRenderable : public RenderableHelper
	{
	public:
		TerrainRenderable()
			: RenderableHelper(L"Terrain")
		{
			BOOST_AUTO(grass, LoadTexture("grass.dds", EAH_GPU_Read));

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("ParticleEditor.fxml")->TechniqueByName("Terrain");

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
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			box_ = MathLib::compute_bounding_box<float>(vertices, vertices + sizeof(vertices) / sizeof(vertices[0]));

			*(technique_->Effect().ParameterByName("grass_tex")) = grass();
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*(technique_->Effect().ParameterByName("depth_min")) = camera.NearPlane();
			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / (camera.FarPlane() - camera.NearPlane());
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

	int const NUM_PARTICLE = 16384;

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

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, NULL);
				rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));

				technique_ = rf.LoadEffect("ParticleEditor.fxml")->TechniqueByName("ParticleWithGS");
			}
			else
			{
				rl_->TopologyType(RenderLayout::TT_TriangleStrip);

				ElementInitData init_data;
				init_data.row_pitch = sizeof(texs);
				init_data.slice_pitch = 0;
				init_data.data = texs;
				GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
				rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, NULL);
				rl_->BindVertexStream(pos_vb,
					boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_ABGR32F)),
					RenderLayout::ST_Instance);

				init_data.row_pitch = sizeof(indices);
				init_data.slice_pitch = 0;
				init_data.data = indices;
				GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
				rl_->BindIndexStream(ib, EF_R16UI);

				technique_ = rf.LoadEffect("ParticleEditor.fxml")->TechniqueByName("Particle");
			}

			*(technique_->Effect().ParameterByName("point_radius")) = 0.08f;
		}

		void SceneTexture(TexturePtr const tex, bool flip)
		{
			*(technique_->Effect().ParameterByName("scene_tex")) = tex;
			*(technique_->Effect().ParameterByName("flip")) = static_cast<int32_t>(flip ? -1 : 1);
		}

		void ParticleTexture(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("particle_tex")) = tex;
		}

		void SetInitLife(float life)
		{
			*(technique_->Effect().ParameterByName("init_life")) = life;
		}

		void SetSizeOverLife(std::vector<float2> const & size_over_life)
		{
			*(technique_->Effect().ParameterByName("size_over_life")) = size_over_life;
			*(technique_->Effect().ParameterByName("num_size_over_life")) = static_cast<int32_t>(size_over_life.size());
		}

		void SetWeightOverLife(std::vector<float2> const & weight_over_life)
		{
			*(technique_->Effect().ParameterByName("weight_over_life")) = weight_over_life;
			*(technique_->Effect().ParameterByName("num_weight_over_life")) = static_cast<int32_t>(weight_over_life.size());
		}

		void SetTransparencyOverLife(std::vector<float2> const & transparency_over_life)
		{
			*(technique_->Effect().ParameterByName("transparency_over_life")) = transparency_over_life;
			*(technique_->Effect().ParameterByName("num_transparency_over_life")) = static_cast<int32_t>(transparency_over_life.size());
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();
			float4x4 const inv_proj = MathLib::inverse(proj);

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset() * 2;
			float const x_offset = texel_to_pixel.x() / re.CurFrameBuffer()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurFrameBuffer()->Height();
			*(technique_->Effect().ParameterByName("offset")) = float2(x_offset, y_offset);

			*(technique_->Effect().ParameterByName("upper_left")) = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("upper_right")) = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_left")) = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_right")) = MathLib::transform_coord(float3(1, -1, 1), inv_proj);

			*(technique_->Effect().ParameterByName("depth_min")) = camera.NearPlane();
			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / (camera.FarPlane() - camera.NearPlane());
		}
	};

	class ParticlesObject : public SceneObjectHelper
	{
	public:
		ParticlesObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = MakeSharedPtr<RenderParticles>();
		}

		void ParticleTexture(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleTexture(tex);
		}

		void SetInitLife(float life)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->SetInitLife(life);
		}

		void SetSizeOverLife(std::vector<float2> const & size_over_life)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->SetSizeOverLife(size_over_life);
		}

		void SetWeightOverLife(std::vector<float2> const & weight_over_life)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->SetWeightOverLife(weight_over_life);
		}

		void SetTransparencyOverLife(std::vector<float2> const & transparency_over_life)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->SetTransparencyOverLife(transparency_over_life);
		}
	};


	template <typename ParticleType>
	class GenParticle
	{
	public:
		GenParticle()
			: random_gen_(boost::lagged_fibonacci607(), boost::uniform_real<float>(-1, 1)),
				angle_(PI / 3), life_(3)
		{
		}

		void SetAngle(float angle)
		{
			angle_ = angle;
		}
		float GetAngle() const
		{
			return angle_;
		}

		void SetLife(float life)
		{
			life_ = life;
		}
		float GetLife() const
		{
			return life_;
		}

		void InitVelocity(float vel)
		{
			velocity_ = vel;
		}

		void operator()(ParticleType& par, float4x4 const & mat)
		{
			par.pos = MathLib::transform_coord(float3(0, 0, 0), mat);
			float theta = random_gen_() * PI;
			float phi = abs(random_gen_()) * angle_ / 2;
			float velocity = (random_gen_() + 1) * velocity_;
			float x = cos(theta) * sin(phi);
			float z = sin(theta) * sin(phi);
			float y = cos(phi);
			par.vel = MathLib::transform_normal(float3(x, y, z) * velocity, mat);
			par.life = life_;
		}

	private:
		boost::variate_generator<boost::lagged_fibonacci607, boost::uniform_real<float> > random_gen_;
		float angle_;
		float life_;
		float velocity_;
	};

	GenParticle<Particle> gen_particle;


	template <typename ParticleType>
	class UpdateParticle
	{
	public:
		UpdateParticle()
			: gravity_(0.1f),
				force_(0, 0, 0),
				media_density_(0.0f)
		{
		}

		void SetInitLife(float life)
		{
			init_life_ = life;
		}

		void SetForce(float3 force)
		{
			force_ = force;
		}

		void MediaDensity(float density)
		{
			media_density_ = density;
		}

		void SizeOverLifeCtrl(UIPolylineEditBox* size_over_life)
		{
			size_over_life_ = size_over_life;
		}

		void operator()(ParticleType& par, float elapse_time)
		{
			buoyancy_ = media_density_ * (size_over_life_->GetValue((init_life_ - par.life) / init_life_) * 2);
			par.vel += (force_ + float3(0, buoyancy_ - gravity_, 0)) * elapse_time;
			par.pos += par.vel * elapse_time;
			par.life -= elapse_time;

			if (par.pos.y() <= 0)
			{
				par.pos.y() += 0.01f;
				par.vel = MathLib::reflect(par.vel, float3(0, 1, 0)) * 0.8f;
			}
		}

	private:
		float init_life_;
		float gravity_;
		float buoyancy_;
		float3 force_;
		float media_density_;

		UIPolylineEditBox* size_over_life_;
	};

	UpdateParticle<Particle> update_particle;


	class CopyPostProcess : public PostProcess
	{
	public:
		CopyPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("ParticleEditor.fxml")->TechniqueByName("Copy"))
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
	ResLoader::Instance().AddPath("../Samples/media/ParticleEditor");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	Context::Instance().SceneManagerInstance(SceneManager::NullObject());

	ParticleEditorApp app("Particle Editor", settings);
	app.Create();
	app.Run();

	return 0;
}

ParticleEditorApp::ParticleEditorApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
}

void ParticleEditorApp::InitObjects()
{
	use_gs = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps().gs_support;

	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	this->LookAt(float3(-1.2f, 0.5f, -1.2f), float3(0, 0.5f, 0));
	this->Proj(0.01f, 100);

	fpsController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&ParticleEditorApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	particles_ = MakeSharedPtr<ParticlesObject>();
	particles_->AddToSceneManager();

	terrain_ = MakeSharedPtr<TerrainObject>();
	terrain_->AddToSceneManager();

	ps_ = MakeSharedPtr<ParticleSystem<Particle> >(NUM_PARTICLE, boost::bind(&GenParticle<Particle>::operator(), &gen_particle, _1, _2),
		boost::bind(&UpdateParticle<Particle>::operator(), &update_particle, _1, _2));

	ps_->AutoEmit(256);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	scene_buffer_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	scene_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;

	copy_pp_ = MakeSharedPtr<CopyPostProcess>();

	UIManager::Instance().Load(ResLoader::Instance().Load("ParticleEditor.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_open_ = dialog_->IDFromName("Open");
	id_save_as_ = dialog_->IDFromName("SaveAs");
	id_angle_static_ = dialog_->IDFromName("AngleStatic");
	id_angle_slider_ = dialog_->IDFromName("AngleSlider");
	id_life_static_ = dialog_->IDFromName("LifeStatic");
	id_life_slider_ = dialog_->IDFromName("LifeSlider");
	id_density_static_ = dialog_->IDFromName("DensityStatic");
	id_density_slider_ = dialog_->IDFromName("DensitySlider");
	id_velocity_static_ = dialog_->IDFromName("VelocityStatic");
	id_velocity_slider_ = dialog_->IDFromName("VelocitySlider");
	id_fps_camera_ = dialog_->IDFromName("FPSCamera");
	id_particle_tex_button_ = dialog_->IDFromName("ParticleTexButton");
	id_size_over_life_ = dialog_->IDFromName("SizeOverLifePolyline");
	id_weight_over_life_ = dialog_->IDFromName("WeightOverLifePolyline");
	id_transparency_over_life_ = dialog_->IDFromName("TransparencyOverLifePolyline");

	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::OpenHandler, this, _1));
	dialog_->Control<UIButton>(id_save_as_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::SaveAsHandler, this, _1));

	dialog_->Control<UISlider>(id_angle_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::AngleChangedHandler, this, _1));
	this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));
	dialog_->Control<UISlider>(id_life_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::LifeChangedHandler, this, _1));
	this->LifeChangedHandler(*dialog_->Control<UISlider>(id_life_slider_));
	dialog_->Control<UISlider>(id_density_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::DensityChangedHandler, this, _1));
	this->DensityChangedHandler(*dialog_->Control<UISlider>(id_density_slider_));
	dialog_->Control<UISlider>(id_velocity_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::VelocityChangedHandler, this, _1));
	this->VelocityChangedHandler(*dialog_->Control<UISlider>(id_velocity_slider_));

	dialog_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(boost::bind(&ParticleEditorApp::FPSCameraHandler, this, _1));

	dialog_->Control<UITexButton>(id_particle_tex_button_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::ChangeParticleTexHandler, this, _1));

	this->LoadParticleSystem(ResLoader::Instance().Locate("Fire.psml"));

	update_particle.SizeOverLifeCtrl(dialog_->Control<UIPolylineEditBox>(id_size_over_life_).get());
}

void ParticleEditorApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	RenderViewPtr ds_view = rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0);

	scene_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	scene_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_tex_, 0, 0));
	scene_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	checked_pointer_cast<RenderParticles>(particles_->GetRenderable())->SceneTexture(scene_tex_, scene_buffer_->RequiresFlipping());

	copy_pp_->Source(scene_tex_, scene_buffer_->RequiresFlipping());
	copy_pp_->Destinate(FrameBufferPtr());

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
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
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
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ".psml";

	if (GetSaveFileNameA(&ofn))
	{
		this->SaveParticleSystem(fn);
	}
#endif
}

void ParticleEditorApp::AngleChangedHandler(KlayGE::UISlider const & sender)
{
	float angle = static_cast<float>(sender.GetValue());
	gen_particle.SetAngle(angle * DEG2RAD);

	std::wostringstream stream;
	stream << angle;
	dialog_->Control<UIStatic>(id_angle_static_)->SetText(stream.str());
}

void ParticleEditorApp::LifeChangedHandler(KlayGE::UISlider const & sender)
{
	init_life_ = static_cast<float>(sender.GetValue());
	gen_particle.SetLife(init_life_);
	update_particle.SetInitLife(init_life_);
	checked_pointer_cast<ParticlesObject>(particles_)->SetInitLife(init_life_);

	std::wostringstream stream;
	stream << init_life_;
	dialog_->Control<UIStatic>(id_life_static_)->SetText(stream.str());
}

void ParticleEditorApp::DensityChangedHandler(KlayGE::UISlider const & sender)
{
	float density = sender.GetValue() / 100.0f;
	update_particle.MediaDensity(density);

	std::wostringstream stream;
	stream << density;
	dialog_->Control<UIStatic>(id_density_static_)->SetText(stream.str());
}

void ParticleEditorApp::VelocityChangedHandler(KlayGE::UISlider const & sender)
{
	float velocity = sender.GetValue() / 100.0f;
	gen_particle.InitVelocity(velocity);

	std::wostringstream stream;
	stream << velocity;
	dialog_->Control<UIStatic>(id_velocity_static_)->SetText(stream.str());
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

void ParticleEditorApp::ChangeParticleTexHandler(KlayGE::UITexButton const & /*sender*/)
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
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		particle_tex_ = fn;
		this->LoadParticleTex(fn);
	}
#endif
}

void ParticleEditorApp::LoadParticleTex(std::string const & name)
{
	TexturePtr tex = LoadTexture(name, EAH_GPU_Read)();
	dialog_->Control<UITexButton>(id_particle_tex_button_)->SetTexture(tex);
	checked_pointer_cast<ParticlesObject>(particles_)->ParticleTexture(tex);
}

void ParticleEditorApp::LoadParticleSystem(std::string const & name)
{
	dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->ClearCtrlPoints();
	dialog_->Control<UIPolylineEditBox>(id_weight_over_life_)->ClearCtrlPoints();
	dialog_->Control<UIPolylineEditBox>(id_transparency_over_life_)->ClearCtrlPoints();

	using boost::lexical_cast;

	ResIdentifierPtr ifs = ResLoader::Instance().Load(name.c_str());

	KlayGE::XMLDocument doc;
	XMLNodePtr root = doc.Parse(ifs);

	XMLAttributePtr attr = root->Attrib("particle_tex");
	particle_tex_ = attr->ValueString();
	this->LoadParticleTex(ResLoader::Instance().Locate(particle_tex_));

	attr = root->Attrib("emit_angle");
	dialog_->Control<UISlider>(id_angle_slider_)->SetValue(attr->ValueInt());
	this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));

	attr = root->Attrib("life");
	dialog_->Control<UISlider>(id_life_slider_)->SetValue(attr->ValueInt());
	this->LifeChangedHandler(*dialog_->Control<UISlider>(id_life_slider_));

	attr = root->Attrib("media_density");
	dialog_->Control<UISlider>(id_density_slider_)->SetValue(static_cast<int>(attr->ValueFloat() * 100.0f + 0.5f));
	this->DensityChangedHandler(*dialog_->Control<UISlider>(id_density_slider_));

	attr = root->Attrib("velocity");
	dialog_->Control<UISlider>(id_velocity_slider_)->SetValue(static_cast<int>(attr->ValueFloat() * 100.0f + 0.5f));
	this->VelocityChangedHandler(*dialog_->Control<UISlider>(id_velocity_slider_));

	for (XMLNodePtr node = root->FirstNode("curve"); node; node = node->NextSibling("curve"))
	{
		std::vector<float2> xys;
		for (XMLNodePtr ctrl_point_node = node->FirstNode("ctrl_point"); ctrl_point_node; ctrl_point_node = ctrl_point_node->NextSibling("ctrl_point"))
		{
			XMLAttributePtr attr_x = ctrl_point_node->Attrib("x");
			XMLAttributePtr attr_y = ctrl_point_node->Attrib("y");

			xys.push_back(float2(attr_x->ValueFloat(), attr_y->ValueFloat()));
		}

		attr = node->Attrib("name");
		if ("size_over_life" == attr->ValueString())
		{
			dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->SetCtrlPoints(xys);
		}
		if ("weight_over_life" == attr->ValueString())
		{
			dialog_->Control<UIPolylineEditBox>(id_weight_over_life_)->SetCtrlPoints(xys);
		}
		if ("transparency_over_life" == attr->ValueString())
		{
			dialog_->Control<UIPolylineEditBox>(id_transparency_over_life_)->SetCtrlPoints(xys);
		}
	}
}

void ParticleEditorApp::SaveParticleSystem(std::string const & name)
{
	using boost::lexical_cast;

	KlayGE::XMLDocument doc;

	XMLNodePtr root = doc.AllocNode(XNT_Element, "particle_system");
	doc.RootNode(root);

	root->AppendAttrib(doc.AllocAttribString("particle_tex", particle_tex_));

	float angle = static_cast<float>(dialog_->Control<UISlider>(id_angle_slider_)->GetValue());
	root->AppendAttrib(doc.AllocAttribFloat("emit_angle", angle));

	float life = static_cast<float>(dialog_->Control<UISlider>(id_life_slider_)->GetValue());
	root->AppendAttrib(doc.AllocAttribFloat("life", life));

	float density = dialog_->Control<UISlider>(id_density_slider_)->GetValue() / 100.0f;
	root->AppendAttrib(doc.AllocAttribFloat("media_density", density));

	float velocity = dialog_->Control<UISlider>(id_velocity_slider_)->GetValue() / 100.0f;
	root->AppendAttrib(doc.AllocAttribFloat("velocity", velocity));

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
	root->AppendNode(size_over_life_node);

	XMLNodePtr weight_over_life_node = doc.AllocNode(XNT_Element, "curve");
	weight_over_life_node->AppendAttrib(doc.AllocAttribString("name", "weight_over_life"));
	for (size_t i = 0; i < dialog_->Control<UIPolylineEditBox>(id_weight_over_life_)->NumCtrlPoints(); ++ i)
	{
		float2 const & pt = dialog_->Control<UIPolylineEditBox>(id_weight_over_life_)->GetCtrlPoint(i);

		XMLNodePtr ctrl_point_node = doc.AllocNode(XNT_Element, "ctrl_point");
		ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("x", pt.x()));
		ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("y", pt.y()));

		weight_over_life_node->AppendNode(ctrl_point_node);
	}
	root->AppendNode(weight_over_life_node);

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
	root->AppendNode(transparency_over_life_node);

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
	stream << this->FPS();

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
		re.BindFrameBuffer(scene_buffer_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

		checked_pointer_cast<ParticlesObject>(particles_)->SetSizeOverLife(dialog_->Control<UIPolylineEditBox>(id_size_over_life_)->GetCtrlPoints());
		checked_pointer_cast<ParticlesObject>(particles_)->SetWeightOverLife(dialog_->Control<UIPolylineEditBox>(id_weight_over_life_)->GetCtrlPoints());
		checked_pointer_cast<ParticlesObject>(particles_)->SetTransparencyOverLife(dialog_->Control<UIPolylineEditBox>(id_transparency_over_life_)->GetCtrlPoints());

		terrain_->Visible(true);
		particles_->Visible(false);
		return App3DFramework::URV_Need_Flush;

	default:
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);

		copy_pp_->Apply();

		float4x4 mat = MathLib::translation(0.0f, 0.1f, 0.0f);
		ps_->ModelMatrix(mat);

		ps_->Update(static_cast<float>(timer_.elapsed()));
		timer_.restart();

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

			instance_gb->Resize(sizeof(float4) * num_pars);
			{
				GraphicsBuffer::Mapper mapper(*instance_gb, BA_Write_Only);
				float4* instance_data = mapper.Pointer<float4>();
				for (uint32_t i = 0; i < num_pars; ++ i, ++ instance_data)
				{
					instance_data->x() = ps_->GetParticle(active_particles[i].first).pos.x();
					instance_data->y() = ps_->GetParticle(active_particles[i].first).pos.y();
					instance_data->z() = ps_->GetParticle(active_particles[i].first).pos.z();
					instance_data->w() = ps_->GetParticle(active_particles[i].first).life;
				}
			}

			particles_->Visible(true);
		}
		terrain_->Visible(false);

		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
