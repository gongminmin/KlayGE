// ParticleSystem.hpp
// KlayGE 例子系统类 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.7.12)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _PARTICLESYSTEM_HPP
#define _PARTICLESYSTEM_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Math.hpp>

#include <vector>

namespace KlayGE
{
	template <typename ParticleType>
	class ParticleSystem
	{
	public:
		ParticleSystem(uint32_t max_num_particles,
						boost::function<void(ParticleType& par, float4x4 const & mat)> const & gen_func,
						boost::function<void(ParticleType& par, float elapse_time)> const & update_func)
			: particles_(max_num_particles),
				gen_func_(gen_func), update_func_(update_func),
				inv_emit_freq_(0), accumulate_time_(0),
				model_mat_(float4x4::Identity())
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

		void AutoEmit(float freq)
		{
			inv_emit_freq_ = 1.0f / freq;

			float time = 0;
			for (std::vector<ParticleType>::iterator iter = particles_.begin();
				iter != particles_.end(); ++ iter)
			{
				iter->birth_time = time;
				time += inv_emit_freq_;
			}
		}

		void Update(float elapse_time)
		{
			accumulate_time_ += elapse_time;
			if (accumulate_time_ >= particles_.size() * inv_emit_freq_)
			{
				accumulate_time_ = 0;
			}

			for (std::vector<ParticleType>::iterator iter = particles_.begin();
				iter != particles_.end(); ++ iter)
			{
				if (iter->life > 0)
				{
					update_func_(*iter, elapse_time);
				}
				else
				{
					float const t = accumulate_time_ - iter->birth_time;
					if ((t >= 0) && (t < elapse_time))
					{
						gen_func_(*iter, model_mat_);
					}
				}
			}
		}

		uint32_t NumParticles() const
		{
			return static_cast<uint32_t>(particles_.size());
		}

		ParticleType const & GetParticle(uint32_t i) const
		{
			assert(i < particles_.size());
			return particles_[i];
		}
		ParticleType& GetParticle(uint32_t i)
		{
			assert(i < particles_.size());
			return particles_[i];
		}

	protected:
		boost::function<void(ParticleType& par, float4x4 const & mat)> gen_func_;
		boost::function<void(ParticleType& par, float elapse_time)> update_func_;

		std::vector<ParticleType> particles_;

		float inv_emit_freq_;
		float accumulate_time_;

		float4x4 model_mat_;
	};

	template <typename ParticleType>
	class UpdateParticleFunc
	{
	public:
		void operator()(ParticleType& par, float elapse_time)
		{
			par.pos += par.vel * elapse_time;
			par.life -= elapse_time;
		}
	};
}

#endif		// _PARTICLESYSTEM_HPP
