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

#include <boost/bind.hpp>
#include <boost/function.hpp>

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
		virtual void* MainThreadStage() = 0;

		virtual bool HasSubThreadStage() const = 0;

		virtual bool Match(ResLoadingDesc const & rhs) const = 0;
	};

	class KLAYGE_CORE_API ResLoader
	{
	public:
		static ResLoader& Instance();

		void AddPath(std::string const & path);

		ResIdentifierPtr Open(std::string const & name);
		std::string Locate(std::string const & name);

		void* SyncQuery(ResLoadingDescPtr const & res_desc);
		boost::function<void*()> ASyncQuery(ResLoadingDescPtr const & res_desc);

		template <typename T>
		boost::shared_ptr<T> SyncQueryT(ResLoadingDescPtr const & res_desc)
		{
			return this->EmptyToT<T>(this->SyncQuery(res_desc));
		}

		template <typename T>
		boost::function<boost::shared_ptr<T>()> ASyncQueryT(ResLoadingDescPtr const & res_desc)
		{
			return boost::bind(&ResLoader::EmptyFuncToT<T>, this->ASyncQuery(res_desc));
		}

	private:		
		void ASyncSubThreadFunc(ResLoadingDescPtr const & res_desc);
		void* ASyncFunc(ResLoadingDescPtr const & res_desc, boost::shared_ptr<joiner<void> > const & loading_thread);

		template <typename T>
		static boost::shared_ptr<T> EmptyToT(void* p)
		{
			return *static_cast<boost::shared_ptr<T>*>(p);
		}

		template <typename T>
		static boost::shared_ptr<T> EmptyFuncToT(boost::function<void*()> const & func)
		{
			return EmptyToT<T>(func());
		}

	private:
		ResLoader();

		std::string exe_path_;
		std::vector<std::string> paths_;

		std::vector<std::pair<ResLoadingDescPtr, void*> > cached_sync_res_;
		std::vector<std::pair<ResLoadingDescPtr, boost::function<void*()> > > cached_async_res_;
	};
}

#endif			// _KFL_RESLOADER_HPP
