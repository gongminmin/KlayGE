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
#pragma warning(disable: 4127 4512 6326 6385)
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

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, NULL);
				rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F),
					vertex_element(VEU_TextureCoord, 0, EF_R32F)));

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
					boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_ABGR32F),
						vertex_element(VEU_TextureCoord, 1, EF_R32F)),
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

		void ParticleAlphaFrom(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("particle_alpha_from_tex")) = tex;
		}

		void ParticleAlphaTo(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("particle_alpha_to_tex")) = tex;
		}

		void ParticleColorFrom(float4 const & clr)
		{
			*(technique_->Effect().ParameterByName("particle_color_from")) = clr;
		}

		void ParticleColorTo(float4 const & clr)
		{
			*(technique_->Effect().ParameterByName("particle_color_to")) = clr;
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

		void ParticleAlphaFrom(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleAlphaFrom(tex);
		}

		void ParticleAlphaTo(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleAlphaTo(tex);
		}

		void ParticleColorFrom(float4 const & clr)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleColorFrom(clr);
		}

		void ParticleColorTo(float4 const & clr)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleColorTo(clr);
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

		void SetMaxPositionDeviation(float3 const & dev)
		{
			max_position_deviation_ = dev;
		}
		float3 const & GetMaxPositionDeviation() const
		{
			return max_position_deviation_;
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

		void InitMinVelocity(float min_vel)
		{
			min_velocity_ = min_vel;
		}
		void InitMaxVelocity(float max_vel)
		{
			max_velocity_ = max_vel;
		}

		void operator()(ParticleType& par, float4x4 const & mat)
		{
			float x = random_gen_() * max_position_deviation_.x();
			float y = random_gen_() * max_position_deviation_.y();
			float z = random_gen_() * max_position_deviation_.z();
			par.pos = MathLib::transform_coord(float3(x, y, z), mat);
			float theta = random_gen_() * PI;
			float phi = abs(random_gen_()) * angle_ / 2;
			float velocity = (random_gen_() * 0.5f + 0.5f) * (max_velocity_ - min_velocity_) + min_velocity_;
			float vx = cos(theta) * sin(phi);
			float vz = sin(theta) * sin(phi);
			float vy = cos(phi);
			par.vel = MathLib::transform_normal(float3(vx, vy, vz) * velocity, mat);
			par.life = life_;
			par.spin = random_gen_() * PI / 2;
		}

	private:
		boost::variate_generator<boost::lagged_fibonacci607, boost::uniform_real<float> > random_gen_;

		float3 max_position_deviation_;
		float angle_;

		float min_velocity_;
		float max_velocity_;

		float life_;
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

		void WeightOverLifeCtrl(UIPolylineEditBox* weight_over_life)
		{
			weight_over_life_ = weight_over_life;
		}

		void operator()(ParticleType& par, float elapse_time)
		{
			float buoyancy = media_density_ * size_over_life_->GetValue((init_life_ - par.life) / init_life_);
			float cur_weight = weight_over_life_->GetValue((init_life_ - par.life) / init_life_);
			float3 accel = (force_ + float3(0, buoyancy, 0)) / cur_weight - float3(0, gravity_, 0);
			par.vel += accel * elapse_time;
			par.pos += par.vel * elapse_time;
			par.life -= elapse_time;
			par.spin += 0.001f;

			if (par.pos.y() <= 0)
			{
				par.pos.y() = 0.01f;
				par.vel = MathLib::reflect(par.vel, float3(0, 1, 0)) * 0.8f;
			}
		}

	private:
		float init_life_;
		float gravity_;
		float3 force_;
		float media_density_;

		UIPolylineEditBox* size_over_life_;
		UIPolylineEditBox* weight_over_life_;
	};

	UpdateParticle<Particle> update_particle;


	enum
	{
		Exit
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape)
	};
}


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	ParticleEditorApp app;
	app.Create();
	app.Run();

	return 0;
}

