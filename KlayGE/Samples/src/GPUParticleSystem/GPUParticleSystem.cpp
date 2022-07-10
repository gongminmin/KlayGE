#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Half.hpp>
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
#include <KlayGE/Mesh.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <fstream>
#include <iterator>
#include <random>
#include <sstream>
#include <vector>

#include "SampleCommon.hpp"
#include "GPUParticleSystem.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	bool use_gs = false;
	bool use_so = false;
	bool use_cs = false;
	bool use_mrt = false;
	bool use_typed_uav = false;

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
		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(0, 255);

		std::vector<uint8_t> data_block(vol_size * vol_size * vol_size * 4);
		ElementInitData init_data;
		init_data.data = &data_block[0];
		init_data.row_pitch = vol_size * 4;
		init_data.slice_pitch = vol_size * vol_size * 4;

		// Gen a bunch of random values
		for (size_t i = 0; i < data_block.size() / 4; ++ i)
		{
			data_block[i * 4 + 3] = static_cast<uint8_t>(dis(gen));
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

					data_block[((z * vol_size + y) * vol_size + x) * 4 + 0] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((normal.x() / 2 + 0.5f) * 255.0f), 0, 255));
					data_block[((z * vol_size + y) * vol_size + x) * 4 + 1] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((normal.y() / 2 + 0.5f) * 255.0f), 0, 255));
					data_block[((z * vol_size + y) * vol_size + x) * 4 + 2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((normal.z() / 2 + 0.5f) * 255.0f), 0, 255));
				}
			}
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
		TexturePtr ret;
		if (caps.max_texture_depth >= vol_size)
		{
			auto const fmt = rf.RenderEngineInstance().DeviceCaps().BestMatchTextureFormat(MakeSpan({EF_ABGR8, EF_ARGB8}));
			BOOST_ASSERT(fmt != EF_Unknown);

			if (fmt == EF_ARGB8)
			{
				for (uint32_t i = 0; i < vol_size * vol_size * vol_size; ++ i)
				{
					std::swap(data_block[i * 4 + 0], data_block[i * 4 + 2]);
				}
			}

			ret = rf.MakeTexture3D(vol_size, vol_size, vol_size, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init_data));
		}

		return ret;
	}

	struct Particle
	{
		float3 pos;
		float3 vel;
		float birth_time;
	};

	class RenderParticles : public Renderable
	{
	public:
		explicit RenderParticles(int max_num_particles)
			: Renderable(L"RenderParticles"),
				tex_width_(256), tex_height_((max_num_particles + 255) / 256)
		{
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

			rls_[0] = rf.MakeRenderLayout();

			effect_ = SyncLoadRenderEffect("GPUParticleSystem.fxml");
			*(effect_->ParameterByName("particle_tex")) = ASyncLoadTexture("particle.dds", EAH_GPU_Read | EAH_Immutable);
			if (use_gs)
			{
				rls_[0]->TopologyType(RenderLayout::TT_PointList);

				if (use_so || use_cs)
				{
					technique_ = effect_->TechniqueByName("ParticlesWithGSSO");
				}
				else
				{
					std::vector<float2> p_in_tex(max_num_particles);
					for (int i = 0; i < max_num_particles; ++ i)
					{
						p_in_tex[i] = float2((i % tex_width_ + 0.5f) / tex_width_,
							(static_cast<float>(i) / tex_width_) / tex_height_);
					}
					GraphicsBufferPtr pos = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
						max_num_particles * sizeof(float2), &p_in_tex[0]);

					rls_[0]->BindVertexStream(pos, VertexElement(VEU_Position, 0, EF_GR32F));
					technique_ = effect_->TechniqueByName("ParticlesWithGS");
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
				GraphicsBufferPtr pos = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					max_num_particles * sizeof(float2), &p_in_tex[0]);

				GraphicsBufferPtr tex0 = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					sizeof(texs), texs);

				GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					sizeof(indices), indices);

				rls_[0]->TopologyType(RenderLayout::TT_TriangleStrip);
				rls_[0]->BindVertexStream(tex0, VertexElement(VEU_TextureCoord, 0, EF_GR32F),
										RenderLayout::ST_Geometry, max_num_particles);
				rls_[0]->BindVertexStream(pos, VertexElement(VEU_Position, 0, EF_GR32F),
										RenderLayout::ST_Instance, 1);
				rls_[0]->BindIndexStream(ib, EF_R16UI);

				technique_ = effect_->TechniqueByName("Particles");
			}

			noise_vol_tex_ = CreateNoiseVolume(32);
			*(effect_->ParameterByName("noise_vol_tex")) = noise_vol_tex_;

			*(effect_->ParameterByName("point_radius")) = 0.1f;
			*(effect_->ParameterByName("init_pos_life")) = float4(0, 0, 0, 8);
		}

		void SceneTexture(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("scene_tex")) = tex;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			*(effect_->ParameterByName("View")) = camera.ViewMatrix();
			*(effect_->ParameterByName("Proj")) = camera.ProjMatrix();
			*(effect_->ParameterByName("inv_view")) = camera.InverseViewMatrix();
			*(effect_->ParameterByName("inv_proj")) = camera.InverseProjMatrix();

			*(effect_->ParameterByName("far_plane")) = app.ActiveCamera().FarPlane();
		}

		void PosTexture(TexturePtr const & particle_pos_tex)
		{
			*(effect_->ParameterByName("particle_pos_tex")) = particle_pos_tex;
		}

		void PosVB(GraphicsBufferPtr const & particle_pos_vb)
		{
			if (use_gs && (use_so || use_cs))
			{
				rls_[0]->BindVertexStream(particle_pos_vb, VertexElement(VEU_Position, 0, EF_ABGR32F));
			}
		}

	private:
		int tex_width_, tex_height_;

		TexturePtr noise_vol_tex_;
	};

	class GPUParticleSystem : public Renderable
	{
	public:
		GPUParticleSystem(int max_num_particles, TexturePtr const & terrain_height_map, TexturePtr const & terrain_normal_map)
			: Renderable(L"GPUParticleSystem"),
				max_num_particles_(max_num_particles),
				tex_width_(256), tex_height_((max_num_particles + 255) / 256),
				rt_index_(true), accumulate_time_(0),
				random_dis_(-0.05f, +0.05f)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEngine& re = rf.RenderEngineInstance();

			effect_ = SyncLoadRenderEffect("GPUParticleSystem.fxml");
			if (use_cs)
			{
				if (use_typed_uav)
				{
					update_cs_tech_ = effect_->TechniqueByName("UpdateTypedUAVCS");
				}
				else
				{
					update_cs_tech_ = effect_->TechniqueByName("UpdateCS");
				}
				technique_ = update_cs_tech_;

				std::vector<float4> p(max_num_particles_);
				for (size_t i = 0; i < p.size(); ++ i)
				{
					p[i] = float4(0, 0, 0, -1);
				}

				uint32_t access_hint = EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered;
				if (!use_typed_uav)
				{
					access_hint |=EAH_GPU_Structured;
				}

				particle_pos_vb_[0] = rf.MakeVertexBuffer(BU_Dynamic, access_hint, max_num_particles_ * sizeof(float4), &p[0],
					sizeof(float4));
				particle_pos_uav_[0] = rf.MakeBufferUav(particle_pos_vb_[0], EF_ABGR32F);
				particle_pos_vb_[1] = particle_pos_vb_[0];
				particle_pos_uav_[1] = particle_pos_uav_[0];
				particle_vel_vb_[0] = rf.MakeVertexBuffer(BU_Dynamic, access_hint, max_num_particles_ * sizeof(float4), nullptr,
					sizeof(float4));
				particle_vel_uav_[0] = rf.MakeBufferUav(particle_vel_vb_[0], EF_ABGR32F);
				particle_vel_vb_[1] = particle_vel_vb_[0];
				particle_vel_uav_[1] = particle_vel_uav_[0];

				if (use_typed_uav)
				{
					*(effect_->ParameterByName("particle_pos_rw_buff")) = particle_pos_uav_[0];
					*(effect_->ParameterByName("particle_vel_rw_buff")) = particle_vel_uav_[0];
				}
				else
				{
					*(effect_->ParameterByName("particle_pos_rw_stru_buff")) = particle_pos_uav_[0];
					*(effect_->ParameterByName("particle_vel_rw_stru_buff")) = particle_vel_uav_[0];
				}

				for (int i = 0; i < max_num_particles_; ++ i)
				{
					float const angel = this->RandomGen() / 0.05f * PI;
					float const r = this->RandomGen() * 3;

					p[i] = float4(r * cos(angel), 0.2f + abs(this->RandomGen()) * 3, r * sin(angel), 0);
				}

				GraphicsBufferPtr particle_init_vel_buff = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					max_num_particles_ * sizeof(float4), &p[0], sizeof(float4));
				*(effect_->ParameterByName("particle_init_vel_buff")) = rf.MakeBufferSrv(particle_init_vel_buff, EF_ABGR32F);
			}
			else if (use_so)
			{
				rls_[0] = rf.MakeRenderLayout();
				rls_[0]->TopologyType(RenderLayout::TT_PointList);
				rls_[0]->NumVertices(max_num_particles);

				update_so_tech_ = effect_->TechniqueByName("UpdateSO");
				technique_ = update_so_tech_;

				std::vector<float4> p(max_num_particles_);
				for (size_t i = 0; i < p.size(); ++ i)
				{
					p[i] = float4(0, 0, 0, -1);
				}

				for (int i = 0; i < 2; ++ i)
				{
					particle_rl_[i] = rf.MakeRenderLayout();

					particle_pos_vb_[i] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, max_num_particles_ * sizeof(float4),
						&p[0], sizeof(float4));
					particle_pos_srv_[i] = rf.MakeBufferSrv(particle_pos_vb_[i], EF_ABGR32F);

					particle_vel_vb_[i] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, max_num_particles_ * sizeof(float4),
						nullptr, sizeof(float4));
					particle_vel_srv_[i] = rf.MakeBufferSrv(particle_vel_vb_[i], EF_ABGR32F);

					particle_rl_[i]->BindVertexStream(particle_pos_vb_[i], VertexElement(VEU_Position, 0, EF_ABGR32F));
					particle_rl_[i]->BindVertexStream(particle_vel_vb_[i], VertexElement(VEU_TextureCoord, 0, EF_ABGR32F));
				}

				for (int i = 0; i < max_num_particles_; ++ i)
				{
					float const angel = this->RandomGen() / 0.05f * PI;
					float const r = this->RandomGen() * 3;

					p[i] = float4(r * cos(angel), 0.2f + abs(this->RandomGen()) * 3, r * sin(angel), 0);
				}

				GraphicsBufferPtr particle_init_vel_buff = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					max_num_particles_ * sizeof(float4), &p[0], sizeof(float4));
				*(effect_->ParameterByName("particle_init_vel_buff")) = rf.MakeBufferSrv(particle_init_vel_buff, EF_ABGR32F);

				particle_pos_buff_param_ = effect_->ParameterByName("particle_pos_buff");
				particle_vel_buff_param_ = effect_->ParameterByName("particle_vel_buff");
			}
			else
			{
				rls_[0] = rf.MakeRenderLayout();
				rls_[0]->TopologyType(RenderLayout::TT_TriangleStrip);
				{
					float3 xyzs[] = 
					{
						float3(-1, +1, 0),
						float3(+1, +1, 0),
						float3(-1, -1, 0),
						float3(+1, -1, 0)
					};

					GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
						sizeof(xyzs), &xyzs[0]);
					rls_[0]->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_BGR32F));

					pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[4]);
				}
				{
					float2 texs[] = 
					{
						float2(0, 0),
						float2(1, 0),
						float2(0, 1),
						float2(1, 1)
					};

					GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
						sizeof(texs), &texs[0]);
					rls_[0]->BindVertexStream(tex_vb, VertexElement(VEU_TextureCoord, 0, EF_GR32F));

					tc_aabb_ = AABBox(float3(0, 0, 0), float3(1, 1, 0));
				}

				if (use_mrt)
				{
					update_mrt_tech_ = effect_->TechniqueByName("Update");
					technique_ = update_mrt_tech_;
				}
				else
				{
					update_pos_tech_ = effect_->TechniqueByName("UpdatePos");
					update_vel_tech_ = effect_->TechniqueByName("UpdateVel");
					technique_ = update_pos_tech_;
				}

				{
					RenderDeviceCaps const & caps = re.DeviceCaps();
					auto const fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR32F, EF_ABGR16F}), 1, 0);
					BOOST_ASSERT(fmt != EF_Unknown);

					std::vector<uint8_t> pos;
					ElementInitData pos_init;
					if (fmt == EF_ABGR32F)
					{
						pos.resize(tex_width_ * tex_height_ * sizeof(float) * 4);
						float* p = reinterpret_cast<float*>(&pos[0]);
						for (int i = 0; i < tex_width_ * tex_height_; ++ i)
						{
							p[i * 4 + 0] = 0.0f;
							p[i * 4 + 1] = 0.0f;
							p[i * 4 + 2] = 0.0f;
							p[i * 4 + 3] = -1.0f;
						}

						pos_init.data = &p[0];
						pos_init.row_pitch = tex_width_ * sizeof(float) * 4;
						pos_init.slice_pitch = 0;
					}
					else
					{
						pos.resize(tex_width_ * tex_height_ * sizeof(half) * 4);
						half* p = reinterpret_cast<half*>(&pos[0]);
						for (int i = 0; i < tex_width_ * tex_height_; ++ i)
						{
							p[i * 4 + 0] = half(0.0f);
							p[i * 4 + 1] = half(0.0f);
							p[i * 4 + 2] = half(0.0f);
							p[i * 4 + 3] = half(-1.0f);
						}

						pos_init.data = &p[0];
						pos_init.row_pitch = tex_width_ * sizeof(half) * 4;
						pos_init.slice_pitch = 0;
					}
					particle_pos_texture_[0] =
						rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, MakeSpan<1>(pos_init));
					particle_pos_texture_[1] =
						rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, MakeSpan<1>(pos_init));
					particle_vel_texture_[0] = rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
					particle_vel_texture_[1] = rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				}

				FrameBufferPtr const & screen_buffer = re.CurFrameBuffer();
				if (use_mrt)
				{
					pos_vel_rt_buffer_[0] = rf.MakeFrameBuffer();
					pos_vel_rt_buffer_[0]->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(particle_pos_texture_[0], 0, 1, 0));
					pos_vel_rt_buffer_[0]->Attach(FrameBuffer::Attachment::Color1, rf.Make2DRtv(particle_vel_texture_[0], 0, 1, 0));

					pos_vel_rt_buffer_[1] = rf.MakeFrameBuffer();
					pos_vel_rt_buffer_[1]->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(particle_pos_texture_[1], 0, 1, 0));
					pos_vel_rt_buffer_[1]->Attach(FrameBuffer::Attachment::Color1, rf.Make2DRtv(particle_vel_texture_[1], 0, 1, 0));

					auto const& camera = screen_buffer->Viewport()->Camera();
					pos_vel_rt_buffer_[0]->Viewport()->Camera(camera);
					pos_vel_rt_buffer_[1]->Viewport()->Camera(camera);
				}
				else
				{
					pos_rt_buffer_[0] = rf.MakeFrameBuffer();
					pos_rt_buffer_[0]->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(particle_pos_texture_[0], 0, 1, 0));

					vel_rt_buffer_[0] = rf.MakeFrameBuffer();
					vel_rt_buffer_[0]->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(particle_vel_texture_[0], 0, 1, 0));

					pos_rt_buffer_[1] = rf.MakeFrameBuffer();
					pos_rt_buffer_[1]->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(particle_pos_texture_[1], 0, 1, 0));

					vel_rt_buffer_[1] = rf.MakeFrameBuffer();
					vel_rt_buffer_[1]->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(particle_vel_texture_[1], 0, 1, 0));

					auto const& camera = screen_buffer->Viewport()->Camera();
					pos_rt_buffer_[0]->Viewport()->Camera(camera);
					pos_rt_buffer_[1]->Viewport()->Camera(camera);
					vel_rt_buffer_[0]->Viewport()->Camera(camera);
					vel_rt_buffer_[1]->Viewport()->Camera(camera);
				}

				{
					std::vector<half> p(tex_width_ * tex_height_ * 4);
					for (size_t i = 0; i < p.size(); i += 4)
					{
						float const angel = this->RandomGen() / 0.05f * PI;
						float const r = this->RandomGen() * 3;

						p[i + 0] = half(r * cos(angel));
						p[i + 1] = half(0.2f + abs(this->RandomGen()) * 3);
						p[i + 2] = half(r * sin(angel));
						p[i + 3] = half(0.0f);
					}
					ElementInitData vel_init;
					vel_init.data = &p[0];
					vel_init.row_pitch = tex_width_ * sizeof(half) * 4;
					vel_init.slice_pitch = 0;

					TexturePtr particle_init_vel = rf.MakeTexture2D(
						tex_width_, tex_height_, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(vel_init));
					*(effect_->ParameterByName("particle_init_vel_tex")) = particle_init_vel;
				}

				particle_pos_tex_param_ = effect_->ParameterByName("particle_pos_tex");
				particle_vel_tex_param_ = effect_->ParameterByName("particle_vel_tex");
			}

			accumulate_time_param_ = effect_->ParameterByName("accumulate_time");
			elapse_time_param_ = effect_->ParameterByName("elapse_time");

			*(effect_->ParameterByName("init_pos_life")) = float4(0, 0, 0, 8);
			*(effect_->ParameterByName("height_map_tex")) = terrain_height_map;
			*(effect_->ParameterByName("normal_map_tex")) = terrain_normal_map;
		}

		void ModelMatrix(float4x4 const & model_mat) override
		{
			Renderable::ModelMatrix(model_mat);
			*(effect_->ParameterByName("ps_model_mat")) = model_mat;
		}

		void AutoEmit(float freq)
		{
			inv_emit_freq_ = 1.0f / freq;

			float time = 0;

			std::vector<half> time_v(tex_width_ * tex_height_);
			for (size_t i = 0; i < time_v.size(); ++ i)
			{
				time_v[i] = half(time);
				time += inv_emit_freq_;
			}

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			if (use_so || use_cs)
			{
				GraphicsBufferPtr particle_birth_time_buff = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					max_num_particles_ * sizeof(half), &time_v[0], sizeof(half));
				*(effect_->ParameterByName("particle_birth_time_buff")) = rf.MakeBufferSrv(particle_birth_time_buff, EF_R16F);
			}
			else
			{
				ElementInitData init_data;
				init_data.data = &time_v[0];
				init_data.row_pitch = tex_width_ * sizeof(half);
				init_data.slice_pitch = init_data.row_pitch * tex_height_;

				TexturePtr particle_birth_time_tex =
					rf.MakeTexture2D(tex_width_, tex_height_, 1, 1, EF_R16F, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init_data));
				*(effect_->ParameterByName("particle_birth_time_tex")) = particle_birth_time_tex;
			}
		}

		void Update(float /*app_time*/, float elapsed_time)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			accumulate_time_ += elapsed_time;
			if (accumulate_time_ >= max_num_particles_ * inv_emit_freq_)
			{
				accumulate_time_ = 0;
			}

			*elapse_time_param_ = elapsed_time;
			*accumulate_time_param_ = accumulate_time_;

			if (use_cs)
			{
				technique_ = update_cs_tech_;
			}
			else if (use_so)
			{
				re.BindSOBuffers(particle_rl_[rt_index_]);
				*particle_pos_buff_param_ = this->PosSrv();
				*particle_vel_buff_param_ = this->VelSrv();

				technique_ = update_so_tech_;
			}
			else
			{
				if (use_mrt)
				{
					technique_ = update_mrt_tech_;
					re.BindFrameBuffer(pos_vel_rt_buffer_[rt_index_]);
				}
				else
				{
					technique_ = update_pos_tech_;
					re.BindFrameBuffer(pos_rt_buffer_[rt_index_]);
				}

				*particle_pos_tex_param_ = this->PosTexture();
				*particle_vel_tex_param_ = this->VelTexture();
			}

			if (use_cs)
			{
				re.BindFrameBuffer(re.DefaultFrameBuffer());
				re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color);

				this->OnRenderBegin();
				re.Dispatch(*effect_, *technique_, (max_num_particles_ + 255) / 256, 1, 1);
				this->OnRenderEnd();
			}
			else
			{
				this->Render();
			}

			if (!use_so && !use_mrt)
			{
				technique_ = update_vel_tech_;
				re.BindFrameBuffer(vel_rt_buffer_[rt_index_]);

				this->Render();
			}

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

		ShaderResourceViewPtr PosSrv() const
		{
			return particle_pos_srv_[!rt_index_];
		}

		ShaderResourceViewPtr VelSrv() const
		{
			return particle_vel_srv_[!rt_index_];
		}

	private:
		float RandomGen()
		{
			return MathLib::clamp(random_dis_(gen_), -0.05f, +0.05f);
		}

	private:
		int max_num_particles_;
		int tex_width_, tex_height_;

		TexturePtr particle_pos_texture_[2];
		TexturePtr particle_vel_texture_[2];

		GraphicsBufferPtr particle_pos_vb_[2];
		UnorderedAccessViewPtr particle_pos_uav_[2];
		ShaderResourceViewPtr particle_pos_srv_[2];
		GraphicsBufferPtr particle_vel_vb_[2];
		UnorderedAccessViewPtr particle_vel_uav_[2];
		ShaderResourceViewPtr particle_vel_srv_[2];
		RenderLayoutPtr particle_rl_[2];

		FrameBufferPtr pos_vel_rt_buffer_[2];
		FrameBufferPtr pos_rt_buffer_[2];
		FrameBufferPtr vel_rt_buffer_[2];

		bool rt_index_;

		float accumulate_time_;
		float inv_emit_freq_;

		ranlux24_base gen_;
		uniform_real_distribution<float> random_dis_;

		RenderTechnique* update_so_tech_;
		RenderTechnique* update_mrt_tech_;
		RenderTechnique* update_pos_tech_;
		RenderTechnique* update_vel_tech_;
		RenderTechnique* update_cs_tech_;
		RenderEffectParameter* particle_pos_tex_param_;
		RenderEffectParameter* particle_vel_tex_param_;
		RenderEffectParameter* particle_pos_buff_param_;
		RenderEffectParameter* particle_vel_buff_param_;
		RenderEffectParameter* accumulate_time_param_;
		RenderEffectParameter* elapse_time_param_;
	};

	class TerrainRenderable : public RenderablePlane
	{
	public:
		explicit TerrainRenderable(TexturePtr const & height_map, TexturePtr const & normal_map)
			: RenderablePlane(4, 4, 64, 64, true, false)
		{
			effect_ = SyncLoadRenderEffect("Terrain.fxml");
			technique_ = effect_->TechniqueByName("Terrain");
			*(effect_->ParameterByName("grass_tex")) = ASyncLoadTexture("grass.dds", EAH_GPU_Read | EAH_Immutable);
			*(effect_->ParameterByName("height_map_tex")) = height_map;
			*(effect_->ParameterByName("normal_map_tex")) = normal_map;
		}

		void OnRenderBegin()
		{
			RenderablePlane::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			*(effect_->ParameterByName("mvp")) = camera.ViewProjMatrix();
			*(effect_->ParameterByName("inv_far")) = 1 / camera.FarPlane();
		}
	};

	std::shared_ptr<GPUParticleSystem> gpu_ps;


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
	GPUParticleSystemApp app;
	app.Create();
	app.Run();

	gpu_ps.reset();

	return 0;
}

