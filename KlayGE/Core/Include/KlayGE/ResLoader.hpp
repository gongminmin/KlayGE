// ResLoader.hpp
// KlayGE 资源载入器 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2004-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 增加了ResIdentifier (2009.5.23)
//
// 2.2.0
// 初次建立 (2004.10.26)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _RESLOADER_HPP
#define _RESLOADER_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>
#include <istream>
#include <vector>
#include <string>

#include <boost/bind.hpp>

#include <KlayGE/thread.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ResIdentifier
	{
	public:
		ResIdentifier(std::string const & name, uint64_t timestamp, boost::shared_ptr<std::istream> const & is)
			: res_name_(name), timestamp_(timestamp), istream_(is)
		{
		}

		void ResName(std::string const & name)
		{
			res_name_ = name;
		}
		std::string const & ResName() const
		{
			return res_name_;
		}

		void Timestamp(uint64_t ts)
		{
			timestamp_ = ts;
		}
		uint64_t Timestamp() const
		{
			return timestamp_;
		}

		void read(void* p, size_t size)
		{
			istream_->read(static_cast<char*>(p), static_cast<std::streamsize>(size));
		}

		int64_t gcount() const
		{
			return static_cast<int64_t>(istream_->gcount());
		}

		void seekg(int64_t offset, std::ios_base::seekdir way)
		{
			istream_->seekg(static_cast<std::istream::off_type>(offset), way);
		}

		int64_t tellg()
		{
			return static_cast<int64_t>(istream_->tellg());
		}

		void clear()
		{
			istream_->clear();
		}

		operator void*() const
		{
			return istream_->operator void*();
		}

		bool operator!() const
		{
			return istream_->operator!();
		}

	private:
		std::string res_name_;
		uint64_t timestamp_;
		boost::shared_ptr<std::istream> istream_;
	};

	class KLAYGE_CORE_API ResLoadingDesc
	{
	public:
		virtual ~ResLoadingDesc()
		{
		}

		virtual void SubThreadStage() = 0;
		virtual void* MainThreadStage() = 0;

		virtual bool HasSubThreadStage() const = 0;
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
			return *static_cast<boost::shared_ptr<T>*>(this->SyncQuery(res_desc));
		}

		template <typename T>
		boost::function<boost::shared_ptr<T>()> ASyncQueryT(ResLoadingDescPtr const & res_desc)
		{
			return boost::bind(&ResLoader::EmptyToT<T>, this->ASyncQuery(res_desc));
		}

	private:		
		void ASyncSubThreadFunc(ResLoadingDescPtr const & res_desc);
		void* ASyncFunc(ResLoadingDescPtr const & res_desc, boost::shared_ptr<joiner<void> > const & loading_thread);

		template <typename T>
		static boost::shared_ptr<T> EmptyToT(boost::function<void*()> func)
		{
			return *static_cast<boost::shared_ptr<T>*>(func());
		}

	private:
		ResLoader();

		std::string exe_path_;
		std::vector<std::string> pathes_;
	};
}

#endif			// _RESLOADER_HPP
