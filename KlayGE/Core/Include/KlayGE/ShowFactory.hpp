// ShowFactory.hpp
// KlayGE 播放引擎抽象工厂 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// 初次建立 (2006.7.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _SHOWFACTORY_HPP
#define _SHOWFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>

#include <KlayGE/Show.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ShowFactory : boost::noncopyable
	{
	public:
		virtual ~ShowFactory() noexcept;

		virtual std::wstring const & Name() const = 0;
		ShowEngine& ShowEngineInstance();

		void Suspend();
		void Resume();

	private:
		virtual std::unique_ptr<ShowEngine> MakeShowEngine() = 0;
		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

	protected:
		std::unique_ptr<ShowEngine> se_;
	};
}

#endif			// _SHOWFACTORY_HPP
