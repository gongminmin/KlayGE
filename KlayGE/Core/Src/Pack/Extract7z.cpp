// Extract7z.cpp
// KlayGE 打包系统7z提取器 实现文件 来自7zip
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2007-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 改用7z.dll (2009.3.8)
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#define INITGUID
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/COMPtr.hpp>

#include "BaseDefines.hpp"

#include <KlayGE/DllLoader.hpp>

#include <string>
#include <algorithm>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#include "IArchive.hpp"

#include "Streams.hpp"
#include "ArchiveExtractCallback.hpp"
#include "ArchiveOpenCallback.hpp"

#include <KlayGE/Extract7z.hpp>

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

#ifndef WINAPI
#ifdef _MSC_VER
#define WINAPI __stdcall
#else
#define WINAPI
#endif
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
		static SevenZipLoader& Instance()
		{
			static SevenZipLoader ret;
			return ret;
		}

		HRESULT CreateObject(const GUID* clsID, const GUID* interfaceID, void** outObject)
		{
			return createObjectFunc_(clsID, interfaceID, outObject);
		}

	private:
		SevenZipLoader()
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			dll_loader_.Load("7z.dll");
#elif defined KLAYGE_PLATFORM_LINUX
			dll_loader_.Load("7z.so");
#endif
			createObjectFunc_ = (CreateObjectFunc)dll_loader_.GetProcAddress("CreateObject");

			BOOST_ASSERT(createObjectFunc_);
		}

	private:
		DllLoader dll_loader_;
		CreateObjectFunc createObjectFunc_;
	};


	void GetArchiveIndex(boost::shared_ptr<IInArchive>& archive, uint32_t& real_index,
								ResIdentifierPtr const & archive_is,
								std::string const & password,
								std::string const & extract_file_path)
	{
		BOOST_ASSERT(archive_is);

		{
			IInArchive* tmp;
			TIF(SevenZipLoader::Instance().CreateObject(&CLSID_CFormat7z, &IID_IInArchive, reinterpret_cast<void**>(&tmp)));
			archive = MakeCOMPtr(tmp);
		}

		boost::shared_ptr<IInStream> file = MakeCOMPtr(new CInStream);
		checked_pointer_cast<CInStream>(file)->Attach(archive_is);

		boost::shared_ptr<IArchiveOpenCallback> ocb = MakeCOMPtr(new CArchiveOpenCallback);
		checked_pointer_cast<CArchiveOpenCallback>(ocb)->Init(password);
		TIF(archive->Open(file.get(), 0, ocb.get()));

		real_index = 0xFFFFFFFF;
		uint32_t num_items;
		TIF(archive->GetNumberOfItems(&num_items));

		for (uint32_t i = 0; i < num_items; ++i)
		{
			bool is_folder = true;
			TIF(IsArchiveItemFolder(archive, i, is_folder));
			if (!is_folder)
			{
				std::string file_path;
				TIF(GetArchiveItemPath(archive, i, file_path));
				std::replace(file_path.begin(), file_path.end(), L'\\', L'/');
				if (!boost::algorithm::ilexicographical_compare(extract_file_path, file_path)
					&& !boost::algorithm::ilexicographical_compare(file_path, extract_file_path))
				{
					real_index = i;
					break;
				}
			}
		}
		if (real_index != 0xFFFFFFFF)
		{
			PROPVARIANT prop;
			prop.vt = VT_EMPTY;
			TIF(archive->GetProperty(real_index, kpidIsAnti, &prop));
			if ((VT_BOOL == prop.vt) && (VARIANT_FALSE == prop.boolVal))
			{
				prop.vt = VT_EMPTY;
				TIF(archive->GetProperty(real_index, kpidPosition, &prop));
				if (prop.vt != VT_EMPTY)
				{
					if ((prop.vt != VT_UI8) || (prop.uhVal.QuadPart != 0))
					{
						real_index = 0xFFFFFFFF;
					}
				}
			}
			else
			{
				real_index = 0xFFFFFFFF;
			}
		}
	}
}

namespace KlayGE
{
	uint32_t Find7z(ResIdentifierPtr const & archive_is,
								std::string const & password,
								std::string const & extract_file_path)
	{
		boost::shared_ptr<IInArchive> archive;
		uint32_t real_index;
		GetArchiveIndex(archive, real_index, archive_is, password, extract_file_path);
		return real_index;
	}

	void Extract7z(ResIdentifierPtr const & archive_is,
							   std::string const & password,
							   std::string const & extract_file_path,
							   boost::shared_ptr<std::ostream> const & os)
	{
		boost::shared_ptr<IInArchive> archive;
		uint32_t real_index;
		GetArchiveIndex(archive, real_index, archive_is, password, extract_file_path);
		if (real_index != 0xFFFFFFFF)
		{
			boost::shared_ptr<ISequentialOutStream> out_stream = MakeCOMPtr(new COutStream);
			checked_pointer_cast<COutStream>(out_stream)->Attach(os);

			boost::shared_ptr<IArchiveExtractCallback> ecb = MakeCOMPtr(new CArchiveExtractCallback);
			checked_pointer_cast<CArchiveExtractCallback>(ecb)->Init(password, out_stream);

			TIF(archive->Extract(&real_index, 1, false, ecb.get()));
		}
	}
}
