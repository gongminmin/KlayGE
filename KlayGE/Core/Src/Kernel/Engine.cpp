#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Config.hpp>
#include <KlayGE/CPU.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/SceneManager.hpp>

#include <KlayGE/Engine.hpp>

namespace KlayGE
{
	MathLib*	Engine::math_;
	MemoryLib*	Engine::memory_;

	App3DFramework* Engine::app_;
	SceneManager*	Engine::sceneManager_;

	RenderFactory*	Engine::renderFactory_;
	AudioFactory*	Engine::audioFactory_;
	InputFactory*	Engine::inputFactory_;

	Engine::Engine()
	{
		CPUOptimal cpu;

#ifdef _3DNOWEX_
		cpu = CPU_AMD3DNowEx;
#else
#ifdef _MMX_
		cpu = CPU_MMX;
#else
#ifdef _STANDARD_
		cpu = CPU_Standard;
#else
		CPUInfo cpuInfo;
		cpuInfo.CheckCpu();

		if (cpuInfo.Is3DNowEXSupport())
		{
			cpu = CPU_AMD3DNowEx;
		}
		else
		{
			if (cpuInfo.IsMMXSupport())
			{
				cpu = CPU_MMX;
			}
			else
			{
				cpu = CPU_Standard;
			}
		}
#endif		// _STANDARD_
#endif		// _MMX_
#endif		// _3DNOWEX_

		math_ = MathLib::Create(cpu);
		memory_ = MemoryLib::Create(cpu);

		sceneManager_ = new SceneManager;
	}

	Engine::~Engine()
	{
		SafeDelete(memory_);
		SafeDelete(math_);

		SafeDelete(sceneManager_);
	}

	Engine engine;
}