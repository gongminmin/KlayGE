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

#ifndef _KFL_THREAD_HPP
#define _KFL_THREAD_HPP

#pragma once

#include <boost/assert.hpp>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <exception>
#include <vector>
#include <functional>

#include <KFL/CXX17/optional.hpp>

namespace KlayGE
{
	struct my_void_t
	{
		typedef my_void_t type;
	};

	typedef std::thread::id thread_id;

	// Threadof operator simulation for threadof(0) expression
	inline thread_id threadof(int)
	{
		return std::this_thread::get_id();
	}

	// Threadof operator simulation for threadof(joiner) expression
	template <typename Joiner>
	inline thread_id threadof(Joiner const & j)
	{
		return j.get_thread_id();
	}

	// Exception thrown when a joiner tries to join with its own thread or the joiner has no thread.
	class bad_join : public std::exception
	{
	public:
		const char* what() const noexcept
		{
			return "bad join";
		}
	};

	// This is the abstract interface of the implementation of joiner functions. A Threader must create a class
	//  derived from joiner_impl_base to initialize a joiner.
	template <typename result_type>
	class joiner_impl_base
	{
	public:
		// Representation of the storage to hold the return type:
		//	if result_type == void
		//		std::optional<my_void_t>
		//	else
		//		std::optional<result_type>
		typedef std::optional<
			typename std::conditional<std::is_same<result_type, void>::value,
				my_void_t, result_type>::type
			>  result_opt;

		typedef typename std::conditional<std::is_same<result_type, void>::value,
			result_type, typename std::add_lvalue_reference<result_type>::type
			>::type const_result_type_ref;

	public:
		joiner_impl_base()
			: joined_(false)
		{
		}

		virtual ~joiner_impl_base()
		{
		}

		// Waits until the threadable function is executed. This can be called simultaneously from different threads.
		//  Other threads can wait to the same threadable completion using different joiner objects referring to
		//  the same thread. If the threadable execution was already joined, it returns immediately with a reference
		//  to the value returned from the threadable.
		// Can throw bad_join if we try to join the launched threadable from that same thread or another error occurs when joining.
		// Can throw std::bad_exception if the launched thread ends with an exception
		const_result_type_ref join()
		{
			// Lock mutex because another joiner object or this object can be used to join the launched
			//  thread from different threads.
			try
			{
				std::lock_guard<std::mutex> locker(joiner_mutex_);

				// Check if already joined
				if (!joined_)
				{
					if (threadof(*this) == threadof(0))
					{
						throw bad_join();
					}

					this->do_join();
					joined_ = true;
				}
			}
			catch (...)
			{
				throw bad_join();
			}
			// If result is not constructed, this means that the thread has thrown an uncached exception
			if (!*result_)
			{
				throw std::bad_exception();
			}
			return const_result_type_ref(*result_);
		}

		void detach()
		{
			this->do_detach();
		}

		thread_id get_thread_id() const
		{
			return id_;
		}

	private:
		virtual void do_join() = 0;
		virtual void do_detach() = 0;

	protected:
		// This is a shared pointer to the optional<result> and it's shared between joiners (that can be in
		//  different threads) and the launched thread that joiners represent
		std::shared_ptr<result_opt>		result_;
		volatile bool					joined_;
		std::mutex						joiner_mutex_;
		thread_id						id_;
	};


	// Joiner class, an object that represent a launched thread the thread can be joined from different threads using this object
	template <typename ResultType>
	class joiner
	{
		typedef joiner_impl_base<ResultType> joiner_base_t;

	public:
		typedef typename joiner_base_t::const_result_type_ref const_result_type_ref;

		thread_id get_thread_id() const
		{
			return handle_ ? handle_->get_thread_id() : threadof(0);
		}

	public:
		joiner()
		{
		}

		explicit joiner(std::shared_ptr<joiner_impl_base<ResultType>> const & handle)
			: handle_(handle)
		{
		}

		joiner(joiner const & other)
			: handle_(other.handle_)
		{
		}

		joiner& operator=(joiner const & other)
		{
			handle_ = other.handle_;
			return *this;
		}

		const_result_type_ref operator()()
		{
			if (!handle_)
			{
				throw bad_join();
			}
			return handle_->join();
		}

