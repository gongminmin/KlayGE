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

	ScriptVariablePtr NullScriptModule::Value(std::string const & name)
	{
		KFL_UNUSED(name);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::Call(std::string const & func_name, std::span<ScriptVariablePtr const> args)
	{
		KFL_UNUSED(func_name);
		KFL_UNUSED(args);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::RunString(std::string const & script)
	{
		KFL_UNUSED(script);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(std::string const& value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(std::string_view value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(char const* value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(char* value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(std::wstring const& value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(std::wstring_view value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(wchar_t const* value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(wchar_t* value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(int8_t value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(int16_t value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(int32_t value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(int64_t value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(uint8_t value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(uint16_t value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(uint32_t value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(uint64_t value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(float value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(double value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable(std::span<ScriptVariablePtr const> value) const
	{
		KFL_UNUSED(value);
		return ScriptVariablePtr();
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
