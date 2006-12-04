#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Timer.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/tuple/tuple.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 4512)
#endif
#include <boost/random.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/bind.hpp>

#include "GPUParticleSystem.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const NUM_PARTICLE = 65536;

	struct Particle
	{
		float3 pos;
		float3 vel;
		float birth_time;
	};

	class RenderParticles : public RenderableHelper
	{
	public:
		explicit RenderParticles(int max_num_particles)
			: RenderableHelper(L"RenderParticles"),
				tex_width_(256), tex_height_(max_num_particles / 256)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			GraphicsBufferPtr tex0 = rf.MakeVertexBuffer(BU_Static);
			GraphicsBufferPtr pos = rf.MakeVertexBuffer(BU_Static);
			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);

			rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleFan);
			rl_->BindVertexStream(tex0, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)),
									RenderLayout::ST_Geometry, max_num_particles);
			rl_->BindVertexStream(pos, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)),
									RenderLayout::ST_Instance, 1);
			rl_->BindIndexStream(ib, EF_R16);

			float2 texs[] =
			{
				float2(0.0f, 0.0f),
				float2(1.0f, 0.0f),
				float2(1.0f, 1.0f),
				float2(0.0f, 1.0f),
			};

			uint16_t indices[] =
			{
				0, 1, 2, 3,
			};

			tex0->Resize(sizeof(texs));
			{
				GraphicsBuffer::Mapper mapper(*tex0, BA_Write_Only);
				std::copy(&texs[0], &texs[4], mapper.Pointer<float2>());
			}

			pos->Resize(max_num_particles * sizeof(float2));
			for (int i = 0; i < max_num_particles; ++ i)
			{
				GraphicsBuffer::Mapper mapper(*pos, BA_Write_Only);
				mapper.Pointer<float2>()[i] = float2((i % tex_width_ + 0.5f) / tex_width_,
					(static_cast<float>(i) / tex_width_) / tex_height_);
			}

			ib->Resize(sizeof(indices));
			{
				GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
				std::copy(&indices[0], &indices[4], mapper.Pointer<uint16_t>());
			}

			technique_ = rf.LoadEffect("GPUParticleSystem.kfx")->TechniqueByName("Particles");

			particle_tex_ = LoadTexture("particle.dds");
		}

		void OnRenderBegin()
		{
			*(technique_->Effect().ParameterByName("particle_sampler")) = particle_tex_;
			*(technique_->Effect().ParameterByName("particle_pos_sampler")) = particle_pos_tex_;

			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;

			*(technique_->Effect().ParameterByName("point_radius")) = 0.04f;
		}

		void PosTexture(TexturePtr particle_pos_tex)
		{
			particle_pos_tex_ = particle_pos_tex;
		}

	private:
		int tex_width_, tex_height_;

		TexturePtr particle_tex_; 
		TexturePtr particle_pos_tex_;
	};

	class ParticlesObject : public SceneObjectHelper
	{
	public:
		explicit ParticlesObject(int max_num_particles)
			: SceneObjectHelper(RenderablePtr(new RenderParticles(max_num_particles)), SOA_Cullable)
		{
		}

		void PosTexture(TexturePtr particle_pos_tex)
		{
			checked_cast<RenderParticles*>(renderable_.get())->PosTexture(particle_pos_tex);
		}
	};

	class GPUParticleSystem : public RenderablePlane
	{
	public:
		explicit GPUParticleSystem(int max_num_particles)
			: RenderablePlane(2, 2, 1, 1, true),
				max_num_particles_(max_num_particles),
				tex_width_(256), tex_height_(max_num_particles / 256),
				model_mat_(float4x4::Identity()),
				rt_index_(true), accumulate_time_(0),
				init_pos_(0, 0, 0), init_life_(8),
				random_gen_(boost::lagged_fibonacci607(), boost::uniform_real<float>(-0.05f, 0.05f))
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEngine& re = rf.RenderEngineInstance();

			technique_ = rf.LoadEffect("GPUParticleSystem.kfx")->TechniqueByName("Update");

			particle_pos_texture_[0] = rf.MakeTexture2D(tex_width_, tex_height_, 1, EF_ABGR32F);
			particle_pos_texture_[1] = rf.MakeTexture2D(tex_width_, tex_height_, 1, EF_ABGR32F);
			particle_vel_texture_[0] = rf.MakeTexture2D(tex_width_, tex_height_, 1, EF_ABGR32F);
			particle_vel_texture_[1] = rf.MakeTexture2D(tex_width_, tex_height_, 1, EF_ABGR32F);
			std::vector<float4> pos_init(max_num_particles, float4(0, 0, 0, 0));
			particle_pos_texture_[0]->CopyMemoryToTexture2D(0, &pos_init[0], EF_ABGR32F,
				tex_width_, tex_height_, 0, 0, tex_width_, tex_height_);
			particle_pos_texture_[1]->CopyMemoryToTexture2D(0, &pos_init[0], EF_ABGR32F,
				tex_width_, tex_height_, 0, 0, tex_width_, tex_height_);

			pos_vel_rt_buffer_[0] = rf.MakeFrameBuffer();
			pos_vel_rt_buffer_[0]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*particle_pos_texture_[0], 0));
			pos_vel_rt_buffer_[0]->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*particle_vel_texture_[0], 0));

			pos_vel_rt_buffer_[1] = rf.MakeFrameBuffer();
			pos_vel_rt_buffer_[1]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*particle_pos_texture_[1], 0));
			pos_vel_rt_buffer_[1]->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*particle_vel_texture_[1], 0));

			RenderTargetPtr screen_buffer = re.CurRenderTarget();
			pos_vel_rt_buffer_[0]->GetViewport().camera = pos_vel_rt_buffer_[1]->GetViewport().camera
				= screen_buffer->GetViewport().camera;

			particle_birth_time_ = rf.MakeTexture2D(tex_width_, tex_height_, 1, EF_R32F);

			particle_init_vel_ = rf.MakeTexture2D(tex_width_, tex_height_, 1, EF_ABGR32F);
			std::vector<float4> vel_init(max_num_particles);
			for (size_t i = 0; i < vel_init.size(); ++ i)
			{
				vel_init[i] = float4(random_gen_(), 0.2f + abs(random_gen_()) * 3, random_gen_(), 0);
			}
			particle_init_vel_->CopyMemoryToTexture2D(0, &vel_init[0], EF_ABGR32F,
				tex_width_, tex_height_, 0, 0, tex_width_, tex_height_);

			ps_model_mat_param_ = technique_->Effect().ParameterByName("ps_model_mat");
			init_pos_life_param_ = technique_->Effect().ParameterByName("init_pos_life");
			particle_pos_sampler_param_ = technique_->Effect().ParameterByName("particle_pos_sampler");
			particle_vel_sampler_param_ = technique_->Effect().ParameterByName("particle_vel_sampler");
			particle_init_vel_sampler_param_ = technique_->Effect().ParameterByName("particle_init_vel_sampler");
			particle_birth_time_sampler_param_ = technique_->Effect().ParameterByName("particle_birth_time_sampler");
			accumulate_time_param_ = technique_->Effect().ParameterByName("accumulate_time");
			elapse_time_param_ = technique_->Effect().ParameterByName("elapse_time");
		}

		void ModelMatrix(float4x4 const & model)
		{
			model_mat_ = model;
		}

		float4x4 const & ModelMatrix() const
		{
			return model_mat_;
		}

		void AutoEmit(float freq)
		{
			inv_emit_freq_ = 1.0f / freq;

			float time = 0;
			std::vector<float> birth_time_init(max_num_particles_);
			for (size_t i = 0; i < birth_time_init.size(); ++ i)
			{
				birth_time_init[i] = time;
				time += inv_emit_freq_;
			}
			particle_birth_time_->CopyMemoryToTexture2D(0, &birth_time_init[0], EF_R32F,
				tex_width_, tex_height_, 0, 0, tex_width_, tex_height_);
		}

		void Update(float elapse_time)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			re.BindRenderTarget(pos_vel_rt_buffer_[rt_index_]);

			elapse_time_ = elapse_time;

			accumulate_time_ += elapse_time;
			if (accumulate_time_ >= max_num_particles_ * inv_emit_freq_)
			{
				accumulate_time_ = 0;
			}

			this->Render();

			rt_index_ = !rt_index_;
		}

		TexturePtr PosTexture() const
		{
			return particle_pos_texture_[!rt_index_];
		}

		TexturePtr VelTexture() const
		{
			return particle_vel_texture_[!rt_index_];
		}

		void OnRenderBegin()
		{
			*ps_model_mat_param_ = model_mat_;
			*init_pos_life_param_ = float4(init_pos_.x(), init_pos_.y(), init_pos_.z(), init_life_);

			*particle_pos_sampler_param_ = this->PosTexture();
			*particle_vel_sampler_param_ = this->VelTexture();

			*particle_init_vel_sampler_param_ = particle_init_vel_;
			*particle_birth_time_sampler_param_ = particle_birth_time_;

			*accumulate_time_param_ = accumulate_time_;
			*elapse_time_param_ = elapse_time_;
		}

	private:
		int max_num_particles_;
		int tex_width_, tex_height_;

		float4x4 model_mat_;

		TexturePtr particle_pos_texture_[2];
		TexturePtr particle_vel_texture_[2];

		FrameBufferPtr pos_vel_rt_buffer_[2];

		bool rt_index_;

		TexturePtr particle_birth_time_;
		TexturePtr particle_init_vel_;

		float accumulate_time_;
		float elapse_time_;
		float inv_emit_freq_;

		float3 init_pos_;
		float init_life_;

		boost::variate_generator<boost::lagged_fibonacci607, boost::uniform_real<float> > random_gen_;

		RenderEffectParameterPtr ps_model_mat_param_;
		RenderEffectParameterPtr init_pos_life_param_;
		RenderEffectParameterPtr vel_offset_param_;
		RenderEffectParameterPtr particle_pos_sampler_param_;
		RenderEffectParameterPtr particle_vel_sampler_param_;
		RenderEffectParameterPtr particle_init_vel_sampler_param_;
		RenderEffectParameterPtr particle_birth_time_sampler_param_;
		RenderEffectParameterPtr accumulate_time_param_;
		RenderEffectParameterPtr elapse_time_param_;
	};

	class TerrainRenderable : public RenderablePlane
	{
	public:
		explicit TerrainRenderable(TexturePtr height_map, TexturePtr normal_map)
			: RenderablePlane(4, 4, 64, 64, true)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("GPUParticleSystem.kfx")->TechniqueByName("Terrain");

			TexturePtr height_32f = rf.MakeTexture2D(height_map->Width(0), height_map->Height(0), 1, EF_R32F);
			height_map->CopyToTexture(*height_32f);

			height_map_tex_ = height_32f;
			normal_map_tex_ = normal_map;

			grass_tex_ = LoadTexture("grass.dds");
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;

			*(technique_->Effect().ParameterByName("height_map_sampler")) = height_map_tex_;
			*(technique_->Effect().ParameterByName("normal_map_sampler")) = normal_map_tex_;

			*(technique_->Effect().ParameterByName("grass_sampler")) = grass_tex_;
		}

	private:
		float4x4 model_;

		TexturePtr height_map_tex_;
		TexturePtr normal_map_tex_;

		TexturePtr grass_tex_;
	};

	class TerrainObject : public SceneObjectHelper
	{
	public:
		TerrainObject(TexturePtr height_map, TexturePtr normal_map)
			: SceneObjectHelper(RenderablePtr(new TerrainRenderable(height_map, normal_map)), SOA_Cullable)
		{
		}
	};


	boost::shared_ptr<GPUParticleSystem> gpu_ps;


	enum
	{
		Exit
	};

	InputActionDefine actions[] = 
	{
		InputActionDefine(Exit, KS_Escape)
	};

	bool ConfirmDevice(RenderDeviceCaps const & caps)
	{
		if (caps.max_shader_model < 3)
		{
			return false;
		}
		return true;
	}
}


