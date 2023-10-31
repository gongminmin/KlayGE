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

#include "NullScript.hpp"

namespace KlayGE
{
	NullScriptModule::NullScriptModule()
	{
	}

	NullScriptModule::~NullScriptModule()
	{
	}

	ScriptVariablePtr NullScriptModule::Value([[maybe_unused]] std::string const & name)
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::Call([[maybe_unused]] std::string const & func_name, [[maybe_unused]] std::span<ScriptVariablePtr const> args)
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::RunString([[maybe_unused]] std::string const & script)
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] std::string const& value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] std::string_view value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] char const* value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] char* value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] std::wstring const& value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] std::wstring_view value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] wchar_t const* value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] wchar_t* value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] int8_t value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] int16_t value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] int32_t value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] int64_t value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] uint8_t value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] uint16_t value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] uint32_t value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] uint64_t value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] float value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] double value) const
	{
		return ScriptVariablePtr();
	}

	ScriptVariablePtr NullScriptModule::MakeVariable([[maybe_unused]] std::span<ScriptVariablePtr const> value) const
	{
		return ScriptVariablePtr();
	}


	NullScriptEngine::NullScriptEngine()
	{
	}

	NullScriptEngine::~NullScriptEngine()
	{
	}

	ScriptModulePtr NullScriptEngine::CreateModule([[maybe_unused]] std::string const & name)
	{
		return MakeSharedPtr<NullScriptModule>();
	}

	void NullScriptEngine::DoSuspend()
	{
	}

	void NullScriptEngine::DoResume()
	{
	}
}
