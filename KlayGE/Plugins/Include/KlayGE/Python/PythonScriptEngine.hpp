/**
 * @file PythonEngine.hpp
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

#ifndef _PYTHONSCRIPTENGINE_HPP
#define _PYTHONSCRIPTENGINE_HPP

#pragma once

#if defined(__MINGW32__)
#ifndef __NO_MINGW_LFS
#define __NO_MINGW_LFS
#endif
#endif

#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable: 4510) // No default constructor for PyHash_FuncDef
#pragma warning(disable: 4512) // No assignment operator for PyHash_FuncDef
#pragma warning(disable: 4610) // No user-defined constructor for PyHash_FuncDef in vc12
#elif defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers" // Some uninitializers in python.h
#elif defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined" // 'Py_USING_UNICODE' redefined
#endif
#include <Python.h>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#elif defined(KLAYGE_COMPILER_GCC)
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
	PyObjectPtr CppType2PyObjectPtr(std::experimental::any const & t);

	// Py Script module
	/////////////////////////////////////////////////////////////////////////////////
	class PythonScriptModule : public ScriptModule
	{
	public:
		explicit PythonScriptModule(std::string const & name);
		~PythonScriptModule();

		virtual std::experimental::any Value(std::string const & name);
		virtual std::experimental::any Call(std::string const & func_name, const AnyDataListType& args);
		virtual std::experimental::any RunString(std::string const & script);

	private:
		PyObjectPtr module_;
		PyObjectPtr dict_;
	};

	class PythonEngine : public ScriptEngine
	{
	public:
		PythonEngine();
		~PythonEngine();

		virtual ScriptModulePtr CreateModule(std::string const & name);

	private:
		virtual void DoSuspend() override;
		virtual void DoResume() override;
	};
}

#endif  // _PYTHON_ENGINE_HPP
