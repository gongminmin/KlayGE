/**
 * @file NullScriptEngine.hpp
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

#ifndef KLAYGE_PLUGINS_NULL_SCRIPT_ENGINE_HPP
#define KLAYGE_PLUGINS_NULL_SCRIPT_ENGINE_HPP

#pragma once

#include <string>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Script.hpp>

namespace KlayGE
{
	class NullScriptModule final : public ScriptModule
	{
	public:
		NullScriptModule();
		~NullScriptModule() override;

		ScriptVariablePtr Value(std::string const & name) override;
		ScriptVariablePtr Call(std::string const & func_name, std::span<ScriptVariablePtr const> args) override;
		ScriptVariablePtr RunString(std::string const & script) override;

		ScriptVariablePtr MakeVariable(std::string const& value) const override;
		ScriptVariablePtr MakeVariable(std::string_view value) const override;
		ScriptVariablePtr MakeVariable(char const* value) const override;
		ScriptVariablePtr MakeVariable(char* value) const override;
		ScriptVariablePtr MakeVariable(std::wstring const& value) const override;
		ScriptVariablePtr MakeVariable(std::wstring_view value) const override;
		ScriptVariablePtr MakeVariable(wchar_t const* value) const override;
		ScriptVariablePtr MakeVariable(wchar_t* value) const override;
		ScriptVariablePtr MakeVariable(int8_t value) const override;
		ScriptVariablePtr MakeVariable(int16_t value) const override;
		ScriptVariablePtr MakeVariable(int32_t value) const override;
		ScriptVariablePtr MakeVariable(int64_t value) const override;
		ScriptVariablePtr MakeVariable(uint8_t value) const override;
		ScriptVariablePtr MakeVariable(uint16_t value) const override;
		ScriptVariablePtr MakeVariable(uint32_t value) const override;
		ScriptVariablePtr MakeVariable(uint64_t value) const override;
		ScriptVariablePtr MakeVariable(float value) const override;
		ScriptVariablePtr MakeVariable(double value) const override;
		ScriptVariablePtr MakeVariable(std::span<ScriptVariablePtr const> value) const override;
	};

	class NullScriptEngine final : public ScriptEngine
	{
	public:
		NullScriptEngine();
		~NullScriptEngine() override;

		ScriptModulePtr CreateModule(std::string const & name) override;

	private:
		void DoSuspend() override;
		void DoResume() override;
	};
}

#endif  // KLAYGE_PLUGINS_NULL_SCRIPT_ENGINE_HPP
