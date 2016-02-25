/**
 * @file Script.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>

#include <boost/assert.hpp>

#include <KlayGE/Script.hpp>

namespace KlayGE
{
	ScriptModule::ScriptModule()
	{
	}

	ScriptModule::~ScriptModule()
	{
	}

	std::experimental::any ScriptModule::Value(std::string const & /*name*/)
	{
		return std::experimental::any();
	}

	std::experimental::any ScriptModule::Call(std::string const & /*func_name*/, const AnyDataListType& /*args*/)
	{
		return std::experimental::any();
	}

	std::experimental::any ScriptModule::RunString(std::string const & /*script*/)
	{
		return std::experimental::any();
	}

	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	ScriptEngine::ScriptEngine()
	{
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	ScriptEngine::~ScriptEngine()
	{
	}

	void ScriptEngine::Suspend()
	{
		this->DoSuspend();
	}

	void ScriptEngine::Resume()
	{
		this->DoResume();
	}

	ScriptModulePtr ScriptEngine::CreateModule(std::string const & /*name*/)
	{
		static ScriptModulePtr obj = MakeSharedPtr<ScriptModule>();
		return obj;
	}
}
