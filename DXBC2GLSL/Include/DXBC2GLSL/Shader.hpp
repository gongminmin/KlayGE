/**************************************************************************
 *
 * Copyright 2013 Shenghua Lin, Minmin Gong
 * Copyright 2010 Luca Barbieri
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef _DXBC2GLSL_SHADER_HPP
#define _DXBC2GLSL_SHADER_HPP

#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>
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
	uint32_t type : 16; //range:0-5 对应于"pvghdc" (ps,vs,gs,hs,ds,cs)
};

#pragma warning(push)
#pragma warning(disable: 4201)
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
			uint32_t domain : 3; // D3D_TESSELLATOR_DOMAIN
		} dcl_tess_domain;
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t partitioning : 3; // D3D_TESSELLATOR_PARTITIONING
		} dcl_tess_partitioning;
		struct
		{
			ShaderOpcode opcode : 11;
			uint32_t primitive : 3; // D3D_TESSELLATOR_OUTPUT_PRIMITIVE
		} dcl_tess_output_primitive;
	};
};
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4201)
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
#pragma warning(pop)

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
		boost::shared_ptr<ShaderOperand> reg;
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
	boost::shared_ptr<ShaderOperand> ops[SM_MAX_OPS];

	ShaderInstruction()
		: resource_target(0), num(0), num_ops(0)
	{
		memset(sample_offset, 0, sizeof(sample_offset));
		memset(resource_return_type, 0, sizeof(resource_return_type));
	}
};

struct ShaderDecl : public TokenizedShaderInstruction
{
	boost::shared_ptr<ShaderOperand> op;
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
	std::vector<boost::shared_ptr<ShaderDecl> > dcls;//declarations
	std::vector<boost::shared_ptr<ShaderInstruction> > insns;//instructions

	std::vector<DXBCSignatureParamDesc> params_in; //input signature
	std::vector<DXBCSignatureParamDesc> params_out;//output signature
	std::vector<DXBCSignatureParamDesc> params_patch;//patch signature
	std::vector<DXBCConstantBuffer> cbuffers;//constant buffers(including tbuffer)
	std::vector<DXBCInputBindDesc> resource_bindings;

	ShaderPrimitive gs_input_primitive;
	std::vector<ShaderPrimitiveTopology> gs_output_topology;
	uint32_t max_gs_output_vertex;

	ShaderProgram()
		: gs_input_primitive(SP_Undefined), max_gs_output_vertex(0)
	{
		memset(&version, 0, sizeof(version));
	}
};

boost::shared_ptr<ShaderProgram> ShaderParse(DXBCContainer const & dxbc);

//获取操作码对应的操作数的类型
inline ShaderImmType GetImmType(uint32_t opcode)
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

#endif		// _DXBC2GLSL_SHADER_HPP
