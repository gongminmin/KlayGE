// CPU.hpp
// KlayGE CPU检测 头文件
// Ver 1.3.8.1
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.enginedev.com
//
// 1.2.8.8
// 把inline放入类声明 (2002.9.28)
//
// 1.2.8.9
// 增加了GetSerialNumber，修改了GetVendor和GetVendorEx的返回类型 (2002.10.23)
// 修改了字符数组的大小 (2002.10.23)
//
// 1.2.8.10
// 用string代替字符串指针 (2002.10.27)
//
// 1.3.8.1
// 增加了对超线程的识别 (2002.12.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _CPU_HPP
#define _CPU_HPP

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	// 以下信息来自Intel和AMD

	enum CpuIDStd
	{
		CPUID_STD_FPU			= 1UL << 0,		// 浮点处理器 0
		CPUID_STD_VME			= 1UL << 1,		// 虚拟机模式 1
		CPUID_STD_DE			= 1UL << 2,		// 调试器 2
		CPUID_STD_PSE			= 1UL << 3,		// 页大小扩展 3
		CPUID_STD_TSC			= 1UL << 4,		// 时钟周期计数器 4
		CPUID_STD_MSR			= 1UL << 5,		// RDMSR / WRMSR 指令 5
		CPUID_STD_PAE			= 1UL << 6,		// 物理地址扩展 6
		CPUID_STD_MCE			= 1UL << 7,		// Machine Check Exception 7
		CPUID_STD_CX8			= 1UL << 8,		// CMPXCHG8B 指令 8
		CPUID_STD_APIC			= 1UL << 9,		// 内含 APIC 硬件 9
		CPUID_STD_SEP			= 1UL << 11,	// SYSENTER / SYSEXIT 指令 11
		CPUID_STD_MTRR			= 1UL << 12,	// Memory Type Range Registers 12
		CPUID_STD_PGF			= 1UL << 13,	// Page Global Enable 13
		CPUID_STD_MCA			= 1UL << 14,	// Machine Check Architecture 14
		CPUID_STD_CMOV			= 1UL << 15,	// 条件移动指令 CMOV 15
		CPUID_STD_PAT			= 1UL << 16,	// 页属性表 16
		CPUID_STD_PSE36			= 1UL << 17,	// 36位页大小扩展 17
		CPUID_STD_PSN			= 1UL << 18,	// 序列号 18
		CPUID_STD_CLFSH			= 1UL << 19,	// CLFLUSH 指令 19
		CPUID_STD_DTS			= 1UL << 21,	// Debug Trace Store 21
		CPUID_STD_ACPI			= 1UL << 22,	// ACPI 支持 22
		CPUID_STD_MMX			= 1UL << 23,	// MMX 指令集 23
		CPUID_STD_FXSR			= 1UL << 24,	// 快速浮点保存恢复 FXSAVE / FXRSTOR 24
		CPUID_STD_SSE			= 1UL << 25,	// Streaming SIMD Extention 25
		CPUID_STD_SSE2			= 1UL << 26,	// Streaming SIMD Extention2 26
		CPUID_STD_SSP			= 1UL << 27,	// Self-Snoop 27
		CPUID_STD_HTT			= 1UL << 28,	// 超线程技术 28
		CPUID_STD_TM			= 1UL << 29,	// Thermal Monitor Support 29
		CPUID_STD_IA64			= 1UL << 30,	// IA-64 30
		CPUID_STD_SBF			= 1UL << 31,	// Signal Break on FERR
	};

	enum CpuIDExt
	{
		CPUID_EXT_FPU			= 1UL << 0,		// 浮点处理器 0
		CPUID_EXT_VME			= 1UL << 1,		// 虚拟机模式 1
		CPUID_EXT_DE			= 1UL << 2,		// 调试器 2
		CPUID_EXT_PSE			= 1UL << 3,		// 页大小扩展 3
		CPUID_EXT_TSC			= 1UL << 4,		// 时钟周期计数器 4
		CPUID_EXT_MSR			= 1UL << 5,		// RDMSR / WRMSR 指令 5
		CPUID_EXT_PAE			= 1UL << 6,		// 物理地址扩展 6
		CPUID_EXT_MCE			= 1UL << 7,		// Machine Check Exception 7
		CPUID_EXT_CX8			= 1UL << 8,		// CMPXCHG8B 指令 8
		CPUID_EXT_APIC			= 1UL << 9,		// 内含 APIC 硬件 9
		CPUID_EXT_SEP			= 1UL << 11,	// SYSENTER / SYSEXIT 指令 11
		CPUID_EXT_MTRR			= 1UL << 12,	// Memory Type Range Registers 12
		CPUID_EXT_PGF			= 1UL << 13,	// Page Global Enable 13
		CPUID_EXT_MCA			= 1UL << 14,	// Machine Check Architecture 14
		CPUID_EXT_CMOV			= 1UL << 15,	// 条件移动指令 CMOV 15
		CPUID_EXT_PAT			= 1UL << 16,	// 页属性表 16
		CPUID_EXT_PSE36			= 1UL << 17,	// 36位页大小扩展 17
		CPUID_EXT_MP			= 1UL << 19,	// 多处理器支持 19
		CPUID_EXT_MMXEX			= 1UL << 22,	// MMX 扩展 22
		CPUID_EXT_MMX			= 1UL << 23,	// MMX 指令集 23
		CPUID_EXT_FXSR			= 1UL << 24,	// 快速浮点保存恢复指令 FXSAVE / FXRSTOR 24
		CPUID_EXT_LM			= 1UL << 29,	// x86-64 长模式 29
		CPUID_EXT_3DNOWEX		= 1UL << 30,	// 3DNow! 扩展 30
		CPUID_EXT_3DNOW			= 1UL << 31,	// 3DNow! 指令集 31
	};

	class CPUInfo
	{
	public:
		void CheckCpu();
		U32 Frequency() const;

		const char* Vendor() const
			{ return this->vendor_; }
		U32 CpuID() const
			{ return this->cpuID_; }
		U32 Type() const
			{ return (this->CpuID() >> 12) & 0xF; }
		U32 Family() const
			{ return (this->CpuID() >> 8) & 0xF; }
		U32 Model() const
			{ return (this->CpuID() >> 4) & 0xF; }
		U32 Stepping() const
			{ return this->CpuID() & 0xF; }

		const char* VendorEx() const
			{ return this->vendorEx_; }
		U32 CpuIDEx() const
			{ return this->cpuIDEx_; }
		U32 TypeEx() const
			{ return (this->CpuIDEx() >> 12) & 0xF; }
		U32 FamilyEx() const
			{ return (this->CpuIDEx() >> 8) & 0xF; }
		U32 ModelEx() const
			{ return (this->CpuIDEx() >> 4) & 0xF; }
		U32 SteppingEx() const
			{ return this->CpuIDEx() & 0xF; }

		const char* SerialNumber() const
			{ return this->serialNumber_; }

		U32 Feature() const
			{ return this->feature_; }
		U32 FeatureEx() const
			{ return this->featureEx_; }

		bool IsMMXSupport() const
			{ return (this->Feature() & CPUID_STD_MMX) != 0; }
		bool IsSSESupport() const
			{ return (this->Feature() & CPUID_STD_SSE) != 0; }
		bool IsSSE2Support() const
			{ return (this->Feature() & CPUID_STD_SSE2) != 0; }
		bool IsMMXEXSupport() const
			{ return (this->FeatureEx() & CPUID_EXT_MMXEX) != 0; }
		bool Is3DNowSupport() const
			{ return (this->FeatureEx() & CPUID_EXT_3DNOW) != 0; }
		bool Is3DNowEXSupport() const
			{ return (this->FeatureEx() & CPUID_EXT_3DNOWEX) != 0; }

		CPUInfo();

	private:
		char vendor_[17];
		char vendorEx_[49];
		char serialNumber_[14];

		U32 level_;
		U32 levelEx_;
		U32 cpuID_;
		U32 cpuIDEx_;
		U32 feature_;
		U32 featureEx_;

		bool extendedLevels_;
	};
}

#endif		// _CPU_HPP

