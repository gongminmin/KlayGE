/**
 * @file Script.cpp
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
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>

#include <boost/assert.hpp>

#include <KlayGE/Script.hpp>

namespace KlayGE
{
	class NullScriptEngine : public ScriptEngine
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring name(L"Null Script Engine");
			return name;
		}
	};

	ScriptModule::ScriptModule()
	{
	}

	ScriptModule::~ScriptModule()
	{
	}

	boost::any ScriptModule::Value(std::string const & /*name*/)
	{
		return boost::any();
	}

	boost::any ScriptModule::Call(std::string const & /*func_name*/, const AnyDataListType& /*args*/)
	{
		return boost::any();
	}

	boost::any ScriptModule::RunString(std::string const & /*script*/)
	{
		return boost::any();
	}

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	ScriptEngine::ScriptEngine()
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	ScriptEngine::~ScriptEngine()
	{
	}

	// 返回空对象
	//////////////////////////////////////////////////////////////////////////////////
	ScriptEnginePtr ScriptEngine::NullObject()
	{
		static ScriptEnginePtr obj = MakeSharedPtr<NullScriptEngine>();
		return obj;
	}

	ScriptModulePtr ScriptEngine::CreateModule(std::string const & /*name*/)
	{
		static ScriptModulePtr obj = MakeSharedPtr<ScriptModule>();
		return obj;
	}
}
