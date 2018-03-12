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
#include <KlayGE/SceneObjectHelper.hpp>

#include <mutex>
#include <random>
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
		float alpha;

		float init_life;
	};

	class KLAYGE_CORE_API ParticleEmitter
	{
	public:
		explicit ParticleEmitter(SceneObjectPtr const & ps);
		virtual ~ParticleEmitter()
		{
		}

		virtual std::string const & Type() const = 0;
		virtual ParticleEmitterPtr Clone() = 0;

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

		uint32_t Update(float elapsed_time);
		virtual void Emit(Particle& par) = 0;

	protected:
		void DoClone(ParticleEmitterPtr const & rhs);

	protected:
		std::weak_ptr<ParticleSystem> ps_;

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
	};

	class KLAYGE_CORE_API ParticleUpdater
	{
	public:
		explicit ParticleUpdater(SceneObjectPtr const & ps);
		virtual ~ParticleUpdater()
		{
		}

		virtual std::string const & Type() const = 0;
		virtual ParticleUpdaterPtr Clone() = 0;

		virtual void Update(Particle& par, float elapse_time) = 0;

	protected:
		void DoClone(ParticleUpdaterPtr const & rhs);

	protected:
		std::weak_ptr<ParticleSystem> ps_;
	};

	class KLAYGE_CORE_API ParticleSystem : public SceneObjectHelper
	{
	public:
		explicit ParticleSystem(uint32_t max_num_particles);

		virtual ParticleSystemPtr Clone();

		void Gravity(float gravity)
		{
			gravity_ = gravity;
		}
		float Gravity() const
		{
			return gravity_;
		}
		void Force(float3 const & force)
		{
			force_ = force;
		}
		float3 const & Force() const
		{
			return force_;
		}
		void MediaDensity(float density)
		{
			media_density_ = density;
		}
		float MediaDensity() const
		{
			return media_density_;
		}

		ParticleEmitterPtr MakeEmitter(std::string_view type);
		ParticleUpdaterPtr MakeUpdater(std::string_view type);

		void AddEmitter(ParticleEmitterPtr const & emitter);
		void DelEmitter(ParticleEmitterPtr const & emitter);
		void ClearEmitters();
		uint32_t NumEmitters() const
		{
			return static_cast<uint32_t>(emitters_.size());
		}
		ParticleEmitterPtr Emitter(uint32_t index) const
		{
			BOOST_ASSERT(index < emitters_.size());
			return emitters_[index];
		}

		void AddUpdater(ParticleUpdaterPtr const & updater);
		void DelUpdater(ParticleUpdaterPtr const & updater);
		void ClearUpdaters();
		uint32_t NumUpdaters() const
		{
			return static_cast<uint32_t>(updaters_.size());
		}
		ParticleUpdaterPtr Updater(uint32_t index) const
		{
			BOOST_ASSERT(index < updaters_.size());
			return updaters_[index];
		}

		virtual void SubThreadUpdate(float app_time, float elapsed_time) override;
		virtual bool MainThreadUpdate(float app_time, float elapsed_time) override;

		uint32_t NumParticles() const
		{
			return static_cast<uint32_t>(particles_.size());
		}
		uint32_t NumActiveParticles() const;
		uint32_t GetActiveParticleIndex(uint32_t i) const;
		Particle const & GetParticle(uint32_t i) const
		{
			BOOST_ASSERT(i < particles_.size());
			return particles_[i];
		}
		Particle& GetParticle(uint32_t i)
		{
			BOOST_ASSERT(i < particles_.size());
			return particles_[i];
		}
		void ClearParticles();

		void ParticleAlphaFromTex(std::string const & tex_name);
		std::string const & ParticleAlphaFromTex() const
		{
			return particle_alpha_from_tex_name_;
		}
		void ParticleAlphaToTex(std::string const & tex_name);
		std::string const & ParticleAlphaToTex() const
		{
			return particle_alpha_to_tex_name_;
		}
		void ParticleColorFrom(Color const & clr);
		Color const & ParticleColorFrom() const
		{
			return particle_color_from_;
		}
		void ParticleColorTo(Color const & clr);
		Color const & ParticleColorTo() const
		{
			return particle_color_to_;
		}

		void SceneDepthTexture(TexturePtr const & depth_tex);

	private:
		void UpdateParticlesNoLock(float elapsed_time, std::vector<std::pair<uint32_t, float>>& active_particles);
		void UpdateParticleBufferNoLock(std::vector<std::pair<uint32_t, float>> const & active_particles);

	protected:
		std::vector<ParticleEmitterPtr> emitters_;
		std::vector<ParticleUpdaterPtr> updaters_;

		std::vector<Particle> particles_;
		std::vector<std::pair<uint32_t, float>> actived_particles_;
		mutable std::mutex actived_particles_mutex_;

		float gravity_;
		float3 force_;
		float media_density_;

		std::string particle_alpha_from_tex_name_;
		std::string particle_alpha_to_tex_name_;
		Color particle_color_from_;
		Color particle_color_to_;

		bool gs_support_;
	};

	KLAYGE_CORE_API ParticleSystemPtr SyncLoadParticleSystem(std::string const & psml_name);
	KLAYGE_CORE_API ParticleSystemPtr ASyncLoadParticleSystem(std::string const & psml_name);

	KLAYGE_CORE_API void SaveParticleSystem(ParticleSystemPtr const & ps, std::string const & psml_name);


	class KLAYGE_CORE_API PointParticleEmitter : public ParticleEmitter
	{
	public:
		explicit PointParticleEmitter(SceneObjectPtr const & ps);

		virtual std::string const & Type() const override;
		virtual ParticleEmitterPtr Clone() override;

		virtual void Emit(Particle& par) override;

	private:
		float RandomGen();

	private:
		std::ranlux24_base gen_;
		std::uniform_int_distribution<> random_dis_;
	};

	class KLAYGE_CORE_API PolylineParticleUpdater : public ParticleUpdater
	{
	public:
		explicit PolylineParticleUpdater(SceneObjectPtr const & ps);

		virtual std::string const & Type() const override;
		virtual ParticleUpdaterPtr Clone() override;

		void SizeOverLife(std::vector<float2> const & size_over_life)
		{
			std::lock_guard<std::mutex> lock(update_mutex_);
			size_over_life_ = size_over_life;
		}
		std::vector<float2> const & SizeOverLife() const
		{
			return size_over_life_;
		}
		void MassOverLife(std::vector<float2> const & mass_over_life)
		{
			std::lock_guard<std::mutex> lock(update_mutex_);
			mass_over_life_ = mass_over_life;
		}
		std::vector<float2> const & MassOverLife() const
		{
			return mass_over_life_;
		}
		void OpacityOverLife(std::vector<float2> const & opacity_over_life)
		{
			std::lock_guard<std::mutex> lock(update_mutex_);
			opacity_over_life_ = opacity_over_life;
		}
		std::vector<float2> const & OpacityOverLife() const
		{
			return opacity_over_life_;
		}

		virtual void Update(Particle& par, float elapse_time) override;

	private:
		std::mutex update_mutex_;
		std::vector<float2> size_over_life_;
		std::vector<float2> mass_over_life_;
		std::vector<float2> opacity_over_life_;
	};
}

#endif		// _PARTICLESYSTEM_HPP
