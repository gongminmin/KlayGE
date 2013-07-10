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

#include <KlayGE/ParticleSystem.hpp>

namespace KlayGE
{
	ParticleSystem::ParticleSystem(uint32_t max_num_particles,
					function<void(Particle& par, float4x4 const & mat)> const & emitter_func,
					function<void(Particle& par, float elapse_time)> const & update_func)
		: emitter_func_(emitter_func), update_func_(update_func),
			particles_(max_num_particles),
			inv_emit_freq_(0), accumulate_time_(0),
			model_mat_(float4x4::Identity())
	{
	}

	void ParticleSystem::ModelMatrix(float4x4 const & model)
	{
		model_mat_ = model;
	}

	float4x4 const & ParticleSystem::ModelMatrix() const
	{
		return model_mat_;
	}

	void ParticleSystem::Frequency(float freq)
	{
		inv_emit_freq_ = 1.0f / freq;

		float time = 0;
		typedef KLAYGE_DECLTYPE(particles_) ParticlesType;
		KLAYGE_FOREACH(ParticlesType::reference particle, particles_)
		{
			particle.life = -1;
			particle.birth_time = time;
			time += inv_emit_freq_;
		}
	}

	float ParticleSystem::Frequency() const
	{
		return 1.0f / inv_emit_freq_;
	}

	void ParticleSystem::Update(float /*app_time*/, float elapsed_time)
	{
		accumulate_time_ += elapsed_time;
		if (accumulate_time_ >= particles_.size() * inv_emit_freq_)
		{
			accumulate_time_ = 0;
		}

		typedef KLAYGE_DECLTYPE(particles_) ParticlesType;
		KLAYGE_FOREACH(ParticlesType::reference particle, particles_)
		{
			if (particle.life > 0)
			{
				update_func_(particle, elapsed_time);
			}
			else
			{
				float const t = accumulate_time_ - particle.birth_time;
				if ((t >= 0) && (t < elapsed_time))
				{
					emitter_func_(particle, model_mat_);
				}
			}
		}
	}

	uint32_t ParticleSystem::NumParticles() const
	{
		return static_cast<uint32_t>(particles_.size());
	}

	Particle const & ParticleSystem::GetParticle(uint32_t i) const
	{
		BOOST_ASSERT(i < particles_.size());
		return particles_[i];
	}

	Particle& ParticleSystem::GetParticle(uint32_t i)
	{
		BOOST_ASSERT(i < particles_.size());
		return particles_[i];
	}


	ConeParticleEmitter::ConeParticleEmitter()
		: random_dis_(-10000, +10000),
			emit_angle_(PI / 3), init_life_(3)
	{
	}

	void ConeParticleEmitter::MaxPositionDeviation(float3 const & dev)
	{
		max_position_deviation_ = dev;
	}

	float3 const & ConeParticleEmitter::MaxPositionDeviation() const
	{
		return max_position_deviation_;
	}

	void ConeParticleEmitter::EmitAngle(float angle)
	{
		emit_angle_ = angle;
	}

	float ConeParticleEmitter::EmitAngle() const
	{
		return emit_angle_;
	}

	void ConeParticleEmitter::InitLife(float life)
	{
		init_life_ = life;
	}

	float ConeParticleEmitter::InitLife() const
	{
		return init_life_;
	}

	void ConeParticleEmitter::InitSize(float size)
	{
		init_size_ = size;
	}

	float ConeParticleEmitter::InitSize() const
	{
		return init_size_;
	}

	void ConeParticleEmitter::InitColor(Color const & clr)
	{
		init_color_ = clr;
	}

	Color const & ConeParticleEmitter::InitColor() const
	{
		return init_color_;
	}

	void ConeParticleEmitter::InitMinVelocity(float min_vel)
	{
		min_velocity_ = min_vel;
	}

	void ConeParticleEmitter::InitMaxVelocity(float max_vel)
	{
		max_velocity_ = max_vel;
	}

	void ConeParticleEmitter::operator()(Particle& par, float4x4 const & mat)
	{
		float x = this->RandomGen() * max_position_deviation_.x();
		float y = this->RandomGen()* max_position_deviation_.y();
		float z = this->RandomGen()* max_position_deviation_.z();
		par.pos = MathLib::transform_coord(float3(x, y, z), mat);
		float theta = this->RandomGen() * PI;
		float phi = abs(this->RandomGen()) * emit_angle_ / 2;
		float velocity = (this->RandomGen() * 0.5f + 0.5f) * (max_velocity_ - min_velocity_) + min_velocity_;
		float vx = cos(theta) * sin(phi);
		float vz = sin(theta) * sin(phi);
		float vy = cos(phi);
		par.vel = MathLib::transform_normal(float3(vx, vy, vz) * velocity, mat);
		par.life = init_life_;
		par.spin = this->RandomGen() * PI / 2;
		par.size = init_size_;
		par.color = init_color_;
	}

	float ConeParticleEmitter::RandomGen()
	{
		return MathLib::clamp(random_dis_(gen_) * 0.0001f, -1.0f, +1.0f);
	}

	ParticleUpdater::ParticleUpdater()
		: gravity_(0.1f),
			force_(0, 0, 0),
			media_density_(0.0f)
	{
	}

	void ParticleUpdater::InitLife(float life)
	{
		init_life_ = life;
	}

	void ParticleUpdater::Force(float3 force)
	{
		force_ = force;
	}

	void ParticleUpdater::MediaDensity(float density)
	{
		media_density_ = density;
	}

	void ParticleUpdater::SizeOverLife(std::vector<float2> const & size_over_life)
	{
		size_over_life_ = size_over_life;
	}

	void ParticleUpdater::WeightOverLife(std::vector<float2> const & weight_over_life)
	{
		weight_over_life_ = weight_over_life;
	}

	void ParticleUpdater::TransparencyOverLife(std::vector<float2> const & transparency_over_life)
	{
		transparency_over_life_ = transparency_over_life;
	}

	void ParticleUpdater::ColorFromTo(Color const & from, Color const & to)
	{
		clr_from_ = from;
		clr_to_ = to;
	}

	void ParticleUpdater::operator()(Particle& par, float elapse_time)
	{
		float pos = (init_life_ - par.life) / init_life_;

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

		float cur_weight = weight_over_life_.back().y();
		for (std::vector<float2>::const_iterator iter = weight_over_life_.begin(); iter != weight_over_life_.end() - 1; ++ iter)
		{
			if ((iter + 1)->x() >= pos)
			{
				float const s = (pos - iter->x()) / ((iter + 1)->x() - iter->x());
				cur_weight = MathLib::lerp(iter->y(), (iter + 1)->y(), s);
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

		float buoyancy = cur_size * media_density_;
		float3 accel = (force_ + float3(0, buoyancy, 0)) / cur_weight - float3(0, gravity_, 0);
		par.vel += accel * elapse_time;
		par.pos += par.vel * elapse_time;
		par.life -= elapse_time;
		par.spin += 0.001f;
		par.size = cur_size;
		*reinterpret_cast<float4*>(&par.color) = MathLib::lerp(*reinterpret_cast<float4*>(&clr_from_),
			*reinterpret_cast<float4*>(&clr_to_), pos);
		par.color.a() = cur_alpha;
	}
}
