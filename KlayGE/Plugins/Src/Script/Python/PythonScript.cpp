/**
 * @file PythonScript.cpp
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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/Vector.hpp>
#include <KFL/Matrix.hpp>
#include <KFL/Plane.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/App3D.hpp>

#include <algorithm>
#include <boost/assert.hpp>

#include <KlayGE/Python/PythonScript.hpp>

namespace KlayGE
{
	// http://stackoverflow.com/questions/4307187/how-to-catch-python-stdout-in-c-code

	PyObject* StdoutWrite(PyObject* self, PyObject* args)
	{
		KFL_UNUSED(self);
		KFL_UNUSED(args);

		char* data = nullptr;
		if (!PyArg_ParseTuple(args, "s", &data))
		{
			return 0;
		}

		if (nullptr == data)
		{
			return 0;
		}

		size_t const len = strlen(data);
		if ((len > 1) || (data[0] != '\n'))
		{
			LogInfo(data);
		}
		return PyLong_FromSize_t(len);
	}
	
	PyObject* StdoutFlush(PyObject* self, PyObject* args)
	{
		KFL_UNUSED(self);
		KFL_UNUSED(args);

		// no-op
		return Py_BuildValue("");
	}
	
	PyMethodDef Stdout_methods[] =
	{
		{ "write", StdoutWrite, METH_VARARGS, "sys.stdout.write" },
		{ "flush", StdoutFlush, METH_VARARGS, "sys.stdout.write" },
		{ 0, 0, 0, 0 } // sentinel
	};
	
	PyTypeObject stdout_type =
	{
		PyVarObject_HEAD_INIT(0, 0)
		"emb.StdoutType",     /* tp_name */
		sizeof(PyObject),     /* tp_basicsize */
		0,                    /* tp_itemsize */
		0,                    /* tp_dealloc */
		0,                    /* tp_print */
		0,                    /* tp_getattr */
		0,                    /* tp_setattr */
		0,                    /* tp_reserved */
		0,                    /* tp_repr */
		0,                    /* tp_as_number */
		0,                    /* tp_as_sequence */
		0,                    /* tp_as_mapping */
		0,                    /* tp_hash  */
		0,                    /* tp_call */
		0,                    /* tp_str */
		0,                    /* tp_getattro */
		0,                    /* tp_setattro */
		0,                    /* tp_as_buffer */
		Py_TPFLAGS_DEFAULT,   /* tp_flags */
		"emb.Stdout objects", /* tp_doc */
		0,                    /* tp_traverse */
		0,                    /* tp_clear */
		0,                    /* tp_richcompare */
		0,                    /* tp_weaklistoffset */
		0,                    /* tp_iter */
		0,                    /* tp_iternext */
		Stdout_methods,       /* tp_methods */
		0,                    /* tp_members */
		0,                    /* tp_getset */
		0,                    /* tp_base */
		0,                    /* tp_dict */
		0,                    /* tp_descr_get */
		0,                    /* tp_descr_set */
		0,                    /* tp_dictoffset */
		0,                    /* tp_init */
		0,                    /* tp_alloc */
		0,                    /* tp_new */
		0,                    /* tp_free */
		0,                    /* tp_is_gc */
		0,                    /* tp_bases */
		0,                    /* tp_mro */
		0,                    /* tp_cache */
		0,                    /* tp_subclasses */
		0,                    /* tp_weaklist */
		0,                    /* tp_del */
		0,                    /* tp_version_tag */
		0,                    /* tp_finalize */

#ifdef COUNT_ALLOCS
		0,                    /* tp_allocs */
		0,                    /* tp_frees */
		0,                    /* tp_maxalloc */
		0,                    /* tp_prev */
		0                     /* tp_next */
