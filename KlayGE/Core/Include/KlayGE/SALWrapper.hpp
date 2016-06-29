/**
 * @file SALWrapper.hpp
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

#ifndef _SALWRAPPER_HPP
#define _SALWRAPPER_HPP

#pragma once

#if defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
	#define __bcount(size)
	#define __in
	#define __in_bcount(size)
	#define __in_ecount(size)
	#define __in_opt
	#define __in_ecount_opt(size)
	#define __in_bcount_opt(size)
	#define __in_xcount_opt(size) 
	#define __out
	#define __out_bcount(size)
	#define __out_ecount(size)
	#define __out_opt
	#define __out_ecount_opt(size)
	#define __out_ecount_part_opt(size,length)
	#define __out_bcount_opt(size)
	#define __inout
	#define __inout_opt
	#define __deref_out
	#define __deref_out_bcount(size)
	#define __deref_opt_out_bcount(size)

	#define _In_
	#define _In_z_
	#define _In_reads_(size)
	#define _In_reads_opt_(size)
	#define _In_reads_bytes_(size)
	#define _In_reads_bytes_opt_(size)
	#define _In_range_(lb, ub)
	#define _Out_
	#define _Out_opt_
	#define _Out_writes_(size)
	#define _Out_writes_opt_(size)
	#define _Out_writes_bytes_(size)
	#define _Out_writes_bytes_to_(size, count)
	#define _Out_writes_bytes_opt_(size)
	#define _Out_writes_to_opt_(size, count)
	#define _Out_writes_all_(size)
	#define _Out_writes_all_opt_(size)
	#define _Inout_
	#define _Inout_opt_
	#define _Inout_updates_bytes_(size)
	#define _Inout_opt_bytecount_(size)
	#define _Outptr_
	#define _Outptr_result_bytebuffer_(size)
	#define _Outptr_result_maybenull_
	#define _Outptr_opt_
	#define _Outptr_opt_result_maybenull_
	#define _Outptr_opt_result_bytebuffer_(size)
	#define _Check_return_
	#define _Pre_null_
	#define _Always_(annos)
	#define _COM_Outptr_
	#define _COM_Outptr_opt_
	#define _COM_Outptr_opt_result_maybenull_
	#define _Field_size_(size)
	#define _Field_size_opt_(size)
	#define _Field_size_full_(size)
	#define _Field_size_bytes_full_(size)
#endif

#endif			// _SALWRAPPER_HPP
