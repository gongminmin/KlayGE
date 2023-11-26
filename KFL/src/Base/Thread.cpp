/**
 * @file Thread.cpp
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

#include <KFL/Thread.hpp>

#include <condition_variable>
#include <mutex>
#include <vector>

#include <boost/assert.hpp>

#include <KFL/CpuInfo.hpp>

namespace KlayGE
{
	// A class used to storage information of the thread pool. It stores the pooled thread information container
	//  and the functor that will envelop users Threadable to return it to the pool.
	class ThreadPool::Impl final
	{
		KLAYGE_NONCOPYABLE(Impl);

		// A class used to storage information of a pooled thread. Each object of this class represents a pooled thread.
		//  It also has mechanisms to notify the pooled thread that it has a work to do. It also offers notification
		//  to definitively tell to the thread that it should die.
		class ThreadInfo final
		{
			friend class Impl;

			KLAYGE_NONCOPYABLE(ThreadInfo);

		public:
			explicit ThreadInfo(Impl& data) noexcept : data_(&data)
			{
			}

			void WakeUp(std::function<void()> func)
			{
				func_ = std::move(func);
				{
					std::lock_guard<std::mutex> lock(wake_up_mutex_);
					wake_up_ = true;
					wake_up_cond_.notify_one();
				}
			}

			// Wakes up a pooled thread saying it should die
			void Kill()
			{
				std::lock_guard<std::mutex> lock(wake_up_mutex_);
				func_ = std::function<void()>();
				wake_up_ = true;
				wake_up_cond_.notify_one();
			}

		private:
			std::function<void()> func_;
			bool wake_up_ = false;
			std::mutex wake_up_mutex_;
			std::condition_variable wake_up_cond_;
			Impl* data_;
		};

	public:
		Impl(uint32_t num_min_cached_threads, uint32_t num_max_cached_threads)
			: num_min_cached_threads_(num_min_cached_threads), num_max_cached_threads_(num_max_cached_threads)
		{
		}
		~Impl()
		{
			std::lock_guard<std::mutex> lock(mutex_);

			// Notify cleanup command to not queued threads
			general_cleanup_ = true;

			for (auto& th_info : threads_)
			{
				th_info->Kill();
			}
			threads_.clear();
		}

		// Creates and adds more threads to the pool.
		void AddWaitingThreads(uint32_t number)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			this->AddWaitingThreadsLocked(lock, number);
		}

		uint32_t NumMinCachedThreads() const noexcept
		{
			return num_min_cached_threads_;
		}
		void NumMinCachedThreads(uint32_t num)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			if (num > num_min_cached_threads_)
			{
				this->AddWaitingThreadsLocked(lock, num - num_min_cached_threads_);
			}
			else
			{
				for (uint32_t i = 0; i < num_min_cached_threads_ - num; ++i)
				{
					threads_.back()->Kill();
					threads_.pop_back();
				}
			}

			num_min_cached_threads_ = num;
		}

		uint32_t NumMaxCachedThreads() const noexcept
		{
			return num_max_cached_threads_;
		}
		void NumMaxCachedThreads(uint32_t num) noexcept
		{
			num_max_cached_threads_ = num;
		}

		void DoQueueThread(std::function<void()> wake_up_func)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			// If there are no threads, add more to the pool
			if (threads_.empty())
			{
				this->AddWaitingThreadsLocked(lock, 1);
			}

			std::shared_ptr<ThreadInfo> th_info = std::move(threads_.front());
			threads_.erase(threads_.begin());
			th_info->WakeUp(std::move(wake_up_func));
		}

	private:
		// Creates and adds more threads to the pool. This function does not lock the pool mutex and that be
		//  only called when we externally have locked that mutex.
		void AddWaitingThreadsLocked([[maybe_unused]] std::lock_guard<std::mutex> const& lock, uint32_t number)
		{
			threads_.resize(number);
			for (uint32_t i = 0; i < number; ++i)
			{
				auto& th_info = threads_[i];
				th_info = MakeSharedPtr<ThreadInfo>(*this);
				std::thread thread(
					[th_info]()
					{
						WaitFunction(th_info);
					});
				thread.detach();
			}
		}

		static void WaitFunction(std::shared_ptr<ThreadInfo> const& info)
		{
			for (;;)
			{
				auto* data = info->data_;
				if (!data->general_cleanup_)
				{
					std::unique_lock<std::mutex> lock(info->wake_up_mutex_);

					// Sleep until someone has a job to do or the pool is being destroyed
					while (!info->wake_up_ && !data->general_cleanup_)
					{
						info->wake_up_cond_.wait(lock);
					}

					// This is an invitation to leave the pool
					if (!info->func_ || data->general_cleanup_)
					{
						return;
					}

					// If function is zero, this is a exit request
					info->wake_up_ = false;
				}
				else
				{
					return;
				}

				info->func_();
				info->func_ = std::function<void()>();

				// Locked code to try to insert the thread again in the thread pool
				if (!data->general_cleanup_)
				{
					std::lock_guard<std::mutex> lock(data->mutex_);

					// If there is a general cleanup request, finish
					if (data->general_cleanup_)
					{
						return;
					}

					// Now return thread data to the queue if there are less than num_max_cached_threads_ threads
					if (data->threads_.size() < data->num_max_cached_threads_)
					{
						data->threads_.push_back(info);
					}
					else
					{
						// This thread shouldn't be cached since we have enough cached threads
						return;
					}
				}
				else
				{
					return;
				}
			}
		}

	private:
		// Shared data between all threads in the pool
		uint32_t num_min_cached_threads_;
		uint32_t num_max_cached_threads_;
		std::mutex mutex_;
		bool general_cleanup_ = false;
		std::vector<std::shared_ptr<ThreadInfo>> threads_;
	};

	ThreadPool::ThreadPool()
		: ThreadPool(1, CpuInfo().NumHWThreads() * 2)
	{
	}

	ThreadPool::ThreadPool(uint32_t num_min_cached_threads, uint32_t num_max_cached_threads)
		: pimpl_(MakeUniquePtr<Impl>(num_min_cached_threads, num_max_cached_threads))
	{
		BOOST_ASSERT(num_max_cached_threads >= num_min_cached_threads);

		pimpl_->AddWaitingThreads(num_min_cached_threads);
	}

	ThreadPool::~ThreadPool() noexcept = default;

	void ThreadPool::DoQueueThread(std::function<void()> wake_up_func)
	{
		return pimpl_->DoQueueThread(std::move(wake_up_func));
	}

	uint32_t ThreadPool::NumMinCachedThreads() const noexcept
	{
		return pimpl_->NumMinCachedThreads();
	}

	void ThreadPool::NumMinCachedThreads(uint32_t num)
	{
		pimpl_->NumMinCachedThreads(num);
	}

	uint32_t ThreadPool::NumMaxCachedThreads() const noexcept
	{
		return pimpl_->NumMaxCachedThreads();
	}

	void ThreadPool::NumMaxCachedThreads(uint32_t num) noexcept
	{
		pimpl_->NumMaxCachedThreads(num);
	}
}
