/**
 * @file ParticleSystem.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/ParticleSystem.hpp>

namespace
{
	using namespace KlayGE;
	
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

	class RenderParticles : public RenderableHelper
	{
	public:
		explicit RenderParticles(bool gs_support)
			: RenderableHelper(L"Particles")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rl_ = rf.MakeRenderLayout();
			if (gs_support)
			{
				rl_->TopologyType(RenderLayout::TT_PointList);

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, nullptr);
				rl_->BindVertexStream(pos_vb, KlayGE::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F),
					vertex_element(VEU_TextureCoord, 0, EF_ABGR32F)));

				technique_ = SyncLoadRenderEffect("Particle.fxml")->TechniqueByName("ParticleWithGS");
			}
			else
			{
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

				technique_ = SyncLoadRenderEffect("Particle.fxml")->TechniqueByName("Particle");
			}

			*(technique_->Effect().ParameterByName("point_radius")) = 0.08f;
		}

		void SceneDepthTexture(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("depth_tex")) = tex;
		}

		void ParticleColorFrom(Color const & clr)
		{
			*(technique_->Effect().ParameterByName("particle_color_from")) = float3(clr.r(), clr.g(), clr.b());
		}

		void ParticleColorTo(Color const & clr)
		{
			*(technique_->Effect().ParameterByName("particle_color_to")) = float3(clr.r(), clr.g(), clr.b());
		}

		void ParticleAlphaFrom(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("particle_alpha_from_tex")) = tex;
		}

		void ParticleAlphaTo(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("particle_alpha_to_tex")) = tex;
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*(technique_->Effect().ParameterByName("view")) = view;
			*(technique_->Effect().ParameterByName("proj")) = proj;

			*(technique_->Effect().ParameterByName("depth_near_far_invfar")) = float3(camera.NearPlane(),
				camera.FarPlane(), 1.0f / camera.FarPlane());
		}

		void PosBound(AABBox const & pos_aabb)
		{
			pos_aabb_ = pos_aabb;
		}

		using RenderableHelper::PosBound;
	};

	class ParticleCmp
	{
	public:
		bool operator()(std::pair<uint32_t, float> const & lhs, std::pair<uint32_t, float> const & rhs) const
		{
			return lhs.second > rhs.second;
		}
	};
}

namespace KlayGE
{
	uint32_t ParticleEmitter::Update(float elapsed_time)
	{
		return static_cast<uint32_t>(elapsed_time * emit_freq_ + 0.5f);
	}


	ParticleSystem::ParticleSystem(uint32_t max_num_particles)
		: SceneObjectHelper(SOA_Moveable),
			particles_(max_num_particles), num_active_particles_(0),
			gravity_(0.5f), force_(0, 0, 0), media_density_(0.0f)
	{
		typedef KLAYGE_DECLTYPE(particles_) ParticlesType;
		KLAYGE_FOREACH(ParticlesType::reference particle, particles_)
		{
			particle.life = 0;
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		gs_support_ = rf.RenderEngineInstance().DeviceCaps().gs_support;
		renderable_ = MakeSharedPtr<RenderParticles>(gs_support_);
	}

	ParticleEmitterPtr ParticleSystem::MakeEmitter(std::string const & name)
	{
		ParticleEmitterPtr ret;
		if ("point" == name)
		{
			ret = MakeSharedPtr<PointParticleEmitter>(this->shared_from_this());
		}
		else
		{
			BOOST_ASSERT(false);
		}

		return ret;
	}

	ParticleUpdaterPtr ParticleSystem::MakeUpdater(std::string const & name)
	{
		ParticleUpdaterPtr ret;
		if ("polyline" == name)
		{
			ret = MakeSharedPtr<PolylineParticleUpdater>(this->shared_from_this());
		}
		else
		{
			BOOST_ASSERT(false);
		}

		return ret;
	}

	void ParticleSystem::AddEmitter(ParticleEmitterPtr const & emitter)
	{
		emitters_.push_back(emitter);
	}

	void ParticleSystem::DelEmitter(ParticleEmitterPtr const & emitter)
	{
		KLAYGE_AUTO(iter, std::find(emitters_.begin(), emitters_.end(), emitter));
		if (iter != emitters_.end())
		{
			emitters_.erase(iter);
		}
	}

	void ParticleSystem::ClearEmitter()
	{
		emitters_.clear();
	}

	void ParticleSystem::AddUpdater(ParticleUpdaterPtr const & updater)
	{
		updaters_.push_back(updater);
	}

	void ParticleSystem::DelUpdater(ParticleUpdaterPtr const & updater)
	{
		KLAYGE_AUTO(iter, std::find(updaters_.begin(), updaters_.end(), updater));
		if (iter != updaters_.end())
		{
			updaters_.erase(iter);
		}
	}

	void ParticleSystem::ClearUpdater()
	{
		updaters_.clear();
	}

	void ParticleSystem::Update(float /*app_time*/, float elapsed_time)
	{
		KLAYGE_AUTO(emitter_iter, emitters_.begin());
		uint32_t new_particle = (*emitter_iter)->Update(elapsed_time);
		
		float4x4 view_mat = Context::Instance().AppInstance().ActiveCamera().ViewMatrix();
		std::vector<std::pair<uint32_t, float> > active_particles;

		float3 min_bb(+1e10f, +1e10f, +1e10f);
		float3 max_bb(-1e10f, -1e10f, -1e10f);
		
		for (uint32_t i = 0; i < particles_.size(); ++ i)
		{
			Particle& particle = particles_[i];

			typedef KLAYGE_DECLTYPE(updaters_) UpdatersType;
			if (particle.life > 0)
			{
				KLAYGE_FOREACH(UpdatersType::reference updater, updaters_)
				{
					updater->Update(particle, elapsed_time);
				}
			}
			else
			{
				if (new_particle > 0)
				{
					(*emitter_iter)->Emit(particle);
					KLAYGE_FOREACH(UpdatersType::reference updater, updaters_)
					{
						updater->Update(particle, 0);
					}
					-- new_particle;
				}
				else
				{
					if (emitter_iter != emitters_.end())
					{
						++ emitter_iter;
						if (emitter_iter != emitters_.end())
						{
							new_particle = (*emitter_iter)->Update(elapsed_time);
						}
					}
				}
			}

			if (particle.life > 0)
			{
				float3 const & pos = particle.pos;
				float p_to_v = (pos.x() * view_mat(0, 2) + pos.y() * view_mat(1, 2) + pos.z() * view_mat(2, 2) + view_mat(3, 2))
					/ (pos.x() * view_mat(0, 3) + pos.y() * view_mat(1, 3) + pos.z() * view_mat(2, 3) + view_mat(3, 3));

				active_particles.push_back(std::make_pair(i, p_to_v));

				min_bb = MathLib::minimize(min_bb, pos);
				max_bb = MathLib::maximize(min_bb, pos);
			}
		}

		num_active_particles_ = active_particles.size();

		if (!active_particles.empty())
		{
			std::sort(active_particles.begin(), active_particles.end(), ParticleCmp());

			checked_pointer_cast<RenderParticles>(renderable_)->PosBound(AABBox(min_bb, max_bb));

			RenderLayoutPtr const & rl = renderable_->GetRenderLayout();
			GraphicsBufferPtr instance_gb;
			if (gs_support_)
			{
				instance_gb = rl->GetVertexStream(0);
			}
			else
			{
				instance_gb = rl->InstanceStream();

				for (uint32_t i = 0; i < rl->NumVertexStreams(); ++ i)
				{
					rl->VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, num_active_particles_);
				}
			}

			instance_gb->Resize(sizeof(ParticleInstance) * num_active_particles_);
			{
				GraphicsBuffer::Mapper mapper(*instance_gb, BA_Write_Only);
				ParticleInstance* instance_data = mapper.Pointer<ParticleInstance>();
				for (uint32_t i = 0; i < num_active_particles_; ++ i, ++ instance_data)
				{
					Particle const & par = particles_[active_particles[i].first];
					instance_data->pos = par.pos;
					instance_data->life = par.life;
					instance_data->spin = par.spin;
					instance_data->size = par.size;
					instance_data->life_factor = (par.init_life - par.life) / par.init_life;
					instance_data->alpha = par.alpha;
				}
			}
		}
	}

	void ParticleSystem::ParticleAlphaFromTex(std::string const & tex_name)
	{
		particle_alpha_from_tex_name_ = tex_name;
		checked_pointer_cast<RenderParticles>(renderable_)->ParticleAlphaFrom(
			SyncLoadTexture(tex_name, EAH_GPU_Read | EAH_Immutable));
	}

	void ParticleSystem::ParticleAlphaToTex(std::string const & tex_name)
	{
		particle_alpha_to_tex_name_ = tex_name;
		checked_pointer_cast<RenderParticles>(renderable_)->ParticleAlphaTo(
			SyncLoadTexture(tex_name, EAH_GPU_Read | EAH_Immutable));
	}

	void ParticleSystem::ParticleColorFrom(Color const & clr)
	{
		particle_color_from_ = clr;
		checked_pointer_cast<RenderParticles>(renderable_)->ParticleColorFrom(clr);
	}

	void ParticleSystem::ParticleColorTo(Color const & clr)
	{
		particle_color_to_ = clr;
		checked_pointer_cast<RenderParticles>(renderable_)->ParticleColorTo(clr);
	}

	void ParticleSystem::SceneDepthTexture(TexturePtr const & depth_tex)
	{
		checked_pointer_cast<RenderParticles>(renderable_)->SceneDepthTexture(depth_tex);
	}


	PointParticleEmitter::PointParticleEmitter(SceneObjectPtr const & ps)
		: ParticleEmitter(ps),
			random_dis_(0, 10000)
	{
	}

	std::string const & PointParticleEmitter::Type() const
	{
		static std::string const type("point");
		return type;
	}

	void PointParticleEmitter::Emit(Particle& par)
	{
		par.pos.x() = MathLib::lerp(min_pos_.x(), max_pos_.x(), this->RandomGen());
		par.pos.y() = MathLib::lerp(min_pos_.y(), max_pos_.y(), this->RandomGen());
		par.pos.z() = MathLib::lerp(min_pos_.z(), max_pos_.z(), this->RandomGen());
		par.pos = MathLib::transform_coord(par.pos, model_mat_);
		float theta = (this->RandomGen() * 2 - 1) * PI;
		float phi = this->RandomGen() * emit_angle_ / 2;
		float velocity = MathLib::lerp(min_vel_, max_vel_, this->RandomGen());
		float vx = cos(theta) * sin(phi);
		float vz = sin(theta) * sin(phi);
		float vy = cos(phi);
		par.vel = MathLib::transform_normal(float3(vx, vy, vz) * velocity, model_mat_);
		par.life = MathLib::lerp(min_life_, max_life_, this->RandomGen());
		par.spin = MathLib::lerp(min_spin_, max_spin_, this->RandomGen());
		par.size = MathLib::lerp(min_size_, max_size_, this->RandomGen());
		par.init_life = par.life;
	}

	float PointParticleEmitter::RandomGen()
	{
		return MathLib::clamp(random_dis_(gen_) * 0.0001f, 0.0f, 1.0f);
	}


	PolylineParticleUpdater::PolylineParticleUpdater(SceneObjectPtr const & ps)
		: ParticleUpdater(ps)
	{
	}

	std::string const & PolylineParticleUpdater::Type() const
	{
		static std::string const type("polyline");
		return type;
	}

	void PolylineParticleUpdater::Update(Particle& par, float elapse_time)
	{
		float pos = (par.init_life - par.life) / par.init_life;

		float cur_size = size_over_life_.back().y();
		for (std::vector<float2>::const_iterator iter = size_over_life_.begin(); iter != size_over_life_.end() - 1; ++ iter)
		{
			if ((iter + 1)->x() >= pos)
			{
				float const s = (pos - iter->x()) / ((iter + 1)->x() - iter->x());
				cur_size = MathLib::lerp(iter->y(), (iter + 1)->y(), s);
				break;
			}
		}

		float cur_mass = mass_over_life_.back().y();
		for (std::vector<float2>::const_iterator iter = mass_over_life_.begin(); iter != mass_over_life_.end() - 1; ++ iter)
		{
			if ((iter + 1)->x() >= pos)
			{
				float const s = (pos - iter->x()) / ((iter + 1)->x() - iter->x());
				cur_mass = MathLib::lerp(iter->y(), (iter + 1)->y(), s);
				break;
			}
		}

		float cur_alpha = transparency_over_life_.back().y();
		for (std::vector<float2>::const_iterator iter = transparency_over_life_.begin(); iter != transparency_over_life_.end() - 1; ++ iter)
		{
			if ((iter + 1)->x() >= pos)
			{
				float const s = (pos - iter->x()) / ((iter + 1)->x() - iter->x());
				cur_alpha = MathLib::lerp(iter->y(), (iter + 1)->y(), s);
				break;
			}
		}

		ParticleSystemPtr ps = ps_.lock();

		float buoyancy = 4.0f / 3 * PI * MathLib::cube(cur_size) * ps->MediaDensity() * ps->Gravity();
		float3 accel = (ps->Force() + float3(0, buoyancy, 0)) / cur_mass - float3(0, ps->Gravity(), 0);
		par.vel += accel * elapse_time;
		par.pos += par.vel * elapse_time;
		par.life -= elapse_time;
		par.spin += 0.001f;
		par.size = cur_size;
		par.alpha = cur_alpha;
	}
}