ParticleEditorApp::ParticleEditorApp()
					: App3DFramework("Particle Editor")
{
	ResLoader::Instance().AddPath("../Samples/media/ParticleEditor");
}

bool ParticleEditorApp::ConfirmDevice() const
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

	update_particle.MediaDensity(0.5f);

	ps_ = MakeSharedPtr<ParticleSystem<Particle> >(NUM_PARTICLE, boost::bind(&GenParticle<Particle>::operator(), &gen_particle, _1, _2),
		boost::bind(&UpdateParticle<Particle>::operator(), &update_particle, _1, _2));

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	scene_buffer_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	scene_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;

	copy_pp_ = LoadPostProcess(ResLoader::Instance().Load("Copy.ppml"), "copy");

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
	id_min_velocity_static_ = dialog_->IDFromName("MinVelocityStatic");
	id_min_velocity_slider_ = dialog_->IDFromName("MinVelocitySlider");
	id_max_velocity_static_ = dialog_->IDFromName("MaxVelocityStatic");
	id_max_velocity_slider_ = dialog_->IDFromName("MaxVelocitySlider");
	id_fps_camera_ = dialog_->IDFromName("FPSCamera");
	id_particle_alpha_from_button_ = dialog_->IDFromName("ParticleAlphaFromButton");
	id_particle_alpha_to_button_ = dialog_->IDFromName("ParticleAlphaToButton");
	id_particle_color_from_button_ = dialog_->IDFromName("ParticleColorFromButton");
	id_particle_color_to_button_ = dialog_->IDFromName("ParticleColorToButton");
	id_curve_type_ = dialog_->IDFromName("CurveTypeCombo");
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
	dialog_->Control<UISlider>(id_min_velocity_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::MinVelocityChangedHandler, this, _1));
	this->MinVelocityChangedHandler(*dialog_->Control<UISlider>(id_min_velocity_slider_));
	dialog_->Control<UISlider>(id_max_velocity_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::MaxVelocityChangedHandler, this, _1));
	this->MaxVelocityChangedHandler(*dialog_->Control<UISlider>(id_max_velocity_slider_));

	dialog_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(boost::bind(&ParticleEditorApp::FPSCameraHandler, this, _1));

	dialog_->Control<UITexButton>(id_particle_alpha_from_button_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::ChangeParticleAlphaFromHandler, this, _1));
	dialog_->Control<UITexButton>(id_particle_alpha_to_button_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::ChangeParticleAlphaToHandler, this, _1));
	dialog_->Control<UITexButton>(id_particle_color_from_button_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::ChangeParticleColorFromHandler, this, _1));
	dialog_->Control<UITexButton>(id_particle_color_to_button_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::ChangeParticleColorToHandler, this, _1));

	dialog_->Control<UIComboBox>(id_curve_type_)->OnSelectionChangedEvent().connect(boost::bind(&ParticleEditorApp::CurveTypeChangedHandler, this, _1));

	this->LoadParticleSystem(ResLoader::Instance().Locate("Fire.psml"));

	update_particle.SizeOverLifeCtrl(dialog_->Control<UIPolylineEditBox>(id_size_over_life_).get());
	update_particle.WeightOverLifeCtrl(dialog_->Control<UIPolylineEditBox>(id_weight_over_life_).get());
}

void ParticleEditorApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	RenderViewPtr ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);

	scene_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	scene_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_tex_, 0, 0));
	scene_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	checked_pointer_cast<RenderParticles>(particles_->GetRenderable())->SceneTexture(scene_tex_, scene_buffer_->RequiresFlipping());

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

void ParticleEditorApp::MinVelocityChangedHandler(KlayGE::UISlider const & sender)
{
	float velocity = sender.GetValue() / 100.0f;
	gen_particle.InitMinVelocity(velocity);

	std::wostringstream stream;
	stream << velocity;
	dialog_->Control<UIStatic>(id_min_velocity_static_)->SetText(stream.str());
}

