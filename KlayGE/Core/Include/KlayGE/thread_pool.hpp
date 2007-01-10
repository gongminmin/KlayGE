// thread_pool.hpp
// KlayGE 线程池 头文件
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

#ifndef _THREADPOOL_HPP
#define _THREADPOOL_HPP

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 4189)
#endif
#include <boost/thread.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <vector>
#include <list>

namespace KlayGE
{
	class thread_pool : private boost::noncopyable
	{
	public:
		thread_pool();
		explicit thread_pool(uint32_t max_num_threads);
		~thread_pool();

		uint32_t max_num_threads() const;
		void max_num_threads(uint32_t max_num_threads);

		bool finished(uint32_t task_id) const;

		uint32_t add_thread(boost::function0<void> const & thread_func);
		void join(uint32_t task_id);
		void join_all();

	private:
		void working_thread_func();

		enum thread_state
		{
			TS_READY,
			TS_BUSY,
			TS_FINISHED
		};

		struct task_desc
		{
			task_desc()
			{
			}
			task_desc(uint32_t task_id, boost::function0<void> const & thread_func)
				: task_id(task_id), func(thread_func)
			{
				mutex_finish.reset(new boost::mutex);
				cond_finish.reset(new boost::condition);

				state = TS_READY;
			}

			uint32_t task_id;
			boost::function0<void> func;

			boost::shared_ptr<boost::mutex> mutex_finish;
			boost::shared_ptr<boost::condition> cond_finish;

			thread_state state;
		};

		std::vector<boost::shared_ptr<boost::thread> > threads_;
		std::list<task_desc> queue_;

		boost::mutex add_mutex_;
		boost::mutex queue_mutex_;
		boost::condition cond_start_;

		uint32_t last_task_id_;
	};
}

#endif		// _THREADPOOL_HPP
