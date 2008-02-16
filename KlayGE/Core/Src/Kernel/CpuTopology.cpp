// CpuTopology.cpp
// KlayGE CPU检测类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2008.2.16)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <vector>
#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>

#include <KlayGE/CpuTopology.hpp>

namespace
{
	using namespace KlayGE;


	class ICpuTopology
	{
	public:
		virtual ~ICpuTopology()
		{
		}

		virtual int NumCores() const = 0;
	};


	class DefaultImpl : public ICpuTopology
	{
	public:
		int NumCores() const
		{
			return 1;
		}
	};


#ifdef KLAYGE_PLATFORM_WINDOWS
	class GlpiImpl : public ICpuTopology
	{
	public:
		GlpiImpl()
		{
			BOOST_ASSERT(this->IsSupported());

			GetLogicalProcessorInformationPtr glpi = GetGlpiFn();
			BOOST_ASSERT(glpi);

			uint32_t cbBuffer = 0;
			glpi(NULL, &cbBuffer);

			slpi_.resize(cbBuffer / sizeof(slpi_[0]));
			glpi(&slpi_[0], &cbBuffer);
		}

		int NumCores() const
		{
			int cores = 0;
			for (size_t i = 0; i < slpi_.size(); ++ i)
			{
				if (::RelationProcessorCore == slpi_[i].Relationship)
				{
					++ cores;
				}
			}
			return cores;
		}

		static bool IsSupported()
		{
			return NULL != GetGlpiFn();
		}

	private:
		typedef BOOL (WINAPI* GetLogicalProcessorInformationPtr)(SYSTEM_LOGICAL_PROCESSOR_INFORMATION*, PDWORD);

		static GetLogicalProcessorInformationPtr GetGlpiFn()
		{
			OSVERSIONINFO os_ver_info;
			::GetVersionEx(&os_ver_info);

			// There is a bug with the implementation of GetLogicalProcessorInformation
			// on Windows Server 2003 and XP64. Therefore, only
			// GetLogicalProcessorInformation on Windows Vista is supported for now.
			if (os_ver_info.dwMajorVersion >= 6)
			{
				GetLogicalProcessorInformationPtr glpi
					= (GetLogicalProcessorInformationPtr)::GetProcAddress(::GetModuleHandle(TEXT("kernel32")),
						"GetLogicalProcessorInformation");
				return glpi;
			}
			else
			{
				return NULL;
			}
		}

		std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> slpi_;
	};


#ifdef KLAYGE_PLATFORM_WIN32
	class ApicExtractor
	{
	public:
		ApicExtractor(uint8_t log_procs_per_pkg = 1, uint8_t cores_per_pkg = 1)
		{
			this->SetPackageTopology(log_procs_per_pkg, cores_per_pkg);
		}

		uint8_t SmtId(uint8_t apic_id) const
		{
			return apic_id & smt_id_mask_.mask;
		}

		uint8_t CoreId(uint8_t apic_id) const
		{
			return (apic_id & core_id_mask_.mask) >> smt_id_mask_.width;
		}

		uint8_t PackageId(uint8_t apic_id) const
		{
			return (apic_id & pkg_id_mask_.mask) >> (smt_id_mask_.width + core_id_mask_.width);
		}

		uint8_t PackageCoreId(uint8_t apic_id) const
		{
			return (apic_id & (pkg_id_mask_.mask | core_id_mask_.mask)) >> smt_id_mask_.width;
		}

		uint8_t LogProcsPerPkg() const
		{
			return log_procs_per_pkg_;
		}

		uint8_t CoresPerPkg() const
		{
			return cores_per_pkg_;
		}

		void SetPackageTopology(uint8_t log_procs_per_pkg, uint8_t cores_per_pkg)
		{
			log_procs_per_pkg_	= log_procs_per_pkg;
			cores_per_pkg_		= cores_per_pkg;

			smt_id_mask_.width	= this->GetMaskWidth(log_procs_per_pkg_ / cores_per_pkg_);
			core_id_mask_.width	= this->GetMaskWidth(cores_per_pkg_);
			pkg_id_mask_.width	= 8 - (smt_id_mask_.width + core_id_mask_.width);

			pkg_id_mask_.mask	= static_cast<uint8_t>(0xFF << (smt_id_mask_.width + core_id_mask_.width));
			core_id_mask_.mask	= static_cast<uint8_t>((0xFF << smt_id_mask_.width) ^ pkg_id_mask_.mask);
			smt_id_mask_.mask	= static_cast<uint8_t>(~(0xFF << smt_id_mask_.width));
		}

