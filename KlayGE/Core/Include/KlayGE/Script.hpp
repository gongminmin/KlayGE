/**
 * @file Script.hpp
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

#ifndef KLAYGE_CORE_SCRIPT_HPP
#define KLAYGE_CORE_SCRIPT_HPP

#pragma once

#include <string>

#include <KFL/CXX20/span.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ScriptVariable : boost::noncopyable
	{
	public:
		virtual ~ScriptVariable() noexcept;

		virtual ScriptVariable& operator=(std::string const& value);
		virtual ScriptVariable& operator=(std::string_view value);
		virtual ScriptVariable& operator=(char const* value);
		virtual ScriptVariable& operator=(char* value);
		virtual ScriptVariable& operator=(std::wstring const& value);
		virtual ScriptVariable& operator=(std::wstring_view value);
		virtual ScriptVariable& operator=(wchar_t const* value);
		virtual ScriptVariable& operator=(wchar_t* value);
		virtual ScriptVariable& operator=(int8_t value);
		virtual ScriptVariable& operator=(int16_t value);
		virtual ScriptVariable& operator=(int32_t value);
		virtual ScriptVariable& operator=(int64_t value);
		virtual ScriptVariable& operator=(uint8_t value);
		virtual ScriptVariable& operator=(uint16_t value);
		virtual ScriptVariable& operator=(uint32_t value);
		virtual ScriptVariable& operator=(uint64_t value);
		virtual ScriptVariable& operator=(float value);
		virtual ScriptVariable& operator=(double value);
		virtual ScriptVariable& operator=(std::span<ScriptVariablePtr const> value);

		virtual bool TryValue(std::string& value) const;
		virtual bool TryValue(std::wstring& value) const;
		virtual bool TryValue(int8_t& value) const;
		virtual bool TryValue(int16_t& value) const;
		virtual bool TryValue(int32_t& value) const;
		virtual bool TryValue(int64_t& value) const;
		virtual bool TryValue(uint8_t& value) const;
		virtual bool TryValue(uint16_t& value) const;
		virtual bool TryValue(uint32_t& value) const;
		virtual bool TryValue(uint64_t& value) const;
		virtual bool TryValue(float& value) const;
		virtual bool TryValue(double& value) const;
		virtual bool TryValue(std::vector<ScriptVariablePtr>& value) const;

		template <typename T>
		void Value(T& value) const
		{
			if (!this->TryValue(value))
			{
				throw std::bad_cast();
			}
		}
	};

	class KLAYGE_CORE_API ScriptModule : boost::noncopyable
	{
	public:
		ScriptModule() noexcept;
		virtual ~ScriptModule() noexcept;

		virtual ScriptVariablePtr Value(std::string const & name) = 0;
		virtual ScriptVariablePtr Call(std::string const & func_name, std::span<ScriptVariablePtr const> args) = 0;
		virtual ScriptVariablePtr RunString(std::string const & script) = 0;

		virtual ScriptVariablePtr MakeVariable(std::string const& value) const = 0;
		virtual ScriptVariablePtr MakeVariable(std::string_view value) const = 0;
		virtual ScriptVariablePtr MakeVariable(char const* value) const = 0;
		virtual ScriptVariablePtr MakeVariable(char* value) const = 0;
		virtual ScriptVariablePtr MakeVariable(std::wstring const& value) const = 0;
		virtual ScriptVariablePtr MakeVariable(std::wstring_view value) const = 0;
		virtual ScriptVariablePtr MakeVariable(wchar_t const* value) const = 0;
		virtual ScriptVariablePtr MakeVariable(wchar_t* value) const = 0;
		virtual ScriptVariablePtr MakeVariable(int8_t value) const = 0;
		virtual ScriptVariablePtr MakeVariable(int16_t value) const = 0;
		virtual ScriptVariablePtr MakeVariable(int32_t value) const = 0;
		virtual ScriptVariablePtr MakeVariable(int64_t value) const = 0;
		virtual ScriptVariablePtr MakeVariable(uint8_t value) const = 0;
		virtual ScriptVariablePtr MakeVariable(uint16_t value) const = 0;
		virtual ScriptVariablePtr MakeVariable(uint32_t value) const = 0;
		virtual ScriptVariablePtr MakeVariable(uint64_t value) const = 0;
		virtual ScriptVariablePtr MakeVariable(float value) const = 0;
		virtual ScriptVariablePtr MakeVariable(double value) const = 0;
		virtual ScriptVariablePtr MakeVariable(std::span<ScriptVariablePtr const> value) const = 0;
	};

	typedef std::shared_ptr<ScriptModule> ScriptModulePtr;


	class KLAYGE_CORE_API ScriptEngine : boost::noncopyable
	{
	public:
		ScriptEngine() noexcept;
		virtual ~ScriptEngine() noexcept;

		void Suspend();
		void Resume();

		virtual ScriptModulePtr CreateModule(std::string const & name) = 0;

	private:
		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;
	};
}

#endif		// KLAYGE_CORE_SCRIPT_HPP
