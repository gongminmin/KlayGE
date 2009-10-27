// Script.hpp
// KlayGE 脚本引擎类 头文件
// Ver 2.1.3
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://klayge.sourceforge.net
//
// 2.1.3
// 增加了以tuple为参数的Call (2004.9.15)
//
// 修改记录
////////////////////////////////////////////////////////////////////////////

#ifndef _SCRIPT_HPP
#define _SCRIPT_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <Python.h>
#include <vector>
#include <string>

#include <boost/noncopyable.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011)
#endif
#include <boost/shared_ptr.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <boost/tuple/tuple.hpp>

namespace KlayGE
{
	class PyObjDeleter
	{
	public:
		void operator()(PyObject* p)
		{
			if (p != NULL)
			{
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
				Py_DECREF(p);
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
			}
		}
	};

	typedef boost::shared_ptr<PyObject> PyObjectPtr;

	// PyObject指针
	/////////////////////////////////////////////////////////////////////////////////
	inline PyObjectPtr
	MakePyObjectPtr(PyObject* p)
	{
		return PyObjectPtr(p, PyObjDeleter());
	};

	template <typename T>
	PyObjectPtr CppType2PyObjectPtr(T const &);

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<std::string>(std::string const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("s", t.c_str()));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<char*>(char* const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("s", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<wchar_t*>(wchar_t* const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("u", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<int8_t>(int8_t const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("b", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<int16_t>(int16_t const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("h", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<int32_t>(int32_t const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("i", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<int64_t>(int64_t const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("L", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<uint8_t>(uint8_t const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("B", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<uint16_t>(uint16_t const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("H", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<uint32_t>(uint32_t const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("I", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<uint64_t>(uint64_t const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("K", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<double>(double const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("d", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<float>(float const & t)
	{
		return MakePyObjectPtr(Py_BuildValue("f", t));
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<PyObject*>(PyObject* const & t)
	{
		return MakePyObjectPtr(t);
	}

	template <>
	inline PyObjectPtr CppType2PyObjectPtr<PyObjectPtr>(PyObjectPtr const & t)
	{
		return t;
	}

	// 从一个.py载入模块
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API ScriptModule
	{
	private:
		template <typename TupleType>
		std::vector<PyObjectPtr> Tuple2Vector(TupleType const & t)
		{
			std::vector<PyObjectPtr> ret;
			ret.push_back(CppType2PyObjectPtr(boost::tuples::get<0>(t)));

			std::vector<PyObjectPtr> tail(Tuple2Vector(t.get_tail()));
			ret.insert(ret.end(), tail.begin(), tail.end());

			return ret;
		}

	public:
		explicit ScriptModule(std::string const & name);
		~ScriptModule();

		PyObjectPtr Value(std::string const & name);

		template <typename TupleType>
		PyObjectPtr Call(std::string const & funcName, const TupleType& t)
		{
			std::vector<PyObjectPtr> v(Tuple2Vector(t));
			return this->Call(funcName, v.begin(), v.end());
		}

		template <typename ForwardIterator>
		PyObjectPtr Call(std::string const & funcName, ForwardIterator first, ForwardIterator last)
		{
			PyObjectPtr func(this->Value(funcName));
			PyObjectPtr args(MakePyObjectPtr(PyTuple_New(last - first)));

			for (ForwardIterator iter = first; iter != last; ++ iter)
			{
				Py_INCREF(iter->get());
				PyTuple_SetItem(args.get(), iter - first, iter->get());
			}

			return MakePyObjectPtr(PyObject_CallObject(func.get(), args.get()));
		}

	private:
		PyObjectPtr module_;
		PyObjectPtr dict_;
	};

	template <>
	inline std::vector<PyObjectPtr>
	ScriptModule::Tuple2Vector<boost::tuples::null_type>(boost::tuples::null_type const & /*t*/)
	{
		return std::vector<PyObjectPtr>();
	}

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
	class KLAYGE_CORE_API RegisterModule
	{
	public:
		typedef PyObject *(*PyCallback)(PyObject*, PyObject*);

		RegisterModule(std::string const & name)
			: moduleName_(name)
			{ }

		std::string const & Name() const
			{ return moduleName_;}

		void AddMethod(std::string const & methodName, PyCallback method);
		void Regiter();

	protected:
		std::string moduleName_;

		std::vector<PyMethodDef>	methods_;
		std::vector<std::string>	methodNames_;
	};


	// 实现脚本引擎的功能
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API ScriptEngine : boost::noncopyable
	{
	public:
		ScriptEngine();
		~ScriptEngine();

		// 从字符串运行脚本
		void ExecString(std::string const & script);
	};
}

#endif		// _SCRIPT_HPP