	private:
		static uint8_t GetMaskWidth(uint8_t max_ids)
		{
			-- max_ids;

			// find index of msb
			uint8_t msb_idx = 8;
			uint8_t msb_mask = 0x80;
			while (msb_mask && !(msb_mask & max_ids))
			{
				-- msb_idx;
				msb_mask >>= 1;
			}
			return msb_idx;
		}

	private:
		struct id_mask_t
		{
			uint8_t width;
			uint8_t mask;
		};

		uint8_t		log_procs_per_pkg_;
		uint8_t		cores_per_pkg_;
		id_mask_t	smt_id_mask_;
		id_mask_t	core_id_mask_;
		id_mask_t	pkg_id_mask_;
	};

	class Cpuid
	{
	public:
		enum FnSet
		{
			FS_Std = 0x00000000,
			FS_Ext = 0x80000000
		};

	public:
		Cpuid()
			: eax_(0), ebx_(0), ecx_(0), edx_(0)
		{
		}

		uint32_t Eax() const
		{
			return eax_;
		}
		uint32_t Ebx() const
		{
			return ebx_;
		}
		uint32_t Ecx() const
		{
			return ecx_;
		}
		uint32_t Edx() const
		{
			return edx_;
		}

		bool Call(FnSet fn_set, uint32_t fn)
		{
			if (this->IsFnSupported(fn_set, fn))
			{
				this->UncheckedCall(fn_set, fn);
				return true;
			}
			return false;
		}

		static bool IsVendor(char const * vendor)
		{
			Cpuid const cpu(FS_Std);  
			return (cpu.Ebx() == *reinterpret_cast<uint32_t const *>(vendor))
				&& (cpu.Ecx() == *reinterpret_cast<uint32_t const *>(vendor + 8))
				&& (cpu.Edx() == *reinterpret_cast<uint32_t const *>(vendor + 4));
		}

		static bool IsFnSupported(FnSet fn_set, uint32_t fn)
		{
			uint32_t const max_std_fn = Cpuid(FS_Std).Eax();
			uint32_t const max_ext_fn = Cpuid(FS_Ext).Eax();

			bool ret = false;
			switch (fn_set)
			{
			case FS_Std:
				ret = (fn <= max_std_fn);
				break;

			case FS_Ext:
				ret = (fn <= max_ext_fn);
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			return ret;
		}

	private:
		explicit Cpuid(FnSet fn_set)
		{
			this->UncheckedCall(fn_set, 0);
		}

		void UncheckedCall(FnSet fn_set, uint32_t fn)
		{
			__asm
			{
				mov ecx, 0
				mov eax, fn
				or  eax, fn_set
				cpuid
				mov edi, this
				mov [edi].eax_, eax
				mov [edi].ebx_, ebx
				mov [edi].ecx_, ecx
				mov [edi].edx_, edx
			}
		}

	private:
		uint32_t eax_;
		uint32_t ebx_;
		uint32_t ecx_;
		uint32_t edx_;
	};

	class CpuidImpl : public ICpuTopology
	{
	public:
		// CpuidFnMasks are used when extracting bit-encoded information retrieved from
		// the CPUID instruction
		enum CpuidFnMasks
		{
			CFM_HTT                     = 0x10000000,   // Fn0000_0001  EDX[28]
			CFM_LogicalProcessorCount   = 0x00FF0000,   // Fn0000_0001  EBX[23:16]
			CFM_ApicId                  = 0xFF000000,   // Fn0000_0001  EBX[31:24]
			CFM_NC_Intel                = 0xFC000000,   // Fn0000_0004  EAX[31:26]
			CFM_NC_Amd                  = 0x000000FF,   // Fn8000_0008  ECX[7:0]
			CFM_CmpLegacy_Amd           = 0x00000002,   // Fn8000_0001  ECX[1]
			CFM_ApicIdCoreIdSize_Amd    = 0x0000F000    // Fn8000_0008  ECX[15:12]
		};

