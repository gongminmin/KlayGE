// Extract7z.cpp
// KlayGE 打包系统7z提取器 实现文件 来自7zip
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#define INITGUID
#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/COMPtr.hpp>

#include "BaseDefines.hpp"

#include "Dll.hpp"

#include <string>
#include <algorithm>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4512)
#endif
#include <boost/algorithm/string.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "IArchive.hpp"

#include "Streams.hpp"
#include "ArchiveExtractCallback.hpp"
#include "ArchiveOpenCallback.hpp"

#include <KlayGE/Extract7z.hpp>


#ifndef KLAYGE_PLATFORM_WINDOWS
typedef KlayGE::int32_t SCODE;

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

enum
{
	kpidNoProperty = 0,

	kpidHandlerItemIndex = 2,
	kpidPath,
	kpidName,
	kpidExtension,
	kpidIsFolder,
	kpidSize,
	kpidPackedSize,
	kpidAttributes,
	kpidCreationTime,
	kpidLastAccessTime,
	kpidLastWriteTime,
	kpidSolid, 
	kpidCommented, 
	kpidEncrypted, 
	kpidSplitBefore, 
	kpidSplitAfter, 
	kpidDictionarySize, 
	kpidCRC, 
	kpidType,
	kpidIsAnti,
	kpidMethod,
	kpidHostOS,
	kpidFileSystem,
	kpidUser,
	kpidGroup,
	kpidBlock,
	kpidComment,
	kpidPosition,
	kpidPrefix,

	kpidTotalSize = 0x1100,
	kpidFreeSpace, 
	kpidClusterSize,
	kpidVolumeName,

	kpidLocalName = 0x1200,
	kpidProvider,

	kpidUserDefined = 0x10000
};

#ifdef _MSC_VER
#define WINAPI __stdcall
#else
#define WINAPI
#endif

namespace
{
	using namespace KlayGE;

    // {23170F69-40C1-278A-1000-000110070000}
    DEFINE_GUID(CLSID_CFormat7z, 
			0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

    typedef KlayGE::uint32_t (WINAPI *CreateObjectFunc)(const GUID* clsID, const GUID* interfaceID, void** outObject);

	HRESULT GetArchiveItemPath(boost::shared_ptr<IInArchive> const & archive, uint32_t index, std::string& result)
	{
		PROPVARIANT prop;
		prop.vt = VT_EMPTY;
		TIF(archive->GetProperty(index, kpidPath, &prop));
        switch (prop.vt)
		{
        case VT_BSTR:
            Convert(result, prop.bstrVal);
            return S_OK;

        case VT_EMPTY:
			result.clear();
            return S_OK;

        default:
			return E_FAIL;
        }
	}

	HRESULT IsArchiveItemFolder(boost::shared_ptr<IInArchive> const & archive, uint32_t index, bool &result)
	{
		PROPVARIANT prop;
		prop.vt = VT_EMPTY;
		TIF(archive->GetProperty(index, kpidIsFolder, &prop));
        switch (prop.vt)
        {
        case VT_BOOL:
            result = (prop.boolVal != VARIANT_FALSE);
            return S_OK;

        case VT_EMPTY:
            result = false;
            return S_OK;

        default:
            return E_FAIL;
        }
	}

    class SevenZipLoader
    {
    public:
        SevenZipLoader()
        {
			dll_loader_.Load("7z.dll");
			createObjectFunc_ = (CreateObjectFunc)dll_loader_.GetProcAddress("CreateObject");

	        BOOST_ASSERT(createObjectFunc_);
        }

        HRESULT CreateObject(const GUID* clsID, const GUID* interfaceID, void** outObject)
        {
            return createObjectFunc_(clsID, interfaceID, outObject);
        }

    private:
		DllLoader dll_loader_;
        CreateObjectFunc createObjectFunc_;
    };

    SevenZipLoader loader;
}

namespace KlayGE
{
	void Extract7z(boost::shared_ptr<std::istream> const & archive_is,
							   std::string const & password,
							   std::string const & extractFilePath,
							   boost::shared_ptr<std::ostream> const & os)
	{
		BOOST_ASSERT(archive_is);

		boost::shared_ptr<IInArchive> archive;
		{
			IInArchive* tmp;
			TIF(loader.CreateObject(&CLSID_CFormat7z, &IID_IInArchive, reinterpret_cast<void**>(&tmp)));
			archive = MakeCOMPtr(tmp);
		}

		boost::shared_ptr<IInStream> file = MakeCOMPtr(new CInStream);
		checked_pointer_cast<CInStream>(file)->Attach(archive_is);

		boost::shared_ptr<IArchiveOpenCallback> ocb = MakeCOMPtr(new CArchiveOpenCallback);
		checked_pointer_cast<CArchiveOpenCallback>(ocb)->Init(password);
		TIF(archive->Open(file.get(), 0, ocb.get()));

		uint32_t realIndex = static_cast<uint32_t>(-1);
		uint32_t numItems;
		TIF(archive->GetNumberOfItems(&numItems));

		for (uint32_t i = 0; i < numItems; ++i)
		{
			bool isFolder;
			TIF(IsArchiveItemFolder(archive, i, isFolder));
			if (!isFolder)
			{
				std::string filePath;
				TIF(GetArchiveItemPath(archive, i, filePath));
				std::replace(filePath.begin(), filePath.end(), L'\\', L'/');
				if (!boost::algorithm::ilexicographical_compare(extractFilePath, filePath)
					&& !boost::algorithm::ilexicographical_compare(filePath, extractFilePath))
				{
					realIndex = i;
					break;
				}
			}
		}
		Verify(realIndex != static_cast<uint32_t>(-1));

		boost::shared_ptr<ISequentialOutStream> outStreamLoc = MakeCOMPtr(new COutStream);
		checked_pointer_cast<COutStream>(outStreamLoc)->Attach(os);

		boost::shared_ptr<IArchiveExtractCallback> extractCallback = MakeCOMPtr(new CArchiveExtractCallback);
		checked_pointer_cast<CArchiveExtractCallback>(extractCallback)->Init(password, outStreamLoc);

		{
			PROPVARIANT prop;
			prop.vt = VT_EMPTY;
			TIF(archive->GetProperty(realIndex, kpidPosition, &prop));
			if (prop.vt != VT_EMPTY)
			{
				BOOST_ASSERT(VT_UI8 == prop.vt);
				BOOST_ASSERT(0 == prop.uhVal.QuadPart);
			}
		}
		{
			PROPVARIANT prop;
			prop.vt = VT_EMPTY;
			TIF(archive->GetProperty(realIndex, kpidIsAnti, &prop));
			BOOST_ASSERT(VT_BOOL == prop.vt);
			BOOST_ASSERT(VARIANT_FALSE == prop.boolVal);
		}

		TIF(archive->Extract(&realIndex, 1, false, extractCallback.get()));
	}
}