		void detach()
		{
			handle_->detach();
		}

	public:
		friend bool operator==(joiner<ResultType> const & lhs, joiner<ResultType> const & rhs)
		{
			return lhs.handle_.get() == rhs.handle_.get();
		}

		friend bool operator!=(joiner<ResultType> const & lhs, joiner<ResultType> const & rhs)
		{
			return !(lhs == rhs);
		}

		friend bool operator<(joiner<ResultType> const & lhs, joiner<ResultType> const & rhs)
		{
			return lhs.handle_.get() < rhs.handle_.get();
		}

		friend bool operator>(joiner<ResultType> const & lhs, joiner<ResultType> const & rhs)
		{
			return rhs < lhs;
		}

		friend bool operator<=(joiner<ResultType> const & lhs, joiner<ResultType> const & rhs)
		{
			return !(rhs < lhs);
		}

		friend bool operator>=(joiner<ResultType> const & lhs, joiner<ResultType> const & rhs)
		{
			return !(lhs < rhs);
		}

	private:
		std::shared_ptr<joiner_impl_base<ResultType>> handle_;
	};

	namespace detail
	{
		// This is the function executed by the underlying thread system that calls the user supplied Threadable object
		//  and takes care of exception handling
		template <typename Threadable, typename JoinerImpl>
		class threaded
		{
			typedef threaded<Threadable, JoinerImpl>				threaded_t;
			typedef typename std::result_of<Threadable()>::type		result_t;
			typedef JoinerImpl										joiner_impl_t;
			typedef typename JoinerImpl::result_opt					result_opt;
			typedef std::optional<my_void_t>						void_optional_t;

			//Helper function to construct the optional from the
			//return value and handle void return types
			template <typename MainFunctionHolder, typename OptionalOut>
			static void construct_result(MainFunctionHolder& in, OptionalOut& out)
			{
				out = in.main_();
			}

			template <typename MainFunctionHolder>
			static void construct_result(MainFunctionHolder& in, void_optional_t& out)
			{
				in.main_();
				out = my_void_t();
			}

		public:
			threaded(Threadable const & main,
						std::shared_ptr<result_opt> const & result)
				: main_(main), result_(result)
			{
			}

			// This function is the function executed by the underlying thread system. It takes ownership of
			//  the threaded object, calls user's Threadable
			static void needle(std::shared_ptr<threaded_t> const & that)
			{
				// Call Threadable
				try
				{
					that->construct_result(*that, *that->result_);
				}
				catch(...)
				{
					// If an exception is thrown, result_opt is not constructed, and this can be detected
					//  using optional's interface by joiners
				}
			}

		private:
			Threadable					main_;
			std::shared_ptr<result_opt>	result_;
		};
	}

	// A threader class that whose operator() launches a threadable	object in a new thread
	class threader
	{
		// This is the implementation of joiner functions created by "create_thread" and "class threader" threaders.
		template <typename result_type>
		class joiner_simple_thread_impl : public joiner_impl_base<result_type>
		{
		public:
			joiner_simple_thread_impl(std::shared_ptr<typename joiner_impl_base<result_type>::result_opt> const & result_op,
											std::function<void()> const & func)
				: thread_(func)
			{
				joiner_impl_base<result_type>::result_ = result_op;
				joiner_impl_base<result_type>::id_ = thread_.get_id();
			}

		private:
			void do_join()
			{
				thread_.join();
			}

			void do_detach()
			{
				thread_.detach();
			}

		private:
			std::thread thread_;
		};

	public:
		// Launches threadable function in a new thread. Returns a joiner that can be used to wait thread completion.
		template <typename Threadable>
		joiner<typename std::result_of<Threadable()>::type> operator()(Threadable const & function)
		{
			typedef typename std::result_of<Threadable()>::type		result_t;
			typedef joiner<result_t>								joiner_t;
			typedef joiner_simple_thread_impl<result_t>				joiner_impl_t;
			typedef typename joiner_impl_t::result_opt				result_opt;
			typedef detail::threaded<Threadable, joiner_impl_t>		threaded_t;

			std::shared_ptr<result_opt> myreturn = MakeSharedPtr<result_opt>();
			std::shared_ptr<threaded_t> mythreaded = MakeSharedPtr<threaded_t>(function, myreturn);
			std::shared_ptr<joiner_impl_base<result_t>> myjoiner_data = MakeSharedPtr<joiner_impl_t>(myreturn,
				[mythreaded] { threaded_t::needle(mythreaded); });

			return joiner_t(myjoiner_data);
		}
	};