#endif
	};
	
	PyModuleDef emb_module =
	{
		PyModuleDef_HEAD_INIT,
		"emb", 0, -1, 0,
		0, 0, 0, 0
	};
	
	PyObjectPtr stdout_obj;
	PyObjectPtr stdout_saved_obj;
	
	PyMODINIT_FUNC PyInit_emb()
	{
		stdout_obj.reset();
		stdout_saved_obj.reset();

		stdout_type.tp_new = PyType_GenericNew;
		if (PyType_Ready(&stdout_type) < 0)
		{
			return 0;
		}

		PyObject* m = PyModule_Create(&emb_module);
		if (m)
		{
			PyObject* o = reinterpret_cast<PyObject*>(&stdout_type);
			Py_IncRef(o);
			PyModule_AddObject(m, "Stdout", o);
		}
		return m;
	}
	
	void SetStdout()
	{
		if (!stdout_obj)
		{
			stdout_saved_obj = MakePyObjectPtr(PySys_GetObject("stdout"));
			Py_IncRef(stdout_saved_obj.get());
			stdout_obj = MakePyObjectPtr(stdout_type.tp_new(&stdout_type, 0, 0));
			Py_IncRef(stdout_obj.get());
		}

		PySys_SetObject("stdout", stdout_obj.get());
	}
	
	void ResetStdout()
	{
		if (stdout_saved_obj)
		{
			PySys_SetObject("stdout", stdout_saved_obj.get());
		}

		stdout_obj.reset();
		stdout_saved_obj.reset();
	}

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

	PyObjectPtr CppType2PyObjectPtr(std::any const & t)
	{
		if (std::any_cast<std::string>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<std::string>(t));
		}
		else if (std::any_cast<char*>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<char*>(t));
		}
		else if (std::any_cast<wchar_t*>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<wchar_t*>(t));
		}
		else if (std::any_cast<int8_t>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<int8_t>(t));
		}
		else if (std::any_cast<int16_t>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<int16_t>(t));
		}
		else if (std::any_cast<int32_t>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<int32_t>(t));
		}
		else if (std::any_cast<int64_t>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<int64_t>(t));
		}
		else if (std::any_cast<uint8_t>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<uint8_t>(t));
		}
		else if (std::any_cast<uint16_t>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<uint16_t>(t));
		}
		else if (std::any_cast<uint32_t>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<uint32_t>(t));
		}
		else if (std::any_cast<uint64_t>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<uint64_t>(t));
		}
		else if (std::any_cast<double>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<double>(t));
		}
		else if (std::any_cast<float>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<float>(t));
		}
		else if (std::any_cast<PyObject*>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<PyObject*>(t));
		}
		else if (std::any_cast<PyObjectPtr>(&t) != nullptr)
		{
			return CppType2PyObjectPtr(std::any_cast<PyObjectPtr>(t));
		}
		else
		{
			KFL_UNREACHABLE("Invalid type");
		}
	}

	std::any PyObjectPtr2CppType(PyObjectPtr const & t)
	{
		std::any ret;
		if (PyObject_TypeCheck(t.get(), &PyUnicode_Type))
		{
			ret = std::any(std::string(PyBytes_AsString(PyUnicode_AsASCIIString(t.get()))));
		}
		else if (PyObject_TypeCheck(t.get(), &PyLong_Type))
		{
			ret = std::any(static_cast<int32_t>(PyLong_AsLong(t.get())));
		}
		else if (PyObject_TypeCheck(t.get(), &PyFloat_Type))
		{
			ret = std::any(static_cast<float>(PyFloat_AsDouble(t.get())));
		}
		else if (PyObject_TypeCheck(t.get(), &PyList_Type))
		{
			size_t len = PyList_Size(t.get());
			std::vector<std::any> v(len);
			for (size_t i = 0; i < len; ++ i)
			{
				PyObjectPtr py_obj = MakePyObjectPtr(PyList_GetItem(t.get(), i));
				Py_IncRef(py_obj.get());
				v[i] = PyObjectPtr2CppType(py_obj);
			}
			ret = std::any(v);
		}
		else if (PyObject_TypeCheck(t.get(), &PyTuple_Type))
		{
			size_t len = PyTuple_Size(t.get());
			std::vector<std::any> v(len);
			for (size_t i = 0; i < len; ++ i)
			{
				PyObjectPtr py_obj = MakePyObjectPtr(PyTuple_GetItem(t.get(), i));
				Py_IncRef(py_obj.get());
				v[i] = PyObjectPtr2CppType(py_obj);
			}
			ret = std::any(v);
		}
		return ret;
	}

	PythonScriptModule::PythonScriptModule(std::string const & name)
	{
		if (name.empty())
		{
			module_ = MakePyObjectPtr(PyImport_AddModule("__main__"));
			Py_IncRef(module_.get());
		}
		else
		{
			module_ = MakePyObjectPtr(PyImport_ImportModule(name.c_str()));
		}
		dict_ = MakePyObjectPtr(PyModule_GetDict(module_.get()));
		Py_IncRef(dict_.get());
	}

	PythonScriptModule::~PythonScriptModule()
	{
		dict_.reset();
		module_.reset();
	}

	std::any PythonScriptModule::Value(std::string const & name)
	{
		PyObject* p = PyDict_GetItemString(dict_.get(), name.c_str());
		Py_IncRef(p);
		return MakePyObjectPtr(p);
	}

	std::any PythonScriptModule::Call(std::string const & func_name, ArrayRef<std::any> args)
	{
		PyObjectPtr py_args = MakePyObjectPtr(PyTuple_New(args.size()));
		for (size_t i = 0; i < args.size(); ++ i)
		{
			PyObjectPtr value = CppType2PyObjectPtr(args[i]);
			Py_IncRef(value.get());
			PyTuple_SetItem(py_args.get(), i, value.get());
		}

		PyObjectPtr func = std::any_cast<PyObjectPtr>(this->Value(func_name));
		return PyObjectPtr2CppType(MakePyObjectPtr(PyObject_CallObject(func.get(), py_args.get())));
	}

	std::any PythonScriptModule::RunString(std::string const & script)
	{
		return MakePyObjectPtr(PyRun_String(script.c_str(), Py_file_input, dict_.get(), dict_.get()));
	}

	PythonEngine::PythonEngine()
	{
		Py_NoSiteFlag = 1;
		std::wstring py_lib;
		Convert(py_lib, ResLoader::Instance().AbsPath(Context::Instance().AppInstance().Name() + "Py.zip"));
		Py_SetPath(&py_lib[0]);

		PyImport_AppendInittab("emb", PyInit_emb);
		Py_InitializeEx(0);
		PyImport_ImportModule("emb");

		SetStdout();
	}

	PythonEngine::~PythonEngine()
	{
		ResetStdout();
		Py_Finalize();
	}

	ScriptModulePtr PythonEngine::CreateModule(std::string const & name)
	{
		return MakeSharedPtr<PythonScriptModule>(name);
	}

	void PythonEngine::DoSuspend()
	{
	}

	void PythonEngine::DoResume()
	{
	}
}
