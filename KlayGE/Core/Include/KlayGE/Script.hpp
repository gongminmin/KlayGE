/**
 * @file Script.hpp
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

#ifndef _SCRIPT_HPP
#define _SCRIPT_HPP

#pragma once

#include <vector>
#include <string>

#include <KFL/CXX17/any.hpp>
#include <KFL/ArrayRef.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ScriptModule : boost::noncopyable
	{
	public:
		ScriptModule();
		virtual ~ScriptModule();

		virtual std::any Value(std::string const & name);
		virtual std::any Call(std::string const & func_name, ArrayRef<std::any> args);
		virtual std::any RunString(std::string const & script);
	};

	typedef std::shared_ptr<ScriptModule> ScriptModulePtr;

	// 实现脚本引擎的功能
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API ScriptEngine : boost::noncopyable
	{
	public:
		ScriptEngine();
		virtual ~ScriptEngine();

		void Suspend();
		void Resume();

		// 创建一个新的脚本模块
		virtual ScriptModulePtr CreateModule(std::string const & name);

	private:
		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;
	};
}

#endif		// _SCRIPT_HPP
