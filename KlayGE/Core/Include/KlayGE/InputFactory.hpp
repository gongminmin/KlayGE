// InputFactory.hpp
// KlayGE 输入引擎抽象工厂 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _INPUTFACTORY_HPP
#define _INPUTFACTORY_HPP

#include <boost/utility.hpp>

namespace KlayGE
{
	class InputFactory
	{
	public:
		virtual ~InputFactory()
			{ }

		virtual std::wstring const & Name() const = 0;
		virtual InputEngine& InputEngineInstance() = 0;
	};

	template <typename InputEngineType>
	class ConcreteInputFactory : boost::noncopyable, public InputFactory
	{
	public:
		ConcreteInputFactory(std::wstring const & name)
				: name_(name)
			{ }

		std::wstring const & Name() const
			{ return name_; }

		InputEngine& InputEngineInstance()
		{
			static InputEngineType inputEngine;
			return inputEngine;
		}

	private:
		std::wstring const name_;
	};
}

#endif			// _INPUTFACTORY_HPP
