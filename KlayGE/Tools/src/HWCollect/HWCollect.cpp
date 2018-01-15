/**
 * @file HWCollect.cpp
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
#include <KFL/Util.hpp>
#include <KFL/CpuInfo.hpp>
#include <KlayGE/HWDetect.hpp>

#include <KlayGE/SALWrapper.hpp>
#if defined KLAYGE_PLATFORM_WINDOWS
	#include <windows.h>
	#if defined(KLAYGE_COMPILER_CLANGC2)
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wmicrosoft-enum-value" // Ignore int enum
	#endif
	#include <dxgi1_2.h>
	#if defined(KLAYGE_COMPILER_CLANGC2)
		#pragma clang diagnostic pop
	#endif
#endif

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;
using namespace KlayGE;

void DetectOSInfo(std::ostream& os)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	SYSTEM_INFO si;

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	typedef NTSTATUS (WINAPI *RtlGetVersionFunc)(OSVERSIONINFOEXW* pVersionInformation);
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	RtlGetVersionFunc RtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(::GetProcAddress(ntdll, "RtlGetVersion"));
	OSVERSIONINFOEXW os_ver_info;
	os_ver_info.dwOSVersionInfoSize = sizeof(os_ver_info);
	RtlGetVersion(&os_ver_info);

	::GetSystemInfo(&si);

	int server_r2 = ::GetSystemMetrics(SM_SERVERR2);

	os << "Windows ";

	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724833%28v=vs.85%29.aspx
	switch (os_ver_info.dwMajorVersion)
	{
	case 5:
		switch (os_ver_info.dwMinorVersion)
		{
		case 0:
			os << "2000";
			break;

		case 1:
			os << "XP";
			break;

		case 2:
			if ((VER_NT_WORKSTATION == os_ver_info.wProductType) && (PROCESSOR_ARCHITECTURE_AMD64 == si.wProcessorArchitecture))
			{
				os << "XP Professional x64 Edition";
			}
			else if (os_ver_info.wSuiteMask & VER_SUITE_WH_SERVER)
			{
				os << "Home Server";
			}
			else if (0 == server_r2)
			{
				os << "Server 2003";
			}
			else
			{
				os << "Server 2003 R2";
			}
			break;

		default:
			os << "(unknown edition)";
			break;
		}
		break;

	case 6:
		switch (os_ver_info.dwMinorVersion)
		{
		case 0:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				os << "Vista";
			}
			else
			{
				os << "Server 2008";
			}
			break;

		case 1:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				os << "7";
			}
			else
			{
				os << "Server 2008 R2";
			}
			break;

		case 2:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				os << "8";
			}
			else
			{
				os << "Server 2012";
			}
			break;

		case 3:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				os << "8.1";
			}
			else
			{
				os << "Server 2012 R2";
			}
			break;

		default:
			os << "(unknown edition)";
			break;
		}
		break;

	case 10:
		switch (os_ver_info.dwMinorVersion)
		{
		case 0:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				os << "10";
			}
			else
			{
				os << "Server 2016";
			}
			break;

		default:
			os << "(unknown edition)";
			break;
		}
		break;

	default:
		os << "Unknown";
		break;
	}

	os << ' ' << os_ver_info.dwMajorVersion << '.' << os_ver_info.dwMinorVersion
		<< " (" << os_ver_info.dwBuildNumber << ')';
	if ((os_ver_info.wServicePackMajor != 0) || (os_ver_info.wServicePackMinor != 0))
	{
		os << "SP " << os_ver_info.wServicePackMajor << '.' << os_ver_info.wServicePackMinor;
	}
#else
	os << "Windows Store";

	::GetNativeSystemInfo(&si);
#endif

	os << ' ';
	switch (si.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_INTEL:
		os << "x86";
		break;
	case PROCESSOR_ARCHITECTURE_ARM:
		os << "arm";
		break;
	case PROCESSOR_ARCHITECTURE_IA64:
		os << "ia64";
		break;
	case PROCESSOR_ARCHITECTURE_AMD64:
		os << "x64";
		break;
#ifdef PROCESSOR_ARCHITECTURE_ARM64
	case PROCESSOR_ARCHITECTURE_ARM64:
		os << "arm64";
		break;
#endif

	default:
		os << "Unknown architecture";
		break;
	}
#elif defined KLAYGE_PLATFORM_DARWIN
	os << "Darwin";
#elif defined KLAYGE_PLATFORM_LINUX
	os << "Linux";
#elif defined KLAYGE_PLATFORM_ANDROID
	os << "Android";
#elif defined KLAYGE_PLATFORM_IOS
	os << "iOS";
#endif

	os << endl;
}

void DetectCpuInfo(std::ostream& os)
{
	CPUInfo cpu_info;
	if ("GenuineIntel" == cpu_info.CPUString())
	{
		os << "Intel CPU";
	}
	else if ("AuthenticAMD" == cpu_info.CPUString())
	{
		os << "AMD CPU";
	}
	else
	{
		os << "Unknown CPU";
	}
	os << endl;
	os << "Brand: " << cpu_info.CPUBrandString() << endl;
	os << "Cores: " << cpu_info.NumCores() << endl;
	os << "Threads: " << cpu_info.NumHWThreads() << endl;
}

void DetectGpuInfo(std::ostream& os)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	typedef HRESULT (WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	HMODULE dxgi = ::LoadLibraryEx(L"dxgi.dll", nullptr, 0);
	if (!dxgi)
	{
		os << "Unknown GPU";
		return;
	}
	CreateDXGIFactory1Func DynamicCreateDXGIFactory1 = (CreateDXGIFactory1Func)::GetProcAddress(dxgi, "CreateDXGIFactory1");
	if (!DynamicCreateDXGIFactory1)
	{
		os << "Unknown GPU";
		return;
	}
#else
	CreateDXGIFactory1Func DynamicCreateDXGIFactory1 = CreateDXGIFactory1;
#endif

	IDXGIFactory1* factory;
	if (SUCCEEDED((*DynamicCreateDXGIFactory1)(IID_IDXGIFactory1, reinterpret_cast<void**>(&factory))))
	{
		UINT adapter_no = 0;
		IDXGIAdapter1* adapter = nullptr;
		while (factory->EnumAdapters1(adapter_no, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			if (adapter != nullptr)
			{
				DXGI_ADAPTER_DESC1 adapter_desc;
				adapter->GetDesc1(&adapter_desc);

				IDXGIAdapter2* adapter2;
				adapter->QueryInterface(IID_IDXGIAdapter2, reinterpret_cast<void**>(&adapter2));
				if (adapter2 != nullptr)
				{
					DXGI_ADAPTER_DESC2 desc2;
					adapter2->GetDesc2(&desc2);
					memcpy(adapter_desc.Description, desc2.Description, sizeof(desc2.Description));
					adapter_desc.VendorId = desc2.VendorId;
					adapter_desc.DeviceId = desc2.DeviceId;
					adapter_desc.SubSysId = desc2.SubSysId;
					adapter_desc.Revision = desc2.Revision;
					adapter_desc.DedicatedVideoMemory = desc2.DedicatedVideoMemory;
					adapter_desc.DedicatedSystemMemory = desc2.DedicatedSystemMemory;
					adapter_desc.SharedSystemMemory = desc2.SharedSystemMemory;
					adapter_desc.AdapterLuid = desc2.AdapterLuid;
					adapter_desc.Flags = desc2.Flags;
					adapter2->Release();
				}

				adapter->Release();

				if (adapter_desc.Flags != DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					os << "Adapter " << adapter_no << endl << endl;
					std::string description;
					Convert(description, adapter_desc.Description);
					os << "Brand: " << description << endl;
					os << "Vendor ID: " << std::hex << std::uppercase << adapter_desc.VendorId << endl;
					os << "Device ID: " << std::hex << std::uppercase << adapter_desc.DeviceId << endl;
					os << "Revision: " << std::hex << std::uppercase << adapter_desc.Revision << endl;
					os << "Dedicated video memory: " << std::dec << adapter_desc.DedicatedVideoMemory / 1024 / 1024 << " MB" << endl;
					os << "Dedicated system memory: " << std::dec << adapter_desc.DedicatedSystemMemory / 1024 / 1024 << " MB" << endl;
					os << "Shared system memory: " << std::dec << adapter_desc.SharedSystemMemory / 1024 / 1024 << " MB" << endl;
				}
			}

			++ adapter_no;
		}
	}

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	::FreeLibrary(dxgi);
#endif
#else
	os << "Unknown GPU" << endl;
#endif
}

void DetectMainboardInfo(std::ostream& os)
{
	if (SMBios::Instance().Available())
	{
		os << "SMBios Ver: " << static_cast<int>(SMBios::Instance().MajorVersion())
			<< '.' << static_cast<int>(SMBios::Instance().MinorVersion()) << endl;
		os << endl;

		Mainboard mainboard;
		os << "Mainboard: " << mainboard.Manufacturer() << ' ' << mainboard.Product() << endl;
		os << "Bios: " << mainboard.BiosVendor() << ' ' << mainboard.BiosVersion() << endl;
		os << "Release Date: " << mainboard.BiosReleaseDate() << endl;
		os << endl;

		MemoryBank mem;
		for (size_t i = 0; i < mem.SlotCount(); ++ i)
		{
			if (mem[i].size != 0)
			{
				os << "Memory slot " << i << ": ";
				os << mem[i].manufacturer << ' ' << mem[i].size << " MB" << endl;
				os << "Part #: " << mem[i].part_number << endl;
				os << "Serial #: " << mem[i].serial_num << endl;
				os << endl;
			}
		}
	}
	else
	{
		os << "Could not read from SMBios." << endl;
	}
}

int main()
{
	std::stringstream ss;

	ss << "=== OS information ===" << endl;
	DetectOSInfo(ss);
	ss << endl;

	ss << "=== CPU information ===" << endl;
	DetectCpuInfo(ss);
	ss << endl;

	ss << "=== GPU information ===" << endl;
	DetectGpuInfo(ss);
	ss << endl;

	ss << "=== Mainboard information ===" << endl;
	DetectMainboardInfo(ss);
	ss << endl;

	cout << ss.str();

	cout << "Do you like to save this informatin to a file? (y/N) ";
	int ch = cin.get();
	if (('y' == ch) || ('Y' == ch))
	{
		ofstream ofs("HWCollect.txt");
		ofs << ss.str();
	}

	cout << endl;

	return 0;
}