	// Threader function that creates a new thread to execute the Threadable. Just creates a temporary object of class threader
	//  and uses operator()(Threadable)
	template <typename Threadable>
	inline joiner<typename std::result_of<Threadable()>::type> create_thread(Threadable const & function)
	{
		return threader()(function);
	}


	// This Threader class creates a pool of threads that can be reused for several Threadable object executions.
	//  If the thread pool runs out of threads it creates more. The user can specify the minimum and maximum
	//  number of pooled threads.
	class thread_pool
	{
		class thread_pool_common_data_t;

		// A class used to synchronize joining with a thread launched from a thread pool and notifying to pooled thread
		//  that it can be recycled
		class thread_pool_join_info
		{
		public:
			thread_pool_join_info();

			// Used by joiner to wait until the thread from the pool completes its task
			void join();

			// Used by a thread from the pool to wake up a blocked joiner()
			void notify_join();

			// Used by the last joiner to notify to the thread from the pool that it can be recycled for the next task
			void recycle();

			// Used by the thread from the pool to wait until the last joiner is destroyed and notifies that the thread
			//  from the pool can be recycled
			void wait_recycle();

		private:
			volatile bool     join_now_;
			volatile bool     can_recycle_thread_;
			std::condition_variable  cond_;
			std::mutex      join_mut_;
		};

		// A class used to storage information of a pooled thread. Each object of this class represents a pooled thread.
		//  It also has mechanisms to notify the pooled thread that it has a work to do. It also offers notification
		//  to definitively tell to the thread that it should die.
		struct thread_pool_thread_info
		{
			explicit thread_pool_thread_info(std::shared_ptr<thread_pool_common_data_t> const & pdata);

			// Wakes up a pooled thread assigning a task to it
			template <typename Threadable>
			void wake_up(Threadable const & func, std::shared_ptr<thread_pool_join_info> const & pthpool_join_info)
			{
				// Assign function to execute and joiner notifier
				thpool_join_info_ = pthpool_join_info;
				func_ = func;
				{
					std::lock_guard<std::mutex> lock(wake_up_mut_);
					wake_up_ = true;
					wake_up_cond_.notify_one();
				}
			}

			// Wakes up a pooled thread saying it should die
			void kill();

			thread_id get_thread_id() const
			{
				return id_;
			}

			void set_thread_id(thread_id id)
			{
				id_ = id;
			}

			std::shared_ptr<thread_pool_join_info> thpool_join_info_;
			std::function<void()>	func_;
			bool					wake_up_;
			std::mutex				wake_up_mut_;
			std::condition_variable	wake_up_cond_;
			std::weak_ptr<thread_pool_common_data_t>	data_;
			thread_id				id_;
		};

		// A class used to storage information of the thread pool. It stores the pooled thread information container
		//  and the functor that will envelop users Threadable to return it to the pool.
		class thread_pool_common_data_t : public std::enable_shared_from_this<thread_pool_common_data_t>
		{
			// This functor is the functor that implements thread pooling logic. Waits until someone fills
			//  the func_ and thpool_join_info_ using the wake_up(...) function of the pooled thread's info.
			class wait_function
			{
			public:
				// Stores a std::shared_ptr with the data that holds the thread pool.
				explicit wait_function(std::shared_ptr<thread_pool_thread_info> const & info);

				// This is the thread pool loop. Waits for task, executes it and if there are not enough threads
				//  in the pool, enqueues itself again in the queue.
				void operator()();

			private:
				std::shared_ptr<thread_pool_thread_info> info_;
			};

		public:
			typedef std::vector<std::shared_ptr<thread_pool_thread_info>> thread_info_queue_t;

			thread_pool_common_data_t(size_t num_min_cached_threads, size_t num_max_cached_threads);

			// Creates and adds more threads to the pool. Can throw
			static void add_waiting_threads(std::shared_ptr<thread_pool_common_data_t> const & pdata, size_t number);