	public:
		CpuidImpl()
		{
			BOOST_ASSERT(this->IsSupported());

			uint8_t log_procs_per_pkg = 1;
			uint8_t cores_per_pkg = 1;

			Cpuid cpu;

			// Determine if hyper-threading is enabled.
			cpu.Call(Cpuid::FS_Std, 1);
			if (cpu.Edx() & CFM_HTT)
			{
				// Determine the total number of logical processors per package.
				log_procs_per_pkg = static_cast<uint8_t>((cpu.Ebx() & CFM_LogicalProcessorCount) >> 16);

				// Determine the total number of cores per package.  This info
				// is extracted differently dependending on the cpu vendor.
				if (Cpuid::IsVendor(GenuineIntel))
				{
					if (cpu.Call(Cpuid::FS_Std, 4))
					{
						cores_per_pkg = static_cast<uint8_t>(((cpu.Eax() & CFM_NC_Intel) >> 26) + 1);
					}
				}
				else
				{
					BOOST_ASSERT(Cpuid::IsVendor(AuthenticAMD));
					if (cpu.Call(Cpuid::FS_Ext, 8))
					{
						// AMD reports the msb width of the CORE_ID bit field of the APIC ID
						// in ApicIdCoreIdSize_Amd.  The maximum value represented by the msb
						// width is the theoretical number of cores the processor can support
						// and not the actual number of current cores, which is how the msb width
						// of the CORE_ID bit field has been traditionally determined.  If the
						// ApicIdCoreIdSize_Amd value is zero, then you use the traditional method
						// to determine the CORE_ID msb width.
						uint32_t msb_width = cpu.Ecx() & CFM_ApicIdCoreIdSize_Amd;
						if (msb_width)
						{
							// Set cores_per_pkg to the maximum theortical number of cores
							// the processor package can support (2 ^ width) so the APIC
							// extractor object can be configured to extract the proper
							// values from an APIC.
							cores_per_pkg = static_cast<uint8_t>(1 << ((msb_width >> 12) - 1));
						}
						else
						{
							// Set cores_per_pkg to the actual number of cores being reported
							// by the CPUID instruction.
							cores_per_pkg = static_cast<uint8_t>((cpu.Ecx() & CFM_NC_Amd) + 1);
						}
					}
				}
			}

			// Configure the APIC extractor object with the information it needs to
			// be able to decode the APIC.
			apic_extractor_.SetPackageTopology(log_procs_per_pkg, cores_per_pkg);

			DWORD_PTR process_affinity, system_affinity;
			HANDLE process_handle = ::GetCurrentProcess();
			HANDLE thread_handle = ::GetCurrentThread();
			::GetProcessAffinityMask(process_handle, &process_affinity, &system_affinity);
			if (1 == system_affinity)
			{
				// Since we only have 1 logical processor present on the system, we
				// can explicitly set a single APIC ID to zero.
				BOOST_ASSERT(1 == log_procs_per_pkg);
				apic_ids_.push_back(0);
			}
			else
			{
				// Set the process affinity to the system affinity if they are not
				// equal so that all logical processors can be accounted for.
				if (process_affinity != system_affinity)
				{
					::SetProcessAffinityMask(process_handle, system_affinity);
				}

				// Call cpuid on each active logical processor in the system affinity.
				DWORD_PTR prev_thread_affinity = 0;
				for (DWORD_PTR thread_affinity = 1; thread_affinity && (thread_affinity <= system_affinity);
					thread_affinity <<= 1)
				{
					if (system_affinity & thread_affinity)
					{
						if (0 == prev_thread_affinity)
						{
							// Save the previous thread affinity so we can return
							// the executing thread affinity back to this state.
							BOOST_ASSERT(apic_ids_.empty());
							prev_thread_affinity = ::SetThreadAffinityMask(thread_handle, thread_affinity);
						}
						else
						{
							BOOST_ASSERT(!apic_ids_.empty());
							::SetThreadAffinityMask(thread_handle, thread_affinity);
						}

						// Allow the thread to switch to masked logical processor.
						::Sleep(0);

						// Store the APIC ID
						cpu.Call(Cpuid::FS_Std, 1);
						apic_ids_.push_back(static_cast<uint8_t>((cpu.Ebx() & CFM_ApicId) >> 24));
					}
				}

				// Restore the previous process and thread affinity state.
				::SetProcessAffinityMask(process_handle, process_affinity);
				::SetThreadAffinityMask(thread_handle, prev_thread_affinity);
				::Sleep(0);
			}
		}