int main()
{
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.depth_stencil_fmt = EF_D24S8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	GPUParticleSystemApp app;
	app.Create("GPU Particle System", settings);
	app.Run();

	return 0;
}

GPUParticleSystemApp::GPUParticleSystemApp()
{
	ResLoader::Instance().AddPath("../../media/Common");
	ResLoader::Instance().AddPath("../../media/GPUParticleSystem");
}

void GPUParticleSystemApp::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	this->LookAt(float3(-1.2f, 2.2f, -1.2f), float3(0, 0.5f, 0));
	this->Proj(0.01f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&GPUParticleSystemApp::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);

	particles_.reset(new ParticlesObject(NUM_PARTICLE));
	particles_->AddToSceneManager();

	gpu_ps.reset(new GPUParticleSystem(NUM_PARTICLE));
	gpu_ps->AutoEmit(500);

	terrain_.reset(new TerrainObject(LoadTexture("terrain_height.dds"), LoadTexture("terrain_normal.dds")));
	terrain_->AddToSceneManager();
}

void GPUParticleSystemApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

uint32_t GPUParticleSystemApp::NumPasses() const
{
	return 1;
}

void GPUParticleSystemApp::DoUpdate(uint32_t /*pass*/)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	float4x4 mat = MathLib::rotation_x(PI / 6) * MathLib::rotation_y(clock() / 300.0f) * MathLib::translation(0.0f, 0.7f, 0.0f);
	gpu_ps->ModelMatrix(mat);

	gpu_ps->Update(static_cast<float>(timer_.elapsed()));
	timer_.restart();

	checked_cast<ParticlesObject*>(particles_.get())->PosTexture(gpu_ps->PosTexture());

	re.BindRenderTarget(RenderTargetPtr());
	re.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);

	fpcController_.Update();

	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU Particle System");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
}
