// Script.hpp
// KlayGE 脚本引擎类 头文件
// Ver 2.1.3
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.klayge.org
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
	typedef boost::shared_ptr<PyObject> PyObjectPtr;

	// PyObject指针
	/////////////////////////////////////////////////////////////////////////////////
	KLAYGE_CORE_API PyObjectPtr MakePyObjectPtr(PyObject* p);

	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(std::string const & t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(char* t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(wchar_t* t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(int8_t t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(int16_t t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(int32_t t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(int64_t t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(uint8_t t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(uint16_t t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(uint32_t t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(uint64_t t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(double t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(float t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(PyObject* t);
	KLAYGE_CORE_API PyObjectPtr CppType2PyObjectPtr(PyObjectPtr const & t);

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
		ScriptModule();
		explicit ScriptModule(std::string const & name);
		~ScriptModule();

		PyObjectPtr Value(std::string const & name);

		template <typename TupleType>
		PyObjectPtr Call(std::string const & func_name, TupleType const & t)
		{
			std::vector<PyObjectPtr> v(Tuple2Vector(t));
			return this->Call(func_name, &v.front(), &v.back() + 1);
		}

		PyObjectPtr Call(std::string const & func_name, PyObjectPtr* first, PyObjectPtr* last);

		PyObjectPtr RunString(std::string const & script);

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

	// 注册一个可以在Python中调用的模块
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API RegisterModule
	{
	public:
		typedef PyObject *(*PyCallback)(PyObject*, PyObject*);

		RegisterModule(std::string const & name)
			: module_name_(name)
		{
		}

		std::string const & Name() const
		{
			return module_name_;
		}

		void AddMethod(std::string const & method_name, PyCallback method);
		void Regiter();

	protected:
		std::string module_name_;

		std::vector<PyMethodDef>	methods_;
		std::vector<std::string>	method_names_;
	};


	// 实现脚本引擎的功能
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API ScriptEngine : boost::noncopyable
	{
	public:
		ScriptEngine();
		~ScriptEngine();

		// 从字符串运行脚本
		void RunString(std::string const & script);
	};
}

#endif		// _SCRIPT_HPP
