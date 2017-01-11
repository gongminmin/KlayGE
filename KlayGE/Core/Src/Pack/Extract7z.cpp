/**
 * @file Extract7z.cpp
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
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/COMPtr.hpp>

#include <CPP/Common/MyWindows.h>

#include <KFL/DllLoader.hpp>

#include <string>
#include <algorithm>

#include <boost/assert.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/algorithm/string.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

#include <CPP/7zip/Archive/IArchive.h>

#include "Streams.hpp"
#include "ArchiveExtractCallback.hpp"
#include "ArchiveOpenCallback.hpp"

#include <KlayGE/Extract7z.hpp>

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

	HRESULT GetArchiveItemPath(std::shared_ptr<IInArchive> const & archive, uint32_t index, std::string& result)
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

	HRESULT IsArchiveItemFolder(std::shared_ptr<IInArchive> const & archive, uint32_t index, bool &result)
	{
		PROPVARIANT prop;
		prop.vt = VT_EMPTY;
		TIF(archive->GetProperty(index, kpidIsDir, &prop));
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
			dll_loader_.Load(DLL_PREFIX "7zxa" DLL_SUFFIX);

			createObjectFunc_ = (CreateObjectFunc)dll_loader_.GetProcAddress("CreateObject");

			BOOST_ASSERT(createObjectFunc_);
		}

	private:
		DllLoader dll_loader_;
		CreateObjectFunc createObjectFunc_;
	};


	void GetArchiveIndex(std::shared_ptr<IInArchive>& archive, uint32_t& real_index,
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

		std::shared_ptr<IInStream> file = MakeCOMPtr(new CInStream);
		checked_pointer_cast<CInStream>(file)->Attach(archive_is);

		std::shared_ptr<IArchiveOpenCallback> ocb = MakeCOMPtr(new CArchiveOpenCallback);
		checked_pointer_cast<CArchiveOpenCallback>(ocb)->Init(password);
		TIF(archive->Open(file.get(), 0, ocb.get()));

		real_index = 0xFFFFFFFF;
		uint32_t num_items;
		TIF(archive->GetNumberOfItems(&num_items));

		for (uint32_t i = 0; i < num_items; ++ i)
		{
			bool is_folder = true;
			TIF(IsArchiveItemFolder(archive, i, is_folder));
			if (!is_folder)
			{
				std::string file_path;
				TIF(GetArchiveItemPath(archive, i, file_path));
				std::replace(file_path.begin(), file_path.end(), '\\', '/');
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
		std::shared_ptr<IInArchive> archive;
		uint32_t real_index;
		GetArchiveIndex(archive, real_index, archive_is, password, extract_file_path);
		return real_index;
	}

	void Extract7z(ResIdentifierPtr const & archive_is,
							   std::string const & password,
							   std::string const & extract_file_path,
		std::shared_ptr<std::ostream> const & os)
	{
		std::shared_ptr<IInArchive> archive;
		uint32_t real_index;
		GetArchiveIndex(archive, real_index, archive_is, password, extract_file_path);
		if (real_index != 0xFFFFFFFF)
		{
			std::shared_ptr<ISequentialOutStream> out_stream = MakeCOMPtr(new COutStream);
			checked_pointer_cast<COutStream>(out_stream)->Attach(os);

			std::shared_ptr<IArchiveExtractCallback> ecb = MakeCOMPtr(new CArchiveExtractCallback);
			checked_pointer_cast<CArchiveExtractCallback>(ecb)->Init(password, out_stream);

			TIF(archive->Extract(&real_index, 1, false, ecb.get()));
		}
	}
}
