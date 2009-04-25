// IStream.hpp
// KlayGE 打包系统输入输出流接口 头文件 来自7zip
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _ISTREAM_HPP
#define _ISTREAM_HPP

#pragma once

#include "BaseDefines.hpp"

// MIDL_INTERFACE("23170F69-40C1-278A-0000-000300xx0000")

#define STREAM_INTERFACE_SUB(i, b, x) \
DEFINE_GUID(IID_ ## i, \
	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, x, 0x00, 0x00); \
struct i : public b

#define STREAM_INTERFACE(i, x) STREAM_INTERFACE_SUB(i, IUnknown, x)

STREAM_INTERFACE(ISequentialInStream, 0x01)
{
	STDMETHOD(Read)(void *data, KlayGE::uint32_t size, KlayGE::uint32_t* processedSize) PURE;
	/*
	Out: if size != 0, return_value = S_OK and (*processedSize == 0),
	then there are no more bytes in stream.
	if (size > 0) && there are bytes in stream,
	this function must read at least 1 byte.
	This function is allowed to read less than number of remaining bytes in stream.
	You must call Read function in loop, if you need exact amount of data
	*/
};

STREAM_INTERFACE(ISequentialOutStream, 0x02)
{
	STDMETHOD(Write)(const void *data, KlayGE::uint32_t size, KlayGE::uint32_t* processedSize) PURE;
	/*
	if (size > 0) this function must write at least 1 byte.
	This function is allowed to write less than "size".
	You must call Write function in loop, if you need to write exact amount of data
	*/
};

STREAM_INTERFACE_SUB(IInStream, ISequentialInStream, 0x03)
{
	STDMETHOD(Seek)(KlayGE::int64_t offset, KlayGE::uint32_t seekOrigin, KlayGE::uint64_t* newPosition) PURE;
};

STREAM_INTERFACE_SUB(IOutStream, ISequentialOutStream, 0x04)
{
	STDMETHOD(Seek)(KlayGE::int64_t offset, KlayGE::uint32_t seekOrigin, KlayGE::uint64_t* newPosition) PURE;
	STDMETHOD(SetSize)(KlayGE::int64_t newSize) PURE;
};

STREAM_INTERFACE(IStreamGetSize, 0x06)
{
	STDMETHOD(GetSize)(KlayGE::uint64_t* size) PURE;
};

#endif		// _ISTREAM_HPP
