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

#include <KFL/Noncopyable.hpp>
#include <KFL/Operators.hpp>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

#include <nonstd/scope.hpp>

namespace KlayGE
{
	namespace Signal
	{
		namespace Detail
		{
			class SignalBase;
		}

		class KLAYGE_CORE_API Connection final
		{
			KLAYGE_NONCOPYABLE(Connection);

		public:
			Connection() noexcept;
			Connection(Connection&& rhs) noexcept;
			Connection(Detail::SignalBase& signal, std::shared_ptr<void> const& slot);

			Connection& operator=(Connection&& rhs) noexcept;

			void Disconnect();

			bool Connected() const;

			void Swap(Connection& rhs);

			Detail::SignalBase& Signal() const noexcept;
			void* Slot() const noexcept;

			bool operator==(Connection const& rhs) const;
			bool operator<(Connection const& rhs) const;

			KLAYGE_DEFAULT_LESS_COMPARE_OPERATOR(Connection);
			KLAYGE_DEFAULT_EQUALITY_COMPARE_OPERATOR(Connection);

		private:
			Detail::SignalBase* signal_ = nullptr;
			std::weak_ptr<void> slot_;
		};

		namespace Detail
		{
			// CLR doesn't support std::mutex. Need a wrapper.
			class KLAYGE_CORE_API Mutex
			{
				KLAYGE_NONCOPYABLE(Mutex);

			public:
				Mutex() noexcept;
				virtual ~Mutex() noexcept;

				virtual void Lock() = 0;
				virtual void Unlock() = 0;
			};

			KLAYGE_CORE_API std::unique_ptr<Mutex> CreateMutex();

			class KLAYGE_CORE_API SignalBase
			{
				KLAYGE_NONCOPYABLE(SignalBase);

				friend class KlayGE::Signal::Connection;

			public:
				SignalBase() noexcept;
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
					auto on_exit = nonstd::make_scope_exit([this] { mutex_->Unlock(); });
					mutex_->Lock();

					slots_.emplace_back(MakeSharedPtr<CallbackFunction>(cb));
					return Connection(*this, std::static_pointer_cast<void>(slots_.back()));
				}

				void Disconnect(Connection const& connection)
				{
					auto on_exit = nonstd::make_scope_exit([this] { mutex_->Unlock(); });
					mutex_->Lock();

					BOOST_ASSERT(&connection.Signal() == this);
					this->Disconnect(connection.Slot());
				}

				CombinerResultType operator()(Args... args) const
				{
					auto on_exit = nonstd::make_scope_exit([this] { mutex_->Unlock(); });
					mutex_->Lock();

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
					auto on_exit = nonstd::make_scope_exit([this] { mutex_->Unlock(); });
					mutex_->Lock();

					return slots_.size();
				}

				bool Empty() const
				{
					auto on_exit = nonstd::make_scope_exit([this] { mutex_->Unlock(); });
					mutex_->Lock();

					return slots_.empty();
				}

				void Swap(SignalTemplateBase& rhs)
				{
					auto on_exit = nonstd::make_scope_exit([this, rhs] {
						rhs.mutex_->Unlock();
						mutex_->Unlock();
					});
					mutex_->Lock();
					rhs.mutex_->Lock();

					std::swap(slots_, rhs.slots_);
				}

			private:
				void Disconnect(void* slot_void) override
				{
					auto iter = std::remove_if(slots_.begin(), slots_.end(), [slot_void](std::shared_ptr<CallbackFunction> const& slot) {
						return reinterpret_cast<void*>(slot.get()) == slot_void;
					});
					slots_.erase(iter, slots_.end());
				}

			private:
				std::vector<std::shared_ptr<CallbackFunction>> slots_;
				std::unique_ptr<Mutex> mutex_ = CreateMutex();
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
