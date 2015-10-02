/**
 * @file D3D12MinGWDefs.hpp
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

#ifndef _D3D12MINGWDEFS_HPP
#define _D3D12MINGWDEFS_HPP

#pragma once

#if defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
	#define __bcount(size)
	#define __in
	#define __in_ecount(size)
	#define __out
	#define __out_ecount(size)
	#define __out_bcount(size)
	#define __inout
	#define __in_bcount(size)
	#define __in_opt
	#define __in_ecount_opt(size)
	#define __in_bcount_opt(size)
	#define __out_opt
	#define __out_ecount_opt(size)
	#define __out_bcount_opt(size)
	#define __inout_opt
	#define __out_ecount_part_opt(size,length)
	#define __in_xcount_opt(size) 

	#define WINAPI_INLINE  WINAPI

	typedef unsigned char UINT8;
#endif

#endif			// _D3D12MINGWDEFS_HPP
