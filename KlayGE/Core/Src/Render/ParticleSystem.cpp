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
	uint32_t ParticleEmitter::Update(float elapsed_time)
	{
		return static_cast<uint32_t>(elapsed_time * emit_freq_ + 0.5f);
	}


	ParticleSystem::ParticleSystem(uint32_t max_num_particles)
		: particles_(max_num_particles)
	{
	}

	ParticleEmitterPtr ParticleSystem::MakeEmitter(std::string const & name)
	{
		ParticleEmitterPtr ret;
		if ("Cone" == name)
		{
			ret = MakeSharedPtr<ConeParticleEmitter>(this->shared_from_this());
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
		if ("Polyline" == name)
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

	void ParticleSystem::Update(float /*app_time*/, float elapsed_time)
	{
		KLAYGE_AUTO(emitter_iter, emitters_.begin());
		uint32_t new_particle = (*emitter_iter)->Update(elapsed_time);

		typedef KLAYGE_DECLTYPE(particles_) ParticlesType;
		KLAYGE_FOREACH(ParticlesType::reference particle, particles_)
		{
			if (particle.life > 0)
			{
				updater_->Update(particle, elapsed_time);
			}
			else
			{
				if (new_particle > 0)
				{
					(*emitter_iter)->Emit(particle);
					updater_->Update(particle, 0);
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


	ConeParticleEmitter::ConeParticleEmitter(ParticleSystemPtr const & ps)
		: ParticleEmitter(ps),
			random_dis_(0, 10000)
	{
	}

	void ConeParticleEmitter::Emit(Particle& par)
	{
		par.pos = MathLib::transform_coord(MathLib::lerp(min_pos_, max_pos_, this->RandomGen()), model_mat_);
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
		par.color = MathLib::lerp(min_clr_, max_clr_, this->RandomGen());
		par.init_life = par.life;
	}

	float ConeParticleEmitter::RandomGen()
	{
		return MathLib::clamp(random_dis_(gen_) * 0.0001f, 0.0f, 1.0f);
	}

	PolylineParticleUpdater::PolylineParticleUpdater(ParticleSystemPtr const & ps)
		: ParticleUpdater(ps),
			gravity_(0.5f), force_(0, 0, 0), media_density_(0.0f)
	{
	}

	void PolylineParticleUpdater::Force(float3 force)
	{
		force_ = force;
	}

	void PolylineParticleUpdater::MediaDensity(float density)
	{
		media_density_ = density;
	}

	void PolylineParticleUpdater::SizeOverLife(std::vector<float2> const & size_over_life)
	{
		size_over_life_ = size_over_life;
	}

	void PolylineParticleUpdater::WeightOverLife(std::vector<float2> const & weight_over_life)
	{
		weight_over_life_ = weight_over_life;
	}

	void PolylineParticleUpdater::TransparencyOverLife(std::vector<float2> const & transparency_over_life)
	{
		transparency_over_life_ = transparency_over_life;
	}

	void PolylineParticleUpdater::ColorFromTo(Color const & from, Color const & to)
	{
		clr_from_ = from;
		clr_to_ = to;
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
