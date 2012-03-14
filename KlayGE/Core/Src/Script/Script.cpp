// Script.cpp
// KlayGE 脚本引擎类 实现文件
// Ver 1.2.8.11
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.klayge.org
//
// 1.2.8.9
// 修正了调用两次Close会出错的问题 (2002.10.11)
//
// 1.2.8.10
// 用string代替了字符串指针 (2002.10.27)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>

#include <boost/assert.hpp>

#include <KlayGE/Script.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#define PYTHON_VER_STR KLAYGE_STRINGIZE(PY_MAJOR_VERSION)KLAYGE_STRINGIZE(PY_MINOR_VERSION)

#ifdef KLAYGE_DEBUG
#define PYTHON_DBG_SUFFIX "_d"
#else
#define PYTHON_DBG_SUFFIX ""
#endif

#define PYTHON_LIB_STR KLAYGE_STRINGIZE("python")KLAYGE_STRINGIZE(FREETYPE_VER_STR)KLAYGE_STRINGIZE(FREETYPE_DBG_SUFFIX)KLAYGE_STRINGIZE(".lib")
#endif

namespace KlayGE
{
	class KLAYGE_CORE_API PyObjDeleter
	{
	public:
		void operator()(PyObject* p)
		{
			if (p != NULL)
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


	ScriptModule::ScriptModule()
	{
		module_	= MakePyObjectPtr(PyImport_AddModule("__main__"));
		dict_	= MakePyObjectPtr(PyModule_GetDict(module_.get()));
		Py_IncRef(dict_.get());
	}

	ScriptModule::ScriptModule(std::string const & name)
	{
		module_	= MakePyObjectPtr(PyImport_ImportModule(name.c_str()));
		dict_	= MakePyObjectPtr(PyModule_GetDict(module_.get()));
		Py_IncRef(dict_.get());
	}

	ScriptModule::~ScriptModule()
	{
		dict_.reset();
		module_.reset();
	}

	PyObjectPtr ScriptModule::Value(std::string const & name)
	{
		PyObject* p = PyDict_GetItemString(dict_.get(), name.c_str());
		Py_IncRef(p);
		return MakePyObjectPtr(p);
	}

	PyObjectPtr ScriptModule::Call(std::string const & func_name, PyObjectPtr* first, PyObjectPtr* last)
	{
		PyObjectPtr func(this->Value(func_name));
		PyObjectPtr args(MakePyObjectPtr(PyTuple_New(last - first)));

		for (PyObjectPtr* iter = first; iter != last; ++ iter)
		{
			Py_IncRef(iter->get());
			PyTuple_SetItem(args.get(), iter - first, iter->get());
		}

		return MakePyObjectPtr(PyObject_CallObject(func.get(), args.get()));
	}

	PyObjectPtr ScriptModule::RunString(std::string const & script)
	{
		return MakePyObjectPtr(PyRun_String(script.c_str(), Py_file_input, dict_.get(), dict_.get()));
	}


	// 向模块声明中添加一个方法
	/////////////////////////////////////////////////////////////////////////////////
	void RegisterModule::AddMethod(std::string const & method_name, PyCallback method)
	{
		method_names_.push_back(method_name);

		PyMethodDef def = { method_name.c_str(), method, METH_VARARGS, NULL };
		methods_.push_back(def);
	}

	// 把模块注册给Python
	/////////////////////////////////////////////////////////////////////////////////
	void RegisterModule::Regiter()
	{
#if PY_MAJOR_VERSION >= 3
		static PyModuleDef module =
		{
			PyModuleDef_HEAD_INIT,
			module_name_.c_str(),
			NULL,
			-1,
			&methods_[0]
		};

		PyModule_Create(&module);
#else
		Py_InitModule(const_cast<char*>(module_name_.c_str()), &methods_[0]);
#endif
	}


	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	ScriptEngine::ScriptEngine()
	{
		Py_Initialize();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	ScriptEngine::~ScriptEngine()
	{
		Py_Finalize();
	}

	// 从字符串运行脚本
	void ScriptEngine::RunString(std::string const & script)
	{
		PyRun_SimpleString(script.c_str());
	}
}
