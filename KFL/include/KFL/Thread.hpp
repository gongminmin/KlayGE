/**
 * @file Thread.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
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

#ifndef KFL_THREAD_HPP
#define KFL_THREAD_HPP

#pragma once

#include <condition_variable>
#include <functional>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4355) // Ignore "this" in member initializer list
#endif
#include <future>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <mutex>
#include <thread>
#include <vector>

#include <KFL/Noncopyable.hpp>

namespace KlayGE
{
	template <typename Threadable>
	inline std::future<std::invoke_result_t<Threadable>> CreateThread(Threadable func)
	{
		using result_t = std::invoke_result_t<Threadable>;

		auto task = std::packaged_task<result_t()>(std::move(func));
		auto ret = task.get_future();
		auto thread = std::thread(std::move(task));
		thread.detach();

		return ret;
	}

	class ThreadPool final
	{
		KLAYGE_NONCOPYABLE(ThreadPool);

		class CommonData;

		// A class used to storage information of a pooled thread. Each object of this class represents a pooled thread.
		//  It also has mechanisms to notify the pooled thread that it has a work to do. It also offers notification
		//  to definitively tell to the thread that it should die.
		struct ThreadInfo final
		{
			KLAYGE_NONCOPYABLE(ThreadInfo);

			explicit ThreadInfo(CommonData& data) noexcept;

			void WakeUp(std::function<void()> func);
			void Kill();

			std::function<void()> func_;
			bool wake_up_ = false;
			std::mutex wake_up_mutex_;
			std::condition_variable wake_up_cond_;
			CommonData* data_;
		};

		// A class used to storage information of the thread pool. It stores the pooled thread information container
		//  and the functor that will envelop users Threadable to return it to the pool.
		class CommonData final
		{
			KLAYGE_NONCOPYABLE(CommonData);

		public:
			CommonData(size_t num_min_cached_threads, size_t num_max_cached_threads);
			~CommonData();

			// Creates and adds more threads to the pool.
			void AddWaitingThreads(size_t number);

			size_t NumMinCachedThreads() const noexcept
			{
				return num_min_cached_threads_;
			}
			void NumMinCachedThreads(size_t num);

			size_t NumMaxCachedThreads() const noexcept
			{
				return num_max_cached_threads_;
			}
			void NumMaxCachedThreads(size_t num) noexcept
			{
				num_max_cached_threads_ = num;
			}

			template <typename Threadable>
			std::future<std::invoke_result_t<Threadable>> QueueThread(Threadable func)
			{
				using result_t = std::invoke_result_t<Threadable>;

				auto task = MakeSharedPtr<std::packaged_task<result_t()>>(std::move(func));
				auto ret = task->get_future();

				{
					std::lock_guard<std::mutex> lock(mutex_);

					// If there are no threads, add more to the pool
					if (threads_.empty())
					{
						this->AddWaitingThreadsLocked(lock, 1);
					}
					auto th_info = std::move(threads_.front());
					threads_.erase(threads_.begin());
					th_info->WakeUp([task]() { (*task)(); });
				}

				return ret;
			}

		private:
			// Creates and adds more threads to the pool. This function does not lock the pool mutex and that be
			//  only called when we externally have locked that mutex.
			void AddWaitingThreadsLocked(std::lock_guard<std::mutex> const& lock, size_t number);

			static void WaitFunction(std::shared_ptr<ThreadInfo> const& info);

		private:
			// Shared data between all threads in the pool
			size_t num_min_cached_threads_;
			size_t num_max_cached_threads_;
			std::mutex mutex_;
			bool general_cleanup_ = false;
			std::vector<std::shared_ptr<ThreadInfo>> threads_;
		};

	public:
		ThreadPool();
		ThreadPool(size_t num_min_cached_threads, size_t num_max_cached_threads);

		// Launches threadable function in a new thread. If there is a pooled thread available, reuses that thread.
		template <typename Threadable>
		std::future<std::invoke_result_t<Threadable>> QueueThread(Threadable func)
		{
			return data_->QueueThread(func);
		}

		size_t NumMinCachedThreads() const noexcept
		{
			return data_->NumMinCachedThreads();
		}
		void NumMinCachedThreads(size_t num)
		{
			data_->NumMinCachedThreads(num);
		}

		size_t NumMaxCachedThreads() const noexcept
		{
			return data_->NumMaxCachedThreads();
		}
		void NumMaxCachedThreads(size_t num) noexcept
		{
			data_->NumMaxCachedThreads(num);
		}

	private:
		std::unique_ptr<CommonData> data_;
	};
}

#endif		// KFL_THREAD_HPP
