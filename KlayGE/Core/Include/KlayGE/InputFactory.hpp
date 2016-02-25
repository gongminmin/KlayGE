// InputFactory.hpp
// KlayGE ����������󹤳� ͷ�ļ�
// Ver 3.1.0
// ��Ȩ����(C) ������, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// ������NullObject (2005.10.29)
//
// 2.0.0
// ���ν��� (2003.8.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _INPUTFACTORY_HPP
#define _INPUTFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API InputFactory
	{
	public:
		virtual ~InputFactory()
		{
		}

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
