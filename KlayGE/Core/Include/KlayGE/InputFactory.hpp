// InputFactory.hpp
// KlayGE 输入引擎抽象工厂 头文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// 增加了NullObject (2005.10.29)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _INPUTFACTORY_HPP
#define _INPUTFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API InputFactory : boost::noncopyable
	{
	public:
		virtual ~InputFactory() noexcept;

		virtual std::wstring const & Name() const = 0;
		InputEngine& InputEngineInstance();

		void Suspend();
		void Resume();

	private:
		virtual std::unique_ptr<InputEngine> DoMakeInputEngine() = 0;
		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

	protected:
		std::unique_ptr<InputEngine> ie_;
	};
}

#endif			// _INPUTFACTORY_HPP
