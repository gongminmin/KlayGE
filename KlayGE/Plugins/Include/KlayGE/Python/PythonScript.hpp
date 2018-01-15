/**
 * @file PythonScript.hpp
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

#ifndef KLAYGE_PLUGINS_PYTHON_SCRIPT_HPP
#define KLAYGE_PLUGINS_PYTHON_SCRIPT_HPP

#pragma once

#if defined(__MINGW32__)
#ifndef __NO_MINGW_LFS
#define __NO_MINGW_LFS
#endif
#endif

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers" // Some uninitializers in python.h
#elif defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined" // 'Py_USING_UNICODE' redefined
#endif
#include <Python.h>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif
#include <vector>
#include <string>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Script.hpp>

#include <vector>

namespace KlayGE
{
	typedef std::shared_ptr<PyObject> PyObjectPtr;

	// PyObject÷∏’Î
	/////////////////////////////////////////////////////////////////////////////////
	PyObjectPtr MakePyObjectPtr(PyObject* p);

	PyObjectPtr CppType2PyObjectPtr(std::string const & t);
	PyObjectPtr CppType2PyObjectPtr(char* t);
	PyObjectPtr CppType2PyObjectPtr(wchar_t* t);
	PyObjectPtr CppType2PyObjectPtr(int8_t t);
	PyObjectPtr CppType2PyObjectPtr(int16_t t);
	PyObjectPtr CppType2PyObjectPtr(int32_t t);
	PyObjectPtr CppType2PyObjectPtr(int64_t t);
	PyObjectPtr CppType2PyObjectPtr(uint8_t t);
	PyObjectPtr CppType2PyObjectPtr(uint16_t t);
	PyObjectPtr CppType2PyObjectPtr(uint32_t t);
	PyObjectPtr CppType2PyObjectPtr(uint64_t t);
	PyObjectPtr CppType2PyObjectPtr(double t);
	PyObjectPtr CppType2PyObjectPtr(float t);
	PyObjectPtr CppType2PyObjectPtr(PyObject* t);
	PyObjectPtr CppType2PyObjectPtr(PyObjectPtr const & t);
	PyObjectPtr CppType2PyObjectPtr(std::any const & t);

	// Py Script module
	/////////////////////////////////////////////////////////////////////////////////
	class PythonScriptModule : public ScriptModule
	{
	public:
		explicit PythonScriptModule(std::string const & name);
		~PythonScriptModule() override;

		std::any Value(std::string const & name) override;
		std::any Call(std::string const & func_name, ArrayRef<std::any> args) override;
		std::any RunString(std::string const & script) override;

	private:
		PyObjectPtr module_;
		PyObjectPtr dict_;
	};

	class PythonEngine : public ScriptEngine
	{
	public:
		PythonEngine();
		~PythonEngine() override;

		ScriptModulePtr CreateModule(std::string const & name) override;

	private:
		void DoSuspend() override;
		void DoResume() override;
	};
}

#endif		// KLAYGE_PLUGINS_PYTHON_SCRIPT_HPP
