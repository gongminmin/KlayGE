// thread_pool.cpp
// KlayGE 线程池 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.28)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <boost/bind.hpp>

#include <KlayGE/thread_pool.hpp>

namespace KlayGE
{
	thread_pool::thread_pool()
					: last_thread_id_(0)
	{
	}

	thread_pool::thread_pool(uint32_t max_num_threads)
					: last_thread_id_(0)
	{
		this->max_num_threads(max_num_threads);
	}

	thread_pool::~thread_pool()
	{
		threads_.clear();
		ready_queue_.clear();
		busy_queue_.clear();
	}

	uint32_t thread_pool::max_num_threads() const
	{
		return static_cast<uint32_t>(threads_.size());
	}

	void thread_pool::max_num_threads(uint32_t max_num_threads)
	{
		uint32_t old_max = static_cast<uint32_t>(threads_.size());
		for (uint32_t i = old_max; i < max_num_threads; ++ i)
		{
			threads_.push_back(boost::shared_ptr<boost::thread>(
				new boost::thread(boost::bind(&thread_pool::working_thread_func, this))));
		}
	}

	uint32_t thread_pool::add_thread(boost::function0<void> const & thread_func)
	{
		{
			boost::mutex::scoped_lock lock(mutex_threads_);

			++ last_thread_id_;
			ready_queue_.push_back(thread_desc(last_thread_id_, thread_func));
		}

		cond_start_.notify_one();

		return last_thread_id_;
	}

	bool thread_pool::finished(boost::uint32_t thread_id)
	{
		boost::mutex::scoped_lock lock(mutex_threads_);

		if (busy_queue_.find(thread_id) == busy_queue_.end())
		{
			for (std::deque<thread_desc>::iterator iter = ready_queue_.begin(); iter != ready_queue_.end(); ++ iter)
			{
				if (iter->thread_id == thread_id)
				{
					return false;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	void thread_pool::join(uint32_t thread_id)
	{
		for (;;)
		{
			if (this->finished(thread_id))
			{
				break;
			}

			boost::mutex::scoped_lock lock(mutex_threads_);
			cond_finish_.wait(lock);
		}
	}

	void thread_pool::join_all()
	{
		for (;;)
		{
			boost::mutex::scoped_lock lock(mutex_threads_);
			if (ready_queue_.empty() && busy_queue_.empty())
			{
				break;
			}

			cond_finish_.wait(lock);
		}
	}

	void thread_pool::working_thread_func()
	{
		for (;;)
		{
			boost::mutex::scoped_lock lock(mutex_threads_);

			if (!ready_queue_.empty())
			{
				boost::function0<void> thread_func = ready_queue_.front().func;
				uint32_t cur_id = ready_queue_.front().thread_id;
				busy_queue_.insert(cur_id);
				ready_queue_.pop_front();
				lock.unlock();

				thread_func();

				lock.lock();
				busy_queue_.erase(cur_id);
				lock.unlock();

				cond_finish_.notify_one();
			}
			else
			{
				cond_start_.wait(lock);
			}
		}
	}
}
