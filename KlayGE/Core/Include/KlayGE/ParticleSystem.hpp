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
		ParticleSystem(boost::function<void(ParticleType& par)> const & gen_func,
						boost::function<void(ParticleType& par)> const & update_func)
			: gen_func_(gen_func), update_func_(update_func)
		{
		}

		void GenParticles(uint32_t num)
		{
			for (uint32_t i = 0; i < num; ++ i)
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
					ParticleType par;
					gen_func_(par);
					particles_.push_back(par);
				}
			}
		}

		void Update()
		{
			for (std::deque<ParticleType>::iterator iter = particles_.begin();
				iter != particles_.end(); ++ iter)
			{
				if (iter->life > 0)
				{
					update_func_(*iter);
				}
			}
		}

		uint32_t NumParticles() const
		{
			return static_cast<uint32_t>(particles_.size());
		}

		ParticleType const & Particles(uint32_t i) const
		{
			assert(i < particles_.size());
			return particles_[i];
		}

	private:
		boost::function<void(ParticleType& par)> gen_func_;
		boost::function<void(ParticleType& par)> update_func_;

		std::deque<ParticleType> particles_;
	};

	template <typename ParticleType>
	class UpdateParticleFunc
	{
	public:
		void operator()(ParticleType& par)
		{
			par.vel += par.accel;
			par.pos += par.vel;
			-- par.life;
		}
	};
}

#endif		// _PARTICLESYSTEM_HPP
