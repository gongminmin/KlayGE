/**
 * @file PythonEngine.cpp
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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/Vector.hpp>
#include <KFL/Matrix.hpp>
#include <KFL/Plane.hpp>

#include <algorithm>
#include <functional>
#include <boost/assert.hpp>

#include <KlayGE/Python/PythonScriptEngine.hpp>

#ifndef KLAYGE_PLATFORM_WINDOWS_METRO

#ifdef KLAYGE_COMPILER_MSVC
#define PYTHON_VER_STR KFL_STRINGIZE(PY_MAJOR_VERSION)KFL_STRINGIZE(PY_MINOR_VERSION)

#ifdef KLAYGE_DEBUG
#define PYTHON_DBG_SUFFIX "_d"
#else
#define PYTHON_DBG_SUFFIX ""
#endif

#define PYTHON_LIB_STR KFL_STRINGIZE("python")KFL_STRINGIZE(FREETYPE_VER_STR)KFL_STRINGIZE(FREETYPE_DBG_SUFFIX)KFL_STRINGIZE(".lib")
#endif

namespace KlayGE
{
	class PyObjDeleter
	{
	public:
		void operator()(PyObject* p)
		{
			if (p != nullptr)
			{
				Py_DecRef(p);
			}
		}
	};


	PyObjectPtr MakePyObjectPtr(PyObject* p)
	{
		return PyObjectPtr(p, PyObjDeleter());
	}


	PyObjectPtr CppType2PyObjectPtr(std::string const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("s", t.c_str()));
	}

	PyObjectPtr CppType2PyObjectPtr(char* t)
	{
		return MakePyObjectPtr(Py_BuildValue("s", t));
	}

	PyObjectPtr CppType2PyObjectPtr(wchar_t* t)
	{
		return MakePyObjectPtr(Py_BuildValue("u", t));
	}

	PyObjectPtr CppType2PyObjectPtr(int8_t t)
	{
		return MakePyObjectPtr(Py_BuildValue("b", t));
	}

	PyObjectPtr CppType2PyObjectPtr(int16_t t)
	{
		return MakePyObjectPtr(Py_BuildValue("h", t));
	}

	PyObjectPtr CppType2PyObjectPtr(int32_t t)
	{
		return MakePyObjectPtr(Py_BuildValue("i", t));
	}

	PyObjectPtr CppType2PyObjectPtr(int64_t t)
	{
		return MakePyObjectPtr(Py_BuildValue("L", t));
	}

	PyObjectPtr CppType2PyObjectPtr(uint8_t t)
	{
		return MakePyObjectPtr(Py_BuildValue("B", t));
	}

	PyObjectPtr CppType2PyObjectPtr(uint16_t t)
	{
		return MakePyObjectPtr(Py_BuildValue("H", t));
	}

	PyObjectPtr CppType2PyObjectPtr(uint32_t t)
	{
		return MakePyObjectPtr(Py_BuildValue("I", t));
	}

	PyObjectPtr CppType2PyObjectPtr(uint64_t t)
	{
		return MakePyObjectPtr(Py_BuildValue("K", t));
	}

	PyObjectPtr CppType2PyObjectPtr(double t)
	{
		return MakePyObjectPtr(Py_BuildValue("d", t));
	}

	PyObjectPtr CppType2PyObjectPtr(float t)
	{
		return MakePyObjectPtr(Py_BuildValue("f", t));
	}

	PyObjectPtr CppType2PyObjectPtr(PyObject* t)
	{
		return MakePyObjectPtr(t);
	}

	PyObjectPtr CppType2PyObjectPtr(PyObjectPtr const & t)
	{
		return t;
	}

	PyObjectPtr CppType2PyObjectPtr(boost::any const & t)
	{
		if (typeid(std::string) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<std::string>(t));
		}
		else if (typeid(char*) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<char*>(t));
		}
		else if (typeid(wchar_t*) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<wchar_t*>(t));
		}
		else if (typeid(int8_t) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<int8_t>(t));
		}
		else if (typeid(int16_t) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<int16_t>(t));
		}
		else if (typeid(int32_t) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<int32_t>(t));
		}
		else if (typeid(int64_t) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<int64_t>(t));
		}
		else if (typeid(uint8_t) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<uint8_t>(t));
		}
		else if (typeid(uint16_t) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<uint16_t>(t));
		}
		else if (typeid(uint32_t) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<uint32_t>(t));
		}
		else if (typeid(uint64_t) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<uint64_t>(t));
		}
		else if (typeid(double) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<double>(t));
		}
		else if (typeid(float) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<float>(t));
		}
		else if (typeid(PyObject*) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<PyObject*>(t));
		}
		else if (typeid(PyObjectPtr) == t.type())
		{
			return CppType2PyObjectPtr(boost::any_cast<PyObjectPtr>(t));
		}
		else
		{
			BOOST_ASSERT(false);
			return CppType2PyObjectPtr(boost::any_cast<int32_t>(t));
		}
	}

	PythonScriptModule::PythonScriptModule()
	{
		module_	= MakePyObjectPtr(PyImport_AddModule("__main__"));
		dict_	= MakePyObjectPtr(PyModule_GetDict(module_.get()));
		Py_IncRef(module_.get());
		Py_IncRef(dict_.get());
	}

	PythonScriptModule::PythonScriptModule(std::string const & name)
	{
		module_	= MakePyObjectPtr(PyImport_ImportModule(name.c_str()));
		dict_	= MakePyObjectPtr(PyModule_GetDict(module_.get()));
		Py_IncRef(module_.get());
		Py_IncRef(dict_.get());
	}

	PythonScriptModule::~PythonScriptModule()
	{
		dict_.reset();
		module_.reset();
	}

	boost::any PythonScriptModule::Value(std::string const & name)
	{
		PyObject* p = PyDict_GetItemString(dict_.get(), name.c_str());
		Py_IncRef(p);
		return MakePyObjectPtr(p);
	}

	boost::any PythonScriptModule::Call(std::string const & func_name, const AnyDataListType& args)
	{
		PyObjectPtr py_args = MakePyObjectPtr(PyTuple_New(args.size()));
		for (size_t i = 0; i < args.size(); ++ i)
		{
			PyObjectPtr value = CppType2PyObjectPtr(args[i]);
			Py_IncRef(value.get());
			PyTuple_SetItem(py_args.get(), i, value.get());
		}

		PyObjectPtr func = boost::any_cast<PyObjectPtr>(this->Value(func_name));
		return MakePyObjectPtr(PyObject_CallObject(func.get(), py_args.get()));
	}

	boost::any PythonScriptModule::RunString(std::string const & script)
	{
		return MakePyObjectPtr(PyRun_String(script.c_str(), Py_file_input, dict_.get(), dict_.get()));
	}

	PythonEngine::PythonEngine()
	{
		Py_Initialize();
	}

	PythonEngine::~PythonEngine()
	{
		Py_Finalize();
	}

	ScriptModulePtr PythonEngine::CreateModule(std::string const & name)
	{
		ScriptModulePtr obj = name.empty() ?
							  MakeSharedPtr<PythonScriptModule>() :
							  MakeSharedPtr<PythonScriptModule>(name);
		return obj;
	}
}

#endif
