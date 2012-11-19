// Streams.cpp
// KlayGE 打包系统输入输出流 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>

#include <boost/assert.hpp>

#include <CPP/Common/MyWindows.h>

#include "Streams.hpp"

namespace KlayGE
{
	void CInStream::Attach(ResIdentifierPtr const & is)
	{
		BOOST_ASSERT(*is);

		is_ = is;
		is_->seekg(0, std::ios_base::end);
		stream_size_ = is_->tellg();
		is_->seekg(0, std::ios_base::beg);
	}

	STDMETHODIMP CInStream::Read(void *data, uint32_t size, uint32_t* processedSize)
	{
		is_->read(data, size);
		if (processedSize)
		{
			*processedSize = static_cast<uint32_t>(is_->gcount());
		}

		return (*is_ != nullptr) ? S_OK: E_FAIL;
	}

	STDMETHODIMP CInStream::Seek(int64_t offset, uint32_t seekOrigin, uint64_t* newPosition)
	{
		std::ios_base::seekdir way;
		switch (seekOrigin)
		{
		case 0:
			way = std::ios_base::beg;
			break;

		case 1:
			way = std::ios_base::cur;
			break;

		case 2:
			way = std::ios_base::end;
			break;

		default:
			return STG_E_INVALIDFUNCTION;
		}

		is_->seekg(static_cast<std::istream::off_type>(offset), way);
		if (newPosition)
		{
			*newPosition = is_->tellg();
		}

		return (*is_ != nullptr) ? S_OK: E_FAIL;
	}

	STDMETHODIMP CInStream::GetSize(uint64_t* size)
	{
		*size = stream_size_;
		return S_OK;
	}


	//////////////////////////
	// COutStream

	void COutStream::Attach(boost::shared_ptr<std::ostream> const & os)
	{
		os_ = os;
	}

	STDMETHODIMP COutStream::Write(const void *data, uint32_t size, uint32_t* processedSize)
	{
		os_->write(static_cast<char const *>(data), size);
		if (processedSize)
		{
			*processedSize = size;
		}

		return (*os_ != nullptr) ? S_OK: E_FAIL;
	}

	STDMETHODIMP COutStream::Seek(int64_t offset, uint32_t seekOrigin, uint64_t* newPosition)
	{
		std::ios_base::seekdir way;
		switch (seekOrigin)
		{
		case 0:
			way = std::ios_base::beg;
			break;

		case 1:
			way = std::ios_base::cur;
			break;

		case 2:
			way = std::ios_base::end;
			break;

		default:
			return STG_E_INVALIDFUNCTION;
		}

		os_->seekp(static_cast<std::ofstream::off_type>(offset), way);
		if (newPosition)
		{
			*newPosition = os_->tellp();
		}

		return (*os_ != nullptr) ? S_OK: E_FAIL;
	}

	STDMETHODIMP COutStream::SetSize(uint64_t /*newSize*/)
	{
		return E_NOTIMPL;
	}
}