void ParticleEditorApp::MaxVelocityChangedHandler(KlayGE::UISlider const & sender)
{
	float velocity = sender.GetValue() / 100.0f;
	gen_particle.InitMaxVelocity(velocity);

	std::wostringstream stream;
	stream << velocity;
	dialog_->Control<UIStatic>(id_max_velocity_static_)->SetText(stream.str());
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
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
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
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
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

	ZeroMemory(&occ, sizeof(occ));
	occ.lStructSize = sizeof(occ);
	occ.hwndOwner = hwnd;
	occ.hInstance = NULL;
	occ.rgbResult = particle_color_from_.ABGR();
	occ.lpCustColors = cust_clrs;
	occ.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	occ.lCustData = 0;
	occ.lpfnHook = NULL;
	occ.lpTemplateName = NULL;

	if (ChooseColorA(&occ))
	{
		particle_color_from_ = Color(occ.rgbResult);
		std::swap(particle_color_from_.r(), particle_color_from_.b());
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

	ZeroMemory(&occ, sizeof(occ));
	occ.lStructSize = sizeof(occ);
	occ.hwndOwner = hwnd;
	occ.hInstance = NULL;
	occ.rgbResult = particle_color_to_.ABGR();
	occ.lpCustColors = cust_clrs;
	occ.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	occ.lCustData = 0;
	occ.lpfnHook = NULL;
	occ.lpTemplateName = NULL;

	if (ChooseColorA(&occ))
	{
		particle_color_to_ = Color(occ.rgbResult);
		std::swap(particle_color_to_.r(), particle_color_to_.b());
		this->LoadParticleColor(id_particle_color_to_button_, particle_color_to_);
	}
#endif
}

void ParticleEditorApp::CurveTypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	uint32_t ct = sender.GetSelectedIndex();
	dialog_->GetControl(id_size_over_life_)->SetVisible(false);
	dialog_->GetControl(id_weight_over_life_)->SetVisible(false);
	dialog_->GetControl(id_transparency_over_life_)->SetVisible(false);
	switch (ct)
	{
	case 0:
		dialog_->GetControl(id_size_over_life_)->SetVisible(true);
		break;

	case 1:
		dialog_->GetControl(id_weight_over_life_)->SetVisible(true);
		break;

	default:
		dialog_->GetControl(id_transparency_over_life_)->SetVisible(true);
		break;
	}
}

