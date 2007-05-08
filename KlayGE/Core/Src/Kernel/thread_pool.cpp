// thread_pool.cpp
// KlayGE 线程池 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 修正了死锁的bug (2007.1.10)
//
// 3.2.0
// 初次建立 (2006.4.28)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/thread_pool.hpp>

namespace KlayGE
{
	thread_pool::thread_pool()
					: last_task_id_(0)
	{
	}

	thread_pool::thread_pool(uint32_t max_num_threads)
					: last_task_id_(0)
	{
		this->max_num_threads(max_num_threads);
	}

	thread_pool::~thread_pool()
	{
		this->join_all();

		threads_.clear();
		queue_.clear();
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

	bool thread_pool::finished(uint32_t task_id) const
	{
		BOOST_FOREACH(BOOST_TYPEOF(queue_)::const_reference task, queue_)
		{
			boost::mutex::scoped_lock lock(*task.mutex_finish);
			if (task.task_id == task_id)
			{
				return (TS_FINISHED == task.state);
			}
		}
		return true;
	}

	uint32_t thread_pool::add_thread(boost::function0<void> const & thread_func)
	{
		boost::mutex::scoped_lock lock(add_mutex_);
		{
			boost::mutex::scoped_lock lock(queue_mutex_);

			++ last_task_id_;
			queue_.push_back(task_desc(last_task_id_, thread_func));
		}

		cond_start_.notify_one();

		return last_task_id_;
	}

	void thread_pool::join(uint32_t task_id)
	{
		boost::mutex::scoped_lock lock(add_mutex_);

		for (BOOST_AUTO(iter, queue_.begin()); iter != queue_.end(); ++ iter)
		{
			boost::mutex::scoped_lock lock(*(iter->mutex_finish));
			if (iter->task_id == task_id)
			{
				if (iter->state != TS_FINISHED)
				{
					iter->cond_finish->wait(lock);
					break;
				}
				else
				{
					lock.unlock();
					queue_.erase(iter);
					break;
				}
			}
		}
	}

	void thread_pool::join_all()
	{
		boost::mutex::scoped_lock lock(add_mutex_);

		BOOST_FOREACH(BOOST_TYPEOF(queue_)::reference task, queue_)
		{
			boost::mutex::scoped_lock lock(*task.mutex_finish);
			if (task.state != TS_FINISHED)
			{
				task.cond_finish->wait(lock);
			}
		}

		{
			boost::mutex::scoped_lock lock(queue_mutex_);
			queue_.clear();
		}
	}

	void thread_pool::working_thread_func()
	{
		for (;;)
		{
			bool run = false;
			boost::mutex::scoped_lock lock(queue_mutex_);
			BOOST_FOREACH(BOOST_TYPEOF(queue_)::reference task, queue_)
			{
				if (TS_READY == task.state)
				{
					boost::mutex::scoped_lock ilock(*task.mutex_finish);
					task.state = TS_BUSY;
					lock.unlock();

					task.func();

					lock.lock();
					task.state = TS_FINISHED;
					task.cond_finish->notify_one();
					run = true;

					break;
				}
			}

			if (!run)
			{
				cond_start_.wait(lock);
			}
		}
	}
}
