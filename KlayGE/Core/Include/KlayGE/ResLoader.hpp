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

#ifndef _KFL_RESLOADER_HPP
#define _KFL_RESLOADER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <istream>
#include <vector>
#include <string>

#include <KFL/ResIdentifier.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ResLoadingDesc
	{
	public:
		virtual ~ResLoadingDesc()
		{
		}

		virtual std::wstring const & Name() const = 0;

		virtual void SubThreadStage() = 0;
		virtual shared_ptr<void> MainThreadStage() = 0;

		virtual bool HasSubThreadStage() const = 0;

		virtual bool Match(ResLoadingDesc const & rhs) const = 0;
		virtual void CopyFrom(ResLoadingDesc const & rhs) = 0;
	};

	class KLAYGE_CORE_API ResLoader
	{
		template <typename T>
		class EmptyFuncToT
		{
		public:
			EmptyFuncToT(function<shared_ptr<void>()> const & func, ResLoadingDescPtr const & res_desc)
				: func_(func), res_desc_(res_desc)
			{
			}

			shared_ptr<T> operator()()
			{
				shared_ptr<void> res = func_();
				ResLoader::Instance().SetFinalResource(res_desc_, res);
				return static_pointer_cast<T>(res);
			}

		private:
			function<shared_ptr<void>()> func_;
			ResLoadingDescPtr res_desc_;
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

		template <typename T>
		shared_ptr<T> SyncQueryT(ResLoadingDescPtr const & res_desc)
		{
			shared_ptr<void> res = this->SyncQuery(res_desc);
			this->SetFinalResource(res_desc, res);
			return static_pointer_cast<T>(res);
		}

		template <typename T>
		function<shared_ptr<T>()> ASyncQueryT(ResLoadingDescPtr const & res_desc)
		{
			return EmptyFuncToT<T>(this->ASyncQuery(res_desc), res_desc);
		}

	private:
		std::string RealPath(std::string const & path);

		void ASyncSubThreadFunc(ResLoadingDescPtr const & res_desc);
		shared_ptr<void> ASyncFunc(ResLoadingDescPtr const & res_desc,
			shared_ptr<joiner<void> > const & loading_thread);

		void SetFinalResource(ResLoadingDescPtr const & res_desc, shared_ptr<void> const & res);

	private:
		static shared_ptr<ResLoader> res_loader_instance_;

		std::string exe_path_;
		std::vector<std::string> paths_;

		std::vector<ResLoadingDescPtr> cached_sync_res_;
		std::vector<std::pair<ResLoadingDescPtr, shared_ptr<joiner<void> > > > cached_async_res_;

		std::vector<std::pair<ResLoadingDescPtr, weak_ptr<void> > > cached_desc_;
	};
}

#endif			// _KFL_RESLOADER_HPP