void ParticleEditorApp::LoadParticleAlpha(int id, std::string const & name)
{
	TexturePtr tex = LoadTexture(name, EAH_GPU_Read)();
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
		TexturePtr cpu_tex = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, EF_R8, 1, 0, EAH_CPU_Read, NULL);
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
			rf.RenderEngineInstance().DeviceCaps().argb8_support ? EF_ARGB8 : EF_ABGR8, 1, 0, EAH_GPU_Read, &init_data);
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
		checked_pointer_cast<ParticlesObject>(particles_)->ParticleColorFrom(float4(clr.r(), clr.g(), clr.b(), 1));
	}
	else
	{
		BOOST_ASSERT(id_particle_color_to_button_ == id);
		checked_pointer_cast<ParticlesObject>(particles_)->ParticleColorTo(float4(clr.r(), clr.g(), clr.b(), 1));
	}

	TexturePtr tex_for_button;

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	uint32_t data;
	data = 0xFF000000 | (rf.RenderEngineInstance().DeviceCaps().argb8_support ? clr.ARGB() : clr.ABGR());
	ElementInitData init_data;
	init_data.data = &data;
	init_data.row_pitch = 4;
	tex_for_button = rf.MakeTexture2D(1, 1, 1, 1,
		rf.RenderEngineInstance().DeviceCaps().argb8_support ? EF_ARGB8 : EF_ABGR8, 1, 0, EAH_GPU_Read, &init_data);

	dialog_->Control<UITexButton>(id)->SetTexture(tex_for_button);
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

	ps_->Frequency(root->Attrib("frequency")->ValueFloat());

	{
		XMLNodePtr particle_node = root->FirstNode("particle");
		{
			XMLNodePtr alpha_node = particle_node->FirstNode("alpha");
			{
				XMLNodePtr from_node = alpha_node->FirstNode("from");
				particle_alpha_from_tex_ = from_node->Attrib("tex")->ValueString();
				this->LoadParticleAlpha(id_particle_alpha_from_button_, ResLoader::Instance().Locate(particle_alpha_from_tex_));

				XMLNodePtr to_node = alpha_node->FirstNode("to");
				particle_alpha_to_tex_ = to_node->Attrib("tex")->ValueString();
				this->LoadParticleAlpha(id_particle_alpha_to_button_, ResLoader::Instance().Locate(particle_alpha_to_tex_));
			}
		}
		{
			XMLNodePtr color_node = particle_node->FirstNode("color");
			{
				XMLNodePtr from_node = color_node->FirstNode("from");
				particle_color_from_.r() = from_node->Attrib("r")->ValueFloat();
				particle_color_from_.g() = from_node->Attrib("g")->ValueFloat();
				particle_color_from_.b() = from_node->Attrib("b")->ValueFloat();
				this->LoadParticleColor(id_particle_color_from_button_, particle_color_from_);

				XMLNodePtr to_node = color_node->FirstNode("to");
				particle_color_to_.r() = to_node->Attrib("r")->ValueFloat();
				particle_color_to_.g() = to_node->Attrib("g")->ValueFloat();
				particle_color_to_.b() = to_node->Attrib("b")->ValueFloat();
				this->LoadParticleColor(id_particle_color_to_button_, particle_color_to_);
			}
		}
	}

	{
		XMLNodePtr emittor_node = root->FirstNode("emittor");

		XMLAttributePtr attr = emittor_node->Attrib("angle");
		dialog_->Control<UISlider>(id_angle_slider_)->SetValue(attr->ValueInt());
		this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));

		attr = emittor_node->Attrib("life");
		dialog_->Control<UISlider>(id_life_slider_)->SetValue(attr->ValueInt());
		this->LifeChangedHandler(*dialog_->Control<UISlider>(id_life_slider_));

		attr = emittor_node->Attrib("min_velocity");
		dialog_->Control<UISlider>(id_min_velocity_slider_)->SetValue(static_cast<int>(attr->ValueFloat() * 100.0f + 0.5f));
		this->MinVelocityChangedHandler(*dialog_->Control<UISlider>(id_min_velocity_slider_));

		attr = emittor_node->Attrib("max_velocity");
		dialog_->Control<UISlider>(id_max_velocity_slider_)->SetValue(static_cast<int>(attr->ValueFloat() * 100.0f + 0.5f));
		this->MaxVelocityChangedHandler(*dialog_->Control<UISlider>(id_max_velocity_slider_));
	}

	{
		XMLNodePtr node = root->FirstNode("max_position_deviation");
		float3 max_pos_dev(0, 0, 0);
		XMLAttributePtr attr = node->Attrib("x");
		if (attr)
		{
			max_pos_dev.x() = attr->ValueFloat();
		}
		attr = node->Attrib("y");
		if (attr)
		{
			max_pos_dev.y() = attr->ValueFloat();
		}
		attr = node->Attrib("z");
		if (attr)
		{
			max_pos_dev.z() = attr->ValueFloat();
		}
		gen_particle.SetMaxPositionDeviation(max_pos_dev);
	}

	for (XMLNodePtr node = root->FirstNode("curve"); node; node = node->NextSibling("curve"))
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

	root->AppendAttrib(doc.AllocAttribFloat("frequency", ps_->Frequency()));

	{
		XMLNodePtr particle_node = doc.AllocNode(XNT_Element, "particle");
		{
			XMLNodePtr alpha_node = doc.AllocNode(XNT_Element, "alpha");
			{
				XMLNodePtr from_node = doc.AllocNode(XNT_Element, "from");
				from_node->AppendAttrib(doc.AllocAttribString("tex", particle_alpha_from_tex_));
				alpha_node->AppendNode(from_node);

				XMLNodePtr to_node = doc.AllocNode(XNT_Element, "to");
				to_node->AppendAttrib(doc.AllocAttribString("tex", particle_alpha_to_tex_));
				alpha_node->AppendNode(to_node);
			}
			particle_node->AppendNode(alpha_node);
		}
		{
			XMLNodePtr color_node = doc.AllocNode(XNT_Element, "color");
			{
				XMLNodePtr from_node = doc.AllocNode(XNT_Element, "from");
				from_node->AppendAttrib(doc.AllocAttribFloat("r", particle_color_from_.r()));
				from_node->AppendAttrib(doc.AllocAttribFloat("g", particle_color_from_.g()));
				from_node->AppendAttrib(doc.AllocAttribFloat("b", particle_color_from_.b()));
				color_node->AppendNode(from_node);

				XMLNodePtr to_node = doc.AllocNode(XNT_Element, "to");
				to_node->AppendAttrib(doc.AllocAttribFloat("r", particle_color_to_.r()));
				to_node->AppendAttrib(doc.AllocAttribFloat("g", particle_color_to_.g()));
				to_node->AppendAttrib(doc.AllocAttribFloat("b", particle_color_to_.b()));
				color_node->AppendNode(to_node);
			}
			particle_node->AppendNode(color_node);
		}
		root->AppendNode(particle_node);
	}

	{
		XMLNodePtr emittor_node = doc.AllocNode(XNT_Element, "emittor");

		emittor_node->AppendAttrib(doc.AllocAttribInt("angle", dialog_->Control<UISlider>(id_angle_slider_)->GetValue()));
		emittor_node->AppendAttrib(doc.AllocAttribInt("life", dialog_->Control<UISlider>(id_life_slider_)->GetValue()));
		emittor_node->AppendAttrib(doc.AllocAttribFloat("min_velocity", dialog_->Control<UISlider>(id_min_velocity_slider_)->GetValue() / 100.0f));
		emittor_node->AppendAttrib(doc.AllocAttribFloat("max_velocity", dialog_->Control<UISlider>(id_max_velocity_slider_)->GetValue() / 100.0f));
		root->AppendNode(emittor_node);
	}

	{
		XMLNodePtr max_position_deviation_node = doc.AllocNode(XNT_Element, "max_position_deviation");
		float3 const & max_pos_dev = gen_particle.GetMaxPositionDeviation();
		max_position_deviation_node->AppendAttrib(doc.AllocAttribFloat("x", max_pos_dev.x()));
		max_position_deviation_node->AppendAttrib(doc.AllocAttribFloat("y", max_pos_dev.y()));
		max_position_deviation_node->AppendAttrib(doc.AllocAttribFloat("z", max_pos_dev.z()));
		root->AppendNode(max_position_deviation_node);
	}

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

			instance_gb->Resize(sizeof(float) * 5 * num_pars);
			{
				GraphicsBuffer::Mapper mapper(*instance_gb, BA_Write_Only);
				float* instance_data = mapper.Pointer<float>();
				for (uint32_t i = 0; i < num_pars; ++ i, instance_data += 5)
				{
					Particle const & par = ps_->GetParticle(active_particles[i].first);
					instance_data[0] = par.pos.x();
					instance_data[1] = par.pos.y();
					instance_data[2] = par.pos.z();
					instance_data[3] = par.life;
					instance_data[4] = par.spin;
				}
			}

			particles_->Visible(true);
		}
		terrain_->Visible(false);

		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
