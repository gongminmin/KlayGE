// CPU.cpp
// KlayGE CPU检测 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.9.28)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/CommFuncs.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/Engine.hpp>

#include <sstream>

#include <KlayGE/Cpu.hpp>

namespace
{
	using namespace KlayGE;

	// 获取CPUID信息
	/////////////////////////////////////////////////////////////////////////////////
	void CpuidInfo(U32 idLevels, U32& regEAX, U32& regEBX, U32& regECX, U32& regEDX)
	{
		U32 a, b, c, d;

		_asm
		{
			mov eax, idLevels
			cpuid
			mov a, eax
			mov b, ebx
			mov c, ecx
			mov d, edx
		}

		regEAX = a;
		regEBX = b;
		regECX = c;
		regEDX = d;
	}

	// 检查CPUID指令是否可用
	/////////////////////////////////////////////////////////////////////////////////
	bool ChkCpuidEnable()
	{
		_asm
		{
			pushfd							// 获取原来的EFLAGS
			pop		ebx
			mov		ecx, ebx
			xor		ebx, 0x200000			// 翻转 EFLAGS 里的 ID 位
			push	ebx						// 保存新的 EFLAGS

			popfd							// 更新现在的 EFLAGS 值
			pushfd							// 获取新的 EFLAGS
			pop		ebx						// 把新的 EFLAGS 存入 ebx
			xor		ebx, ecx				// 比较 ID 位
			jnz		SUPPORT

			xor		eax, eax				// 返回 false

SUPPORT:
			mov		eax, 1
		}
	}
}

namespace KlayGE
{
	// 构造函数
	CPUInfo::CPUInfo()
	{
		memset(vendor_, 0, sizeof(vendor_));
		memset(vendorEx_, 0, sizeof(vendorEx_));
		memset(serialNumber_, 0, sizeof(serialNumber_));

		level_ = levelEx_ = 0;
		cpuID_ = cpuIDEx_ = 0;
		feature_ = featureEx_ = 0;
		extendedLevels_ = false;
	}

	// 检查CPU
	/////////////////////////////////////////////////////////////////////////////////
	void CPUInfo::CheckCpu() 
	{
		U32 regEAX, regEBX, regECX, regEDX;
		U32 vendorTemp[4];

		if (ChkCpuidEnable())
		{
			CpuidInfo(0x80000000, regEAX, regEBX, regECX, regEDX);

			if (regEAX & 0x80000000)
			{
				// 支持扩展属性
				extendedLevels_ = true;

				levelEx_ = regEAX;

				CpuidInfo(0x80000001, regEAX, regEBX, regECX, regEDX);
				cpuIDEx_		= regEAX;
				featureEx_	= regEDX;

				// 取生产商名扩展字符串
				CpuidInfo(0x80000002, regEAX, regEBX, regECX, regEDX);
				vendorTemp[0] = regEAX;
				vendorTemp[1] = regEBX;
				vendorTemp[2] = regECX;
				vendorTemp[3] = regEDX;
				memcpy(vendorEx_, vendorTemp, 16);

				CpuidInfo(0x80000003, regEAX, regEBX, regECX, regEDX);
				vendorTemp[0] = regEAX;
				vendorTemp[1] = regEBX;
				vendorTemp[2] = regECX;
				vendorTemp[3] = regEDX;
				memcpy(&vendorEx_[16], vendorTemp, 16);

				CpuidInfo(0x80000004, regEAX, regEBX, regECX, regEDX);
				vendorTemp[0] = regEAX;
				vendorTemp[1] = regEBX;
				vendorTemp[2] = regECX;
				vendorTemp[3] = regEDX;
				memcpy(&vendorEx_[32], vendorTemp, 16);
			}

			CpuidInfo(0, regEAX, regEBX, regECX, regEDX);
			level_ = regEAX;

			// 取生产商名字符串
			vendorTemp[0] = regEBX;
			vendorTemp[1] = regEDX;
			vendorTemp[2] = regECX;
			memcpy(vendor_, vendorTemp, 12);

			CpuidInfo(1, regEAX, regEBX, regECX, regEDX);
			cpuID_		= regEAX;
			feature_	= regEDX;

			if (feature_ & CPUID_STD_PSN)
			{
				std::basic_ostringstream<char, std::char_traits<char>, alloc<char> > sstream;

				// 获取PIII以上支持的序列号
				CpuidInfo(3, regEAX, regEBX, regECX, regEDX);
				sstream.width(4);
				sstream << std::ios_base::hex << cpuID_ << regEDX << regECX;

				sstream.str().copy(serialNumber_, sizeof(serialNumber_));
			}
		}
	}

	// 计算CPU频率
	/////////////////////////////////////////////////////////////////////////////////
	U32 CPUInfo::Frequency() const
	{
		if (this->Feature() & CPUID_STD_TSC)
		{
			U32 regEAX, regEDX;

			const float lastTime(Timer::Instance().AppTime());
			_asm
			{
				rdtsc					// 获取系统启动到现在的时钟周期
				mov		regEAX, eax		// 低32位保存在eax中
				mov		regEDX, edx		// 高32位保存在edx中
			}
			Sleep(50);
			_asm
			{
				rdtsc
				sub		eax, regEAX
				sbb		edx, regEDX
				mov		regEAX, eax
				mov		regEDX, edx
			}

			const float elapsedTime(Timer::Instance().AppTime() - lastTime);
			return static_cast<U32>(static_cast<float>((regEDX << 32) + regEAX) / elapsedTime);
		}

		return 0;
	}
}
