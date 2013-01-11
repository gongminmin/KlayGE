/**
 * @file COMPtr.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
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

#ifndef _KFL_COMPTR_HPP
#define _KFL_COMPTR_HPP

#pragma once

#ifdef KLAYGE_PLATFORM_WIN32
	#ifndef KLAYGE_CPU_ARM
		#ifndef BOOST_MEM_FN_ENABLE_STDCALL
			#define BOOST_MEM_FN_ENABLE_STDCALL
		#endif
	#endif
#endif
#include <boost/mem_fn.hpp>

namespace KlayGE
{
	// 得到COM对象的智能指针
	template <typename T>
	inline boost::shared_ptr<T>
	MakeCOMPtr(T* p)
	{
		return p ? boost::shared_ptr<T>(p, boost::mem_fn(&T::Release)) : boost::shared_ptr<T>();
	}
}

#endif		// _KFL_COMPTR_HPP
