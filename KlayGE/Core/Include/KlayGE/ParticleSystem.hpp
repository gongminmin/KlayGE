/**
 * @file ParticleSystem.hpp
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

#ifndef _PARTICLESYSTEM_HPP
#define _PARTICLESYSTEM_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Math.hpp>

#include <vector>

namespace KlayGE
{
	struct Particle
	{
		float3 pos;
		float3 vel;
		float life;
		float spin;
		float size;
		Color color;

		float init_life;
	};

	class KLAYGE_CORE_API ParticleEmitter
	{
	public:
		explicit ParticleEmitter(ParticleSystemPtr const & ps)
			: ps_(ps), model_mat_(float4x4::Identity())
		{
		}
		virtual ~ParticleEmitter()
		{
		}

		void ModelMatrix(float4x4 const & model)
		{
			model_mat_ = model;
		}
		float4x4 const & ModelMatrix() const
		{
			return model_mat_;
		}

		void Frequency(float freq)
		{
			emit_freq_ = freq;
		}
		float Frequency() const
		{
			return emit_freq_;
		}

		void EmitAngle(float angle)
		{
			emit_angle_ = angle;
		}
		float EmitAngle() const
		{
			return emit_angle_;
		}

		void MinPosition(float3 const & pos)
		{
			min_pos_ = pos;
		}
		void MaxPosition(float3 const & pos)
		{
			max_pos_ = pos;
		}
		float3 const & MinPosition() const
		{
			return min_pos_;
		}
		float3 const & MaxPosition() const
		{
			return max_pos_;
		}

		void MinVelocity(float vel)
		{
			min_vel_ = vel;
		}
		void MaxVelocity(float vel)
		{
			max_vel_ = vel;
		}
		float MinVelocity() const
		{
			return min_vel_;
		}
		float MaxVelocity() const
		{
			return max_vel_;
		}

		void MinLife(float life)
		{
			min_life_ = life;
		}
		void MaxLife(float life)
		{
			max_life_ = life;
		}
		float MinLife() const
		{
			return min_life_;
		}
		float MaxLife() const
		{
			return max_life_;
		}

		void MinSpin(float spin)
		{
			min_spin_ = spin;
		}
		void MaxSpin(float spin)
		{
			max_spin_ = spin;
		}
		float MinSpin() const
		{
			return min_spin_;
		}
		float MaxSpin() const
		{
			return max_spin_;
		}

		void MinSize(float size)
		{
			min_size_ = size;
		}
		void MaxSize(float size)
		{
			max_size_ = size;
		}
		float MinSize() const
		{
			return min_size_;
		}
		float MaxSize() const
		{
			return max_size_;
		}

		void MinColor(Color const & clr)
		{
			min_clr_ = clr;
		}
		void MaxColor(Color const & clr)
		{
			max_clr_ = clr;
		}
		Color const & MinColor() const
		{
			return min_clr_;
		}
		Color const & MaxColor() const
		{
			return max_clr_;
		}

		uint32_t Update(float elapsed_time);
		virtual void Emit(Particle& par) = 0;

	protected:
		weak_ptr<ParticleSystem> ps_;

		float emit_freq_;
		
		float4x4 model_mat_;
		float emit_angle_;

		float3 min_pos_;
		float3 max_pos_;
		float min_vel_;
		float max_vel_;
		float min_life_;
		float max_life_;
		float min_spin_;
		float max_spin_;
		float min_size_;
		float max_size_;
		Color min_clr_;
		Color max_clr_;
	};

	class KLAYGE_CORE_API ParticleUpdater
	{
	public:
		explicit ParticleUpdater(ParticleSystemPtr const & ps)
			: ps_(ps)
		{
		}
		virtual ~ParticleUpdater()
		{
		}

		virtual void Force(float3 force) = 0;
		virtual void MediaDensity(float density) = 0;

		virtual void Update(Particle& par, float elapse_time) = 0;

	private:
		weak_ptr<ParticleSystem> ps_;
	};

	class KLAYGE_CORE_API ParticleSystem : public enable_shared_from_this<ParticleSystem>
	{
	public:
		explicit ParticleSystem(uint32_t max_num_particles);

		ParticleEmitterPtr MakeEmitter(std::string const & name);
		ParticleUpdaterPtr MakeUpdater(std::string const & name);

		void AddEmitter(ParticleEmitterPtr const & emitter);
		void DelEmitter(ParticleEmitterPtr const & emitter);
		ParticleEmitterPtr Emitter(uint32_t index) const
		{
			BOOST_ASSERT(index < emitters_.size());
			return emitters_[index];
		}

		void Updater(ParticleUpdaterPtr const & updater)
		{
			updater_ = updater;
		}
		ParticleUpdaterPtr Updater() const
		{
			return updater_;
		}

		void Update(float app_time, float elapsed_time);

		uint32_t NumParticles() const;

		Particle const & GetParticle(uint32_t i) const;
		Particle& GetParticle(uint32_t i);

	protected:
		std::vector<ParticleEmitterPtr> emitters_;
		ParticleUpdaterPtr updater_;

		std::vector<Particle> particles_;
	};

	class KLAYGE_CORE_API ConeParticleEmitter : public ParticleEmitter
	{
	public:
		explicit ConeParticleEmitter(ParticleSystemPtr const & ps);

		virtual void Emit(Particle& par) KLAYGE_OVERRIDE;

	private:
		float RandomGen();

	private:
		ranlux24_base gen_;
		uniform_int_distribution<> random_dis_;
	};

	class KLAYGE_CORE_API PolylineParticleUpdater : public ParticleUpdater
	{
	public:
		explicit PolylineParticleUpdater(ParticleSystemPtr const & ps);

		virtual void Force(float3 force) KLAYGE_OVERRIDE;
		virtual void MediaDensity(float density) KLAYGE_OVERRIDE;

		void SizeOverLife(std::vector<float2> const & size_over_life);
		void WeightOverLife(std::vector<float2> const & weight_over_life);
		void TransparencyOverLife(std::vector<float2> const & transparency_over_life);
		
		void ColorFromTo(Color const & from, Color const & to);

		virtual void Update(Particle& par, float elapse_time) KLAYGE_OVERRIDE;

	private:
		float gravity_;
		float3 force_;
		float media_density_;

		std::vector<float2> size_over_life_;
		std::vector<float2> weight_over_life_;
		std::vector<float2> transparency_over_life_;

		Color clr_from_, clr_to_;
	};
}

#endif		// _PARTICLESYSTEM_HPP
