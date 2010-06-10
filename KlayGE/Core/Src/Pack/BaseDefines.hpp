// BaseDefines.hpp
// KlayGE 打包系统基本定义 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _BASEDEFINES_HPP
#define _BASEDEFINES_HPP

#pragma once

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#else

#include <boost/cstdint.hpp>

struct GUID
{
	KlayGE::uint32_t Data1;
	KlayGE::uint16_t Data2;
	KlayGE::uint16_t Data3;
	KlayGE::uint8_t Data4[8];
};

typedef GUID const & REFGUID;
typedef REFGUID REFIID;

inline int operator==(REFGUID g1, REFGUID g2)
{
	using KlayGE::uint8_t;
	for (size_t i = 0; i < sizeof(g1); ++ i)
	{
		if (reinterpret_cast<uint8_t const *>(&g1)[i] != reinterpret_cast<uint8_t const *>(&g2)[i])
		{
			return 0;
		}
	}
	return 1;
}
inline int operator!=(REFGUID g1, REFGUID g2)
{
	return !(g1 == g2);
}

#ifdef DEFINE_GUID
#undef DEFINE_GUID
#endif

#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	extern "C" const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	extern "C" const GUID name
#endif


typedef char CHAR;
typedef unsigned char UCHAR;
typedef short SHORT;
typedef unsigned short USHORT;
typedef int INT;
typedef KlayGE::int32_t INT32;
typedef unsigned int UINT;
typedef KlayGE::uint32_t UINT32;
typedef KlayGE::int32_t LONG;   // LONG, ULONG and DWORD must be 32-bit
typedef KlayGE::uint32_t ULONG;
typedef KlayGE::int64_t LONGLONG;
typedef KlayGE::uint64_t ULONGLONG;
typedef KlayGE::int32_t SCODE;
typedef wchar_t* BSTR;

struct LARGE_INTEGER
{
	KlayGE::int64_t QuadPart;
};
struct ULARGE_INTEGER
{
	KlayGE::uint64_t QuadPart;
};

struct FILETIME
{
	KlayGE::uint32_t dwLowDateTime;
	KlayGE::uint32_t dwHighDateTime;
};

typedef long HRESULT;
#define FAILED(Status) ((HRESULT)(Status)<0)
typedef KlayGE::uint32_t PROPID;

#define S_OK    ((HRESULT)0x00000000L)
#define S_FALSE ((HRESULT)0x00000001L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_ABORT ((HRESULT)0x80004004L)
#define E_FAIL ((HRESULT)0x80004005L)
#define STG_E_INVALIDFUNCTION ((HRESULT)0x80030001L)

#ifdef _MSC_VER
#define STDMETHODCALLTYPE	__stdcall
#else
#define STDMETHODCALLTYPE
#endif

#define STDMETHOD_(t, f) virtual t STDMETHODCALLTYPE f
#define STDMETHOD(f) STDMETHOD_(HRESULT, f)
#define STDMETHODIMP_(type) type STDMETHODCALLTYPE
#define STDMETHODIMP STDMETHODIMP_(HRESULT)

#define PURE = 0

struct IUnknown
{
	STDMETHOD(QueryInterface) (REFIID iid, void** outObject) PURE;
	STDMETHOD_(ULONG, AddRef)() PURE;
	STDMETHOD_(ULONG, Release)() PURE;
#ifndef KLAYGE_PLATFORM_WINDOWS
	virtual ~IUnknown()
	{
	}
#endif
};

typedef short VARIANT_BOOL;
#define VARIANT_TRUE ((VARIANT_BOOL)-1)
#define VARIANT_FALSE ((VARIANT_BOOL)0)

enum VARENUM
{
	VT_EMPTY = 0,
	VT_NULL = 1,
	VT_I2 = 2,
	VT_I4 = 3,
	VT_R4 = 4,
	VT_R8 = 5,
	VT_CY = 6,
	VT_DATE = 7,
	VT_BSTR = 8,
	VT_DISPATCH = 9,
	VT_ERROR = 10,
	VT_BOOL = 11,
	VT_VARIANT = 12,
	VT_UNKNOWN = 13,
	VT_DECIMAL = 14,
	VT_I1 = 16,
	VT_UI1 = 17,
	VT_UI2 = 18,
	VT_UI4 = 19,
	VT_I8 = 20,
	VT_UI8 = 21,
	VT_INT = 22,
	VT_UINT = 23,
	VT_VOID = 24,
	VT_HRESULT = 25,
	VT_FILETIME = 64
};

typedef unsigned short VARTYPE;
typedef KlayGE::uint16_t PROPVAR_PAD1;
typedef KlayGE::uint16_t PROPVAR_PAD2;
typedef KlayGE::uint16_t PROPVAR_PAD3;

struct PROPVARIANT
{
	VARTYPE vt;
	PROPVAR_PAD1 wReserved1;
	PROPVAR_PAD2 wReserved2;
	PROPVAR_PAD3 wReserved3;
	union
	{
		KlayGE::int8_t cVal;
		KlayGE::uint8_t bVal;
		KlayGE::int16_t iVal;
		KlayGE::uint16_t uiVal;
		KlayGE::int32_t lVal;
		KlayGE::uint32_t ulVal;
		KlayGE::int32_t intVal;
		KlayGE::uint32_t uintVal;
		LARGE_INTEGER hVal;
		ULARGE_INTEGER uhVal;
		VARIANT_BOOL boolVal;
		SCODE scode;
		FILETIME filetime;
		BSTR bstrVal;
	};
};

#endif

#endif		// _BASEDEFINES_HPP
