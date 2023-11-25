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

#include <KFL/KFL.hpp>

#include <boost/assert.hpp>

#include <KFL/CpuInfo.hpp>
#include <KFL/Thread.hpp>

namespace KlayGE
{
	ThreadPool::ThreadInfo::ThreadInfo(CommonData& data) noexcept
		: data_(data.shared_from_this())
	{
	}

	void ThreadPool::ThreadInfo::WakeUp(std::function<void()> func)
	{
		func_ = std::move(func);
		{
			std::lock_guard<std::mutex> lock(wake_up_mutex_);
			wake_up_ = true;
			wake_up_cond_.notify_one();
		}
	}

	// Wakes up a pooled thread saying it should die
	void ThreadPool::ThreadInfo::Kill()
	{
		std::lock_guard<std::mutex> lock(wake_up_mutex_);
		func_ = std::function<void()>();
		wake_up_ = true;
		wake_up_cond_.notify_one();
	}

	void ThreadPool::CommonData::WaitFunction(std::shared_ptr<ThreadPool::ThreadInfo> const& info)
	{
		for (;;)
		{
			{
				auto data = info->data_.lock();
				if (data)
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
			}

			info->func_();
			info->func_ = std::function<void()>();

			// Locked code to try to insert the thread again in the thread pool
			{
				auto data = info->data_.lock();
				if (data)
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
	}

	ThreadPool::CommonData::CommonData(size_t num_min_cached_threads, size_t num_max_cached_threads)
		: num_min_cached_threads_(num_min_cached_threads), num_max_cached_threads_(num_max_cached_threads)
	{
	}

	ThreadPool::CommonData::~CommonData()
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

	void ThreadPool::CommonData::AddWaitingThreads(size_t number)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		this->AddWaitingThreadsLocked(lock, number);
	}

	void ThreadPool::CommonData::AddWaitingThreadsLocked([[maybe_unused]] std::lock_guard<std::mutex> const& lock, size_t number)
	{
		for (size_t i = 0; i < number; ++ i)
		{
			auto& th_info = threads_.emplace_back(MakeSharedPtr<ThreadInfo>(*this));
			auto thread = std::thread([th_info]() { WaitFunction(th_info); });
			thread.detach();
		}
	}

	void ThreadPool::CommonData::NumMinCachedThreads(size_t num)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		if (num > num_min_cached_threads_)
		{
			this->AddWaitingThreadsLocked(lock, num - num_min_cached_threads_);
		}
		else
		{
			for (size_t i = 0; i < num_min_cached_threads_ - num; ++ i)
			{
				threads_.back()->Kill();
				threads_.pop_back();
			}
		}

		num_min_cached_threads_ = num;
	}


	ThreadPool::ThreadPool()
		: ThreadPool(1, CpuInfo().NumHWThreads() * 2)
	{
	}

	ThreadPool::ThreadPool(size_t num_min_cached_threads, size_t num_max_cached_threads)
		: data_(MakeSharedPtr<CommonData>(num_min_cached_threads, num_max_cached_threads))
	{
		BOOST_ASSERT(num_max_cached_threads >= num_min_cached_threads);

		data_->AddWaitingThreads(num_min_cached_threads);
	}
}
