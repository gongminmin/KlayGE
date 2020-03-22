/**
 * @file Signal.hpp
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

#ifndef KLAYGE_CORE_BASE_SIGNAL_HPP
#define KLAYGE_CORE_BASE_SIGNAL_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <algorithm>
#include <functional>
#include <memory>
#ifndef __CLR_VER
#include <mutex>
#endif
#include <vector>

#include <boost/noncopyable.hpp>

namespace KlayGE
{
	namespace Signal
	{
		namespace Detail
		{
			class SignalBase;
		}

		class KLAYGE_CORE_API Connection final : boost::noncopyable
		{
		public:
			Connection() noexcept;
			Connection(Connection&& rhs) noexcept;
			Connection(Detail::SignalBase& signal, std::shared_ptr<void> const& slot);

			Connection& operator=(Connection&& rhs) noexcept;

			void Disconnect();

			bool Connected() const;

			void Swap(Connection& rhs);

			Detail::SignalBase& Signal() const;
			void* Slot() const;

		private:
			Detail::SignalBase* signal_ = nullptr;
			std::weak_ptr<void> slot_;
		};

		bool operator==(Connection const& lhs, Connection const& rhs);
		bool operator!=(Connection const& lhs, Connection const& rhs);
		bool operator<(Connection const& lhs, Connection const& rhs);

		namespace Detail
		{
			class KLAYGE_CORE_API SignalBase : boost::noncopyable
			{
				friend class KlayGE::Signal::Connection;

			public:
				virtual ~SignalBase() noexcept;

			private:
				virtual void Disconnect(void* slot) = 0;
			};


			template <typename Combiner, typename R>
			struct CombinerInvocation;

			template <typename Combiner, typename R, typename... Args>
			struct CombinerInvocation<Combiner, R(Args...)>
			{
				bool Invoke(Combiner& combiner, std::function<R(Args...)> const& cbf, Args... args) const
				{
					return combiner(cbf(args...));
				}
			};

			template <typename Combiner, typename... Args>
			struct CombinerInvocation<Combiner, void(Args...)>
			{
				bool Invoke(Combiner& combiner, std::function<void(Args...)> const& cbf, Args... args) const
				{
					cbf(args...);
					return combiner();
				}
			};


			template <typename Combiner, typename R>
			class SignalTemplateBase;

			template <typename Combiner, typename R, typename... Args>
			class SignalTemplateBase<R(Args...), Combiner> : public SignalBase, private CombinerInvocation<Combiner, R(Args...)>
			{
			protected:
				using CallbackFunction = std::function<R(Args...)>;
				using CombinerResultType = typename Combiner::ResultType;

			public:
				Connection Connect(CallbackFunction const& cb)
				{
#ifndef __CLR_VER
					std::unique_lock<std::mutex> lock(mutex_);
#endif

					slots_.emplace_back(MakeSharedPtr<CallbackFunction>(cb));
					return Connection(*this, std::static_pointer_cast<void>(slots_.back()));
				}

				void Disconnect(Connection const& connection)
				{
#ifndef __CLR_VER
					std::unique_lock<std::mutex> lock(mutex_);
#endif

					BOOST_ASSERT(&connection.Signal() == this);
					this->Disconnect(connection.Slot());
				}

				CombinerResultType operator()(Args... args) const
				{
#ifndef __CLR_VER
					std::unique_lock<std::mutex> lock(mutex_);
#endif

					Combiner combiner;
					for (auto const& slot : slots_)
					{
						if (slot)
						{
							if (!this->Invoke(combiner, *slot, args...))
							{
								break;
							}
						}
					}
					return combiner.Result();
				}

				size_t Size() const
				{
#ifndef __CLR_VER
					std::unique_lock<std::mutex> lock(mutex_);
#endif
					return slots_.size();
				}

				bool Empty() const
				{
#ifndef __CLR_VER
					std::unique_lock<std::mutex> lock(mutex_);
#endif
					return slots_.empty();
				}

				void Swap(SignalTemplateBase& rhs)
				{
#ifndef __CLR_VER
					std::unique_lock<std::mutex> lhs_lock(mutex_, std::defer_lock);
					std::unique_lock<std::mutex> rhs_lock(rhs.mutex_, std::defer_lock);
					std::lock(lhs_lock, rhs_lock);
#endif

					std::swap(slots_, rhs.slots_);
				}

			private:
				void Disconnect(void* slot_void) override
				{
#ifndef __CLR_VER
					std::unique_lock<std::mutex> lock(mutex_);
#endif

					auto iter = std::remove_if(slots_.begin(), slots_.end(), [slot_void](std::shared_ptr<CallbackFunction> const& slot) {
						return reinterpret_cast<void*>(slot.get()) == slot_void;
					});
					slots_.erase(iter, slots_.end());
				}

			private:
				std::vector<std::shared_ptr<CallbackFunction>> slots_;
#ifndef __CLR_VER
				mutable std::mutex mutex_;
#endif
			};
		} // namespace Detail

		template <typename R>
		struct CombinerDefault final
		{
			using ResultType = R;

			CombinerDefault() : last_()
			{
			}

			bool operator()(ResultType r)
			{
				last_ = r;
				return true;
			}

			ResultType Result()
			{
				return last_;
			}

		private:
			ResultType last_;
		};

		template <>
		struct CombinerDefault<void> final
		{
			using ResultType = void;

			bool operator()()
			{
				return true;
			}

			void Result()
			{
			}
		};

		template <typename SignalSignature, typename Combiner = CombinerDefault<typename std::function<SignalSignature>::result_type>>
		class Signal final : public Detail::SignalTemplateBase<SignalSignature, Combiner>
		{
		public:
			using SignalTemplateBase = Detail::SignalTemplateBase<SignalSignature, Combiner>;
			using CallbackFunction = typename SignalTemplateBase::CallbackFunction;
		};
	} // namespace Signal
} // namespace KlayGE

#endif // KLAYGE_CORE_BASE_SIGNAL_HPP
