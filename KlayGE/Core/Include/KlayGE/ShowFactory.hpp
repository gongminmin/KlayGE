// ShowFactory.hpp
// KlayGE 播放引擎抽象工厂 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.7.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _SHOWFACTORY_HPP
#define _SHOWFACTORY_HPP

#pragma KLAYGE_ONCE

#include <KlayGE/PreDeclare.hpp>

#include <string>
#include <boost/utility.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ShowFactory
	{
	public:
		virtual ~ShowFactory()
			{ }

		static ShowFactoryPtr NullObject();

		virtual std::wstring const & Name() const = 0;
		ShowEngine& ShowEngineInstance();

	private:
		virtual ShowEnginePtr MakeShowEngine() = 0;

	protected:
		ShowEnginePtr se_;
	};

	template <typename ShowEngineType>
	class ConcreteShowFactory : boost::noncopyable, public ShowFactory
	{
	public:
		ConcreteShowFactory(std::wstring const & name)
				: name_(name)
			{ }

		std::wstring const & Name() const
			{ return name_; }

	private:
		ShowEnginePtr MakeShowEngine()
		{
			return MakeSharedPtr<ShowEngineType>();
		}

	private:
		std::wstring const name_;
	};
}

#endif			// _SHOWFACTORY_HPP
