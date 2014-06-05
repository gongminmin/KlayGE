/**
 * @file ShaderDefs.cpp
 * @author Shenghua Lin, Minmin Gong
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

#include <DXBC2GLSL/ShaderDefs.hpp>
#include <DXBC2GLSL/Utils.hpp>

namespace
{
	char const * shader_operand_type_names[] =
	{
		"temp",
		"input",
		"output",
		"indexable_temp",
		"immediate32",
		"immediate64",
		"sampler",
		"resource",
		"constant_buffer",
		"immediate_constant_buffer",
		"label",
		"input_primitiveid",
		"output_depth",
		"null",
		"rasterizer",
		"output_coverage_mask",
		"stream",
		"function_body",
		"function_table",
		"interface",
		"function_input",
		"function_output",
		"output_control_point_id",
		"input_fork_instance_id",
		"input_join_instance_id",
		"input_control_point",
		"output_control_point",
		"input_patch_constant",
		"input_domain_point",
		"this_pointer",
		"unordered_access_view",
		"thread_group_shared_memory",
		"input_thread_id",
		"input_thread_group_id",
		"input_thread_id_in_group",
		"input_coverage_mask",
		"input_thread_id_in_group_flattened",
		"input_gs_instance_id",
		"output_depth_greater_equal",
		"output_depth_less_equal",
		"cycle_counter",
	};

	char const * shader_operand_type_short_names[] =
	{
		"r",
		"v",
		"o",
		"x",
		"l",
		"d",
		"s",
		"t",
		"cb",
		"icb",
		"label",
		"vPrim",
		"oDepth",
		"null",
		"rasterizer",
		"oMask",
		"m",
		"function_body",
		"function_table",
		"interface",
		"function_input",
		"function_output",
		"vOutputControlPointID",
		"vForkInstanceID",
		"vJoinInstanceID",
		"vicp",
		"vocp",
		"vpc",
		"vDomain",
		"this",
		"u",
		"g",
		"vThreadID",
		"vThreadGrouID",
		"vThreadIDInGroup",
		"vCoverage",
		"vThreadIDInGroupFlattened",
		"vGSInstanceID",
		"oDepthGE",
		"oDepthLE",
		"vCycleCounter",
	};

	char const * shader_interpolation_mode_names[] =
	{
		"undefined",
		"constant",
		"linear",
		"linear centroid",
		"linear noperspective",
		"linear noperspective centroid",
		"linear sample",
		"linear noperspective sample",
	};

	char const * shader_opcode_names[] =
	{
		"add",
		"and",
		"break",
		"breakc",
		"call",
		"callc",
		"case",
		"continue",
		"continuec",
		"cut",
		"default",
		"deriv_rtx",
		"deriv_rty",
		"discard",
		"div",
		"dp2",
		"dp3",
		"dp4",
		"else",
		"emit",
		"emitthencut",
		"endif",
		"endloop",
		"endswitch",
		"eq",
		"exp",
		"frc",
		"ftoi",
		"ftou",
		"ge",
		"iadd",
		"if",
		"ieq",
		"ige",
		"ilt",
		"imad",
		"imax",
		"imin",
		"imul",
		"ine",
		"ineg",
		"ishl",
		"ishr",
		"itof",
		"label",
		"ld_indexable",
		"ld_ms_indexable",
		"log",
		"loop",
		"lt",
		"mad",
		"min",
		"max",
		"dcl_immediateConstantBuffer",
		"mov",
		"movc",
		"mul",
		"ne",
		"nop",
		"not",
		"or",
		"resinfo_indexable",
		"ret",
		"retc",
		"round_ne",
		"round_ni",
		"round_pi",
		"round_z",
		"rsq",
		"sample_indexable",
		"sample_c_indexable",
		"sample_c_lz_indexable",
		"sample_l_indexable",
		"sample_d_indexable",
		"sample_b_indexable",
		"sqrt",
		"switch",
		"sincos",
		"udiv",
		"ult",
		"uge",
		"umul",
		"umad",
		"umax",
		"umin",
		"ushr",
		"utof",
		"xor",
		"dcl_resource",
		"dcl_constantbuffer",
		"dcl_sampler",
		"dcl_index_range",
		"dcl_outputtopology",
		"dcl_inputprimitive",
		"dcl_maxout",
		"dcl_input",
		"dcl_input_sgv",
		"dcl_input_siv",
		"dcl_input_ps",
		"dcl_input_ps_sgv",
		"dcl_input_ps_siv",
		"dcl_output",
		"dcl_output_sgv",
		"dcl_output_siv",
		"dcl_temps",
		"dcl_indexableTemp",
		"dcl_globalFlags",
		"d3d10_count",
		"lod",
		"gather4_indexable",
		"sample_pos",
		"sampleinfo",
		"d3d10_1_count",
		"hs_decls",
		"hs_control_point_phase",
		"hs_fork_phase",
		"hs_join_phase",
		"emit_stream",
		"cut_stream",
		"emitthencut_stream",
		"interface_call",
		"bufinfo_indexable",
		"deriv_rtx_coarse",
		"deriv_rtx_fine",
		"deriv_rty_coarse",
		"deriv_rty_fine",
		"gather4_c_indexable",
		"gather4_po_indexable",
		"gather4_po_c_indexable",
		"rcp",
		"f32tof16",
		"f16tof32",
		"uaddc",
		"usubb",
		"countbits",
		"firstbit_hi",
		"firstbit_lo",
		"firstbit_shi",
		"ubfe",
		"ibfe",
		"bfi",
		"bfrev",
		"swapc",
		"dcl_stream",
		"dcl_function_body",
		"dcl_function_table",
		"dcl_interface",
		"dcl_input_control_point_count",
		"dcl_output_control_point_count",
		"dcl_tessellator_domain",
		"dcl_tessellator_partitioning",
		"dcl_tessellator_output_primitive",
		"dcl_hs_max_tessfactor",
		"dcl_hs_fork_phase_instance_count",
		"dcl_hs_join_phase_instance_count",
		"dcl_thread_group",
		"dcl_uav_typed",
		"dcl_unordered_access_view_raw",
		"dcl_unordered_access_view_structured",
		"dcl_thread_group_shared_memory_raw",
		"dcl_thread_group_shared_memory_structured",
		"dcl_resource_raw",
		"dcl_resource_structured",
		"ld_uav_typed",
		"store_uav_typed",
		"ld_raw",
		"store_raw",
		"ld_structured",
		"store_structured",
		"atomic_and",
		"atomic_or",
		"atomic_xor",
		"atomic_cmp_store",
		"atomic_iadd",
		"atomic_imax",
		"atomic_imin",
		"atomic_umax",
		"atomic_umin",
		"imm_atomic_alloc",
		"imm_atomic_consume",
		"imm_atomic_iadd",
		"imm_atomic_and",
		"imm_atomic_or",
		"imm_atomic_xor",
		"imm_atomic_exch",
		"imm_atomic_cmp_exch",
		"imm_atomic_imax",
		"imm_atomic_imin",
		"imm_atomic_umax",
		"imm_atomic_umin",
		"sync",
		"dadd",
		"dmax",
		"dmin",
		"dmul",
		"deq",
		"dge",
		"dlt",
		"dne",
		"dmov",
		"dmovc",
		"dtof",
		"ftod",
		"eval_snapped",
		"eval_sample_index",
		"eval_centroid",
		"dcl_gsinstances",
	};

	char const * shader_system_value_names[] =
	{
		"undefined",
		"position",
		"clip_distance",
		"cull_distance",
		"render_target_array_index",
		"viewport_array_index",
		"vertex_id",
		"primitive_id",
		"instance_id",
		"is_front_face",
		"sample_index",
		"final_quad_u_eq_0_edge_tessfactor",
		"final_quad_v_eq_0_edge_tessfactor",
		"final_quad_u_eq_1_edge_tessfactor",
		"final_quad_v_eq_1_edge_tessfactor",
		"final_quad_u_inside_tessfactor",
		"final_quad_v_inside_tessfactor",
		"final_tri_u_eq_0_edge_tessfactor",
		"final_tri_v_eq_0_edge_tessfactor",
		"final_tri_w_eq_0_edge_tessfactor",
		"final_tri_inside_tessfactor",
		"final_line_detail_tessfactor",
		"final_line_density_tessfactor",
	};

	char const * shader_resource_dimension_names[] =
	{
		"unknown",
		"buffer",
		"texture1d",
		"texture2d",
		"texture2dms",
		"texture3d",
		"texturecube",
		"texture1darray",
		"texture2darray",
		"texture2dmsarray",
		"texturecubearray",
		"raw_buffer",
		"structured_buffer",
	};

	char const * shader_extended_opcode_names[] =
	{
		"empty",
		"sample_controls",
		"resource_dim",
		"resource_return_type",
	};

	char const * shader_extended_operand_names[] =
	{
		"empty",
		"modifier",
	};

	char const * shader_resource_return_type_names[] =
	{
		"unknown",
		"unorm",
		"snorm",
		"sint",
		"uint",
		"float",
		"mixed",
		"double",
		"continued",
	};

	char const * shader_res_info_return_type_names[] =
	{
		"",
		"rcpFloat",
		"uint",
	};

	char const * shader_sample_info_return_type_names[] =
	{
		"",
		"_uint",
	};

	char const * shader_variable_class_names[] =
	{
		"scalar",
		"vector",
		"matrix_rows",
		"matrix_columns",
		"object",
		"struct",
		"interface_class",
		"interface_pointer",
	};

	char const * shader_variable_type_names[] =
	{
		"void",
		"bool",
		"int",
		"float",
		"string",
		"texture",
		"texture1d",
		"texture2d",
		"texture3d",
		"texturecube",
		"sampler",
		"sampler1d",
		"sampler2d",
		"sampler3d",
		"samplercube",
		"pixelshader",
		"vertexshader",
		"pixelfragment",
		"vertexfragment",
		"uint",
		"uint8",
		"geometryshader",
		"rasterizer",
		"depthstencil",
		"blend",
		"buffer",
		"cbuffer",
		"tbuffer",
		"texture1darray",
		"texture2darray",
		"rendertargetview",
		"depthstencilview",
		"texture2dms",
		"texture2dmsarray",
		"texturecubearray",
		"hullshader",
		"domainshader",
		"interface_pointer",
		"computeshader",
		"double",
		"rwtexture1d",
		"rwtexture1darray",
		"rwtexture2d",
		"rwtexture2darray",
		"rwtexture3d",
		"rwbuffer",
		"byteaddress_buffer",
		"rwbyteaddress_buffer",
		"structured_buffer",
		"rwstructured_buffer",
		"append_structured_buffer",
		"consume_structured_buffer",
	};

	char const * shader_cbuffer_type_names[] =
	{
		"cbuffer",
		"tbuffer",
		"interface_pointers",
		"resource_bind_info",
	};

	char const * shader_input_type_names[] =
	{
		"cbuffer",
		"tbuffer",
		"texture",
		"sampler",
		"uav_rwtyped",
		"structured",
		"uav_rwstructured",
		"byteaddress",
		"uav_rwbyteaddress",
		"uav_append_structured",
		"uav_consume_structured",
		"uav_rwstructured_with_counter",
	};

	char const * shader_register_component_type_names[] =
	{
		"unknown",
		"uint",
		"int",
		"float",
	};

	char const * shader_primitive_names[] =
	{
		"undefined",
		"point",
		"line",
		"triangle",
		"lineadj",
		"triangleadj"
	};

	char const * shader_primitive_topology_names[] = 
	{
		"undefined",
		"pointlist",
		"linelist",
		"linestrip",
		"trianglelist",
		"trianglestrip",
		"linelistadj",
		"linestripadj",
		"trianglelistadj",
		"trianglestripadj"
	};

	char const * shader_tessellator_domain_names[] =
	{
		"undefined",
		"domain_isoline",
		"domain_tri",
		"domain_quad"
	};

	char const * shader_tessellator_partitioning_names[] =
	{
		"undefined",
		"partitioning_integer",
		"partitioning_pow2",
		"partitioning_odd",
		"partitioning_even"
	};

	char const * shader_tessellator_output_primitive_names[] =
	{
		"undefined",
		"output_point",
		"output_line",
		"output_triangle_cw",
		"output_triangle_ccw"
	};
}

char const * ShaderOperandTypeName(ShaderOperandType v)
{
	return shader_operand_type_names[v];
}

char const * ShaderOperandTypeShortName(ShaderOperandType v)
{
	return shader_operand_type_short_names[v];
}

char const * ShaderInterpolationModeName(ShaderInterpolationMode v)
{
	return shader_interpolation_mode_names[v];
}

char const * ShaderOpcodeName(ShaderOpcode v)
{
	return shader_opcode_names[v];
}

char const * ShaderSystemValueName(ShaderSystemValue v)
{
	return shader_system_value_names[v];
}

char const * ShaderResourceDimensionName(ShaderResourceDimension v)
{
	return shader_resource_dimension_names[v];
}

char const * ShaderExtendedOpcodeName(ShaderExtendedOpcode v)
{
	return shader_extended_opcode_names[v];
}

char const * ShaderExtendedOperandName(ShaderExtendedOperand v)
{
	return shader_extended_operand_names[v];
}

char const * ShaderResourceReturnTypeName(ShaderResourceReturnType v)
{
	return shader_resource_return_type_names[v];
}

char const * ShaderResInfoReturnTypeName(ShaderResInfoReturnType v)
{
	return shader_res_info_return_type_names[v];
}

char const * ShaderSampleInfoReturnTypeName(ShaderSampleInfoReturnType v)
{
	return shader_sample_info_return_type_names[v];
}

char const * ShaderVariableClassName(ShaderVariableClass v)
{
	return shader_variable_class_names[v];
}

char const * ShaderVariableTypeName(ShaderVariableType v)
{
	return shader_variable_type_names[v];
}

char const * ShaderCBufferTypeName(ShaderCBufferType v)
{
	return shader_cbuffer_type_names[v];
}

char const * ShaderInputTypeName(ShaderInputType v)
{
	return shader_input_type_names[v];
}

char const * ShaderRegisterComponentTypeName(ShaderRegisterComponentType v)
{
	return shader_register_component_type_names[v];
}

char const * ShaderPrimitiveName(ShaderPrimitive v)
{
	return shader_primitive_names[v];
}

char const * ShaderPrimitiveTopologyName(ShaderPrimitiveTopology v)
{
	return shader_primitive_topology_names[v];
}

char const * ShaderTessellatorDomainName(ShaderTessellatorDomain v)
{
	return shader_tessellator_domain_names[v];
}

char const * ShaderTessellatorPartitioningName(ShaderTessellatorPartitioning v)
{
	return shader_tessellator_partitioning_names[v];
}

char const * ShaderTessellatorOutputPrimitiveName(ShaderTessellatorOutputPrimitive v)
{
	return shader_tessellator_output_primitive_names[v];
}
