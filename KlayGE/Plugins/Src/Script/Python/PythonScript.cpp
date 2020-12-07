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
			LogInfo() << data << std::endl;
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

#if defined(KLAYGE_COMPILER_CLANG) || defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // tp_print is deprecated
#endif
	PyTypeObject stdout_type =
	{
		PyVarObject_HEAD_INIT(0, 0)
		"emb.StdoutType",     /* tp_name */
		sizeof(PyObject),     /* tp_basicsize */
		0,                    /* tp_itemsize */
		0,                    /* tp_dealloc */
		0,                    /* tp_vectorcall_offset */
		0,                    /* tp_getattr */
		0,                    /* tp_setattr */
		0,                    /* tp_as_async */
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
		0,                    /* tp_vectorcall */
		0,                    /* tp_print */

#ifdef COUNT_ALLOCS
		0,                    /* tp_allocs */
		0,                    /* tp_frees */
		0,                    /* tp_maxalloc */
		0,                    /* tp_prev */
		0                     /* tp_next */
#endif
	};
#if defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif

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


	class PythonScriptVariable : public ScriptVariable
	{
	public:
		PyObjectPtr const& GetPyObject() const
		{
			return val_;
		}

		void SetPyObject(PyObjectPtr const& value)
		{
			val_ = value;
		}

	protected:
		PyObjectPtr val_;
	};

	class PythonScriptVariableString final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(std::string const& value) override
		{
			return this->operator=(value.c_str());
		}

		ScriptVariable& operator=(std::string_view value) override
		{
			return this->operator=(std::string(value));
		}

		ScriptVariable& operator=(char const* value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("s", value));
			return *this;
		}

		ScriptVariable& operator=(char* value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("s", value));
			return *this;
		}

		bool TryValue(std::string& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyUnicode_Type))
			{
				value = std::string(PyBytes_AsString(PyUnicode_AsASCIIString(val_.get())));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableWString final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(std::wstring const& value) override
		{
			return this->operator=(value.c_str());
		}

		ScriptVariable& operator=(std::wstring_view value) override
		{
			return this->operator=(std::wstring(value));
		}

		ScriptVariable& operator=(wchar_t const* value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("u", value));
			return *this;
		}

		ScriptVariable& operator=(wchar_t* value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("u", value));
			return *this;
		}

		bool TryValue(std::wstring& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyUnicode_Type))
			{
				Py_ssize_t size;
				wchar_t* str = PyUnicode_AsWideCharString(val_.get(), &size);
				value = std::wstring(str, str + size);
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableInt8 final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(int8_t value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("b", value));
			return *this;
		}

		bool TryValue(int8_t& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyLong_Type))
			{
				value = static_cast<int8_t>(PyLong_AsLong(val_.get()));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableInt16 final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(int16_t value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("h", value));
			return *this;
		}

		bool TryValue(int16_t& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyLong_Type))
			{
				value = static_cast<int16_t>(PyLong_AsLong(val_.get()));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableInt32 final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(int32_t value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("i", value));
			return *this;
		}

		bool TryValue(int32_t& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyLong_Type))
			{
				value = static_cast<int32_t>(PyLong_AsLong(val_.get()));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableInt64 final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(int64_t value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("L", value));
			return *this;
		}

		bool TryValue(int64_t& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyLong_Type))
			{
				value = static_cast<int64_t>(PyLong_AsLongLong(val_.get()));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableUInt8 final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(uint8_t value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("B", value));
			return *this;
		}

		bool TryValue(uint8_t& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyLong_Type))
			{
				value = static_cast<uint8_t>(PyLong_AsUnsignedLong(val_.get()));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableUInt16 final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(uint16_t value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("H", value));
			return *this;
		}

		bool TryValue(uint16_t& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyLong_Type))
			{
				value = static_cast<uint16_t>(PyLong_AsUnsignedLong(val_.get()));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableUInt32 final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(uint32_t value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("I", value));
			return *this;
		}

		bool TryValue(uint32_t& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyLong_Type))
			{
				value = static_cast<uint32_t>(PyLong_AsUnsignedLong(val_.get()));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableUInt64 final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(uint64_t value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("K", value));
			return *this;
		}

		bool TryValue(uint64_t& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyLong_Type))
			{
				value = static_cast<uint64_t>(PyLong_AsUnsignedLongLong(val_.get()));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableFloat final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(float value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("f", value));
			return *this;
		}

		bool TryValue(float& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyFloat_Type))
			{
				value = static_cast<float>(PyFloat_AsDouble(val_.get()));
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableDouble final : public PythonScriptVariable
	{
	public:
		ScriptVariable& operator=(double value) override
		{
			val_ = MakePyObjectPtr(Py_BuildValue("d", value));
			return *this;
		}

		bool TryValue(double& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyFloat_Type))
			{
				value = PyFloat_AsDouble(val_.get());
				return true;
			}
			return false;
		}
	};

	class PythonScriptVariableVector final : public PythonScriptVariable
	{
	public:
		PythonScriptVariableVector(PythonScriptModule const& script_module) : script_module_(script_module)
		{
		}

		ScriptVariable& operator=(std::span<ScriptVariablePtr const> value) override
		{
			val_ = MakePyObjectPtr(PyTuple_New(value.size()));
			for (size_t i = 0; i < value.size(); ++i)
			{
				PyObjectPtr py_item = checked_cast<PythonScriptVariable&>(*value[i]).GetPyObject();
				Py_IncRef(py_item.get());
				PyTuple_SetItem(val_.get(), i, py_item.get());
			}
			return *this;
		}

		bool TryValue(std::vector<ScriptVariablePtr>& value) const override
		{
			if (PyObject_TypeCheck(val_.get(), &PyList_Type))
			{
				size_t len = PyList_Size(val_.get());
				value.resize(len);
				for (size_t i = 0; i < len; ++i)
				{
					PyObjectPtr py_item = MakePyObjectPtr(PyList_GetItem(val_.get(), i));
					Py_IncRef(py_item.get());
					value[i] = script_module_.MakeVariable(py_item);
				}

				return true;
			}
			else if (PyObject_TypeCheck(val_.get(), &PyTuple_Type))
			{
				size_t len = PyTuple_Size(val_.get());
				value.resize(len);
				for (size_t i = 0; i < len; ++i)
				{
					PyObjectPtr py_item = MakePyObjectPtr(PyTuple_GetItem(val_.get(), i));
					Py_IncRef(py_item.get());
					value[i] = script_module_.MakeVariable(py_item);
				}

				return true;
			}

			return false;
		}

	private:
		PythonScriptModule const& script_module_;
	};


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

	ScriptVariablePtr PythonScriptModule::Value(std::string const & name)
	{
		PyObjectPtr p = MakePyObjectPtr(PyDict_GetItemString(dict_.get(), name.c_str()));
		Py_IncRef(p.get());
		return this->MakeVariable(p);
	}

	ScriptVariablePtr PythonScriptModule::Call(std::string const & func_name, std::span<ScriptVariablePtr const> args)
	{
		PyObjectPtr py_args = MakePyObjectPtr(PyTuple_New(args.size()));
		for (size_t i = 0; i < args.size(); ++i)
		{
			PyObjectPtr py_value = checked_cast<PythonScriptVariable&>(*args[i]).GetPyObject();
			Py_IncRef(py_value.get());
			PyTuple_SetItem(py_args.get(), i, py_value.get());
		}

		PyObjectPtr func = checked_cast<PythonScriptVariable&>(*this->Value(func_name)).GetPyObject();
		return this->MakeVariable(MakePyObjectPtr(PyObject_CallObject(func.get(), py_args.get())));
	}

	ScriptVariablePtr PythonScriptModule::RunString(std::string const & script)
	{
		return this->MakeVariable(MakePyObjectPtr(PyRun_String(script.c_str(), Py_file_input, dict_.get(), dict_.get())));
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(std::string const& value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableString>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(std::string_view value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableString>();
		*ret = std::move(value);
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(char const* value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableString>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(char* value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableString>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(std::wstring const& value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableWString>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(std::wstring_view value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableWString>();
		*ret = std::move(value);
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(wchar_t const* value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableWString>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(wchar_t* value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableWString>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(int8_t value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableInt8>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(int16_t value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableInt16>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(int32_t value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableInt32>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(int64_t value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableInt64>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(uint8_t value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableUInt8>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(uint16_t value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableUInt16>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(uint32_t value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableUInt32>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(uint64_t value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableUInt64>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(float value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableFloat>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(double value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableDouble>();
		*ret = value;
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(std::span<ScriptVariablePtr const> value) const
	{
		auto ret = MakeSharedPtr<PythonScriptVariableVector>(*this);
		*ret = std::move(value);
		return ret;
	}

	ScriptVariablePtr PythonScriptModule::MakeVariable(PyObjectPtr const& value) const
	{
		if (PyObject_TypeCheck(value.get(), &PyUnicode_Type))
		{
			return this->MakeVariable(std::string(PyBytes_AsString(PyUnicode_AsASCIIString(value.get()))));
		}
		else if (PyObject_TypeCheck(value.get(), &PyLong_Type))
		{
			return this->MakeVariable(static_cast<int32_t>(PyLong_AsLong(value.get())));
		}
		else if (PyObject_TypeCheck(value.get(), &PyFloat_Type))
		{
			return this->MakeVariable(static_cast<float>(PyFloat_AsDouble(value.get())));
		}
		else if (PyObject_TypeCheck(value.get(), &PyList_Type))
		{
			size_t const len = PyList_Size(value.get());
			std::vector<ScriptVariablePtr> v(len);
			for (size_t i = 0; i < len; ++i)
			{
				PyObjectPtr py_obj = MakePyObjectPtr(PyList_GetItem(value.get(), i));
				Py_IncRef(py_obj.get());
				v[i] = this->MakeVariable(py_obj);
			}
			return this->MakeVariable(v);
		}
		else if (PyObject_TypeCheck(value.get(), &PyTuple_Type))
		{
			size_t const len = PyTuple_Size(value.get());
			std::vector<ScriptVariablePtr> v(len);
			for (size_t i = 0; i < len; ++i)
			{
				PyObjectPtr py_obj = MakePyObjectPtr(PyTuple_GetItem(value.get(), i));
				Py_IncRef(py_obj.get());
				v[i] = this->MakeVariable(py_obj);
			}
			return this->MakeVariable(v);
		}
		else
		{
			auto ret = MakeSharedPtr<PythonScriptVariable>();
			ret->SetPyObject(value);
			return ret;
		}
	}


	PythonEngine::PythonEngine()
	{
		Py_NoSiteFlag = 1;

		PyPreConfig preconfig;
		PyPreConfig_InitPythonConfig(&preconfig);

		preconfig.utf8_mode = 1;

		PyStatus status = Py_PreInitialize(&preconfig);
		if (PyStatus_Exception(status))
		{
			Py_ExitStatusException(status);
		}

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
