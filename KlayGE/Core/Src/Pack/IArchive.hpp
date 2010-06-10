// IArchive.hpp
// KlayGE 打包系统文档接口 头文件 来自7zip
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _IARCHIVE_HPP
#define _IARCHIVE_HPP

#pragma once

#include "IStream.hpp"
#include "IProgress.hpp"
#include "BSTR.hpp"

// MIDL_INTERFACE("23170F69-40C1-278A-0000-000600xx0000")
#define ARCHIVE_INTERFACE_SUB(i, base,  x) \
DEFINE_GUID(IID_ ## i, \
	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, x, 0x00, 0x00); \
struct i : public base

#define ARCHIVE_INTERFACE(i, x) ARCHIVE_INTERFACE_SUB(i, IUnknown, x)

ARCHIVE_INTERFACE(IArchiveOpenCallback, 0x10)
{
	STDMETHOD(SetTotal)(const KlayGE::uint64_t* files, const KlayGE::uint64_t* bytes) PURE;
	STDMETHOD(SetCompleted)(const KlayGE::uint64_t* files, const KlayGE::uint64_t* bytes) PURE;
};


ARCHIVE_INTERFACE_SUB(IArchiveExtractCallback, IProgress, 0x20)
{
	STDMETHOD(GetStream)(KlayGE::uint32_t index, ISequentialOutStream** outStream,
		KlayGE::int32_t askExtractMode) PURE;
	// GetStream OUT: S_OK - OK, S_FALSE - skeep this file
	STDMETHOD(PrepareOperation)(KlayGE::int32_t askExtractMode) PURE;
	STDMETHOD(SetOperationResult)(KlayGE::int32_t resultEOperationResult) PURE;
};


ARCHIVE_INTERFACE(IInArchive, 0x60)
{
	STDMETHOD(Open)(IInStream* stream, const KlayGE::uint64_t* maxCheckStartPosition,
		IArchiveOpenCallback* openArchiveCallback) PURE;
	STDMETHOD(Close)() PURE;
	STDMETHOD(GetNumberOfItems)(KlayGE::uint32_t* numItems) PURE;
	STDMETHOD(GetProperty)(KlayGE::uint32_t index, PROPID propID, PROPVARIANT* value) PURE;
	STDMETHOD(Extract)(const KlayGE::uint32_t* indices, KlayGE::uint32_t numItems,
		KlayGE::int32_t testMode, IArchiveExtractCallback* extractCallback) PURE;
	// indices must be sorted
	// numItems = 0xFFFFFFFF means all files
	// testMode != 0 means "test files operation"

	STDMETHOD(GetArchiveProperty)(PROPID propID, PROPVARIANT* value) PURE;

	STDMETHOD(GetNumberOfProperties)(KlayGE::uint32_t* numProperties) PURE;
	STDMETHOD(GetPropertyInfo)(KlayGE::uint32_t index,
		BSTR* name, PROPID* propID, VARTYPE* varType) PURE;

	STDMETHOD(GetNumberOfArchiveProperties)(KlayGE::uint32_t* numProperties) PURE;
	STDMETHOD(GetArchivePropertyInfo)(KlayGE::uint32_t index,
		BSTR* name, PROPID* propID, VARTYPE* varType) PURE;
};

#endif		// _IARCHIVE_HPP
