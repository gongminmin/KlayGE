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

#include <KFL/ErrorHandling.hpp>

#include <KlayGE/Script.hpp>

namespace KlayGE
{
	ScriptVariable::ScriptVariable() noexcept = default;
	ScriptVariable::~ScriptVariable() noexcept = default;

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] ScriptVariable const& rhs)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] std::string const& value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] std::string_view value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] char const* value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] char* value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] std::wstring const& value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] std::wstring_view value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] wchar_t const* value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] wchar_t* value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] int8_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] int16_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] int32_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] int64_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] uint8_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] uint16_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] uint32_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] uint64_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] float value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] double value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=([[maybe_unused]] std::span<ScriptVariablePtr const> value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	bool ScriptVariable::TryValue([[maybe_unused]] std::string& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] std::wstring& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] int8_t& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] int16_t& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] int32_t& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] int64_t& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] uint8_t& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] uint16_t& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] uint32_t& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] uint64_t& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] float& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] double& value) const
	{
		return false;
	}

	bool ScriptVariable::TryValue([[maybe_unused]] std::vector<ScriptVariablePtr>& value) const
	{
		return false;
	}


	ScriptModule::ScriptModule() noexcept = default;
	ScriptModule::~ScriptModule() noexcept = default;


	ScriptEngine::ScriptEngine() noexcept = default;
	ScriptEngine::~ScriptEngine() noexcept = default;

	void ScriptEngine::Suspend()
	{
		this->DoSuspend();
	}

	void ScriptEngine::Resume()
	{
		this->DoResume();
	}
}
