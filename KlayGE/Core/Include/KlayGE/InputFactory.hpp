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

namespace KlayGE
{
	class InputFactory
	{
	public:
		virtual ~InputFactory()
			{ }

		virtual const std::wstring& Name() const = 0;
		virtual InputEngine& InputEngineInstance() = 0;
	};

	template <typename InputEngineType>
	class ConcreteInputFactory : public InputFactory
	{
	public:
		ConcreteInputFactory(const std::wstring& name)
				: name_(name)
			{ }

		const std::wstring& Name() const
			{ return name_; }

		InputEngine& InputEngineInstance()
		{
			static InputEngineType inputEngine;
			return inputEngine;
		}

	private:
		const std::wstring name_;
	};
}

#endif			// _INPUTFACTORY_HPP
