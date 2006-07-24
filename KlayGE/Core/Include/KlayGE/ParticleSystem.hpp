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
			: max_num_particles_(max_num_particles),
				gen_func_(gen_func), update_func_(update_func),
				auto_emit_freq_(0), accumulate_time_(0),
				model_mat_(float4x4::Identity())
		{
		}

		virtual ~ParticleSystem()
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

		void Emit(uint32_t num)
		{
			for (uint32_t i = 0; i < num; ++ i)
			{
				this->EmitOne();
			}
		}

		void AutoEmit(float freq)
		{
			auto_emit_freq_ = freq;
		}

		void Update(float elapse_time)
		{
			if (auto_emit_freq_ > 0)
			{
				accumulate_time_ += elapse_time;

				if (accumulate_time_ >= 1.0f / auto_emit_freq_)
				{
					this->EmitOne();
					accumulate_time_ = 0;
				}
			}

			this->UpdateAll(elapse_time);
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

	protected:
		virtual void EmitOne()
		{
			if (particles_.size() >= max_num_particles_)
			{
				for (std::vector<ParticleType>::iterator iter = particles_.begin();
					iter != particles_.end(); ++ iter)
				{
					if (iter->life <= 0)
					{
						gen_func_(*iter, model_mat_);
						break;
					}
				}
			}
			else
			{
				ParticleType par;
				gen_func_(par, model_mat_);
				particles_.push_back(par);
			}
		}

		virtual void UpdateAll(float elapse_time)
		{
			for (std::vector<ParticleType>::iterator iter = particles_.begin();
				iter != particles_.end(); ++ iter)
			{
				if (iter->life > 0)
				{
					update_func_(*iter, elapse_time);
				}
			}
		}

	protected:
		uint32_t max_num_particles_;

		boost::function<void(ParticleType& par, float4x4 const & mat)> gen_func_;
		boost::function<void(ParticleType& par, float elapse_time)> update_func_;

		std::vector<ParticleType> particles_;

		float auto_emit_freq_;
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