			// Creates and adds more threads to the pool. This function does not lock the pool mutex and that be
			//  only called when we externally have locked that mutex. Can throw
			static void add_waiting_threads_no_lock(std::shared_ptr<thread_pool_common_data_t> const & data, size_t number);

			// Notifies all pooled threads that this is the end
			void kill_all();

			std::mutex& mut()
			{
				return mut_;
			}

			thread_info_queue_t& threads()
			{
				return threads_;
			}

			size_t num_min_cached_threads() const
			{
				return num_min_cached_threads_;
			}
			void num_min_cached_threads(size_t num);

			size_t num_max_cached_threads() const
			{
				return num_max_cached_threads_;
			}
			void num_max_cached_threads(size_t num)
			{
				num_max_cached_threads_ = num;
			}

		private:
			// Shared data between thread_pool, all threads and joiners
			size_t						num_min_cached_threads_;
			size_t						num_max_cached_threads_;
			std::mutex					mut_;
			bool						general_cleanup_;
			thread_info_queue_t			threads_;
			threader					threader_;
		};

		// This is the implementation of joiner functions created by "create_thread" and "class threader" threaders.
		template <typename result_type>
		class joiner_thread_pool_impl : public joiner_impl_base<result_type>
		{
			friend class thread_pool;

		public:
			template <typename Threadable>
			joiner_thread_pool_impl(std::shared_ptr<thread_pool_common_data_t> const & data,
						std::shared_ptr<typename joiner_impl_base<result_type>::result_opt> const & result_op,
						Threadable const & func)
				: thread_pool_join_info_(MakeSharedPtr<thread_pool_join_info>())
			{
				joiner_impl_base<result_type>::result_ = result_op;

				std::shared_ptr<thread_pool_thread_info> th_info;
				std::lock_guard<std::mutex> lock(data->mut());

				// If there are no threads, add more to the pool
				if (data->threads().empty())
				{
					thread_pool_common_data_t::add_waiting_threads_no_lock(data, 1);
				}
				th_info = data->threads().front();
				joiner_impl_base<result_type>::id_ = th_info->get_thread_id();
				data->threads().erase(data->threads().begin());
				th_info->wake_up(func, thread_pool_join_info_);
			}

			~joiner_thread_pool_impl()
			{
				try
				{
					thread_pool_join_info_->recycle();
				}
				catch (...)
				{
				}
			}

		private:
			void do_join()
			{
				thread_pool_join_info_->join();
			}

			void do_detach()
			{
			}

		private:
			std::shared_ptr<thread_pool_join_info> thread_pool_join_info_;
		};

	public:
		thread_pool(size_t num_min_cached_threads, size_t num_max_cached_threads);
		~thread_pool();

		// Launches threadable function in a new thread. If there is a pooled thread available, reuses that thread.
		template <typename Threadable>
		joiner<typename std::result_of<Threadable()>::type> operator()(Threadable const & function)
		{
			typedef typename std::result_of<Threadable()>::type		result_t;
			typedef joiner<result_t>								joiner_t;
			typedef joiner_thread_pool_impl<result_t>				joiner_impl_t;
			typedef typename joiner_impl_t::result_opt				result_opt;
			typedef detail::threaded<Threadable, joiner_impl_t>		threaded_t;

			std::shared_ptr<result_opt> myreturn = MakeSharedPtr<result_opt>();
			std::shared_ptr<threaded_t> mythreaded = MakeSharedPtr<threaded_t>(function, myreturn);
			std::shared_ptr<joiner_impl_base<result_t>> myjoiner_data = MakeSharedPtr<joiner_impl_t>(data_,
				myreturn, [mythreaded] { threaded_t::needle(mythreaded); });

			return joiner_t(myjoiner_data);
		}

		size_t num_min_cached_threads() const
		{
			return data_->num_min_cached_threads();
		}
		void num_min_cached_threads(size_t num)
		{
			data_->num_min_cached_threads(num);
		}

		size_t num_max_cached_threads() const
		{
			return data_->num_max_cached_threads();
		}
		void num_max_cached_threads(size_t num)
		{
			data_->num_max_cached_threads(num);
		}

	private:
		std::shared_ptr<thread_pool_common_data_t> data_;
	};
}

#endif		// _KFL_THREAD_HPP
