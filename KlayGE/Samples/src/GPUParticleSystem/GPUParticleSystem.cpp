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
#include <KlayGE/KMesh.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Timer.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <boost/tuple/tuple.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 4512 6385)
#endif
#include <boost/random.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>

#include "GPUParticleSystem.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	bool use_gs = false;
	bool use_so = false;

	int const NUM_PARTICLE = 65536;

	float GetDensity(int x, int y, int z, std::vector<uint8_t> const & data, uint32_t vol_size)
	{
		if (x < 0)
		{
			x += vol_size;
		}
		if (y < 0)
		{
			y += vol_size;
		}
		if (z < 0)
		{
			z += vol_size;
		}

		x = x % vol_size;
		y = y % vol_size;
		z = z % vol_size;

		return data[((z * vol_size + y) * vol_size + x) * 4 + 3] / 255.0f - 0.5f;
	}

	TexturePtr CreateNoiseVolume(uint32_t vol_size)
	{
		boost::variate_generator<boost::lagged_fibonacci607, boost::uniform_real<float> > random_gen(boost::lagged_fibonacci607(), boost::uniform_real<float>(0, 255));

		std::vector<uint8_t> data_block(vol_size * vol_size * vol_size * 4);
		ElementInitData init_data;
		init_data.data = &data_block[0];
		init_data.row_pitch = vol_size * 4;
		init_data.slice_pitch = vol_size * vol_size * 4;

		// Gen a bunch of random values
		for (size_t i = 0; i < data_block.size() / 4; ++ i)
		{
			data_block[i * 4 + 3] = static_cast<uint8_t>(random_gen());
		}

		// Generate normals from the density gradient
		float height_adjust = 0.5f;
		float3 normal;
		for (uint32_t z = 0; z < vol_size; ++ z)
		{
			for (uint32_t y = 0; y < vol_size; ++ y)
			{
				for (uint32_t x = 0; x < vol_size; ++ x)
				{
					normal.x() = (GetDensity(x + 1, y, z, data_block, vol_size) - GetDensity(x - 1, y, z, data_block, vol_size)) / height_adjust;
					normal.y() = (GetDensity(x, y + 1, z, data_block, vol_size) - GetDensity(x, y - 1, z, data_block, vol_size)) / height_adjust;
					normal.z() = (GetDensity(x, y, z + 1, data_block, vol_size) - GetDensity(x, y, z - 1, data_block, vol_size)) / height_adjust;

					normal = MathLib::normalize(normal);

					data_block[((z * vol_size + y) * vol_size + x) * 4 + 2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((normal.x() / 2 + 0.5f) * 255.0f), 0, 255));
					data_block[((z * vol_size + y) * vol_size + x) * 4 + 1] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((normal.y() / 2 + 0.5f) * 255.0f), 0, 255));
					data_block[((z * vol_size + y) * vol_size + x) * 4 + 0] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((normal.z() / 2 + 0.5f) * 255.0f), 0, 255));
				}
			}
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		TexturePtr ret;
		if (rf.RenderEngineInstance().DeviceCaps().argb8_support)
		{
			ret = rf.MakeTexture3D(vol_size, vol_size, vol_size, 1, 1, EF_ARGB8, 1, 0, EAH_GPU_Read, &init_data);
		}
		else
		{
			for (uint32_t i = 0; i < vol_size * vol_size * vol_size; ++ i)
			{
				std::swap(data_block[i * 4 + 0], data_block[i * 4 + 2]);
			}

			ret = rf.MakeTexture3D(vol_size, vol_size, vol_size, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read, &init_data);
		}

		return ret;
	}

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
				tex_width_(256), tex_height_((max_num_particles + 255) / 256)
		{
			BOOST_AUTO(pt, LoadTexture("particle.dds", EAH_GPU_Read));

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			
			float2 texs[] =
			{
				float2(-1.0f, 1.0f),
				float2(1.0f, 1.0f),
				float2(-1.0f, -1.0f),
				float2(1.0f, -1.0f),
			};

			uint16_t indices[] =
			{
				0, 1, 2, 3,
			};

			ElementInitData init_data;

			rl_ = rf.MakeRenderLayout();

			if (use_gs)
			{
				rl_->TopologyType(RenderLayout::TT_PointList);

				if (use_so)
				{
					technique_ = rf.LoadEffect("GPUParticleSystem.fxml")->TechniqueByName("ParticlesWithGSSO");
					rl_->BindDummyVertexStream(max_num_particles);
				}
				else
				{
					std::vector<float2> p_in_tex(max_num_particles);
					for (int i = 0; i < max_num_particles; ++ i)
					{
						p_in_tex[i] = float2((i % tex_width_ + 0.5f) / tex_width_,
							(static_cast<float>(i) / tex_width_) / tex_height_);
					}
					init_data.row_pitch = max_num_particles * sizeof(float2);
					init_data.slice_pitch = 0;
					init_data.data = &p_in_tex[0];
					GraphicsBufferPtr pos = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

					rl_->BindVertexStream(pos, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));
					technique_ = rf.LoadEffect("GPUParticleSystem.fxml")->TechniqueByName("ParticlesWithGS");
				}
			}
			else
			{
				std::vector<float2> p_in_tex(max_num_particles);
				for (int i = 0; i < max_num_particles; ++ i)
				{
					p_in_tex[i] = float2((i % tex_width_ + 0.5f) / tex_width_,
						(static_cast<float>(i) / tex_width_) / tex_height_);
				}
				init_data.row_pitch = max_num_particles * sizeof(float2);
				init_data.slice_pitch = 0;
				init_data.data = &p_in_tex[0];
				GraphicsBufferPtr pos = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

				init_data.row_pitch = sizeof(texs);
				init_data.slice_pitch = 0;
				init_data.data = texs;
				GraphicsBufferPtr tex0 = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

				init_data.row_pitch = sizeof(indices);
				init_data.slice_pitch = 0;
				init_data.data = indices;
				GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);

				rl_->TopologyType(RenderLayout::TT_TriangleStrip);
				rl_->BindVertexStream(tex0, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)),
										RenderLayout::ST_Geometry, max_num_particles);
				rl_->BindVertexStream(pos, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)),
										RenderLayout::ST_Instance, 1);
				rl_->BindIndexStream(ib, EF_R16UI);

				technique_ = rf.LoadEffect("GPUParticleSystem.fxml")->TechniqueByName("Particles");
			}

			noise_vol_tex_ = CreateNoiseVolume(32);
			*(technique_->Effect().ParameterByName("noise_vol_tex")) = noise_vol_tex_;

			*(technique_->Effect().ParameterByName("particle_tex")) = particle_tex_ = pt();

			*(technique_->Effect().ParameterByName("point_radius")) = 0.1f;
			*(technique_->Effect().ParameterByName("init_pos_life")) = float4(0, 0, 0, 8);
		}

		void SceneTexture(TexturePtr tex, bool flip)
		{
			*(technique_->Effect().ParameterByName("scene_tex")) = tex;
			*(technique_->Effect().ParameterByName("flip")) = static_cast<int32_t>(flip ? -1 : +1);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();
			float4x4 const inv_proj = MathLib::inverse(proj);

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;
			*(technique_->Effect().ParameterByName("inv_view")) = MathLib::inverse(view);

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset() * 2;
			float const x_offset = texel_to_pixel.x() / re.CurFrameBuffer()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurFrameBuffer()->Height();
			*(technique_->Effect().ParameterByName("offset")) = float2(x_offset, y_offset);

			*(technique_->Effect().ParameterByName("upper_left")) = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("upper_right")) = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_left")) = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_right")) = MathLib::transform_coord(float3(1, -1, 1), inv_proj);
		}

		void PosTexture(TexturePtr const & particle_pos_tex)
		{
			*(technique_->Effect().ParameterByName("particle_pos_tex")) = particle_pos_tex;
		}

		void PosVB(GraphicsBufferPtr const & particle_pos_vb)
		{
			*(technique_->Effect().ParameterByName("particle_pos_buff")) = particle_pos_vb;
		}

	private:
		int tex_width_, tex_height_;

		TexturePtr particle_tex_;
		TexturePtr noise_vol_tex_;
	};

	class ParticlesObject : public SceneObjectHelper
	{
	public:
		explicit ParticlesObject(int max_num_particles)
			: SceneObjectHelper(MakeSharedPtr<RenderParticles>(max_num_particles), SOA_Cullable)
		{
		}

		void PosTexture(TexturePtr const & particle_pos_tex)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->PosTexture(particle_pos_tex);
		}

		void PosVB(GraphicsBufferPtr const & particle_pos_vb)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->PosVB(particle_pos_vb);
		}
	};

	class GPUParticleSystem : public RenderableHelper
	{
	public:
		GPUParticleSystem(int max_num_particles, TexturePtr const & terrain_height_map, TexturePtr const & terrain_normal_map)
			: RenderableHelper(L"GPUParticleSystem"),
				max_num_particles_(max_num_particles),
				tex_width_(256), tex_height_((max_num_particles + 255) / 256),
				model_mat_(float4x4::Identity()),
				rt_index_(true), accumulate_time_(0),
				random_gen_(boost::lagged_fibonacci607(), boost::uniform_real<float>(-0.05f, 0.05f))
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEngine& re = rf.RenderEngineInstance();

			if (use_so)
			{
				rl_ = rf.MakeRenderLayout();
				rl_->TopologyType(RenderLayout::TT_PointList);
				rl_->BindDummyVertexStream(max_num_particles);

				technique_ = rf.LoadEffect("GPUParticleSystem.fxml")->TechniqueByName("UpdateSO");

				std::vector<float4> p(max_num_particles_);
				for (size_t i = 0; i < p.size(); ++ i)
				{
					p[i] = float4(0, 0, 0, -1);
				}
				ElementInitData pos_init;
				pos_init.data = &p[0];
				pos_init.row_pitch = max_num_particles_ * sizeof(float4);
				pos_init.slice_pitch = 0;

				particle_rl_[0] = rf.MakeRenderLayout();
				particle_rl_[1] = rf.MakeRenderLayout();

				particle_pos_vb_[0] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, &pos_init, EF_ABGR32F);
				particle_pos_vb_[1] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, &pos_init, EF_ABGR32F);
				particle_vel_vb_[0] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, NULL, EF_ABGR32F);
				particle_vel_vb_[1] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, NULL, EF_ABGR32F);
				particle_vel_vb_[0]->Resize(max_num_particles_ * sizeof(float4));
				particle_vel_vb_[1]->Resize(max_num_particles_ * sizeof(float4));

				particle_rl_[0]->BindVertexStream(particle_pos_vb_[0], boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));
				particle_rl_[0]->BindVertexStream(particle_vel_vb_[0], boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_ABGR32F)));
				particle_rl_[1]->BindVertexStream(particle_pos_vb_[1], boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));
				particle_rl_[1]->BindVertexStream(particle_vel_vb_[1], boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_ABGR32F)));

				for (int i = 0; i < max_num_particles_; ++ i)
				{
					float const angel = random_gen_() / 0.05f * PI;
					float const r = random_gen_() * 3;

					p[i] = float4(r * cos(angel), 0.2f + abs(random_gen_()) * 3, r * sin(angel), 0);
				}
				ElementInitData vel_init;
				vel_init.data = &p[0];
				vel_init.row_pitch = max_num_particles_ * sizeof(float4);
				vel_init.slice_pitch = 0;

				GraphicsBufferPtr particle_init_vel_buff = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &vel_init, EF_ABGR32F);
				*(technique_->Effect().ParameterByName("particle_init_vel_buff")) = particle_init_vel_buff;

				particle_pos_buff_param_ = technique_->Effect().ParameterByName("particle_pos_buff");
				particle_vel_buff_param_ = technique_->Effect().ParameterByName("particle_vel_buff");
			}
			else
			{
				rl_ = rf.MakeRenderLayout();
				rl_->TopologyType(RenderLayout::TT_TriangleStrip);
				{
					float3 xyzs[] = 
					{
						float3(-1, +1, 0),
						float3(+1, +1, 0),
						float3(-1, -1, 0),
						float3(+1, -1, 0)
					};
					ElementInitData init_data;
					init_data.row_pitch = sizeof(xyzs);
					init_data.slice_pitch = 0;
					init_data.data = &xyzs[0];

					GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
					rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

					box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[4]);
				}
				{
					float2 texs[] = 
					{
						float2(0, 0),
						float2(1, 0),
						float2(0, 1),
						float2(1, 1)
					};
					ElementInitData init_data;
					init_data.row_pitch = sizeof(texs);
					init_data.slice_pitch = 0;
					init_data.data = &texs[0];

					GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
					rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
				}

				technique_ = rf.LoadEffect("GPUParticleSystem.fxml")->TechniqueByName("Update");

				std::vector<float4> p(tex_width_ * tex_height_);
				for (size_t i = 0; i < p.size(); ++ i)
				{
					p[i] = float4(0, 0, 0, -1);
				}
				ElementInitData pos_init;
				pos_init.data = &p[0];
				pos_init.row_pitch = tex_width_ * sizeof(float4);
				pos_init.slice_pitch = 0;

				particle_pos_texture_[0] = rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &pos_init);
				particle_pos_texture_[1] = rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &pos_init);
				particle_vel_texture_[0] = rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
				particle_vel_texture_[1] = rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

				pos_vel_rt_buffer_[0] = rf.MakeFrameBuffer();
				pos_vel_rt_buffer_[0]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*particle_pos_texture_[0], 0, 0));
				pos_vel_rt_buffer_[0]->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*particle_vel_texture_[0], 0, 0));

				pos_vel_rt_buffer_[1] = rf.MakeFrameBuffer();
				pos_vel_rt_buffer_[1]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*particle_pos_texture_[1], 0, 0));
				pos_vel_rt_buffer_[1]->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*particle_vel_texture_[1], 0, 0));

				FrameBufferPtr screen_buffer = re.CurFrameBuffer();
				pos_vel_rt_buffer_[0]->GetViewport().camera = pos_vel_rt_buffer_[1]->GetViewport().camera
					= screen_buffer->GetViewport().camera;
			
				for (int i = 0; i < tex_width_ * tex_height_; ++ i)
				{
					float const angel = random_gen_() / 0.05f * PI;
					float const r = random_gen_() * 3;

					p[i] = float4(r * cos(angel), 0.2f + abs(random_gen_()) * 3, r * sin(angel), 0);
				}
				ElementInitData vel_init;
				vel_init.data = &p[0];
				vel_init.row_pitch = tex_width_ * sizeof(float4);
				vel_init.slice_pitch = 0;

				TexturePtr particle_init_vel = rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read, &vel_init);
				*(technique_->Effect().ParameterByName("particle_init_vel_tex")) = particle_init_vel;

				particle_pos_tex_param_ = technique_->Effect().ParameterByName("particle_pos_tex");
				particle_vel_tex_param_ = technique_->Effect().ParameterByName("particle_vel_tex");
			}

			accumulate_time_param_ = technique_->Effect().ParameterByName("accumulate_time");
			elapse_time_param_ = technique_->Effect().ParameterByName("elapse_time");

			*(technique_->Effect().ParameterByName("init_pos_life")) = float4(0, 0, 0, 8);
			*(technique_->Effect().ParameterByName("height_map_tex")) = terrain_height_map;
			*(technique_->Effect().ParameterByName("normal_map_tex")) = terrain_normal_map;
		}

		void ModelMatrix(float4x4 const & model)
		{
			model_mat_ = model;
			*(technique_->Effect().ParameterByName("ps_model_mat")) = model;
		}

		float4x4 const & ModelMatrix() const
		{
			return model_mat_;
		}

		void AutoEmit(float freq)
		{
			inv_emit_freq_ = 1.0f / freq;

			float time = 0;

			std::vector<float> time_v(tex_width_ * tex_height_);
			for (size_t i = 0; i < time_v.size(); ++ i)
			{
				time_v[i] = time;
				time += inv_emit_freq_;
			}
			ElementInitData init_data;
			init_data.data = &time_v[0];
			init_data.slice_pitch = 0;

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			if (use_so)
			{
				init_data.row_pitch = max_num_particles_ * sizeof(float);

				GraphicsBufferPtr particle_birth_time_buff = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data, EF_R32F);
				*(technique_->Effect().ParameterByName("particle_birth_time_buff")) = particle_birth_time_buff;
			}
			else
			{
				init_data.row_pitch = tex_width_ * sizeof(float);

				TexturePtr particle_birth_time_tex = rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read, &init_data);
				*(technique_->Effect().ParameterByName("particle_birth_time_tex")) = particle_birth_time_tex;
			}
		}

		void Update(float elapse_time)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			accumulate_time_ += elapse_time;
			if (accumulate_time_ >= max_num_particles_ * inv_emit_freq_)
			{
				accumulate_time_ = 0;
			}

			*elapse_time_param_ = elapse_time;
			*accumulate_time_param_ = accumulate_time_;

			if (use_so)
			{
				re.BindSOBuffers(particle_rl_[rt_index_]);
				*particle_pos_buff_param_ = this->PosVB();
				*particle_vel_buff_param_ = this->VelVB();
			}
			else
			{
				re.BindFrameBuffer(pos_vel_rt_buffer_[rt_index_]);
				*particle_pos_tex_param_ = this->PosTexture();
				*particle_vel_tex_param_ = this->VelTexture();
			}

			this->Render();

			if (use_so)
			{
				re.BindSOBuffers(RenderLayoutPtr());
			}

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

		GraphicsBufferPtr PosVB() const
		{
			return particle_pos_vb_[!rt_index_];
		}

		GraphicsBufferPtr VelVB() const
		{
			return particle_vel_vb_[!rt_index_];
		}

	private:
		int max_num_particles_;
		int tex_width_, tex_height_;

		float4x4 model_mat_;

		TexturePtr particle_pos_texture_[2];
		TexturePtr particle_vel_texture_[2];

		GraphicsBufferPtr particle_pos_vb_[2];
		GraphicsBufferPtr particle_vel_vb_[2];
		RenderLayoutPtr particle_rl_[2];

		FrameBufferPtr pos_vel_rt_buffer_[2];

		bool rt_index_;

		float accumulate_time_;
		float inv_emit_freq_;

		boost::variate_generator<boost::lagged_fibonacci607, boost::uniform_real<float> > random_gen_;

		RenderEffectParameterPtr vel_offset_param_;
		RenderEffectParameterPtr particle_pos_tex_param_;
		RenderEffectParameterPtr particle_vel_tex_param_;
		RenderEffectParameterPtr particle_pos_buff_param_;
		RenderEffectParameterPtr particle_vel_buff_param_;
		RenderEffectParameterPtr accumulate_time_param_;
		RenderEffectParameterPtr elapse_time_param_;
	};

	class TerrainRenderable : public RenderablePlane
	{
	public:
		explicit TerrainRenderable(TexturePtr const & height_map, TexturePtr const & normal_map)
			: RenderablePlane(4, 4, 64, 64, true)
		{
			BOOST_AUTO(grass, LoadTexture("grass.dds", EAH_GPU_Read));

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("GPUParticleSystem.fxml")->TechniqueByName("Terrain");

			TexturePtr height_32f = rf.MakeTexture2D(height_map->Width(0), height_map->Height(0), 1, 1, EF_R32F, 1, 0, EAH_GPU_Read, NULL);
			height_map->CopyToTexture(*height_32f);

			*(technique_->Effect().ParameterByName("height_map_tex")) = height_32f;
			*(technique_->Effect().ParameterByName("normal_map_tex")) = normal_map;

			*(technique_->Effect().ParameterByName("grass_tex")) = grass();
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;
			*(technique_->Effect().ParameterByName("mvp")) = view * proj;

			Camera const & camera = app.ActiveCamera();
			*(technique_->Effect().ParameterByName("inv_far")) = 1 / camera.FarPlane();
		}

	private:
		float4x4 model_;
	};

	class TerrainObject : public SceneObjectHelper
	{
	public:
		TerrainObject(TexturePtr const & height_map, TexturePtr const & normal_map)
			: SceneObjectHelper(MakeSharedPtr<TerrainRenderable>(height_map, normal_map), SOA_Cullable)
		{
		}
	};

	class BlendPostProcess : public PostProcess
	{
	public:
		BlendPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("GPUParticleSystem.fxml")->TechniqueByName("Blend"))
		{
		}

		void TexWithAlpha(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("tex_with_alpha")) = tex;
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

	bool ConfirmDevice()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 3)
		{
			return false;
		}
		if (caps.max_simultaneous_rts < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(256, 256, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0, 0);

			temp_tex = rf.MakeTexture2D(256, 256, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
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
	ResLoader::Instance().AddPath("../Samples/media/GPUParticleSystem");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	Context::Instance().SceneManagerInstance(SceneManager::NullObject());

	GPUParticleSystemApp app("GPU Particle System", settings);
	app.Create();
	app.Run();

	return 0;
}

GPUParticleSystemApp::GPUParticleSystemApp(std::string const & name, RenderSettings const & settings)
							: App3DFramework(name, settings)
{
}

void GPUParticleSystemApp::InitObjects()
{
	use_gs = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps().gs_support;
	use_so = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps().stream_output_support;

	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	BOOST_AUTO(terrain_height, LoadTexture("terrain_height.dds", EAH_GPU_Read));
	BOOST_AUTO(terrain_normal, LoadTexture("terrain_normal.dds", EAH_GPU_Read));

	this->LookAt(float3(-1.2f, 2.2f, -1.2f), float3(0, 0.5f, 0));
	this->Proj(0.01f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&GPUParticleSystemApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	particles_ = MakeSharedPtr<ParticlesObject>(NUM_PARTICLE);
	particles_->AddToSceneManager();

	gpu_ps = MakeSharedPtr<GPUParticleSystem>(NUM_PARTICLE, terrain_height(), terrain_normal());
	gpu_ps->AutoEmit(500);

	terrain_ = MakeSharedPtr<TerrainObject>(terrain_height(), terrain_normal());
	terrain_->AddToSceneManager();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	
	scene_buffer_ = rf.MakeFrameBuffer();
	scene_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;
	fog_buffer_ = rf.MakeFrameBuffer();
	fog_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;

	blend_pp_ = MakeSharedPtr<BlendPostProcess>();
}

void GPUParticleSystemApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	RenderViewPtr ds_view = rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0);

	scene_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	scene_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_tex_, 0, 0));
	scene_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	fog_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	fog_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*fog_tex_, 0, 0));
	fog_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	checked_pointer_cast<RenderParticles>(particles_->GetRenderable())->SceneTexture(scene_tex_, scene_buffer_->RequiresFlipping());

	blend_pp_->Source(scene_tex_, scene_buffer_->RequiresFlipping());
	blend_pp_->Destinate(FrameBufferPtr());
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

