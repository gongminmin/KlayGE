/**
 * @file NullScriptEngine.cpp
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

#include <KlayGE/NullScript/NullScript.hpp>

namespace KlayGE
{
	NullScriptModule::NullScriptModule()
	{
	}

	NullScriptModule::~NullScriptModule()
	{
	}

	std::any NullScriptModule::Value(std::string const & name)
	{
		KFL_UNUSED(name);
		return std::any();
	}

	std::any NullScriptModule::Call(std::string const & func_name, std::span<std::any const> args)
	{
		KFL_UNUSED(func_name);
		KFL_UNUSED(args);
		return std::any();
	}

	std::any NullScriptModule::RunString(std::string const & script)
	{
		KFL_UNUSED(script);
		return std::any();
	}


	NullScriptEngine::NullScriptEngine()
	{
	}

	NullScriptEngine::~NullScriptEngine()
	{
	}

	ScriptModulePtr NullScriptEngine::CreateModule(std::string const & name)
	{
		KFL_UNUSED(name);
		return MakeSharedPtr<NullScriptModule>();
	}

	void NullScriptEngine::DoSuspend()
	{
	}

	void NullScriptEngine::DoResume()
	{
	}
}
