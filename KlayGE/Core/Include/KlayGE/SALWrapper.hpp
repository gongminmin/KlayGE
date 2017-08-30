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

#ifndef __bcount
	#define __bcount(size)
#endif
#ifndef __in
	#define __in
#endif
#ifndef __in_bcount
	#define __in_bcount(size)
#endif
#ifndef __in_ecount
	#define __in_ecount(size)
#endif
#ifndef __in_opt
	#define __in_opt
#endif
#ifndef __in_ecount_opt
	#define __in_ecount_opt(size)
#endif
#ifndef __in_bcount_opt
	#define __in_bcount_opt(size)
#endif
#ifndef __in_xcount_opt
	#define __in_xcount_opt(size) 
#endif
#ifndef __out
	#define __out
#endif
#ifndef __out_bcount
	#define __out_bcount(size)
#endif
#ifndef __out_ecount
	#define __out_ecount(size)
#endif
#ifndef __out_opt
	#define __out_opt
#endif
#ifndef __out_ecount_opt
	#define __out_ecount_opt(size)
#endif
#ifndef __out_ecount_part_opt
	#define __out_ecount_part_opt(size,length)
#endif
#ifndef __out_bcount_opt
	#define __out_bcount_opt(size)
#endif
#ifndef __inout
	#define __inout
#endif
#ifndef __inout_opt
	#define __inout_opt
#endif
#ifndef __deref_out
	#define __deref_out
#endif
#ifndef __deref_out_bcount
	#define __deref_out_bcount(size)
#endif
#ifndef __deref_opt_out_bcount
	#define __deref_opt_out_bcount(size)
#endif
#ifndef __reserved
	#define __reserved
#endif

#ifndef _Out_writes_bytes_opt_
	#define _Out_writes_bytes_opt_(size)
#endif
#ifndef _Inout_opt_bytecount_
	#define _Inout_opt_bytecount_(size)
#endif
#ifndef _Pre_null_
	#define _Pre_null_
#endif

#ifndef _In_
	#define _In_
#endif
#ifndef _In_z_
	#define _In_z_
#endif
#ifndef _In_reads_
	#define _In_reads_(size)
#endif
#ifndef _In_reads_opt_
	#define _In_reads_opt_(size)
#endif
#ifndef _In_reads_bytes_
	#define _In_reads_bytes_(size)
#endif
#ifndef _In_reads_bytes_opt_
	#define _In_reads_bytes_opt_(size)
#endif
#ifndef _In_range_
	#define _In_range_(lb, ub)
#endif
#ifndef _Out_
	#define _Out_
#endif
#ifndef _Out_opt_
	#define _Out_opt_
#endif
#ifndef _Out_writes_
	#define _Out_writes_(size)
#endif
#ifndef _Out_writes_opt_
	#define _Out_writes_opt_(size)
#endif
#ifndef _Out_writes_bytes_
	#define _Out_writes_bytes_(size)
#endif
#ifndef _Out_writes_bytes_to_
	#define _Out_writes_bytes_to_(size, count)
#endif
#ifndef _Out_writes_to_opt_
	#define _Out_writes_to_opt_(size, count)
#endif
#ifndef _Out_writes_all_
	#define _Out_writes_all_(size)
#endif
#ifndef _Out_writes_all_opt_
	#define _Out_writes_all_opt_(size)
#endif
#ifndef _Inout_
	#define _Inout_
#endif
#ifndef _Inout_opt_
	#define _Inout_opt_
#endif
#ifndef _Inout_updates_bytes_
	#define _Inout_updates_bytes_(size)
#endif
#ifndef _Outptr_
	#define _Outptr_
#endif
#ifndef _Outptr_result_bytebuffer_
	#define _Outptr_result_bytebuffer_(size)
#endif
#ifndef _Outptr_result_maybenull_
	#define _Outptr_result_maybenull_
#endif
#ifndef _Outptr_opt_
	#define _Outptr_opt_
#endif
#ifndef _Outptr_opt_result_maybenull_
	#define _Outptr_opt_result_maybenull_
#endif
#ifndef _Outptr_opt_result_bytebuffer_
	#define _Outptr_opt_result_bytebuffer_(size)
#endif
#ifndef _Check_return_
	#define _Check_return_
#endif
#ifndef _Always_
	#define _Always_(annos)
#endif
#ifndef _COM_Outptr_
	#define _COM_Outptr_
#endif
#ifndef _COM_Outptr_opt_
	#define _COM_Outptr_opt_
#endif
#ifndef _COM_Outptr_opt_result_maybenull_
	#define _COM_Outptr_opt_result_maybenull_
#endif
#ifndef _Field_size_
	#define _Field_size_(size)
#endif
#ifndef _Field_size_opt_
	#define _Field_size_opt_(size)
#endif
#ifndef _Field_size_full_
	#define _Field_size_full_(size)
#endif
#ifndef _Field_size_bytes_full_
	#define _Field_size_bytes_full_(size)
#endif
#ifndef _Field_size_full_opt_
	#define _Field_size_full_opt_(size)
#endif

#endif

#endif			// _SALWRAPPER_HPP