void GPUParticleSystemApp::DoUpdateOverlay()
{
	std::wostringstream stream;
	stream << this->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU Particle System", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str(), 16);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	FrameBuffer& rw(*checked_pointer_cast<FrameBuffer>(re.CurFrameBuffer()));
	font_->RenderText(0, 36, Color(1, 1, 0, 1), rw.Description(), 16);
}

uint32_t GPUParticleSystemApp::DoUpdate(uint32_t pass)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	switch (pass)
	{
	case 0:
		re.BindFrameBuffer(scene_buffer_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

		terrain_->Visible(true);
		particles_->Visible(false);
		return App3DFramework::URV_Need_Flush;

	case 1:
		{
			terrain_->Visible(false);
			particles_->Visible(true);

			float4x4 mat = MathLib::translation(0.0f, 0.7f, 0.0f);
			gpu_ps->ModelMatrix(mat);

			gpu_ps->Update(static_cast<float>(timer_.elapsed()));
			timer_.restart();

			if (use_so)
			{
				checked_pointer_cast<ParticlesObject>(particles_)->PosVB(gpu_ps->PosVB());
			}
			else
			{
				checked_pointer_cast<ParticlesObject>(particles_)->PosTexture(gpu_ps->PosTexture());
			}

			re.BindFrameBuffer(fog_buffer_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1.0f, 0);

			return App3DFramework::URV_Need_Flush;
		}

	default:
		{
			terrain_->Visible(false);
			particles_->Visible(false);

			checked_pointer_cast<BlendPostProcess>(blend_pp_)->TexWithAlpha(fog_tex_);

			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

			blend_pp_->Apply();

			return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
		}
	}
}
