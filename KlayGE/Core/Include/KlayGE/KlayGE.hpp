/**
 * @file KlayGE.hpp
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

#ifndef _KLAYGE_HPP
#define _KLAYGE_HPP

#pragma once

#include <KFL/KFL.hpp>

#define KLAYGE_NAME			KlayGE
#define KLAYGE_VER_MAJOR	4
#define KLAYGE_VER_MINOR	12
#define KLAYGE_VER_RELEASE	0
#define KLAYGE_VER_STR		KFL_STRINGIZE(KLAYGE_NAME) " " KFL_STRINGIZE(KLAYGE_VER_MAJOR) "." KFL_STRINGIZE(KLAYGE_VER_MINOR) "." KFL_STRINGIZE(KLAYGE_VER_RELEASE)

#define KLAYGE_COMPILER_TOOLSET KFL_STRINGIZE(KFL_JOIN(KLAYGE_COMPILER_NAME, KLAYGE_COMPILER_VERSION))

#ifndef KLAYGE_CORE_SOURCE
	#define KLAYGE_LIB_NAME KlayGE_Core
	#include <KFL/Detail/AutoLink.hpp>
#endif

#ifdef KLAYGE_CORE_SOURCE		// Build dll
	#define KLAYGE_CORE_API KLAYGE_SYMBOL_EXPORT
#else							// Use dll
	#define KLAYGE_CORE_API KLAYGE_SYMBOL_IMPORT
#endif

#include <vector>
#include <string>

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Context.hpp>

#endif		// _KLAYGE_HPP
