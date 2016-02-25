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

#include <boost/noncopyable.hpp>
#ifdef KLAYGE_TS_LIBRARY_ANY_SUPPORT
	#include <experimental/any>
#else
	#include <boost/any.hpp>
	namespace std
	{
		namespace experimental
		{
			using boost::any;
			using boost::any_cast;
			using boost::bad_any_cast;
		}
	}
#endif

namespace KlayGE
{
	typedef std::vector<std::experimental::any> AnyDataListType;

	class KLAYGE_CORE_API ScriptModule
	{
	private:
		template <typename tuple_type, int N>
		struct Tuple2Vector
		{
			static AnyDataListType Do(tuple_type const & t)
			{
				AnyDataListType ret = Tuple2Vector<tuple_type, N - 1>::Do(t);
				ret.push_back(std::get<N - 1>(t));
				return ret;
			}
		};

		template <typename tuple_type>
		struct Tuple2Vector<tuple_type, 1>
		{
			static AnyDataListType Do(tuple_type const & t)
			{
				return AnyDataListType(1, std::get<0>(t));
			}
		};

	public:
		ScriptModule();
		virtual ~ScriptModule();

		template <typename TupleType>
		std::experimental::any Call(std::string const & func_name, TupleType const & t)
		{
			AnyDataListType v(Tuple2Vector<TupleType, std::tuple_size<TupleType>::value>::Do(t));
			return Call(func_name, v);
		}

		virtual std::experimental::any Value(std::string const & name);
		virtual std::experimental::any Call(std::string const & func_name, const AnyDataListType& args);
		virtual std::experimental::any RunString(std::string const & script);
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
