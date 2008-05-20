// BaseDefines.hpp
// KlayGE 打包系统基本定义 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _BASEDEFINES_HPP
#define _BASEDEFINES_HPP

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#else

#include <boost/cstdint.hpp>

struct GUID
{
	boost::uint32_t Data1;
	boost::uint16_t Data2;
	boost::uint16_t Data3;
	boost::uint8_t Data4[8];
};

typedef GUID const & REFGUID;
typedef REFGUID REFIID;

inline int operator==(REFGUID g1, REFGUID g2)
{
	for (size_t i = 0; i < sizeof(g1); ++ i)
	{
		if (reinterpret_cast<boost::uint8_t const *>(&g1)[i] != reinterpret_cast<boost::uint8_t const *>(&g2)[i])
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
typedef boost::int32_t INT32;
typedef unsigned int UINT;
typedef boost::uint32_t UINT32;
typedef boost::int32_t LONG;   // LONG, ULONG and DWORD must be 32-bit
typedef boost::uint32_t ULONG;
typedef boost::int64_t LONGLONG;
typedef boost::uint64_t ULONGLONG;
typedef short VARIANT_BOOL;
typedef LONG SCODE;
typedef wchar_t* BSTR;

struct LARGE_INTEGER
{
	LONGLONG QuadPart;
};

struct ULARGE_INTEGER
{
	ULONGLONG QuadPart;
};

struct FILETIME
{
	boost::uint32_t dwLowDateTime;
	boost::uint32_t dwHighDateTime;
};

typedef long HRESULT;
#define FAILED(Status) ((HRESULT)(Status)<0)
typedef boost::uint32_t PROPID;

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

typedef unsigned short VARTYPE;
typedef unsigned short PROPVAR_PAD1;
typedef unsigned short PROPVAR_PAD2;
typedef unsigned short PROPVAR_PAD3;

struct PROPVARIANT
{
	VARTYPE vt;
	PROPVAR_PAD1 wReserved1;
	PROPVAR_PAD2 wReserved2;
	PROPVAR_PAD3 wReserved3;
	union 
	{
		CHAR cVal;
		UCHAR bVal;
		SHORT iVal;
		USHORT uiVal;
		LONG lVal;
		ULONG ulVal;
		INT intVal;
		UINT uintVal;
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
