// Script.cpp
// KlayGE 脚本引擎类 实现文件
// Ver 1.2.8.11
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.enginedev.com
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

#include <cassert>

#include <KlayGE/Script.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "python24d.lib")
#else
	#pragma comment(lib, "python24.lib")
#endif

namespace KlayGE
{
	// 向模块声明中添加一个方法
	/////////////////////////////////////////////////////////////////////////////////
	void RegisterModule::AddMethod(std::string const & methodName, PyCallback method)
	{
		methodNames_.push_back(methodName);

		PyMethodDef def = { const_cast<char*>(methodName.c_str()), method, METH_VARARGS, NULL };
		methods_.push_back(def);
	}

	// 把模块注册给Python
	/////////////////////////////////////////////////////////////////////////////////
	void RegisterModule::Regiter()
	{
		Py_InitModule(const_cast<char*>(moduleName_.c_str()), &methods_[0]);
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
	void ScriptEngine::ExecString(std::string const & script)
	{
		PyRun_SimpleString(script.c_str());
	}
}
