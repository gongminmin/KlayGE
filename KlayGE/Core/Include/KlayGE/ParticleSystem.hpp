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

namespace KlayGE
{
	template <typename ParticleType>
	class ParticleSystem
	{
	public:
		ParticleSystem(uint32_t max_num_particles, boost::function<void(ParticleType& par)> const & gen_func,
						boost::function<void(ParticleType& par, float elapse_time)> const & update_func)
			: max_num_particles_(max_num_particles),
				gen_func_(gen_func), update_func_(update_func),
				auto_emit_freq_(0), total_elapse_time_(0)
		{
		}

		void EmitOne()
		{
			bool emit = false;
			if (particles_.size() >= max_num_particles_)
			{
				bool found = false;
				for (std::deque<ParticleType>::iterator iter = particles_.begin();
					iter != particles_.end(); ++ iter)
				{
					if (iter->life <= 0)
					{
						gen_func_(*iter);
						found = true;
					}
				}

				if (!found)
				{
					emit = true;
				}
			}
			else
			{
				emit = true;
			}

			if (emit)
			{
				ParticleType par;
				gen_func_(par);
				particles_.push_back(par);
			}
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
				total_elapse_time_ += elapse_time;

				if (total_elapse_time_ >= 1.0f / auto_emit_freq_)
				{
					this->EmitOne();
					total_elapse_time_ = 0;
				}
			}

			for (std::deque<ParticleType>::iterator iter = particles_.begin();
				iter != particles_.end(); ++ iter)
			{
				if (iter->life > 0)
				{
					update_func_(*iter, elapse_time);
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

	private:
		uint32_t max_num_particles_;

		boost::function<void(ParticleType& par)> gen_func_;
		boost::function<void(ParticleType& par, float elapse_time)> update_func_;

		std::deque<ParticleType> particles_;

		float auto_emit_freq_;
		float total_elapse_time_;
	};

	template <typename ParticleType>
	class UpdateParticleFunc
	{
	public:
		void operator()(ParticleType& par, float elapse_time)
		{
			par.vel += par.accel * elapse_time;
			par.pos += par.vel * elapse_time;
			par.life -= elapse_time;
		}
	};
}

#endif		// _PARTICLESYSTEM_HPP
