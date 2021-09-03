/**
 * @file ShaderDefs.hpp
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

#ifndef _DXBC2GLSL_SHADERDEFS_HPP
#define _DXBC2GLSL_SHADERDEFS_HPP

#pragma once

#include <cstdint>

enum ShaderOperandType
{
	SOT_TEMP = 0,
	SOT_INPUT,
	SOT_OUTPUT,
	SOT_INDEXABLE_TEMP,
	SOT_IMMEDIATE32,
	SOT_IMMEDIATE64,
	SOT_SAMPLER,
	SOT_RESOURCE,
	SOT_CONSTANT_BUFFER,
	SOT_IMMEDIATE_CONSTANT_BUFFER,
	SOT_LABEL,
	SOT_INPUT_PRIMITIVEID,
	SOT_OUTPUT_DEPTH,
	SOT_NULL,
	SOT_RASTERIZER,
	SOT_OUTPUT_COVERAGE_MASK,
	SOT_STREAM,
	SOT_FUNCTION_BODY,
	SOT_FUNCTION_TABLE,
	SOT_INTERFACE,
	SOT_FUNCTION_INPUT,
	SOT_FUNCTION_OUTPUT,
	SOT_OUTPUT_CONTROL_POINT_ID,
	SOT_INPUT_FORK_INSTANCE_ID,
	SOT_INPUT_JOIN_INSTANCE_ID,
	SOT_INPUT_CONTROL_POINT,//"vicp"
	SOT_OUTPUT_CONTROL_POINT,//"vocp"
	SOT_INPUT_PATCH_CONSTANT,
	SOT_INPUT_DOMAIN_POINT,
	SOT_THIS_POINTER,
	SOT_UNORDERED_ACCESS_VIEW,
	SOT_THREAD_GROUP_SHARED_MEMORY,
	SOT_INPUT_THREAD_ID,
	SOT_INPUT_THREAD_GROUP_ID,
	SOT_INPUT_THREAD_ID_IN_GROUP,
	SOT_INPUT_COVERAGE_MASK,
	SOT_INPUT_THREAD_ID_IN_GROUP_FLATTENED,
	SOT_INPUT_GS_INSTANCE_ID,
	SOT_OUTPUT_DEPTH_GREATER_EQUAL,
	SOT_OUTPUT_DEPTH_LESS_EQUAL,
	SOT_CYCLE_COUNTER,

	SOT_COUNT
};

enum ShaderInterpolationMode
{
	SIM_Undefined,
	SIM_Constant,
	SIM_Linear,
	SIM_LinearCentroid,
	SIM_LinearNoPerspective,
	SIM_LinearNoPerspectiveCentroid,
	SIM_LinearSample, // D3D 10.1
	SIM_LinearNoPerspectiveSample // D3D 10.1
};

enum ShaderOpcode
{
	SO_ADD,
	SO_AND,
	SO_BREAK,
	SO_BREAKC,
	SO_CALL,
	SO_CALLC,
	SO_CASE,
	SO_CONTINUE,
	SO_CONTINUEC,
	SO_CUT,
	SO_DEFAULT,
	SO_DERIV_RTX,
	SO_DERIV_RTY,
	SO_DISCARD,
	SO_DIV,
	SO_DP2,
	SO_DP3,
	SO_DP4,
	SO_ELSE,
	SO_EMIT,
	SO_EMITTHENCUT,
	SO_ENDIF,
	SO_ENDLOOP,
	SO_ENDSWITCH,
	SO_EQ,
	SO_EXP,
	SO_FRC,
	SO_FTOI,
	SO_FTOU,
	SO_GE,
	SO_IADD,
	SO_IF,
	SO_IEQ,
	SO_IGE,
	SO_ILT,
	SO_IMAD,
	SO_IMAX,
	SO_IMIN,
	SO_IMUL,
	SO_INE,
	SO_INEG,
	SO_ISHL,
	SO_ISHR,
	SO_ITOF,
	SO_LABEL,
	SO_LD,
	SO_LD_MS,
	SO_LOG,
	SO_LOOP,
	SO_LT,
	SO_MAD,
	SO_MIN,
	SO_MAX,
	SO_IMMEDIATE_CONSTANT_BUFFER,
	SO_MOV,
	SO_MOVC,
	SO_MUL,
	SO_NE,
	SO_NOP,
	SO_NOT,
	SO_OR,
	SO_RESINFO,
	SO_RET,
	SO_RETC,
	SO_ROUND_NE,
	SO_ROUND_NI,
	SO_ROUND_PI,
	SO_ROUND_Z,
	SO_RSQ,
	SO_SAMPLE,
	SO_SAMPLE_C,
	SO_SAMPLE_C_LZ,
	SO_SAMPLE_L,
	SO_SAMPLE_D,
	SO_SAMPLE_B,
	SO_SQRT,
	SO_SWITCH,
	SO_SINCOS,
	SO_UDIV,
	SO_ULT,
	SO_UGE,
	SO_UMUL,
	SO_UMAD,
	SO_UMAX,
	SO_UMIN,
	SO_USHR,
	SO_UTOF,
	SO_XOR,
	SO_DCL_RESOURCE,
	SO_DCL_CONSTANT_BUFFER,
	SO_DCL_SAMPLER,
	SO_DCL_INDEX_RANGE,
	SO_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY,
	SO_DCL_GS_INPUT_PRIMITIVE,
	SO_DCL_MAX_OUTPUT_VERTEX_COUNT,
	SO_DCL_INPUT,
	SO_DCL_INPUT_SGV,
	SO_DCL_INPUT_SIV,
	SO_DCL_INPUT_PS,
	SO_DCL_INPUT_PS_SGV,
	SO_DCL_INPUT_PS_SIV,
	SO_DCL_OUTPUT,
	SO_DCL_OUTPUT_SGV,
	SO_DCL_OUTPUT_SIV,
	SO_DCL_TEMPS,
	SO_DCL_INDEXABLE_TEMP,
	SO_DCL_GLOBAL_FLAGS,
	SO_SM10_COUNT,
	SO_LOD,
	SO_GATHER4,
	SO_SAMPLE_POS,
	SO_SAMPLE_INFO,
	SO_SM10_1_COUNT,
	SO_HS_DECLS,
	SO_HS_CONTROL_POINT_PHASE,
	SO_HS_FORK_PHASE,
	SO_HS_JOIN_PHASE,
	SO_EMIT_STREAM,
	SO_CUT_STREAM,
	SO_EMITTHENCUT_STREAM,
	SO_INTERFACE_CALL,
	SO_BUFINFO,
	SO_DERIV_RTX_COARSE,
	SO_DERIV_RTX_FINE,
	SO_DERIV_RTY_COARSE,
	SO_DERIV_RTY_FINE,
	SO_GATHER4_C,
	SO_GATHER4_PO,
	SO_GATHER4_PO_C,
	SO_RCP,
	SO_F32TOF16,
	SO_F16TOF32,
	SO_UADDC,
	SO_USUBB,
	SO_COUNTBITS,
	SO_FIRSTBIT_HI,
	SO_FIRSTBIT_LO,
	SO_FIRSTBIT_SHI,
	SO_UBFE,
	SO_IBFE,
	SO_BFI,
	SO_BFREV,
	SO_SWAPC,
	SO_DCL_STREAM,
	SO_DCL_FUNCTION_BODY,
	SO_DCL_FUNCTION_TABLE,
	SO_DCL_INTERFACE,
	SO_DCL_INPUT_CONTROL_POINT_COUNT,
	SO_DCL_OUTPUT_CONTROL_POINT_COUNT,
	SO_DCL_TESS_DOMAIN,
	SO_DCL_TESS_PARTITIONING,
	SO_DCL_TESS_OUTPUT_PRIMITIVE,
	SO_DCL_HS_MAX_TESSFACTOR,
	SO_DCL_HS_FORK_PHASE_INSTANCE_COUNT,
	SO_DCL_HS_JOIN_PHASE_INSTANCE_COUNT,
	SO_DCL_THREAD_GROUP,
	SO_DCL_UNORDERED_ACCESS_VIEW_TYPED,
	SO_DCL_UNORDERED_ACCESS_VIEW_RAW,
	SO_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED,
	SO_DCL_THREAD_GROUP_SHARED_MEMORY_RAW,
	SO_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED,
	SO_DCL_RESOURCE_RAW,
	SO_DCL_RESOURCE_STRUCTURED,
	SO_LD_UAV_TYPED,
	SO_STORE_UAV_TYPED,
	SO_LD_RAW,
	SO_STORE_RAW,
	SO_LD_STRUCTURED,
	SO_STORE_STRUCTURED,
	SO_ATOMIC_AND,
	SO_ATOMIC_OR,
	SO_ATOMIC_XOR,
	SO_ATOMIC_CMP_STORE,
	SO_ATOMIC_IADD,
	SO_ATOMIC_IMAX,
	SO_ATOMIC_IMIN,
	SO_ATOMIC_UMAX,
	SO_ATOMIC_UMIN,
	SO_IMM_ATOMIC_ALLOC,
	SO_IMM_ATOMIC_CONSUME,
	SO_IMM_ATOMIC_IADD,
	SO_IMM_ATOMIC_AND,
	SO_IMM_ATOMIC_OR,
	SO_IMM_ATOMIC_XOR,
	SO_IMM_ATOMIC_EXCH,
	SO_IMM_ATOMIC_CMP_EXCH,
	SO_IMM_ATOMIC_IMAX,
	SO_IMM_ATOMIC_IMIN,
	SO_IMM_ATOMIC_UMAX,
	SO_IMM_ATOMIC_UMIN,
	SO_SYNC,
	SO_DADD,
	SO_DMAX,
	SO_DMIN,
	SO_DMUL,
	SO_DEQ,
	SO_DGE,
	SO_DLT,
	SO_DNE,
	SO_DMOV,
	SO_DMOVC,
	SO_DTOF,
	SO_FTOD,
	SO_EVAL_SNAPPED,
	SO_EVAL_SAMPLE_INDEX,
	SO_EVAL_CENTROID,
	SO_DCL_GS_INSTANCE_COUNT,

	SO_COUNT
};

enum ShaderOperandNumComponents
{
	SONC_0,
	SONC_1,
	SONC_4
};

enum ShaderOperandIndexRepresentation
{
	SOIP_IMM32,
	SOIP_IMM64,
	SOIP_RELATIVE,
	SOIP_IMM32_PLUS_RELATIVE,
	SOIP_IMM64_PLUS_RELATIVE
};

enum ShaderOperandSelectionMode
{
	SOSM_MASK,
	SOSM_SWIZZLE,
	SOSM_SCALAR
};

enum ShaderSystemValue
{
	SSV_UNDEFINED,
	SSV_POSITION,
	SSV_CLIP_DISTANCE,
	SSV_CULL_DISTANCE,
	SSV_RENDER_TARGET_ARRAY_INDEX,
	SSV_VIEWPORT_ARRAY_INDEX,
	SSV_VERTEX_ID,
	SSV_PRIMITIVE_ID,
	SSV_INSTANCE_ID,
	SSV_IS_FRONT_FACE,
	SSV_SAMPLE_INDEX,
	// D3D11+
	SSV_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR,
	SSV_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR,
	SSV_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR,
	SSV_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR,
	SSV_FINAL_QUAD_U_INSIDE_TESSFACTOR,
	SSV_FINAL_QUAD_V_INSIDE_TESSFACTOR,
	SSV_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR,
	SSV_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR,
	SSV_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR,
	SSV_FINAL_TRI_INSIDE_TESSFACTOR,
	SSV_FINAL_LINE_DETAIL_TESSFACTOR,
	SSV_FINAL_LINE_DENSITY_TESSFACTOR
};

enum ShaderResourceDimension
{
	SRD_UNKNOWN,
	SRD_BUFFER,
	SRD_TEXTURE1D,
	SRD_TEXTURE2D,
	SRD_TEXTURE2DMS,
	SRD_TEXTURE3D,
	SRD_TEXTURECUBE,
	SRD_TEXTURE1DARRAY,
	SRD_TEXTURE2DARRAY,
	SRD_TEXTURE2DMSARRAY,
	SRD_TEXTURECUBEARRAY,
	SRD_RAW_BUFFER,
	SRD_STRUCTURED_BUFFER
};

enum ShaderExtendedOpcode
{
	SEOP_EMPTY,
	SEOP_SAMPLE_CONTROLS,
	SEOP_RESOURCE_DIM,
	SEOP_RESOURCE_RETURN_TYPE
};

enum ShaderExtendedOperand
{
	SEO_EMPTY,
	SEO_MODIFIER
};

enum ShaderResourceReturnType : uint8_t
{
	SRRT_UNKNOWN = 0,
	SRRT_UNORM = 1,
	SRRT_SNORM = 2,
	SRRT_SINT = 3,
	SRRT_UINT = 4,
	SRRT_FLOAT = 5,
	SRRT_MIXED = 6,
	SRRT_DOUBLE = 7,
	SRRT_CONTINUED = 8
};

enum ShaderResInfoReturnType
{
	SRIRT_FLOAT = 0,
	SRIRT_RCPFLOAT = 1,
	SRIRT_UINT = 2
};

enum ShaderSampleInfoReturnType
{
	SSIRT_FLOAT = 0,
	SSIRT_UINT = 1
};

enum ShaderVariableClass
{
	SVC_SCALAR	= 0,
	SVC_VECTOR,
	SVC_MATRIX_ROWS,
	SVC_MATRIX_COLUMNS,
	SVC_OBJECT,
	SVC_STRUCT,
	SVC_INTERFACE_CLASS,
	SVC_INTERFACE_POINTER,
	SVC_FORCE_DWORD = 0x7FFFFFFF
};

enum ShaderVariableType
{
	SVT_VOID = 0,
	SVT_BOOL = 1,
	SVT_INT = 2,
	SVT_FLOAT = 3,
	SVT_STRING = 4,
	SVT_TEXTURE = 5,
	SVT_TEXTURE1D = 6,
	SVT_TEXTURE2D = 7,
	SVT_TEXTURE3D = 8,
	SVT_TEXTURECUBE = 9,
	SVT_SAMPLER = 10,
	SVT_SAMPLER1D = 11,
	SVT_SAMPLER2D = 12,
	SVT_SAMPLER3D = 13,
	SVT_SAMPLERCUBE = 14,
	SVT_PIXELSHADER = 15,
	SVT_VERTEXSHADER = 16,
	SVT_PIXELFRAGMENT = 17,
	SVT_VERTEXFRAGMENT = 18,
	SVT_UINT = 19,
	SVT_UINT8 = 20,
	SVT_GEOMETRYSHADER = 21,
	SVT_RASTERIZER = 22,
	SVT_DEPTHSTENCIL = 23,
	SVT_BLEND = 24,
	SVT_BUFFER = 25,
	SVT_CBUFFER = 26,
	SVT_TBUFFER = 27,
	SVT_TEXTURE1DARRAY = 28,
	SVT_TEXTURE2DARRAY = 29,
	SVT_RENDERTARGETVIEW = 30,
	SVT_DEPTHSTENCILVIEW = 31,
	SVT_TEXTURE2DMS = 32,
	SVT_TEXTURE2DMSARRAY = 33,
	SVT_TEXTURECUBEARRAY = 34,
	SVT_HULLSHADER = 35,
	SVT_DOMAINSHADER = 36,
	SVT_INTERFACE_POINTER = 37,
	SVT_COMPUTESHADER = 38,
	SVT_DOUBLE = 39,
	SVT_RWTEXTURE1D = 40,
	SVT_RWTEXTURE1DARRAY = 41,
	SVT_RWTEXTURE2D = 42,
	SVT_RWTEXTURE2DARRAY = 43,
	SVT_RWTEXTURE3D = 44,
	SVT_RWBUFFER = 45,
	SVT_BYTEADDRESS_BUFFER = 46,
	SVT_RWBYTEADDRESS_BUFFER = 47,
	SVT_STRUCTURED_BUFFER = 48,
	SVT_RWSTRUCTURED_BUFFER = 49,
	SVT_APPEND_STRUCTURED_BUFFER = 50,
	SVT_CONSUME_STRUCTURED_BUFFER = 51,
	SVT_FORCE_DWORD	= 0x7FFFFFFF
};

enum ShaderCBufferType
{
	SCBT_CBUFFER	= 0,
	SCBT_TBUFFER,
	SCBT_INTERFACE_POINTERS,
	SCBT_RESOURCE_BIND_INFO,
};

enum ShaderInputType
{
	SIT_CBUFFER	= 0,
	SIT_TBUFFER,
	SIT_TEXTURE,
	SIT_SAMPLER,
	SIT_UAV_RWTYPED,
	SIT_STRUCTURED,
	SIT_UAV_RWSTRUCTURED,
	SIT_BYTEADDRESS,
	SIT_UAV_RWBYTEADDRESS,
	SIT_UAV_APPEND_STRUCTURED,
	SIT_UAV_CONSUME_STRUCTURED,
	SIT_UAV_RWSTRUCTURED_WITH_COUNTER,
	SIT_UNDEFINED
};

enum ShaderType
{
	ST_PS = 0,
	ST_VS,
	ST_GS,
	ST_HS,
	ST_DS,
	ST_CS
};

enum ShaderRegisterComponentType
{
	SRCT_UNKNOWN = 0,
	SRCT_UINT32 = 1,
	SRCT_SINT32 = 2,
	SRCT_FLOAT32 = 3
};

enum ShaderSRVDimension
{
	SSD_UNKNOWN = 0,
	SSD_BUFFER = 1,
	SSD_TEXTURE1D = 2,
	SSD_TEXTURE1DARRAY = 3,
	SSD_TEXTURE2D = 4,
	SSD_TEXTURE2DARRAY = 5,
	SSD_TEXTURE2DMS = 6,
	SSD_TEXTURE2DMSARRAY = 7,
	SSD_TEXTURE3D = 8,
	SSD_TEXTURECUBE = 9,
	SSD_TEXTURECUBEARRAY = 10,
	SSD_BUFFEREX = 11
};

// TODO: Figure out the usage of ShaderName and ShaderSystemValue
enum ShaderName
{
	SN_UNDEFINED = 0,
	SN_POSITION = 1,
	SN_CLIP_DISTANCE = 2,
	SN_CULL_DISTANCE = 3,
	SN_RENDER_TARGET_ARRAY_INDEX = 4,
	SN_VIEWPORT_ARRAY_INDEX = 5,
	SN_VERTEX_ID = 6,
	SN_PRIMITIVE_ID = 7,
	SN_INSTANCE_ID = 8,
	SN_IS_FRONT_FACE = 9,
	SN_SAMPLE_INDEX = 10,
	SN_FINAL_QUAD_EDGE_TESSFACTOR = 11,
	SN_FINAL_QUAD_INSIDE_TESSFACTOR = 12,
	SN_FINAL_TRI_EDGE_TESSFACTOR = 13,
	SN_FINAL_TRI_INSIDE_TESSFACTOR = 14,
	SN_FINAL_LINE_DETAIL_TESSFACTOR = 15,
	SN_FINAL_LINE_DENSITY_TESSFACTOR = 16,
	SN_TARGET = 64,
	SN_DEPTH = 65,
	SN_COVERAGE = 66,
	SN_DEPTH_GREATER_EQUAL = 67,
	SN_DEPTH_LESS_EQUAL = 68
};

enum ShaderPrimitive
{
	SP_Undefined = 0,
	SP_Point = 1,
	SP_Line = 2,
	SP_Triangle = 3,
	SP_LineAdj = 0x4 | SP_Line,
	SP_TriangleAdj = 0x4 | SP_Triangle
};

enum ShaderPrimitiveTopology
{
	SPT_Undefined = 0,
	SPT_PointList = 1,
	SPT_LineList = 2,
	SPT_LineStrip = 3,
	SPT_TriangleList = 4,
	SPT_TriangleStrip = 5,
	SPT_LineListAdj = 0x8 | SPT_LineList,
	SPT_LineStripAdj = 0x8 | SPT_LineStrip,
	SPT_TriangleListAdj = 0x8 | SPT_TriangleList,
	SPT_TriangleStripAdj = 0x8 | SPT_TriangleStrip
};

enum ShaderTessellatorDomain
{
	SDT_Undefined = 0,
	SDT_Isoline = 1,
	SDT_Triangle = 2,
	SDT_Quad = 3
};

enum ShaderTessellatorPartitioning : uint8_t
{
	STP_Undefined = 0,
	STP_Integer = 1,
	STP_Pow2 = 2,
	STP_Fractional_Odd = 3,
	STP_Fractional_Even = 4
};

enum ShaderTessellatorOutputPrimitive : uint8_t
{
	STOP_Undefined = 0,
	STOP_Point = 1,
	STOP_Line = 2,
	STOP_Triangle_CW = 3,
	STOP_Triangle_CCW = 4
};

char const * ShaderOperandTypeName(ShaderOperandType v);
char const * ShaderOperandTypeShortName(ShaderOperandType v);
char const * ShaderInterpolationModeName(ShaderInterpolationMode v);
char const * ShaderOpcodeName(ShaderOpcode v);
char const * ShaderSystemValueName(ShaderSystemValue v);
char const * ShaderResourceDimensionName(ShaderResourceDimension v);
char const * ShaderExtendedOpcodeName(ShaderExtendedOpcode v);
char const * ShaderExtendedOperandName(ShaderExtendedOperand v);
char const * ShaderResourceReturnTypeName(ShaderResourceReturnType v);
char const * ShaderResInfoReturnTypeName(ShaderResInfoReturnType v);
char const * ShaderSampleInfoReturnTypeName(ShaderSampleInfoReturnType v);
char const * ShaderVariableClassName(ShaderVariableClass v);
char const * ShaderVariableTypeName(ShaderVariableType v);
char const * ShaderCBufferTypeName(ShaderCBufferType v);
char const * ShaderInputTypeName(ShaderInputType v);
char const * ShaderRegisterComponentTypeName(ShaderRegisterComponentType v);
char const * ShaderPrimitiveName(ShaderPrimitive v);
char const * ShaderPrimitiveTopologyName(ShaderPrimitiveTopology v);
char const * ShaderTessellatorDomainName(ShaderTessellatorDomain v);
char const * ShaderTessellatorPartitioningName(ShaderTessellatorPartitioning v);
char const * ShaderTessellatorOutputPrimitiveName(ShaderTessellatorOutputPrimitive v);

#endif		// _DXBC2GLSL_SHADERDEFS_HPP
