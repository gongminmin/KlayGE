/**
 * @file KFL.hpp
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

#ifndef _KFL_KFL_HPP
#define _KFL_KFL_HPP

#pragma once

#include <KFL/Config.hpp>
#include <KFL/Types.hpp>

#include <vector>
#include <string>

#include <boost/assert.hpp>

#include <KFL/PreDeclare.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>

#ifndef KFL_SOURCE
	#ifdef KLAYGE_COMPILER_MSVC
		#ifdef KLAYGE_DEBUG
			#define DEBUG_SUFFIX "_d"
		#else
			#define DEBUG_SUFFIX ""
		#endif

		#define LIB_FILE_NAME "KFL_" KFL_STRINGIZE(KLAYGE_COMPILER_NAME) "_" KFL_STRINGIZE(KLAYGE_COMPILER_TARGET) DEBUG_SUFFIX ".lib"

		#pragma comment(lib, LIB_FILE_NAME)
		//#pragma message("Linking to lib file: " LIB_FILE_NAME)
		#undef LIB_FILE_NAME
		#undef DEBUG_SUFFIX
	#endif	// KLAYGE_COMPILER_MSVC
#endif	// KFL_SOURCE

#endif		// _KFL_KFL_HPP
