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
		float birth_time;
		float size;
		Color color;
	};

	class KLAYGE_CORE_API ParticleSystem
	{
	public:
		ParticleSystem(uint32_t max_num_particles,
			function<void(Particle& par, float4x4 const & mat)> const & emitter_func,
			function<void(Particle& par, float elapse_time)> const & update_func);

		void ModelMatrix(float4x4 const & model);
		float4x4 const & ModelMatrix() const;

		void Frequency(float freq);
		float Frequency() const;

		void Update(float app_time, float elapsed_time);

		uint32_t NumParticles() const;

		Particle const & GetParticle(uint32_t i) const;
		Particle& GetParticle(uint32_t i);

	protected:
		function<void(Particle& par, float4x4 const & mat)> emitter_func_;
		function<void(Particle& par, float elapse_time)> update_func_;

		std::vector<Particle> particles_;

		float inv_emit_freq_;
		float accumulate_time_;

		float4x4 model_mat_;
	};

	class KLAYGE_CORE_API ConeParticleEmitter
	{
	public:
		ConeParticleEmitter();

		void MaxPositionDeviation(float3 const & dev);
		float3 const & MaxPositionDeviation() const;

		void EmitAngle(float angle);
		float EmitAngle() const;

		void InitLife(float life);
		float InitLife() const;
		void InitSize(float size);
		float InitSize() const;
		void InitColor(Color const & clr);
		Color const & InitColor() const;

		void InitMinVelocity(float min_vel);
		void InitMaxVelocity(float max_vel);

		void operator()(Particle& par, float4x4 const & mat);

	private:
		float RandomGen();

	private:
		ranlux24_base gen_;
		uniform_int_distribution<> random_dis_;

		float3 max_position_deviation_;
		float emit_angle_;

		float min_velocity_;
		float max_velocity_;

		float init_life_;
		float init_size_;
		Color init_color_;
	};

	class KLAYGE_CORE_API ParticleUpdater
	{
	public:
		ParticleUpdater();

		void InitLife(float life);

		void Force(float3 force);

		void MediaDensity(float density);

		void SizeOverLife(std::vector<float2> const & size_over_life);
		void WeightOverLife(std::vector<float2> const & weight_over_life);
		void TransparencyOverLife(std::vector<float2> const & transparency_over_life);
		
		void ColorFromTo(Color const & from, Color const & to);

		void operator()(Particle& par, float elapse_time);

	private:
		float init_life_;
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
