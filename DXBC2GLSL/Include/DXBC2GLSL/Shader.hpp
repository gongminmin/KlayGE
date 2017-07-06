/**
 * @file Shader.hpp
 * @author Shenghua Lin, Minmin Gong, Luca Barbieri
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

#ifndef _DXBC2GLSL_SHADER_HPP
#define _DXBC2GLSL_SHADER_HPP

#pragma once

#include <KFL/KFL.hpp>
#include <vector>
#include <cstring>
#include <DXBC2GLSL/DXBC.hpp>
#include <DXBC2GLSL/Utils.hpp>
#include <DXBC2GLSL/ShaderDefs.hpp>

// store texture-sampler pairs, in glsl, texture must bind with sampler
struct SamplerInfo
{
	int64_t index;
	bool shadow;
};
struct TextureSamplerInfo
{
	int64_t tex_index;
	std::vector<SamplerInfo> samplers;
	uint32_t type;
};

struct DclIndexRangeInfo
{
	ShaderOperandType op_type; 
	int64_t start;
	uint32_t num;
};

struct TokenizedShaderVersion // 32-bit
{
	uint32_t minor : 4;
	uint32_t major : 4;
	uint32_t format : 8;
	ShaderType type : 16;
};

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4201) // Allow unnamed struct.
#endif
struct TokenizedShaderInstruction // 32-bit
{
	// we don't make it an union directly because unions can't be inherited from but struct can be 
	union
	{
		// length and extended are always present, but they are only here to reduce duplication
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t _11_23 : 13;
			uint32_t length : 7;
			uint32_t extended : 1;
		};
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t resinfo_return_type : 2;
			uint32_t sat : 1;
			uint32_t _14_17 : 4;
			uint32_t test_nz : 1; // bit 18
			uint32_t precise_mask : 4;
			uint32_t _23 : 1;
			uint32_t length : 7;
			uint32_t extended : 1;
		} insn;
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t threads_in_group : 1;
			uint32_t shared_memory : 1;
			uint32_t uav_group : 1;
			uint32_t uav_global : 1;
			uint32_t _15_17 : 3;
		} sync;
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t allow_refactoring : 1;
			uint32_t fp64 : 1;
			uint32_t early_depth_stencil : 1;
			uint32_t enable_raw_and_structured_in_non_cs : 1;
			uint32_t skip_optimization : 1;
			uint32_t enable_minimum_precision : 1;
			uint32_t enable_double_extensions : 1;
			uint32_t enable_shader_extensions : 1;
		} dcl_global_flags;
		struct
		{
			ShaderOpcode opcode : 11;
			ShaderResourceDimension target : 5;
			uint32_t nr_samples : 7;
		} dcl_resource;
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t shadow : 1;
			uint32_t mono : 1;
		} dcl_sampler;
		struct
		{
			ShaderOpcode opcode : 11;
			ShaderInterpolationMode interpolation : 5;
		} dcl_input_ps;
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t dynamic : 1;
		} dcl_constant_buffer;
		struct
		{
			ShaderOpcode opcode : 11;
			ShaderPrimitive primitive : 6;
		} dcl_gs_input_primitive;
		struct
		{
			ShaderOpcode opcode : 11;
			ShaderPrimitiveTopology primitive_topology : 7;
		} dcl_gs_output_primitive_topology;
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t control_points : 6;
		} dcl_input_control_point_count;
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t control_points : 6;
		} dcl_output_control_point_count;
		struct
		{
			ShaderOpcode opcode : 11;
			ShaderTessellatorDomain domain: 3; // D3D_TESSELLATOR_DOMAIN
		} dcl_tess_domain;
		struct
		{
			ShaderOpcode opcode : 11;
			ShaderTessellatorPartitioning partitioning : 3; // D3D_TESSELLATOR_PARTITIONING
		} dcl_tess_partitioning;
		struct
		{
			ShaderOpcode opcode : 11;
			ShaderTessellatorOutputPrimitive primitive : 3; // D3D_TESSELLATOR_OUTPUT_PRIMITIVE
		} dcl_tess_output_primitive;
	};
};

union TokenizedShaderInstructionExtended
{
	struct
	{
		uint32_t type : 6;
		uint32_t _6_30 : 25;
		uint32_t extended :1;
	};
	struct
	{
		uint32_t type : 6;
		uint32_t _6_8 : 3;
		int32_t offset_u : 4;
		int32_t offset_v : 4;
		int32_t offset_w : 4;
	} sample_controls;
	struct
	{
		uint32_t type : 6;
		uint32_t target : 5;
	} resource_target;
	struct
	{
		uint32_t type : 6;
		uint32_t x : 4;
		uint32_t y : 4;
		uint32_t z : 4;
		uint32_t w : 4;
	} resource_return_type;
};
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

struct TokenizedShaderResourceReturnType
{
	ShaderResourceReturnType x : 4;
	ShaderResourceReturnType y : 4;
	ShaderResourceReturnType z : 4;
	ShaderResourceReturnType w : 4;
};

struct TokenizedShaderOperand
{
	uint32_t comps_enum : 2; // sm_operands_comps
	uint32_t mode : 2; // sm_operand_mode
	uint32_t sel : 8;
	uint32_t op_type : 8; // ShaderOperandType
	uint32_t num_indices : 2;
	uint32_t index0_repr : 3; // sm_operand_index_repr
	uint32_t index1_repr : 3; // sm_operand_index_repr
	uint32_t index2_repr : 3; // sm_operand_index_repr
	uint32_t extended : 1;
};

#define SM_OPERAND_SEL_MASK(sel) ((sel) & 0xf)
#define SM_OPERAND_SEL_SWZ(sel, i) (((sel) >> ((i) * 2)) & 3)
#define SM_OPERAND_SEL_SCALAR(sel) ((sel) & 3)

struct TokenizedShaderOperandExtended
{
	uint32_t type : 6;
	uint32_t neg : 1;
	uint32_t abs : 1;
};

union ShaderAny
{
	double f64;
	float f32;
	int64_t i64;
	int32_t i32;
	uint64_t u64;
	int64_t u32;
};

enum ShaderImmType
{
	SIT_Unknown,
	SIT_Int,
	SIT_UInt,
	SIT_Float,
	SIT_Double
};

struct ShaderOperand
{
	uint8_t mode;
	uint8_t comps;
	uint8_t mask;
	uint8_t num_indices;
	uint8_t swizzle[4];
	ShaderOperandType type;
	ShaderAny imm_values[4];
	bool neg;
	bool abs;
	struct
	{
		int64_t disp;
		std::shared_ptr<ShaderOperand> reg;
	} indices[3];

	bool IsIndexSimple(uint32_t i) const
	{
		 return !indices[i].reg && (indices[i].disp >= 0)
			 && (static_cast<int64_t>(static_cast<int32_t>(indices[i].disp)) == indices[i].disp);
	}

	bool HasSimpleIndex() const//类似[1]的这种索引形式
	{
		return (1 == num_indices) && this->IsIndexSimple(0);
	}

	ShaderOperand()
		: mode(0), comps(0), mask(0), num_indices(0),
			type(SOT_TEMP), neg(false), abs(false)
	{
		memset(swizzle, 0, sizeof(swizzle));
		memset(imm_values, 0, sizeof(imm_values));
		indices[0].disp = indices[1].disp = indices[2].disp = 0;
	}
};

// for sample_d
uint32_t const SM_MAX_OPS = 6;

struct ShaderInstruction : public TokenizedShaderInstruction
{
	int8_t sample_offset[3];
	uint8_t resource_target;
	uint8_t resource_return_type[4];

	uint32_t num;
	uint32_t num_ops;
	std::shared_ptr<ShaderOperand> ops[SM_MAX_OPS];

	ShaderInstruction()
		: resource_target(0), num(0), num_ops(0)
	{
		memset(sample_offset, 0, sizeof(sample_offset));
		memset(resource_return_type, 0, sizeof(resource_return_type));
	}
};

struct ShaderDecl : public TokenizedShaderInstruction
{
	std::shared_ptr<ShaderOperand> op;
	union
	{
		uint32_t num;
		float f32;
		ShaderSystemValue sv;
		struct
		{
			uint32_t id;
			uint32_t expected_function_table_length;
			uint32_t table_length;
			uint32_t array_length;
		} intf;
		uint32_t thread_group_size[3];
		TokenizedShaderResourceReturnType rrt;
		struct
		{
			uint32_t num;
			uint32_t comps;
		} indexable_temp;
		struct
		{
			uint32_t stride;
			uint32_t count;
		} structured;
	};

	std::vector<uint8_t> data;

	ShaderDecl()
	{
		memset(&insn, 0, sizeof(insn));
		memset(&intf, 0, sizeof(intf));
	}
};

struct LabelInfo
{
	uint32_t start_num; // the first instruction in label code after label l#
	uint32_t end_num; // the last insn in label etc. ret
};

struct ShaderProgram
{
	TokenizedShaderVersion version;//program version
	std::vector<std::shared_ptr<ShaderDecl>> dcls;//declarations
	std::vector<std::shared_ptr<ShaderInstruction>> insns;//instructions

	std::vector<DXBCSignatureParamDesc> params_in; //input signature
	std::vector<DXBCSignatureParamDesc> params_out;//output signature
	std::vector<DXBCSignatureParamDesc> params_patch;//patch signature
	std::vector<DXBCConstantBuffer> cbuffers;//constant buffers(including tbuffer)
	std::vector<DXBCInputBindDesc> resource_bindings;
	//gs stuff
	ShaderPrimitive gs_input_primitive;
	std::vector<ShaderPrimitiveTopology> gs_output_topology;
	uint32_t max_gs_output_vertex;
	uint32_t gs_instance_count;
	//hs ds stuff
	uint32_t hs_input_control_point_count;
	uint32_t hs_output_control_point_count;
	ShaderTessellatorDomain ds_tessellator_domain;
	ShaderTessellatorPartitioning ds_tessellator_partitioning;
	ShaderTessellatorOutputPrimitive ds_tessellator_output_primitive;
	//cs stuff
	uint32_t cs_thread_group_size[3];

	ShaderProgram()
		: gs_input_primitive(SP_Undefined), max_gs_output_vertex(0),
			gs_instance_count(0), hs_input_control_point_count(0),
			hs_output_control_point_count(0), ds_tessellator_domain(SDT_Undefined),
			ds_tessellator_partitioning(STP_Undefined), ds_tessellator_output_primitive(STOP_Undefined)
	{
		memset(&version, 0, sizeof(version));
		memset(cs_thread_group_size, 0, sizeof(cs_thread_group_size));
	}
};

std::shared_ptr<ShaderProgram> ShaderParse(DXBCContainer const & dxbc);

// Return the opcode's input type
inline ShaderImmType GetOpInType(uint32_t opcode)
{
	ShaderImmType sit;
	switch (opcode)
	{
	case SO_IADD:
	case SO_IEQ:
	case SO_IGE:
	case SO_ILT:
	case SO_IMAD:
	case SO_IMAX:
	case SO_IMIN:
	case SO_IMUL:
	case SO_INE:
	case SO_INEG:
	case SO_ISHL:
	case SO_ISHR:
	case SO_ITOF:
	case SO_ATOMIC_IADD:
	case SO_ATOMIC_IMAX:
	case SO_ATOMIC_IMIN:
	case SO_IMM_ATOMIC_IADD:
	case SO_IMM_ATOMIC_IMAX:
	case SO_IMM_ATOMIC_IMIN:
	case SO_CASE:
	case SO_IBFE:
	case SO_FIRSTBIT_SHI:
	case SO_BREAKC:
	case SO_IF:
	case SO_CONTINUEC:
	case SO_RETC:
	case SO_DISCARD:
	case SO_CALL:
	case SO_CALLC:
		sit = SIT_Int;
		break;

	case SO_AND:
	case SO_XOR:
	case SO_OR:
	case SO_NOT:
	case SO_ATOMIC_AND:
	case SO_ATOMIC_OR:
	case SO_ATOMIC_XOR:
	case SO_IMM_ATOMIC_AND:
	case SO_IMM_ATOMIC_OR:
	case SO_IMM_ATOMIC_XOR:
	case SO_UDIV:
	case SO_ULT:
	case SO_UGE:
	case SO_UMUL:
	case SO_UMAD:
	case SO_UMAX:
	case SO_UMIN:
	case SO_USHR:
	case SO_UTOF:
	case SO_UADDC:
	case SO_USUBB:
	case SO_ATOMIC_UMAX:
	case SO_ATOMIC_UMIN:
	case SO_IMM_ATOMIC_UMAX:
	case SO_IMM_ATOMIC_UMIN:
	case SO_COUNTBITS:
	case SO_BFI:
	case SO_BFREV:
	case SO_UBFE:
	case SO_FIRSTBIT_LO:
	case SO_FIRSTBIT_HI:
	case SO_F16TOF32:
	case SO_F32TOF16:
	case SO_RESINFO:
		sit = SIT_UInt;
		break;

	case SO_DADD:
	case SO_DMAX:
	case SO_DMIN:
	case SO_DMUL:
	case SO_DEQ:
	case SO_DGE:
	case SO_DLT:
	case SO_DNE:
	case SO_DMOV:
	case SO_DMOVC:
	case SO_DTOF:
		sit = SIT_Double;
		break;

	default:
		sit = SIT_Float;
		break;
	}

	return sit;
}

// Return the opcode's output type
inline ShaderImmType GetOpOutType(uint32_t opcode)
{
	ShaderImmType sit;
	switch (opcode)
	{
	case SO_EQ:
	case SO_FTOI:
	case SO_IADD:
	case SO_IEQ:
	case SO_IGE:
	case SO_ILT:
	case SO_IMAD:
	case SO_IMAX:
	case SO_IMIN:
	case SO_IMUL:
	case SO_INE:
	case SO_INEG:
	case SO_ISHL:
	case SO_ISHR:
	case SO_ATOMIC_IADD:
	case SO_ATOMIC_IMAX:
	case SO_ATOMIC_IMIN:
	case SO_IMM_ATOMIC_IADD:
	case SO_IMM_ATOMIC_IMAX:
	case SO_IMM_ATOMIC_IMIN:
	case SO_CASE:
	case SO_IBFE:
	case SO_FIRSTBIT_SHI:
	case SO_BREAKC:
	case SO_IF:
	case SO_CONTINUEC:
	case SO_RETC:
	case SO_DISCARD:
	case SO_CALL:
	case SO_CALLC:
	case SO_NE:
	case SO_LT:
	case SO_ULT:
	case SO_GE:
	case SO_UGE:
		sit = SIT_Int;
		break;

	case SO_AND:
	case SO_XOR:
	case SO_OR:
	case SO_NOT:
	case SO_ATOMIC_AND:
	case SO_ATOMIC_OR:
	case SO_ATOMIC_XOR:
	case SO_IMM_ATOMIC_AND:
	case SO_IMM_ATOMIC_OR:
	case SO_IMM_ATOMIC_XOR:
	case SO_UDIV:
	case SO_UMUL:
	case SO_UMAD:
	case SO_UMAX:
	case SO_UMIN:
	case SO_USHR:
	case SO_UADDC:
	case SO_USUBB:
	case SO_ATOMIC_UMAX:
	case SO_ATOMIC_UMIN:
	case SO_IMM_ATOMIC_UMAX:
	case SO_IMM_ATOMIC_UMIN:
	case SO_COUNTBITS:
	case SO_BFI:
	case SO_BFREV:
	case SO_UBFE:
	case SO_FIRSTBIT_LO:
	case SO_FIRSTBIT_HI:
	case SO_F16TOF32:
	case SO_F32TOF16:
	case SO_RESINFO:
		sit = SIT_UInt;
		break;

	case SO_DADD:
	case SO_DMAX:
	case SO_DMIN:
	case SO_DMUL:
	case SO_DEQ:
	case SO_DGE:
	case SO_DLT:
	case SO_DNE:
	case SO_DMOV:
	case SO_DMOVC:
	case SO_FTOD:
		sit = SIT_Double;
		break;

	default:
		sit = SIT_Float;
		break;
	}

	return sit;
}

// Return the number of outputs
inline uint32_t GetNumOutputs(uint32_t opcode)
{
	uint32_t ret;
	switch (opcode)
	{
	case SO_SINCOS:
	case SO_SWAPC:
	case SO_IMUL:
	case SO_UMUL:
	case SO_UDIV:
	case SO_UADDC:
	case SO_USUBB:
		ret = 2;
		break;

	case SO_RET:
	case SO_NOP:
	case SO_LOOP:
	case SO_ENDLOOP:
	case SO_BREAK:
	case SO_CONTINUE:
	case SO_DEFAULT:
	case SO_ENDSWITCH:
	case SO_ENDIF:
	case SO_ELSE:
	case SO_EMIT:
	case SO_EMITTHENCUT:
		ret = 0;
		break;

	default:
		ret = 1;
		break;
	}

	return ret;
}

#endif		// _DXBC2GLSL_SHADER_HPP
