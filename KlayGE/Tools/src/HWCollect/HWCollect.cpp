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
#include <KlayGE/HWDetect.hpp>

#include <KFL/CpuInfo.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS
	#include <windows.h>
#endif

#include <iostream>

using namespace std;
using namespace KlayGE;

void DetectOSInfo()
{
#if defined KLAYGE_PLATFORM_WINDOWS
	SYSTEM_INFO si;

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	typedef NTSTATUS(WINAPI *RtlGetVersionFunc)(OSVERSIONINFOEXW* pVersionInformation);
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	RtlGetVersionFunc RtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(GetProcAddress(ntdll, "RtlGetVersion"));
	OSVERSIONINFOEXW os_ver_info;
	os_ver_info.dwOSVersionInfoSize = sizeof(os_ver_info);
	RtlGetVersion(&os_ver_info);

	GetSystemInfo(&si);

	int server_r2 = GetSystemMetrics(SM_SERVERR2);

	cout << "Windows ";

	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724833%28v=vs.85%29.aspx
	switch (os_ver_info.dwMajorVersion)
	{
	case 5:
		switch (os_ver_info.dwMinorVersion)
		{
		case 0:
			cout << "2000";
			break;

		case 1:
			cout << "XP";
			break;

		case 2:
			if ((VER_NT_WORKSTATION == os_ver_info.wProductType) && (PROCESSOR_ARCHITECTURE_AMD64 == si.wProcessorArchitecture))
			{
				cout << "XP Professional x64 Edition";
			}
			else if (os_ver_info.wSuiteMask & VER_SUITE_WH_SERVER)
			{
				cout << "Home Server";
			}
			else if (0 == server_r2)
			{
				cout << "Server 2003";
			}
			else
			{
				cout << "Server 2003 R2";
			}
			break;

		default:
			cout << "(unknown edition)";
			break;
		}
		break;

	case 6:
		switch (os_ver_info.dwMinorVersion)
		{
		case 0:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				cout << "Vista";
			}
			else
			{
				cout << "Server 2008";
			}
			break;

		case 1:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				cout << "7";
			}
			else
			{
				cout << "Server 2008 R2";
			}
			break;

		case 2:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				cout << "8";
			}
			else
			{
				cout << "Server 2012";
			}
			break;

		case 3:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				cout << "8.1";
			}
			else
			{
				cout << "Server 2012 R2";
			}
			break;

		default:
			cout << "(unknown edition)";
			break;
		}
		break;

	case 10:
		switch (os_ver_info.dwMinorVersion)
		{
		case 0:
			if (VER_NT_WORKSTATION == os_ver_info.wProductType)
			{
				cout << "10";
			}
			else
			{
				cout << "Server 2016";
			}
			break;

		default:
			cout << "(unknown edition)";
			break;
		}
		break;

	default:
		cout << "Unknown";
		break;
	}

	cout << ' ' << os_ver_info.dwMajorVersion << '.' << os_ver_info.dwMinorVersion
		<< " (" << os_ver_info.dwBuildNumber << ')';
	if ((os_ver_info.wServicePackMajor != 0) || (os_ver_info.wServicePackMinor != 0))
	{
		cout << "SP " << os_ver_info.wServicePackMajor << '.' << os_ver_info.wServicePackMinor;
	}
#else
#if defined KLAYGE_PLATFORM_WINDOWS_UWP
	cout << "Windows UWP";
#elif defined KLAYGE_PLATFORM_WINDOWS_STORE
	cout << "Windows Store";
#elif defined KLAYGE_PLATFORM_WINDOWS_PHONE
	cout << "Windows Phone";
#endif

	::GetNativeSystemInfo(&si);
#endif

	cout << ' ';
	switch (si.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_INTEL:
		cout << "x86";
		break;
	case PROCESSOR_ARCHITECTURE_ARM:
		cout << "arm";
		break;
	case PROCESSOR_ARCHITECTURE_IA64:
		cout << "ia64";
		break;
	case PROCESSOR_ARCHITECTURE_AMD64:
		cout << "x64";
		break;
	case PROCESSOR_ARCHITECTURE_ARM64:
		cout << "arm64";
		break;

	default:
		cout << "Unknown architecture";
		break;
	}

	cout << endl;
#elif defined KLAYGE_PLATFORM_DARWIN
	cout << "Darwin" << endl;
#elif defined KLAYGE_PLATFORM_LINUX
	cout << "Linux" << endl;
#elif defined KLAYGE_PLATFORM_ANDROID
	cout << "Android" << endl;
#elif defined KLAYGE_PLATFORM_IOS
	cout << "iOS" << endl;
#endif
}

void DetectCpuInfo()
{
	CPUInfo cpu_info;
	if ("GenuineIntel" == cpu_info.CPUString())
	{
		cout << "Intel CPU";
	}
	else if ("AuthenticAMD" == cpu_info.CPUString())
	{
		cout << "AMD CPU";
	}
	else
	{
		cout << "Unknown CPU";
	}
	cout << endl;
	cout << "Brand: " << cpu_info.CPUBrandString() << endl;
	cout << "Cores: " << cpu_info.NumCores() << endl;
	cout << "Threads: " << cpu_info.NumHWThreads() << endl;
}

void DetectMainboardInfo()
{
	if (SMBios::Instance().Available())
	{
		cout << "SMBios Ver: " << static_cast<int>(SMBios::Instance().MajorVersion())
			<< '.' << static_cast<int>(SMBios::Instance().MinorVersion()) << endl;
		cout << endl;

		Mainboard mainboard;
		cout << "Mainboard: " << mainboard.Manufacturer() << ' ' << mainboard.Product() << endl;
		cout << "Bios: " << mainboard.BiosVendor() << ' ' << mainboard.BiosVersion() << endl;
		cout << "Release Date: " << mainboard.BiosReleaseDate() << endl;
		cout << endl;

		MemoryBank mem;
		for (size_t i = 0; i < mem.SlotCount(); ++i)
		{
			if (mem[i].size != 0)
			{
				cout << "Memory slot " << i << ": ";
				cout << mem[i].manufacturer << ' ' << mem[i].size << " MB" << endl;
				cout << "Part #: " << mem[i].part_number << endl;
				cout << "Serial #: " << mem[i].serial_num << endl;
				cout << endl;
			}
		}
	}
	else
	{
		cout << "Could not read from SMBios." << endl;
	}
}

int main()
{
	cout << "=== OS information ===" << endl;
	DetectOSInfo();
	cout << endl;

	cout << "=== CPU information ===" << endl;
	DetectCpuInfo();
	cout << endl;

	cout << "=== Mainboard information ===" << endl;
	DetectMainboardInfo();
	cout << endl;

	return 0;
}