		int NumCores() const
		{
			std::vector<uint8_t> pkg_core_ids(apic_ids_.size());
			for (size_t i = 0; i < apic_ids_.size(); ++ i)
			{
				pkg_core_ids[i] = apic_extractor_.PackageCoreId(apic_ids_[i]);
			}
			std::sort(pkg_core_ids.begin(), pkg_core_ids.end());
			pkg_core_ids.erase(std::unique(pkg_core_ids.begin(), pkg_core_ids.end()), pkg_core_ids.end());
			return static_cast<int>(pkg_core_ids.size());
		}

		static bool IsSupported()
		{
			// Indicates if a CpuidImpl object is supported on this platform.
			// Support is only granted on Intel and AMD platforms where the current
			// calling process has security rights to query process affinity and
			// change it if the process and system affinity differ.  CpuidImpl is
			// also not supported if thread affinity cannot be set on systems with
			// more than 1 logical processor.

			bool supported = Cpuid::IsVendor(GenuineIntel) || Cpuid::IsVendor(AuthenticAMD);

			if (supported)
			{
				DWORD_PTR process_affinity, system_affinity;
				HANDLE process_handle = ::GetCurrentProcess();

				// Query process affinity mask
				supported = (::GetProcessAffinityMask(process_handle, &process_affinity, &system_affinity) != 0);
				if (supported)
				{
					if (process_affinity != system_affinity)
					{
						// The process and system affinities differ.  Attempt to set
						// the process affinity to the system affinity.
						supported = (::SetProcessAffinityMask(process_handle, system_affinity) != 0);
						if (supported)
						{
							// Restore previous process affinity
							supported = (::SetProcessAffinityMask(process_handle, process_affinity) != 0);
						}
					}

					if (supported && (system_affinity > 1))
					{
						// Attempt to set the thread affinity 
						HANDLE thread_handle = ::GetCurrentThread();
						DWORD_PTR thread_affinity = ::SetThreadAffinityMask(thread_handle, process_affinity);
						if (thread_affinity)
						{
							// Restore the previous thread affinity
							supported = (::SetThreadAffinityMask(thread_handle, thread_affinity) != 0);
						}
						else
						{
							supported = false;
						}
					}
				}
			}

			return supported;
		}

	private:
		std::vector<uint8_t> apic_ids_;
		ApicExtractor apic_extractor_;

		static char const GenuineIntel[];
		static char const AuthenticAMD[];
	};

	char const CpuidImpl::GenuineIntel[] = "GenuineIntel";
	char const CpuidImpl::AuthenticAMD[] = "AuthenticAMD";
#endif
#endif

	boost::shared_ptr<ICpuTopology> cpu_topo_impl;
}


namespace KlayGE
{
	CpuTopology::CpuTopology(bool force_cpuid)
	{
		UNREF_PARAM(force_cpuid);

#ifdef KLAYGE_PLATFORM_WINDOWS
#ifdef KLAYGE_PLATFORM_WIN32
		if (!force_cpuid && GlpiImpl::IsSupported())
		{
			cpu_topo_impl.reset(new GlpiImpl);
		}
		else
		{
			if (CpuidImpl::IsSupported())
			{
				cpu_topo_impl.reset(new CpuidImpl);
			}
			else
			{
				cpu_topo_impl.reset(new DefaultImpl);
			}
		}
#elif defined KLAYGE_PLATFORM_WIN64
		cpu_topo_impl.reset(new GlpiImpl);
#endif
#else
		cpu_topo_impl.reset(new DefaultImpl);
#endif
	}

	int CpuTopology::NumHWThreads() const
	{
#ifdef KLAYGE_PLATFORM_WINDOWS
		SYSTEM_INFO si = { 0 };
		::GetSystemInfo(&si);
		return si.dwNumberOfProcessors;
#else
		return 1;
#endif
	}

	int CpuTopology::NumCores() const
	{
		return cpu_topo_impl->NumCores();
	}
}