GPUParticleSystemApp::GPUParticleSystemApp()
							: App3DFramework("GPU Particle System")
{
	ResLoader::Instance().AddPath("../../Samples/media/GPUParticleSystem");
}

void GPUParticleSystemApp::OnCreate()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	use_gs = caps.gs_support;
	use_cs = caps.cs_support && (caps.max_shader_model >= ShaderModel(5, 0));
	if (use_cs)
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_STORE
		// Shaders are compiled to d3d11_0 for Windows store apps. No typed UAV support.
		use_typed_uav = false;
#else
		use_typed_uav = caps.UavFormatSupport(EF_ABGR16F);
#endif
	}
	else
	{
		use_so = caps.load_from_buffer_support;
	}
	use_mrt = caps.max_simultaneous_rts > 1;

	font_ = SyncLoadFont("gkai00mp.kfont");

	TexturePtr terrain_height_tex = SyncLoadTexture("terrain_height.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr terrain_normal_tex = SyncLoadTexture("terrain_normal.dds", EAH_GPU_Read | EAH_Immutable);

	this->LookAt(float3(-1.2f, 2.2f, -1.2f), float3(0, 0.5f, 0));
	this->Proj(0.01f, 100);

	tb_controller_.AttachCamera(this->ActiveCamera());
	tb_controller_.Scalers(0.003f, 0.003f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	particles_renderable_ = MakeSharedPtr<RenderParticles>(NUM_PARTICLE);
	particles_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(particles_renderable_), SceneNode::SOA_Moveable);
	Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(particles_);

	gpu_ps = MakeSharedPtr<GPUParticleSystem>(NUM_PARTICLE, terrain_height_tex, terrain_normal_tex);
	gpu_ps->AutoEmit(256);

	terrain_ = MakeSharedPtr<SceneNode>(
		MakeSharedPtr<RenderableComponent>(MakeSharedPtr<TerrainRenderable>(terrain_height_tex, terrain_normal_tex)),
		SceneNode::SOA_Cullable);
	Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(terrain_);

	FrameBufferPtr const & screen_buffer = re.CurFrameBuffer();
	
	scene_buffer_ = rf.MakeFrameBuffer();
	scene_buffer_->Viewport()->Camera(screen_buffer->Viewport()->Camera());
	fog_buffer_ = rf.MakeFrameBuffer();
	fog_buffer_->Viewport()->Camera(screen_buffer->Viewport()->Camera());

	blend_pp_ = SyncLoadPostProcess("Blend.ppml", "blend");

	if (use_cs)
	{
		checked_pointer_cast<RenderParticles>(particles_renderable_)->PosVB(gpu_ps->PosVB());
	}

	UIManager::Instance().Load(*ResLoader::Instance().Open("GPUParticleSystem.uiml"));
}

void GPUParticleSystemApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	auto ds_view = rf.Make2DDsv(width, height, EF_D16, 1, 0);

	scene_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	scene_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(scene_tex_, 0, 1, 0));
	scene_buffer_->Attach(ds_view);

	fog_tex_ = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	fog_srv_ = rf.MakeTextureSrv(fog_tex_);
	fog_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(fog_tex_, 0, 1, 0));
	fog_buffer_->Attach(ds_view);

	checked_pointer_cast<RenderParticles>(particles_renderable_)->SceneTexture(scene_tex_);

	blend_pp_->InputPin(0, rf.MakeTextureSrv(scene_tex_));

	UIManager::Instance().SettleCtrls();
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
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"GPU Particle System", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str(), 16);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	font_->RenderText(0, 36, Color(1, 1, 0, 1), re.ScreenFrameBuffer()->Description(), 16);
}

uint32_t GPUParticleSystemApp::DoUpdate(uint32_t pass)
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

			terrain_->Visible(true);
			particles_->Visible(false);
		}
		return App3DFramework::URV_NeedFlush;

	case 1:
		{
			terrain_->Visible(false);
			particles_->Visible(true);

			float4x4 mat = MathLib::translation(0.0f, 0.7f, 0.0f);
			gpu_ps->ModelMatrix(mat);

			gpu_ps->Update(this->AppTime(), this->FrameTime());

			if (use_so)
			{
				checked_pointer_cast<RenderParticles>(particles_renderable_)->PosVB(gpu_ps->PosVB());
			}
			else
			{
				checked_pointer_cast<RenderParticles>(particles_renderable_)->PosTexture(gpu_ps->PosTexture());
			}

			re.BindFrameBuffer(fog_buffer_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1.0f, 0);

			return App3DFramework::URV_NeedFlush;
		}

	default:
		{
			terrain_->Visible(false);
			particles_->Visible(false);

			blend_pp_->InputPin(1, fog_srv_);

			re.BindFrameBuffer(FrameBufferPtr());
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1.0f, 0);

			blend_pp_->Apply();

			return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
		}
	}
}
