// thread_pool.hpp
// KlayGE 线程池 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
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
#include <deque>
#include <set>

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

		uint32_t add_thread(boost::function0<void> const & thread_func);
		bool finished(boost::uint32_t thread_id);
		void join(uint32_t thread_id);
		void join_all();

	private:
		void working_thread_func();

		struct thread_desc
		{
			thread_desc(uint32_t thread_id, boost::function0<void> const & thread_func)
				: thread_id(thread_id), func(thread_func)
			{
			}

			uint32_t thread_id;
			boost::function0<void> func;
		};

		std::vector<boost::shared_ptr<boost::thread> > threads_;
		std::deque<thread_desc> ready_queue_;
		std::set<uint32_t> busy_queue_;
		boost::mutex mutex_threads_;

		boost::condition cond_start_;
		boost::condition cond_finish_;

		uint32_t last_thread_id_;
	};
}

#endif		// _THREADPOOL_HPP
