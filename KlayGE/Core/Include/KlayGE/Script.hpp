// Script.hpp
// KlayGE 脚本引擎类 头文件
// Ver 1.2.8.11
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.enginedev.com
//
//
// 修改记录
////////////////////////////////////////////////////////////////////////////

#ifndef _SCRIPT_HPP
#define _SCRIPT_HPP

#include <python.h>
#include <vector>

#include <KlayGE/ResPtr.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	// PyObject引用计数策略
	/////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class PyObjectRefCountedOP
	{
	public:
		PyObjectRefCountedOP()
			{ }

		static T Clone(const T& p)
		{
			if (p != 0)
			{
				Py_INCREF(p);
			}
			return p;
		}
		static bool Release(const T& p)
		{
			if (p != 0)
			{
				Py_DECREF(p);
			}
			return false;
		}

		void Swap(PyObjectRefCountedOP& /*rhs*/)
			{ }
	};

	// PyObject指针
	/////////////////////////////////////////////////////////////////////////////////
	class PyObjectPtr : public ResPtr<PyObject, PyObjectRefCountedOP>
	{
	public:
		PyObjectPtr()
			: ResPtr<PyObject, PyObjectRefCountedOP>()
			{ }
		explicit PyObjectPtr(PyObject* p)
			: ResPtr<PyObject, PyObjectRefCountedOP>(p)
			{ }
		PyObjectPtr(const PyObjectPtr& rhs)
			: ResPtr<PyObject, PyObjectRefCountedOP>(rhs)
			{ }
	};

	// 从一个.py载入模块
	/////////////////////////////////////////////////////////////////////////////////
	class ScriptModule
	{
	public:
		ScriptModule(const String& name)
		{
			module_	= PyObjectPtr(PyImport_Import(PyString_FromString(name.c_str())));
			dict_	= PyObjectPtr(PyModule_GetDict(module_.Get()));
		}

		PyObjectPtr Value(const String& name)
		{
			return PyObjectPtr(PyDict_GetItemString(dict_.Get(), name.c_str()));
		}

		template <typename ForwardIterator>
		PyObjectPtr Call(const String& funcName, ForwardIterator first, ForwardIterator last)
		{
			PyObjectPtr func(this->Value(funcName));
			PyObjectPtr args(PyTuple_New(last - first));

			for (ForwardIterator iter = first; iter != last; ++ iter)
			{
				PyTuple_SetItem(args.Get(), iter - first, iter->Get());
			}

			return PyObjectPtr(PyObject_CallObject(func.Get(), args.Get()));
		}

	private:
		PyObjectPtr module_;
		PyObjectPtr dict_;
	};

	#define BEGIN_REG() 	methods.clear() 
	#define ADD(x, y) 		AddMethod(x,y) 
	#define END_REG() \
				do\
				{\
					PyMethodDef __tmp = {NULL, NULL, 0, NULL};\
					methods.push_back(__tmp);\
				} while(0)

	// 注册一个可以在Python中调用的模块
	/////////////////////////////////////////////////////////////////////////////////
	class RegisterModule
	{
	public:
		typedef PyObject *(*PyCallback)(PyObject*, PyObject*);

		RegisterModule(const String& name)
			: moduleName_(name)
			{ }

		const String& Name() const
			{ return moduleName_;}

		void AddMethod(const String& methodName, PyCallback method);
		void Regiter();

	protected:
		String moduleName_;

		std::vector<PyMethodDef, alloc<PyMethodDef> >	methods_;
		std::vector<String, alloc<String> >				methodNames_;
	};


	// 实现脚本引擎的功能
	/////////////////////////////////////////////////////////////////////////////////
	class ScriptEngine
	{
	public:
		ScriptEngine();
		~ScriptEngine();

		// 从字符串运行脚本
		void ExecString(const String& script);

	private:
		ScriptEngine(const ScriptEngine&);
		ScriptEngine& operator=(const ScriptEngine&);
	};
}

#endif		// _SCRIPT_HPP