/**
 * @file ResLoader.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
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

#ifndef _KLAYGE_RESLOADER_HPP
#define _KLAYGE_RESLOADER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <istream>
#include <vector>
#include <string>
#include <boost/lockfree/spsc_queue.hpp>

#include <KFL/ResIdentifier.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ResLoadingDesc
	{
	public:
		virtual ~ResLoadingDesc()
		{
		}

		virtual uint64_t Type() const = 0;

		virtual bool StateLess() const = 0;

		virtual void SubThreadStage() = 0;
		virtual shared_ptr<void> MainThreadStage() = 0;

		virtual bool HasSubThreadStage() const = 0;

		virtual bool Match(ResLoadingDesc const & rhs) const = 0;
		virtual void CopyDataFrom(ResLoadingDesc const & rhs) = 0;
		virtual shared_ptr<void> CloneResourceFrom(shared_ptr<void> const & resource) = 0;
	};

	class KLAYGE_CORE_API ResLoader
	{
		template <typename T>
		class EmptyFuncToT
		{
		public:
			explicit EmptyFuncToT(function<shared_ptr<void>()> const & func)
				: func_(func)
			{
			}

			shared_ptr<T> operator()()
			{
				return static_pointer_cast<T>(func_());
			}

		private:
			function<shared_ptr<void>()> func_;
		};

		class ASyncFunctor
		{
		public:
			explicit ASyncFunctor(shared_ptr<void> const & res);

			shared_ptr<void> operator()(ResLoadingDescPtr const & res_desc, shared_ptr<volatile bool> const & is_done);
			shared_ptr<void> operator()();

		private:
			shared_ptr<void> res_;
		};

	public:
		ResLoader();
		~ResLoader();

		static ResLoader& Instance();
		static void Destroy();

		void AddPath(std::string const & path);
		void DelPath(std::string const & path);

		ResIdentifierPtr Open(std::string const & name);
		std::string Locate(std::string const & name);

		shared_ptr<void> SyncQuery(ResLoadingDescPtr const & res_desc);
		function<shared_ptr<void>()> ASyncQuery(ResLoadingDescPtr const & res_desc);
		void Unload(shared_ptr<void> const & res);

		template <typename T>
		shared_ptr<T> SyncQueryT(ResLoadingDescPtr const & res_desc)
		{
			return static_pointer_cast<T>(this->SyncQuery(res_desc));
		}

		template <typename T>
		function<shared_ptr<T>()> ASyncQueryT(ResLoadingDescPtr const & res_desc)
		{
			return EmptyFuncToT<T>(this->ASyncQuery(res_desc));
		}

		template <typename T>
		void Unload(shared_ptr<T> const & res)
		{
			this->Unload(static_pointer_cast<void>(res));
		}

		void Update();

	private:
		std::string RealPath(std::string const & path);

		void AddLoadedResource(ResLoadingDescPtr const & res_desc, shared_ptr<void> const & res);
		shared_ptr<void> FindMatchLoadedResource(ResLoadingDescPtr const & res_desc);
		void RemoveUnrefResources();

		void LoadingThreadFunc();

	private:
		static shared_ptr<ResLoader> res_loader_instance_;

		std::string exe_path_;
		std::vector<std::string> paths_;

		std::vector<std::pair<ResLoadingDescPtr, weak_ptr<void> > > loaded_res_;
		std::vector<std::pair<ResLoadingDescPtr, shared_ptr<volatile bool> > > loading_res_;
		boost::lockfree::spsc_queue<std::pair<ResLoadingDescPtr, shared_ptr<volatile bool> >,
			boost::lockfree::capacity<1024> > loading_res_queue_;

		shared_ptr<joiner<void> > loading_thread_;
		bool quit_;
	};
}

#endif			// _KLAYGE_RESLOADER_HPP
