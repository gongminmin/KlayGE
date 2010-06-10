// Streams.hpp
// KlayGE 打包系统输入输出流 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _STREAMS_HPP
#define _STREAMS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/atomic.hpp>

#include <fstream>
#include <string>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011)
#endif
#include <boost/smart_ptr.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "IStream.hpp"

namespace KlayGE
{
	class CInStream : public IInStream, public IStreamGetSize
	{
	public:
		STDMETHOD_(ULONG, AddRef)()
		{
			++ ref_count_;
			return ref_count_.value();
		}
		STDMETHOD_(ULONG, Release)()
		{
			-- ref_count_;
			if (0 == ref_count_.value())
			{
				delete this;
				return 0;
			}
			return ref_count_.value();
		}

		STDMETHOD(QueryInterface)(REFGUID iid, void** outObject)
		{
			if (IID_IInStream == iid)
			{
				*outObject = static_cast<IInStream*>(this);
				this->AddRef();
				return S_OK;
			}
			else
			{
				if (IID_IStreamGetSize == iid)
				{
					*outObject = static_cast<IStreamGetSize*>(this);
					this->AddRef();
					return S_OK;
				}
				else
				{
					return E_NOINTERFACE;
				}
			}
		}


		CInStream()
			: ref_count_(1)
		{
		}
		virtual ~CInStream()
		{
		}

		void Attach(ResIdentifierPtr const & is);

		STDMETHOD(Read)(void* data, uint32_t size, uint32_t* processedSize);
		STDMETHOD(Seek)(int64_t offset, uint32_t seekOrigin, uint64_t* newPosition);

		STDMETHOD(GetSize)(uint64_t* size);

	private:
		atomic<int32_t> ref_count_;

		ResIdentifierPtr is_;
		uint64_t stream_size_;
	};

	class COutStream : public IOutStream
	{
	public:
		STDMETHOD_(ULONG, AddRef)()
		{
			++ ref_count_;
			return ref_count_.value();
		}
		STDMETHOD_(ULONG, Release)()
		{
			-- ref_count_;
			if (0 == ref_count_.value())
			{
				delete this;
				return 0;
			}
			return ref_count_.value();
		}

		STDMETHOD(QueryInterface)(REFGUID iid, void** outObject)
		{
			if (IID_IOutStream == iid)
			{
				*outObject = static_cast<void*>(this);
				this->AddRef();
				return S_OK;
			}
			else
			{
				return E_NOINTERFACE;
			}
		}

		COutStream()
			: ref_count_(1)
		{
		}
		virtual ~COutStream()
		{
		}

		void Attach(boost::shared_ptr<std::ostream> const & os);

		STDMETHOD(Write)(const void* data, uint32_t size, uint32_t* processedSize);
		STDMETHOD(Seek)(int64_t offset, uint32_t seekOrigin, uint64_t* newPosition);
		STDMETHOD(SetSize)(int64_t newSize);

	private:
		atomic<int32_t> ref_count_;

		boost::shared_ptr<std::ostream> os_;
	};
}

#endif		// _STREAMS_HPP
