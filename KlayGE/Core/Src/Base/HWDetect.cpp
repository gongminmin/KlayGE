/**
* @file HWDetect.cpp
* @author Boxiang Pei, Minmin Gong
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
#include <KFL/CXX17/iterator.hpp>

#include <cstring>
#include <boost/assert.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <KFL/COMPtr.hpp>
#ifndef __wbemdisp_h__
#define __wbemdisp_h__	// Force not to include wbemdisp.h
#endif
#include <WbemIdl.h>

#if defined(KLAYGE_COMPILER_MSVC) || defined(KLAYGE_COMPILER_CLANGC2)
DEFINE_GUID(IID_IWbemLocator, 0xdc12a687, 0x737f, 0x11cf, 0x88, 0x4d, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24);
#endif
#if !defined(KLAYGE_COMPILER_GCC)
DEFINE_GUID(CLSID_WbemLocator, 0x4590f811, 0x1d3a, 0x11d0, 0x89, 0x1f, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24);
#endif
#endif

#include <KlayGE/HWDetect.hpp>

namespace KlayGE
{
	char const * Mainboard::mainboard_type_name_[MainboardInfo::MT_Num] =
	{
		"Unknown",
		"Unknown",
		"Other",
		"ServerBlade",
		"ConnectivitySwitch",
		"SystemManagementModule",
		"ProcessorModule",
		"IOModule",
		"MemoryModule",
		"DaughterBoard",
		"MainBoard",
		"ProcessorMemorymModule",
		"ProcessorIOModule",
		"InterconnectBoard"
	};

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	class WMI : boost::noncopyable
	{
	public:
		bool Init()
		{
			if (!(this->InitComponent() && this->InitSecurity() && this->CreateInstance()))
			{
				return false;
			}

			return true;
		}

		bool ConnectServer(BSTR network_resource, BSTR username = nullptr, BSTR password = nullptr)
		{
			BOOST_ASSERT(wbem_locator_);

			IWbemServices* wbem_services = nullptr;
			HRESULT hr = wbem_locator_->ConnectServer(network_resource, username, password, nullptr,
				0, nullptr, nullptr, &wbem_services);
			wbem_services_ = MakeCOMPtr(wbem_services);
			if (FAILED(hr) || !this->SetProxyBlanket())
			{
				wbem_services_.reset();
				return false;
			}

			return true;
		}

		bool ExecuteQuery(BSTR wql)
		{
			BOOST_ASSERT(wbem_services_);

			IEnumWbemClassObject* wbem_enum_result = nullptr;
			wchar_t query_lang[] = L"WQL";
			HRESULT hr = wbem_services_->ExecQuery(query_lang, wql,
				WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &wbem_enum_result);
			if (FAILED(hr))
			{
				return false;
			}
			wbem_enum_result_ = MakeCOMPtr(wbem_enum_result);

			return true;
		}

		bool GetEnumerate()
		{
			HRESULT hr = S_OK;
			ULONG result = 0;
			if (wbem_enum_result_)
			{
				IWbemClassObject* wbem_object = nullptr;
				hr = wbem_enum_result_->Next(WBEM_INFINITE, 1, &wbem_object, &result);
				if (FAILED(hr) || (0 == result) || !wbem_object)
				{
					return false;
				}
				wbem_object_ = MakeCOMPtr(wbem_object);
			}

			return true;
		}

		bool GetResult(LPCWSTR field, VARIANT& result, CIMTYPE* type = nullptr)
		{
			BOOST_ASSERT(wbem_object_);

			HRESULT hr = wbem_object_->Get(field, 0, &result, type, nullptr);
			if (FAILED(hr))
			{
				::VariantClear(&result);
				return false;
			}

			return true;
		}

	private:
		bool InitComponent()
		{
			HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
			if (FAILED(hr))
			{
				hr = ::CoInitialize(nullptr);
			}

			return SUCCEEDED(hr);
		}

		bool InitSecurity()
		{
			HRESULT hr = ::CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
				RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
			return (SUCCEEDED(hr) || (RPC_E_TOO_LATE == hr));
		}

		bool CreateInstance()
		{
			IWbemLocator* locator;
			HRESULT hr = ::CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator,
				reinterpret_cast<void**>(&locator));
			wbem_locator_ = MakeCOMPtr(locator);
			if (FAILED(hr))
			{
				wbem_locator_.reset();
				return false;
			}

			return true;
		}

		bool SetProxyBlanket()
		{
			BOOST_ASSERT(wbem_services_);

			HRESULT hr = ::CoSetProxyBlanket(wbem_services_.get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
				RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
			return SUCCEEDED(hr);
		}

	private:
		std::shared_ptr<IWbemClassObject> wbem_object_;
		std::shared_ptr<IEnumWbemClassObject> wbem_enum_result_;
		std::shared_ptr<IWbemLocator> wbem_locator_;
		std::shared_ptr<IWbemServices> wbem_services_;
	};
#endif


	SMBios& SMBios::Instance()
	{
		static SMBios smbios;
		return smbios;
	}

	SMBios::SMBios()
		: smbios_data_(nullptr), smbios_size_(0),
			major_ver_(0), minor_ver_(0)
	{
		if (this->ReadVersionAndData())
		{
			this->EnumEachTable();
		}
	}

	bool SMBios::ReadVersionAndData()
	{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		WMI wmi;

		if (!wmi.Init())
		{
			return false;
		}

		wchar_t network_resource[] = L"root\\WMI";
		if (!wmi.ConnectServer(network_resource))
		{
			return false;
		}

		wchar_t wql[] = L"SELECT * FROM MSSMBios_RawSMBiosTables";
		if (!wmi.ExecuteQuery(wql))
		{
			return false;
		}

		if (wmi.GetEnumerate())
		{
			VARIANT val;
			wmi.GetResult(L"SmbiosMajorVersion", val);
			major_ver_ = val.bVal;

			wmi.GetResult(L"SmbiosMinorVersion", val);
			minor_ver_ = val.bVal;

			wmi.GetResult(L"SMBiosData", val);
			BOOST_ASSERT((VT_UI1 | VT_ARRAY) == val.vt);

			SAFEARRAY* safe_array = V_ARRAY(&val);
			smbios_data_ = reinterpret_cast<uint8_t*>(safe_array->pvData);
			smbios_size_ = safe_array->rgsabound[0].cElements;

			return true;
		}
#endif

		return false;
	}

	uint32_t SMBios::TypeCount(uint8_t type) const
	{
		uint32_t ret = 0;
		for (auto const & table : smbios_tables_)
		{
			if (table.type == type)
			{
				++ ret;
			}
		}
		return ret;
	}

	void SMBios::EnumEachTable()
	{
		BOOST_ASSERT(smbios_data_);

		uint32_t index = 0;
		TableInfo table;

		while (index < smbios_size_)
		{
			table.offset = index;
			table.type = smbios_data_[index];
			table.data_size = smbios_data_[index + 1];
			table.real_size = this->GetTableRealSize(&smbios_data_[index], table.data_size);

			index += table.real_size;

			smbios_tables_.push_back(table);
		}
	}

	uint16_t SMBios::GetTableRealSize(uint8_t const * data, uint8_t data_size)
	{
		uint16_t res_size = data_size;
		while (*(reinterpret_cast<uint16_t const *>(data + res_size)) != 0)
		{
			++ res_size;
		}

		return res_size + 2;
	}

	bool SMBios::FindFirstTargetType(uint8_t type)
	{
		BOOST_ASSERT(smbios_data_);

		smbios_index_iter_ = smbios_tables_.end();
		for (auto iter = smbios_tables_.begin(); iter != smbios_tables_.end(); ++ iter)
		{
			if (iter->type == type)
			{
				smbios_index_iter_ = iter;
				break;
			}
		}

		return smbios_index_iter_ != smbios_tables_.end();
	}

	bool SMBios::FindNextTargetType()
	{
		BOOST_ASSERT(smbios_data_);

		if (smbios_index_iter_ != smbios_tables_.end())
		{
			uint8_t const type = smbios_index_iter_->type;
			auto iter = smbios_index_iter_ + 1;
			smbios_index_iter_ = smbios_tables_.end();
			for (; iter != smbios_tables_.end(); ++ iter)
			{
				if (iter->type == type)
				{
					smbios_index_iter_ = iter;
					break;
				}
			}
		}

		return smbios_index_iter_ != smbios_tables_.end();
	}

	void SMBios::FillDataField(void* dst, uint32_t size, uint32_t id)
	{
		if ((id > 0) && (id < smbios_index_iter_->data_size))
		{
			uint8_t* pointer = smbios_data_ + smbios_index_iter_->offset;
			memcpy(dst, pointer + id, size);
		}
	}

	void SMBios::FillStringField(char const *& dst, uint32_t id)
	{
		if ((id > 0) && (id < smbios_index_iter_->data_size))
		{
			char const * data = reinterpret_cast<char const *>(smbios_data_) + smbios_index_iter_->offset;

			uint8_t index = 1;
			char str_index = data[id];
			char const * str_index_ptr = data + smbios_index_iter_->data_size;
			while (str_index_ptr != data + smbios_index_iter_->real_size)
			{
				if (index == str_index)
				{
					dst = static_cast<char const *>(str_index_ptr);
					return;
				}

				if ('\0' == *str_index_ptr)
				{
					++ index;
				}

				++ str_index_ptr;
			}
		}
		else
		{
			dst = nullptr;
		}
	}


	Mainboard::Mainboard()
	{
		this->ReadMainboard();
		this->ReadBios();
	}

	bool Mainboard::ReadMainboard()
	{
		if (SMBios::Instance().FindFirstTargetType(SMBT_Mainboard))
		{
			memset(&mainboard_, 0, sizeof(mainboard_));
			this->FillData(mainboard_);
			return true;
		}

		return false;
	}

	bool Mainboard::ReadBios()
	{
		if (SMBios::Instance().FindFirstTargetType(SMBT_Bios))
		{
			this->FillData(bios_);
			return true;
		}

		return false;
	}

	void Mainboard::FillData(MainboardInfo& mb_info)
	{
		SMBios::Instance().FillDataField(mb_info.feature_flag, 0x09);
		SMBios::Instance().FillDataField(mb_info.board_type, 0x0D);
		SMBios::Instance().FillDataField(mb_info.contain_object_handler_num, 0x0E);

		SMBios::Instance().FillDataField(mb_info.chas_handler, 0x0B);

		SMBios::Instance().FillStringField(mb_info.manufacturer, 0x04);
		SMBios::Instance().FillStringField(mb_info.product, 0x05);
		SMBios::Instance().FillStringField(mb_info.version, 0x06);
		SMBios::Instance().FillStringField(mb_info.serial_num, 0x07);
		SMBios::Instance().FillStringField(mb_info.asset_tag, 0x08);
		SMBios::Instance().FillStringField(mb_info.location_within_chassis, 0x0A);
	}

	void Mainboard::FillData(BiosInfo& bios_info)
	{
		SMBios::Instance().FillStringField(bios_info.vendor, 0x04);
		SMBios::Instance().FillStringField(bios_info.version, 0x05);
		SMBios::Instance().FillStringField(bios_info.release_date, 0x08);

		SMBios::Instance().FillDataField(bios_info.start_addr_segment, 0x06);
		SMBios::Instance().FillDataField(bios_info.rom_size, 0x09);
		SMBios::Instance().FillDataField(bios_info.characteristics, 0x0A);
		SMBios::Instance().FillDataField(bios_info.extension1, 0x12);
		SMBios::Instance().FillDataField(bios_info.extension2, 0x13);
		SMBios::Instance().FillDataField(bios_info.major, 0x14);
		SMBios::Instance().FillDataField(bios_info.minor, 0x15);
		SMBios::Instance().FillDataField(bios_info.firmware_major, 0x16);
		SMBios::Instance().FillDataField(bios_info.firmware_minor, 0x17);
	}

	char const * Mainboard::Manufacturer() const
	{
		return mainboard_.manufacturer;
	}

	char const * Mainboard::Product() const
	{
		return mainboard_.product;
	}

	char const * Mainboard::BiosVendor() const
	{
		return bios_.vendor;
	}

	char const * Mainboard::BiosVersion() const
	{
		return bios_.version;
	}

	char const * Mainboard::BiosReleaseDate() const
	{
		return bios_.release_date;
	}
	
	char const * Mainboard::Version() const
	{
		return mainboard_.version;
	}

	char const * Mainboard::BoardTypeName() const
	{
		BOOST_ASSERT(mainboard_.board_type < std::size(mainboard_type_name_));
		return mainboard_type_name_[mainboard_.board_type];
	}


	MemoryBank::MemoryBank()
	{
		if (SMBios::Instance().FindFirstTargetType(SMBT_MemDevice))
		{
			do
			{
				MemoryDeviceInfo device;
				memset(&device, 0, sizeof(device));
				this->DataFill(device);
				devices_.push_back(device);
			} while (SMBios::Instance().FindNextTargetType());
		}
	}

	void MemoryBank::DataFill(MemoryDeviceInfo& md_info)
	{
		SMBios::Instance().FillDataField(md_info.physical_memory_array_handle, 0x04);
		SMBios::Instance().FillDataField(md_info.mem_error_info_handle, 0x06);
		SMBios::Instance().FillDataField(md_info.total_width, 0x08);
		SMBios::Instance().FillDataField(md_info.data_width, 0x0A);
		SMBios::Instance().FillDataField(md_info.size, 0x0C);
		SMBios::Instance().FillDataField(md_info.form_factor, 0x0E);
		SMBios::Instance().FillDataField(md_info.device_set, 0x0F);
		SMBios::Instance().FillDataField(md_info.memory_type, 0x12);
		SMBios::Instance().FillDataField(md_info.type_detail, 0x13);
		SMBios::Instance().FillDataField(md_info.speed, 0x15);
		SMBios::Instance().FillDataField(md_info.attributes, 0x1B);
		SMBios::Instance().FillDataField(md_info.extended_size, 0x1C);
		SMBios::Instance().FillDataField(md_info.configured_memory_clk_speed, 0x20);
		SMBios::Instance().FillDataField(md_info.min_voltage, 0x22);
		SMBios::Instance().FillDataField(md_info.max_voltage, 0x24);
		SMBios::Instance().FillDataField(md_info.configured_voltage, 0x26);

		SMBios::Instance().FillStringField(md_info.device_locator, 0x10);
		SMBios::Instance().FillStringField(md_info.bank_locator, 0x11);
		SMBios::Instance().FillStringField(md_info.manufacturer, 0x17);
		SMBios::Instance().FillStringField(md_info.serial_num, 0x18);
		SMBios::Instance().FillStringField(md_info.asset_tag, 0x19);
		SMBios::Instance().FillStringField(md_info.part_number, 0x1A);
	}
}
