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

#pragma once

#include <functional>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4355) // Ignore "this" in member initializer list
#endif
#include <future>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <thread>

#include <KFL/Noncopyable.hpp>

namespace KlayGE
{
	template <typename Threadable>
	inline std::future<std::invoke_result_t<Threadable>> CreateThread(Threadable func)
	{
		using ResultT = std::invoke_result_t<Threadable>;

		std::packaged_task<ResultT()> task(std::move(func));
		auto ret = task.get_future();
		std::thread thread(std::move(task));
		thread.detach();

		return ret;
	}

	class ThreadPool final
	{
		KLAYGE_NONCOPYABLE(ThreadPool);

	public:
		ThreadPool();
		ThreadPool(uint32_t num_min_cached_threads, uint32_t num_max_cached_threads);
		~ThreadPool() noexcept;

		// Launches threadable function in a new thread. If there is a pooled thread available, reuses that thread.
		template <typename Threadable>
		std::future<std::invoke_result_t<Threadable>> QueueThread(Threadable func)
		{
			using ResultT = std::invoke_result_t<Threadable>;

			auto task = MakeSharedPtr<std::packaged_task<ResultT()>>(std::move(func));
			auto ret = task->get_future();
			this->DoQueueThread(
				[task]()
				{
					(*task)();
				});

			return ret;
		}

		uint32_t NumMinCachedThreads() const noexcept;
		void NumMinCachedThreads(uint32_t num);

		uint32_t NumMaxCachedThreads() const noexcept;
		void NumMaxCachedThreads(uint32_t num) noexcept;

	private:
		void DoQueueThread(std::function<void()> wake_up_func);

	private:
		class Impl;
		std::unique_ptr<Impl> pimpl_;
	};
}
