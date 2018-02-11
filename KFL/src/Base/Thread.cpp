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

#include <KFL/Thread.hpp>

namespace KlayGE
{
	thread_pool::thread_pool_join_info::thread_pool_join_info()
		: join_now_(false), can_recycle_thread_(false)
	{
	}

	void thread_pool::thread_pool_join_info::join()
	{
		std::unique_lock<std::mutex> lock(join_mut_);
		while (!join_now_)
		{
			cond_.wait(lock);
		}
		join_now_ = false;
	}

	void thread_pool::thread_pool_join_info::notify_join()
	{
		std::lock_guard<std::mutex> lock(join_mut_);
		join_now_ = true;
		cond_.notify_one();
	}

	void thread_pool::thread_pool_join_info::recycle()
	{
		std::lock_guard<std::mutex> lock(join_mut_);
		can_recycle_thread_ = true;
		cond_.notify_one();
	}

	void thread_pool::thread_pool_join_info::wait_recycle()
	{
		std::unique_lock<std::mutex> lock(join_mut_);
		while (!can_recycle_thread_)
		{
			cond_.wait(lock);
		}
		can_recycle_thread_ = false;
	}


	thread_pool::thread_pool_thread_info::thread_pool_thread_info(std::shared_ptr<thread_pool::thread_pool_common_data_t> const & pdata)
		:  wake_up_(false), data_(std::weak_ptr<thread_pool::thread_pool_common_data_t>(pdata))
	{
	}

	// Wakes up a pooled thread saying it should die
	void thread_pool::thread_pool_thread_info::kill()
	{
		std::lock_guard<std::mutex> lock(wake_up_mut_);
		func_ = std::function<void()>();
		wake_up_ = true;
		wake_up_cond_.notify_one();
	}

	thread_pool::thread_pool_common_data_t::wait_function::wait_function(std::shared_ptr<thread_pool::thread_pool_thread_info> const & info)
		:  info_(info)
	{
	}

	// This is the thread pool loop. Waits for task, executes it and if there are not enough threads
	//  in the pool, enqueues itself again in the queue.
	void thread_pool::thread_pool_common_data_t::wait_function::operator()()
	{
		for (;;)
		{
			{
				std::shared_ptr<thread_pool_common_data_t> data = info_->data_.lock();
				if (data)
				{
					std::unique_lock<std::mutex> lock(info_->wake_up_mut_);

					// Sleep until someone has a job to do or the pool is being destroyed
					while (!info_->wake_up_ && !data->general_cleanup_)
					{
						info_->wake_up_cond_.wait(lock);
					}

					// This is an invitation to leave the pool
					if (!info_->func_ || data->general_cleanup_)
					{
						return;
					}

					// If function is zero, this is a exit request
					info_->wake_up_ = false;
				}
				else
				{
					return;
				}
			}

			// Execute requested functor
			info_->func_();

			// Reset execution functor
			info_->func_ = std::function<void()>();

			// First notify joiner_thread_pool_impl that data is ready and wake-up if it's blocked waiting for data
			info_->thpool_join_info_->notify_join();

			// Now we have to wait until joiner_thread_pool_impl is destroyed and it notifies that this thread
			//  can pick another job
			info_->thpool_join_info_->wait_recycle();

			// Reset synchronization object
			info_->thpool_join_info_.reset();

			// Locked code to try to insert the thread again in the thread pool
			{
				std::shared_ptr<thread_pool_common_data_t> data = info_->data_.lock();
				if (data)
				{
					std::lock_guard<std::mutex> lock(data->mut_);

					// If there is a general cleanup request, finish
					if (data->general_cleanup_)
					{
						return;
					}

					// Now return thread data to the queue if there are less than num_max_cached_threads_ threads
					if (data->threads_.size() < data->num_max_cached_threads_)
					{
						data->threads_.push_back(info_);
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

	thread_pool::thread_pool_common_data_t::thread_pool_common_data_t(size_t num_min_cached_threads, size_t num_max_cached_threads)
		: num_min_cached_threads_(num_min_cached_threads),
			num_max_cached_threads_(num_max_cached_threads),
			general_cleanup_(false)
	{
	}

	void thread_pool::thread_pool_common_data_t::add_waiting_threads(std::shared_ptr<thread_pool::thread_pool_common_data_t> const & pdata, size_t number)
	{
		std::lock_guard<std::mutex> lock(pdata->mut_);
		add_waiting_threads_no_lock(pdata, number);
	}

	void thread_pool::thread_pool_common_data_t::add_waiting_threads_no_lock(std::shared_ptr<thread_pool::thread_pool_common_data_t> const & data, size_t number)
	{
		for (size_t i = 0; i < number; ++ i)
		{
			std::shared_ptr<thread_pool_thread_info> th_info = MakeSharedPtr<thread_pool_thread_info>(data);
			joiner<void> j = data->threader_(wait_function(th_info));
			th_info->set_thread_id(j.get_thread_id());
			data->threads_.push_back(th_info);
			j.detach();
		}
	}

	void thread_pool::thread_pool_common_data_t::kill_all()
	{
		std::lock_guard<std::mutex> lock(mut_);

		// Notify cleanup command to not queued threads
		general_cleanup_ = true;

		// Wake up queued threads
		size_t const num_cached = threads_.size();
		for (size_t i = 0; i < num_cached; ++ i)
		{
			threads_.back()->kill();
			threads_.pop_back();
		}
	}

	void thread_pool::thread_pool_common_data_t::num_min_cached_threads(size_t num)
	{
		std::lock_guard<std::mutex> lock(mut_);

		if (num > num_min_cached_threads_)
		{
			this->add_waiting_threads_no_lock(this->shared_from_this(), num - num_min_cached_threads_);
		}
		else
		{
			for (size_t i = 0; i < num_min_cached_threads_ - num; ++ i)
			{
				threads_.back()->kill();
				threads_.pop_back();
			}
		}

		num_min_cached_threads_ = num;
	}


	thread_pool::thread_pool(size_t num_min_cached_threads, size_t num_max_cached_threads)
		: data_(MakeSharedPtr<thread_pool_common_data_t>(num_min_cached_threads, num_max_cached_threads))
	{
		BOOST_ASSERT(num_max_cached_threads >= num_min_cached_threads);

		thread_pool_common_data_t::add_waiting_threads(data_, num_min_cached_threads);
	}

	thread_pool::~thread_pool()
	{
		data_->kill_all();
	}
}
