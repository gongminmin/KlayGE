// thread.hpp
// KlayGE 线程 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 增加了num_min_cached_threads和num_max_cached_threads (2007.8.7)
//
// 3.6.0
// 初次建立 (2007.7.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _THREAD_HPP
#define _THREAD_HPP

#include <boost/assert.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4244 4512 4702)
#endif
#include <boost/thread.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/void.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <exception>
#include <vector>

#include <KlayGE/Util.hpp>


namespace KlayGE
{
	typedef boost::thread::id thread_id;

	// Threadof operator simulation for threadof(0) expression
	inline thread_id threadof(int)
	{
		return boost::this_thread::get_id();
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
		const char* what() const throw()
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
		//		boost::optional<boost::mpl::void_>
		//	else
		//		boost::optional<result_type>
		typedef boost::optional<
			typename boost::mpl::if_c<boost::is_same<result_type, void>::value,
				boost::mpl::void_, result_type>::type
			>  result_opt;

		typedef typename boost::mpl::if_c<boost::is_same<result_type, void>::value,
			result_type, typename boost::add_reference<result_type>::type
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
				boost::mutex::scoped_lock locker(joiner_mutex_);

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
			catch (boost::lock_error const &)
			{
				throw bad_join();
			}
			// If result is not constructed, this means that the thread has thrown an uncached exception
			if (!*result_)
			{
				throw std::bad_exception();
			}
			return const_result_type_ref(result_->get());
		}

		thread_id get_thread_id() const
		{
			return id_;
		}

	private:
		virtual void do_join() = 0;

	protected:
		// This is a shared pointer to the optional<result> and it's shared between joiners (that can be in
		//  different threads) and the launched thread that joiners represent
		boost::shared_ptr<result_opt>	result_;
		volatile bool					joined_;
		boost::mutex					joiner_mutex_;
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

