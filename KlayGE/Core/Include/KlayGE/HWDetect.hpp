/**
* @file HWDetect.hpp
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

#ifndef _KLAYGE_HWDETECT_HPP
#define _KLAYGE_HWDETECT_HPP

#pragma once

namespace KlayGE
{
	enum SMBiosType
	{
		SMBT_Bios		= 0x00,
		SMBT_System		= 0x01,
		SMBT_Mainboard	= 0x02,
		SMBT_Enclosure	= 0x03,
		SMBT_Processor	= 0x04,
		SMBT_Cache		= 0x07,
		SMBT_Slots		= 0x09,
		SMBT_MemArray	= 0x10,
		SMBT_MemDevice	= 0x11
	};

	struct BiosInfo
	{
		char const * vendor;				// 0x04 Bios Vendor's Name
		char const * version;				// 0x05 Free form version info
		uint16_t start_addr_segment;		// 0x06 Segment location of Bios starting address
		char const * release_date;			// 0x08 mm/dd/yy or mm/dd/yyyy
		uint8_t rom_size;					// 0x09 Size(n) where 64k * (n + 1) is size of physical device device containing bios in bytes
		uint64_t characteristics;			// 0x0A Characteristics
		uint8_t extension1;					// 0x12 Optional space reserved for extensions. (Length in offset 1 - 0x12)
		uint8_t extension2;					// 0x13 Optional space reserved for extensions. (Length in offset 1 - 0x12)
		uint8_t major;						// 0x14 Major release for system bios (e.g. bios = 10.2, sys_bios_major = 0x0A)
		uint8_t minor;						// 0x15 Minor release for system bios (e.g. bios = 10.2, sys_bios_minor = 0x02)
		uint8_t firmware_major;				// 0x16 Major release for embedded firmware
		uint8_t firmware_minor;				// 0x17 Minor release for embedded firmware

		enum BIOSCharacteristics : uint32_t
		{
			BCT_Reserved1 = 1UL << 0,
			BCT_Reserved2 = 1UL << 1,
			BCT_Unknown = 1UL << 2,
			BCT_NotSupported = 1UL << 3,
			BCT_ISASupported = 1UL << 4,
			BCT_MCSSupported = 1UL << 5,
			BCT_EsiaSupported = 1UL << 6,
			BCT_PciSupported = 1UL << 7,
			BCT_PcmciaSupported = 1UL << 8,
			BCT_PnpSupported = 1UL << 9,
			BCT_ApmSupported = 1UL << 10,
			BCT_BiosFlashable = 1UL << 11,
			BCT_BiosShadowSupported = 1UL << 12,
			BCT_VlVesaSupported = 1UL << 13,
			BCT_EscdSupported = 1UL << 14,
			BCT_CdBootSupported = 1UL << 15,
			BCT_SelectBootSupported = 1UL << 16,
			BCT_BiosRomSocketed = 1UL << 17,
			BCT_PccardBootSupported = 1UL << 18,
			BCT_EddSupported = 1UL << 19,
			BCT_Int13HJapFloppyNecSupported = 1UL << 20,
			BCT_Int13HJapFloppyToshibaSupported = 1UL << 21,
			BCT_Int13H360KBFloppySupported = 1UL << 22,
			BCT_Int13H12MBFloppySupported = 1UL << 23,
			BCT_Int13H720KBFloppySupported = 1UL << 24,
			BCT_Int13H288MBFloppySupported = 1UL << 25,
			BCT_Int5HPrintScreenSupported = 1UL << 26,
			BCT_Int9H842KeyboardSupported = 1UL << 27,
			BCT_Int14HSerialSupported = 1UL << 28,
			BCT_Int17HPrinterSupported = 1UL << 29,
			BCT_Int10HCGAMonoVideoSupported = 1UL << 30,
			BCT_NecPC98 = 1UL << 31
			// Bit 32 - 47: Reserved for bios vendor
			// Bit 48 - 63: Reserved for system vendor
		};

		enum BiosExtension1
		{
			BE1_ACPISupported = 1UL << 0,
			BE1_USBLegacySupported = 1UL << 1,
			BE1_AGPSupported = 1UL << 2,
			BE1_I20Supported = 1UL << 3,
			BE1_LS120Supported = 1UL << 4,
			BE1_ATAPIZipBootSupported = 1UL << 5,
			BE1_Boot1394Supported = 1UL << 6,
			BE1_SmartBatterySupported = 1UL << 7
		};

		enum BiosExtension2
		{
			BE2_BiosBootSpecSupported = 1UL << 0,
			BE2_FuncKeyInitNetBootSupported = 1UL << 1,
			BE2_EnableTargetedContentDistri = 1UL << 2,
			BE2_UEFISpecSupported = 1UL << 3,
			BE2_SMBiosVirtualMachine = 1UL << 4
			// Bit 5 - 7: Reserved
		};
	};

	struct MainboardInfo
	{
		char const * manufacturer;					// 0x04 Mainboard manufacturer name
		char const * product;						// 0x05 Mainboard product name
		char const * version;						// 0x06 String representation of mainboard version
		char const * serial_num;					// 0x07 Mainboard serial number
		char const * asset_tag;						// 0x08 Mainboard asset tag 
		uint8_t feature_flag;		  				// 0x09 Mainboard feature flags
		char const * location_within_chassis;		// 0x0A Describes this board's location within the chassis referenced by the Chassis Handle below.
		uint16_t chas_handler;						// 0x0B The handle, or instance number, associated with the chassis in which this board resides
		uint8_t board_type;							// 0x0D Identifies the type of board
		uint8_t contain_object_handler_num;			// 0x0E Identifies the number (0 to 255) of Contained Object Handles that follow.
		uint16_t const * contain_object_handler;	// 0x0F A list of handles of other structures

		enum MainboardFeatureFlag
		{
			MFF_HostingBoard = 1UL << 0,
			MFF_HaveOtherBoard = 1UL << 1,
			MFF_Removeable = 1UL << 2,
			MFF_Replaceable = 1UL << 3,
			MFF_HotSwappable = 1UL << 4
			// Bit 5 - 7: Reserved for future
		};

		enum MainboardType
		{
			MT_Unknown = 0x01,
			MT_Other,
			MT_ServerBlade,
			MT_ConnectivitySwitch,
			MT_SystemManagementModule,
			MT_ProcessorModule,
			MT_IOModule,
			MT_MemoryModule,
			MT_DaughterBoard,
			MT_MainBoard,
			MT_ProcessorMemoryModule,
			MT_ProcessorIOModule,
			MT_InterconnectBoard,

			MT_Num
		};
	};

	struct MemoryArrayInfo
	{
		uint8_t location;
		uint8_t use;
		uint8_t mem_error_correct;
		uint32_t max_capacity;
		uint16_t mem_error_info_handle;
		uint16_t number_of_memory_dev;
		uint64_t extended_max_capacity;

		enum MemoryArrayLocation
		{
			MAL_Other = 0x01,
			MAL_Unknown,
			MAL_SysOrMain,
			MAL_ISAAddr,
			MAL_EISAAddr,
			MAL_PCIAddr,
			MAL_MCAAddr,
			MAL_PCMCIAAddr,
			MAL_ProprietyAddr,
			MAL_NUBus,
			MAL_PC98C20Addr = 0xA0,
			MAL_PC98C24Addr,
			MAL_PC98EAddr,
			MAL_PC98LocalBusAddr
		};

		enum MemoryArrayUse
		{
			MAU_Other = 0x01,
			MAU_Unknown,
			MAU_SysMem,
			MAU_VideoMem,
			MAU_FlashMem,
			MAU_NonVolatileMem,
			MAU_CacheMem
		};
	};

	struct MemoryDeviceInfo
	{
		uint16_t physical_memory_array_handle;
		uint16_t mem_error_info_handle;
		uint16_t total_width;
		uint16_t data_width;
		uint16_t size;
		uint8_t form_factor;
		uint8_t device_set;
		char const * device_locator;
		char const * bank_locator;
		uint8_t memory_type;
		uint16_t type_detail;
		uint16_t speed;
		char const * manufacturer;
		char const * serial_num;
		char const * asset_tag;
		char const * part_number;
		uint8_t attributes;
		uint32_t extended_size;
		uint16_t configured_memory_clk_speed;
		uint16_t min_voltage;
		uint16_t max_voltage;
		uint16_t configured_voltage;

		enum FormFactorType
		{
			FFT_Other = 0x01,
			FFT_Unkown,
			FFT_SIMM,
			FFT_SIP,
			FFT_Chip,
			FFT_DIP,
			FFT_ZIP,
			FFT_ProCard,
			FFT_DIMM,
			FFT_TSOP,
			FFT_RowOfChips,
			FFT_RIMM,
			FFT_SODIMM,
			FFT_SRIMM,
			FFT_FBDIMM
		};

		enum MemoryType
		{
			MT_Other = 0x01,
			MT_Unknown,
			MT_DRAM,
			MT_EDRAM,
			MT_VRAM,
			MT_SRAM,
			MT_RAM,
			MT_ROM,
			MT_Flash,
			MT_EEPROM,
			MT_FEPROM,
			MT_EPROM,
			MT_CDRAM,
			MT_3DRAM,
			MT_SDRAM,
			MT_SGRAM,
			MT_RDRAM,
			MT_DDR1,
			MT_DDR2,
			MT_DDR2_FB_DIMM,
			MT_RESERVED1,
			MT_RESERVED2,
			MT_RESERVED3,
			MT_DDR3,
			MT_FBD2
		};

		enum TypeDetail
		{
			TD_Reserved = 0,
			TD_Other = 1UL << 1,
			TD_Unknown = 1UL << 2,
			TD_FastPaged = 1UL << 3,
			TD_StaticColumn = 1UL << 4,
			TD_PseudoStatic = 1UL << 5,
			TD_Rambus = 1UL << 6,
			TD_Synchronous = 1UL << 7,
			TD_CMOS = 1UL << 8,
			TD_EDO = 1UL << 9,
			TD_WindowDRAM = 1UL << 10,
			TD_CacheDRAM = 1UL << 11,
			TD_NonVolatile = 1UL << 12,
			TD_Registered = 1UL << 13,
			TD_Unbuffered = 1UL << 14,
			TD_LRDIMM = 1UL << 15
		};
	};


	class KLAYGE_CORE_API SMBios final : boost::noncopyable
	{
	public:
		static SMBios& Instance();

		bool Available() const
		{
			return smbios_data_ != nullptr;
		}

		uint8_t MajorVersion() const
		{
			return major_ver_;
		}
		uint8_t MinorVersion() const
		{
			return minor_ver_;
		}

		uint32_t TypeCount(uint8_t type) const;
		bool FindFirstTargetType(uint8_t type);
		bool FindNextTargetType();
		void FillDataField(void* dst, uint32_t size, uint32_t id);
		void FillStringField(char const *& dst, uint32_t id);

		template <typename T>
		void FillDataField(T& dst, uint32_t id)
		{
			this->FillDataField(&dst, sizeof(dst), id);
		}

	private:
		SMBios();

		bool ReadVersionAndData();
		void EnumEachTable();
		uint16_t GetTableRealSize(uint8_t const * data, uint8_t data_size);

	private:
		struct TableInfo
		{
			TableInfo()
			{
			}
			explicit TableInfo(uint8_t t)
				: type(t)
			{
			}

			uint8_t	type;
			uint8_t	data_size;
			uint16_t real_size;
			uint32_t offset;
		};

		uint8_t* smbios_data_;
		uint32_t smbios_size_;

		uint8_t major_ver_;
		uint8_t minor_ver_;

		std::vector<TableInfo> smbios_tables_;
		std::vector<TableInfo>::iterator smbios_index_iter_;
	};

	class KLAYGE_CORE_API Mainboard final : boost::noncopyable
	{
	public:
		Mainboard();

		char const * Manufacturer() const;
		char const * Product() const;
		char const * BiosVendor() const;
		char const * BiosVersion() const;
		char const * BiosReleaseDate() const;
		char const * Version() const;
		char const * BoardTypeName() const;

	private:
		bool ReadMainboard();
		bool ReadBios();
		void FillData(MainboardInfo& mb_info);
		void FillData(BiosInfo& bios_info);

	private:
		MainboardInfo mainboard_;
		BiosInfo bios_;
		static char const * mainboard_type_name_[MainboardInfo::MT_Num];
	};

	class KLAYGE_CORE_API MemoryBank final : boost::noncopyable
	{
	public:
		MemoryBank();

		size_t SlotCount() const
		{
			return devices_.size();
		}

		MemoryDeviceInfo& operator[](size_t index)
		{
			return devices_[index];
		}
		MemoryDeviceInfo const & operator[](size_t index) const
		{
			return devices_[index];
		}

	private:
		void DataFill(MemoryDeviceInfo& md_info);

	private:
		std::vector<MemoryDeviceInfo> devices_;
	};
}

#endif			// _KLAYGE_HWDETECT_HPP
