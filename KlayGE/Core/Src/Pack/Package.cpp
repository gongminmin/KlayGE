/**
 * @file Package.cpp
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

#include <KlayGE/KlayGE.hpp>
#define INITGUID
#include <KFL/com_ptr.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/ResIdentifier.hpp>
#include <KFL/StringUtil.hpp>
#include <KFL/Util.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <unknwnbase.h>
#endif
#include <CPP/Common/MyWindows.h>

#include <KFL/DllLoader.hpp>

#include <algorithm>
#include <sstream>
#include <string>

#include <boost/assert.hpp>

#include <CPP/7zip/Archive/IArchive.h>

#include "Streams.hpp"
#include "ArchiveExtractCallback.hpp"
#include "ArchiveOpenCallback.hpp"

#include <KlayGE/Package.hpp>

#ifndef WINAPI
#ifdef KLAYGE_COMPILER_MSVC
#define WINAPI __stdcall
#else
#define WINAPI
#endif
#endif

DEFINE_UUID_OF(IArchiveExtractCallback);
DEFINE_UUID_OF(IArchiveOpenCallback);
DEFINE_UUID_OF(ICryptoGetTextPassword);
DEFINE_UUID_OF(IInArchive);
DEFINE_UUID_OF(IInStream);
DEFINE_UUID_OF(IStreamGetSize);
DEFINE_UUID_OF(IOutStream);

namespace
{
	using namespace KlayGE;

	// {23170F69-40C1-278A-1000-000110070000}
	DEFINE_GUID(CLSID_CFormat7z,
			0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

	typedef KlayGE::uint32_t (WINAPI *CreateObjectFunc)(const GUID* clsID, const GUID* interfaceID, void** outObject);

	HRESULT GetArchiveItemPath(IInArchive* archive, uint32_t index, std::string& result)
	{
		PROPVARIANT prop;
		prop.vt = VT_EMPTY;
		TIFHR(archive->GetProperty(index, kpidPath, &prop));
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

	HRESULT IsArchiveItemFolder(IInArchive* archive, uint32_t index, bool &result)
	{
		PROPVARIANT prop;
		prop.vt = VT_EMPTY;
		TIFHR(archive->GetProperty(index, kpidIsDir, &prop));
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

		HRESULT CreateObject(GUID const& cls_id, GUID const& interface_id, void** out_object)
		{
			return create_object_(&cls_id, &interface_id, out_object);
		}

	private:
		SevenZipLoader()
		{
			dll_loader_.Load(DLL_PREFIX "7zxa" DLL_SUFFIX);

			create_object_ = reinterpret_cast<CreateObjectFunc>(dll_loader_.GetProcAddress("CreateObject"));
			BOOST_ASSERT(create_object_);
		}

	private:
		DllLoader dll_loader_;
		CreateObjectFunc create_object_;
	};
}

namespace KlayGE
{
	Package::Package(ResIdentifierPtr const & archive_is)
		: Package(archive_is, "")
	{
	}

	Package::Package(ResIdentifierPtr const & archive_is, std::string_view password)
		: archive_is_(archive_is), password_(password)
	{
		BOOST_ASSERT(archive_is);

		com_ptr<IInArchive> archive;
		TIFHR(SevenZipLoader::Instance().CreateObject(
			CLSID_CFormat7z, reinterpret_cast<GUID const&>(UuidOf<IInArchive>()), archive.put_void()));

		com_ptr<IInStream> file(new InStream(archive_is), false);
		com_ptr<IArchiveOpenCallback> ocb(new ArchiveOpenCallback(password), false);
		TIFHR(archive->Open(file.get(), nullptr, ocb.get()));

		TIFHR(archive->GetNumberOfItems(&num_items_));

		archive_ = std::shared_ptr<IInArchive>(archive.detach(), std::mem_fn(&IInArchive::Release));
	}

	bool Package::Locate(std::string_view extract_file_path)
	{
		uint32_t real_index = this->Find(extract_file_path);
		return (real_index != 0xFFFFFFFF);
	}

	ResIdentifierPtr Package::Extract(std::string_view extract_file_path, std::string_view res_name)
	{
		uint32_t real_index = this->Find(extract_file_path);
		if (real_index != 0xFFFFFFFF)
		{
			auto decoded_file = MakeSharedPtr<std::stringstream>();
			com_ptr<IOutStream> out_stream(new OutStream(decoded_file), false);
			com_ptr<IArchiveExtractCallback> ecb(new ArchiveExtractCallback(password_, out_stream.get()), false);
			TIFHR(archive_->Extract(&real_index, 1, false, ecb.get()));

			PROPVARIANT prop;
			prop.vt = VT_EMPTY;
			TIFHR(archive_->GetProperty(real_index, kpidMTime, &prop));
			uint64_t mtime;
			if (prop.vt == VT_FILETIME)
			{
				mtime = (static_cast<uint64_t>(prop.filetime.dwHighDateTime) << 32)
					+ prop.filetime.dwLowDateTime;
				mtime -= 116444736000000000ULL;
			}
			else
			{
				mtime = archive_is_->Timestamp();
			}

			return MakeSharedPtr<ResIdentifier>(res_name, mtime, decoded_file);
		}
		return ResIdentifierPtr();
	}

	uint32_t Package::Find(std::string_view extract_file_path)
	{
		uint32_t real_index = 0xFFFFFFFF;

		for (uint32_t i = 0; i < num_items_; ++ i)
		{
			bool is_folder = true;
			TIFHR(IsArchiveItemFolder(archive_.get(), i, is_folder));
			if (!is_folder)
			{
				std::string file_path;
				TIFHR(GetArchiveItemPath(archive_.get(), i, file_path));
				std::replace(file_path.begin(), file_path.end(), '\\', '/');
				if (!StringUtil::CaseInsensitiveLexicographicalCompare(extract_file_path, file_path) &&
					!StringUtil::CaseInsensitiveLexicographicalCompare(file_path, extract_file_path))
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
			TIFHR(archive_->GetProperty(real_index, kpidIsAnti, &prop));
			if ((VT_BOOL == prop.vt) && (VARIANT_FALSE == prop.boolVal))
			{
				prop.vt = VT_EMPTY;
				TIFHR(archive_->GetProperty(real_index, kpidPosition, &prop));
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

		return real_index;
	}
}
