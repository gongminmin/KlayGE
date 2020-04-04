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
	ScriptVariable::~ScriptVariable() noexcept = default;

	ScriptVariable& ScriptVariable::operator=(std::string const& value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(std::string_view value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(char const* value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(char* value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(std::wstring const& value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(std::wstring_view value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(wchar_t const* value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(wchar_t* value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(int8_t value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(int16_t value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(int32_t value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(int64_t value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(uint8_t value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(uint16_t value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(uint32_t value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(uint64_t value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(float value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(double value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	ScriptVariable& ScriptVariable::operator=(std::span<ScriptVariablePtr const> value)
	{
		KFL_UNUSED(value);
		KFL_UNREACHABLE("Can't be called");
	}

	bool ScriptVariable::TryValue(std::string& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(std::wstring& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(int8_t& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(int16_t& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(int32_t& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(int64_t& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(uint8_t& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(uint16_t& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(uint32_t& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(uint64_t& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(float& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(double& value) const
	{
		KFL_UNUSED(value);
		return false;
	}

	bool ScriptVariable::TryValue(std::vector<ScriptVariablePtr>& value) const
	{
		KFL_UNUSED(value);
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