		explicit joiner(boost::shared_ptr<joiner_impl_base<ResultType> > const & handle)
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
		boost::shared_ptr<joiner_impl_base<ResultType> >  handle_;
	};

	namespace detail
	{
		// This is the function executed by the underlying thread system that calls the user supplied Threadable object
		//  and takes care of exception handling
		template <typename Threadable, typename JoinerImpl>
		class threaded
		{
			typedef threaded<Threadable, JoinerImpl>				threaded_t;
			typedef typename boost::result_of<Threadable()>::type	result_t;
			typedef JoinerImpl										joiner_impl_t;
			typedef typename JoinerImpl::result_opt					result_opt;
			typedef boost::mpl::void_								void_t;
			typedef boost::optional<void_t>							void_optional_t;

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
				out = void_t();
			}

		public:
			threaded(Threadable const & main,
						boost::shared_ptr<result_opt> const & result)
				: main_(main), result_(result)
			{
			}

			// This function is the function executed by the underlying thread system. It takes ownership of
			//  the threaded object, calls user's Threadable
			static void needle(boost::shared_ptr<threaded_t> const & that)
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
			Threadable                    main_;
			boost::shared_ptr<result_opt> result_;
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
			joiner_simple_thread_impl(boost::shared_ptr<typename joiner_impl_base<result_type>::result_opt> const & result_op,
											boost::function0<void> const & func)
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

		private:
			boost::thread thread_;
		};

	public:
		// Launches threadable function in a new thread. Returns a joiner that can be used to wait thread completion.
		template <typename Threadable>
		joiner<typename boost::result_of<Threadable()>::type> operator()(Threadable const & function)
		{
			typedef typename boost::result_of<Threadable()>::type	result_t;
			typedef joiner<result_t>								joiner_t;
			typedef joiner_simple_thread_impl<result_t>				joiner_impl_t;
			typedef typename joiner_impl_t::result_opt				result_opt;
			typedef detail::threaded<Threadable, joiner_impl_t>		threaded_t;

			boost::shared_ptr<result_opt> myreturn = MakeSharedPtr<result_opt>();
			boost::shared_ptr<threaded_t> mythreaded = MakeSharedPtr<threaded_t>(function, myreturn);
			boost::shared_ptr<joiner_impl_base<result_t> > myjoiner_data = MakeSharedPtr<joiner_impl_t>(myreturn,
				boost::bind(&threaded_t::needle, mythreaded));

			return joiner_t(myjoiner_data);
		}
	};

	// Threader function that creates a new thread to execute the Threadable. Just creates a temporary object of class threader
	//  and uses operator()(Threadable)
	template <typename Threadable>
	inline joiner<typename boost::result_of<Threadable()>::type> create_thread(Threadable const & function)
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
			thread_pool_join_info()
				: join_now_(false), can_recycle_thread_(false)
			{
			}

			// Used by joiner to wait until the thread from the pool completes its task
			void join()
			{
				boost::mutex::scoped_lock lock(join_mut_);
				while (!join_now_)
				{
					cond_.wait(lock);
				}
				join_now_ = false;
			}

			// Used by a thread from the pool to wake up a blocked joiner()
			void notify_join()
			{
				boost::mutex::scoped_lock lock(join_mut_);
				join_now_ = true;
				cond_.notify_one();
			}

			// Used by the last joiner to notify to the thread from the pool that it can be recycled for the next task
			void recycle()
			{
				boost::mutex::scoped_lock lock(join_mut_);
				can_recycle_thread_ = true;
				cond_.notify_one();
			}

			// Used by the thread from the pool to wait until the last joiner is destroyed and notifies that the thread
			//  from the pool can be recycled
			void wait_recycle()
			{
				boost::mutex::scoped_lock lock(join_mut_);
				while (!can_recycle_thread_)
				{
					cond_.wait(lock);
				}
				can_recycle_thread_ = false;
			}

		private:
			volatile bool     join_now_;
			volatile bool     can_recycle_thread_;
			boost::condition_variable  cond_;
			boost::mutex      join_mut_;
		};

		// A class used to storage information of a pooled thread. Each object of this class represents a pooled thread.
		//  It also has mechanisms to notify the pooled thread that it has a work to do. It also offers notification
		//  to definitively tell to the thread that it should die.
		struct thread_pool_thread_info
		{
			explicit thread_pool_thread_info(boost::shared_ptr<thread_pool_common_data_t> const & pdata)
				:  wake_up_(false), data_(pdata)
			{
			}

			// Wakes up a pooled thread assigning a task to it
			template <typename Threadable>
			void wake_up(Threadable const & func, boost::shared_ptr<thread_pool_join_info> const & pthpool_join_info)
			{
				// Assign function to execute and joiner notifier
				thpool_join_info_ = pthpool_join_info;
				func_ = func;
				{
					boost::mutex::scoped_lock lock(wake_up_mut_);
					wake_up_ = true;
					wake_up_cond_.notify_one();
				}
			}

			// Wakes up a pooled thread saying it should die
			void kill()
			{
				boost::mutex::scoped_lock lock(wake_up_mut_);
				func_.clear();
				wake_up_ = true;
				wake_up_cond_.notify_one();
			}

			thread_id get_thread_id() const
			{
				return id_;
			}

			void set_thread_id(thread_id id)
			{
				id_ = id;
			}

			boost::shared_ptr<thread_pool_join_info> thpool_join_info_;
			boost::function0<void>	func_;
			bool					wake_up_;
			boost::mutex			wake_up_mut_;
			boost::condition_variable		wake_up_cond_;
			boost::shared_ptr<thread_pool_common_data_t>	data_;
			thread_id				id_;
		};

		// A class used to storage information of the thread pool. It stores the pooled thread information container
		//  and the functor that will envelop users Threadable to return it to the pool.
		class thread_pool_common_data_t : public boost::enable_shared_from_this<thread_pool_common_data_t>
		{
			// This functor is the functor that implements thread pooling logic. Waits until someone fills
			//  the func_ and thpool_join_info_ using the wake_up(...) function of the pooled thread's info.
			class wait_function
			{
			public:
				// Stores a shared_ptr with the data that holds the thread pool.
				explicit wait_function(boost::shared_ptr<thread_pool_thread_info> const & info)
					:  info_(info)
				{
				}

				// This is the thread pool loop. Waits for task, executes it and if there are not enough threads
				//  in the pool, enqueues itself again in the queue.
				void operator()()
				{
					for (;;)
					{
						{
							boost::mutex::scoped_lock lock(info_->wake_up_mut_);

							// Sleep until someone has a job to do or the pool is being destroyed
							while (!info_->wake_up_ && !info_->data_->general_cleanup_)
							{
								info_->wake_up_cond_.wait(lock);
							}

							// This is an invitation to leave the pool
							if (!info_->func_ || info_->data_->general_cleanup_)
							{
								return;
							}

							// If function is zero, this is a exit request
							info_->wake_up_ = false;
						}

						// Execute requested functor
						info_->func_();

						// Reset execution functor
						info_->func_.clear();

						// First notify joiner_thread_pool_impl that data is ready and wake-up if it's blocked waiting for data
						info_->thpool_join_info_->notify_join();

						// Now we have to wait until joiner_thread_pool_impl is destroyed and it notifies that this thread
						//  can pick another job
						info_->thpool_join_info_->wait_recycle();

						// Reset synchronization object
						info_->thpool_join_info_.reset();

						// Locked code to try to insert the thread again in the thread pool
						{
							boost::mutex::scoped_lock lock(info_->data_->mut_);

							// If there is a general cleanup request, finish
							if (info_->data_->general_cleanup_)
							{
								return;
							}

							// Now return thread data to the queue if there are less than num_max_cached_threads_ threads
							if (info_->data_->threads_.size() < info_->data_->num_max_cached_threads_)
							{
								info_->data_->threads_.push_back(info_);
							}
							else
							{
								// This thread shouldn't be cached since we have enough cached threads
								return;
							}
						}
					}
				}

			private:
				boost::shared_ptr<thread_pool_thread_info> info_;
			};

		public:
			typedef std::vector<boost::shared_ptr<thread_pool_thread_info> > thread_info_queue_t;

			thread_pool_common_data_t(size_t num_min_cached_threads, size_t num_max_cached_threads)
				: num_min_cached_threads_(num_min_cached_threads),
					num_max_cached_threads_(num_max_cached_threads),
					general_cleanup_(false)
			{
			}

			// Creates and adds more threads to the pool. Can throw
			static void add_waiting_threads(boost::shared_ptr<thread_pool_common_data_t> pdata, size_t number)
			{
				boost::mutex::scoped_lock lock(pdata->mut_);
				add_waiting_threads_no_lock(pdata, number);
			}

			// Creates and adds more threads to the pool. This function does not lock the pool mutex and that be
			//  only called when we externally have locked that mutex. Can throw
			static void add_waiting_threads_no_lock(boost::shared_ptr<thread_pool_common_data_t> data, size_t number)
			{
				for (size_t i = 0; i < number; ++ i)
				{
					boost::shared_ptr<thread_pool_thread_info> th_info = MakeSharedPtr<thread_pool_thread_info>(data);
					joiner<void> j = data->threader_(wait_function(th_info));
					th_info->set_thread_id(threadof(j));
					data->threads_.push_back(th_info);
				}
			}

			// Notifies all pooled threads that this is the end
			void kill_all()
			{
				boost::mutex::scoped_lock lock(mut_);

				// Notify cleanup command to not queued threads
				general_cleanup_ = true;

				// Wake up queued threads
				size_t num_cached = threads_.size();
				for (size_t i = 0; i < num_cached; ++ i)
				{
					threads_.back()->kill();
					threads_.pop_back();
				}
			}

			boost::mutex& mut()
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
			void num_min_cached_threads(size_t num)
			{
				boost::mutex::scoped_lock lock(mut_);

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
			boost::mutex				mut_;
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
			joiner_thread_pool_impl(boost::shared_ptr<thread_pool_common_data_t> data,
						boost::shared_ptr<typename joiner_impl_base<result_type>::result_opt> const & result_op,
						Threadable const & func)
				: thread_pool_join_info_(MakeSharedPtr<thread_pool_join_info>())
			{
				joiner_impl_base<result_type>::result_ = result_op;

				boost::shared_ptr<thread_pool_thread_info> th_info;
				boost::mutex::scoped_lock lock(data->mut());

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

		private:
			boost::shared_ptr<thread_pool_join_info>  thread_pool_join_info_;
		};

	public:
		thread_pool(size_t num_min_cached_threads, size_t num_max_cached_threads)
			: data_(MakeSharedPtr<thread_pool_common_data_t>(num_min_cached_threads, num_max_cached_threads))
		{
			BOOST_ASSERT(num_max_cached_threads >= num_min_cached_threads);

			thread_pool_common_data_t::add_waiting_threads(data_, num_min_cached_threads);
		}

		~thread_pool()
		{
			data_->kill_all();
		}

		// Launches threadable function in a new thread. If there is a pooled thread available, reuses that thread.
		template <typename Threadable>
		joiner<typename boost::result_of<Threadable()>::type> operator()(Threadable const & function)
		{
			typedef typename boost::result_of<Threadable()>::type	result_t;
			typedef joiner<result_t>								joiner_t;
			typedef joiner_thread_pool_impl<result_t>				joiner_impl_t;
			typedef typename joiner_impl_t::result_opt				result_opt;
			typedef detail::threaded<Threadable, joiner_impl_t>		threaded_t;

			boost::shared_ptr<result_opt> myreturn = MakeSharedPtr<result_opt>();
			boost::shared_ptr<threaded_t> mythreaded = MakeSharedPtr<threaded_t>(function, myreturn);
			boost::shared_ptr<joiner_impl_base<result_t> > myjoiner_data = MakeSharedPtr<joiner_impl_t>(data_,
				myreturn, boost::bind(&threaded_t::needle, mythreaded));

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
		boost::shared_ptr<thread_pool_common_data_t> data_;
	};

	inline thread_pool& GlobalThreadPool()
	{
		static thread_pool ret(1, 16);
		return ret;
	}
}


#endif		// _THREAD_HPP
