/**
 * @file ASMGen.cpp
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

#include <KFL/KFL.hpp>
#include "ASMGen.hpp"
#include "DXBC2GLSL/Utils.hpp"
#include <iomanip>
#include <iostream>

// TODO: we should fix this to output the same syntax as fxc, if sm_dump_short_syntax is set

bool sm_dump_short_syntax = true;

ASMGen::ASMGen(std::shared_ptr<ShaderProgram> const & program)
	: program_(program)
{
}

//disasm operands 
//有immediate32(l(a,b,c,d)) immediate64(d(a,b,c,d)) 变量名（r,v,o)这三种形式
void ASMGen::Disasm(std::ostream& out, ShaderOperand const & op, ShaderImmType imm_type)
{
	if (op.neg)
	{
		out << '-';
	}
	if (op.abs)
	{
		out << '|';
	}
	if (SOT_IMMEDIATE32 == op.type)//l(a,b,c,d)
	{
		out << "l(";
		for (uint32_t i = 0; i < op.comps; ++ i)
		{
			if (i != 0)
			{
				out << ", ";
			}
			switch (imm_type)
			{
			case SIT_Float:
				// Normalized float test
				if (ValidFloat(op.imm_values[i].f32))
				{
					out << op.imm_values[i].f32;
				}
				else
				{
					out << op.imm_values[i].i32;
				}
				break;

			case SIT_Int:
				out << op.imm_values[i].i32;
				break;

			case SIT_UInt:
				out << op.imm_values[i].u32;
				break;

			case SIT_Double:
			default:
				BOOST_ASSERT(false);
				break;
			}
		}
		out << ")";
	}
	else if (SOT_IMMEDIATE64 == op.type)//d(a,b,c,d)
	{
		out << "d(";
		for (uint32_t i = 0; i < op.comps; ++ i)
		{
			if (i != 0)
			{
				out << ", ";
			}
			switch (imm_type)
			{
			case SIT_Float:
			case SIT_Int:
			case SIT_UInt:
			default:
				BOOST_ASSERT(false);
				break;

			case SIT_Double:
				out << op.imm_values[i].f64;
				break;
			}
		}
		out << ")";
	}
	else//应该是类似v1 v2 o0 cb[1]的这种变量名形式
	{
		bool naked = false;
		//判断naked是否为ture
		if (sm_dump_short_syntax)//这个值已经在本文件开头设置为true
		{
			switch (op.type)
			{
			case SOT_TEMP:
			case SOT_OUTPUT:
			case SOT_CONSTANT_BUFFER:
			case SOT_INDEXABLE_TEMP:
			case SOT_UNORDERED_ACCESS_VIEW:
			case SOT_THREAD_GROUP_SHARED_MEMORY:
			case SOT_RESOURCE:
			case SOT_SAMPLER:
			case SOT_STREAM:
				naked = true;
				break;

			default:
				naked = false;
				break;
			}
		}
		//输出变量名如r，v，o等
		out << (sm_dump_short_syntax ? ShaderOperandTypeShortName(op.type) : ShaderOperandTypeName(op.type));

		if (op.indices[0].reg)
		{
			naked = false;//naked为true代表indices[0]里面没有表达式形如r0[数字或表达式],
			//naked为false形如r[数字或表达式]。区别在于有没有常数后缀。
		}
		//索引
		for (uint32_t i = 0; i < op.num_indices; ++ i)
		{
			if (!naked || (i != 0))//第一层索引不需要[]，如cb0[22]0为第一层，22是第二层才有[]
			{
				out << '[';
			}
			if (op.indices[i].reg)
			{
				this->Disasm(out, *op.indices[i].reg, imm_type);
				if (op.indices[i].disp)
				{
					out << '+' << op.indices[i].disp;
				}
			}
			else
			{
				out << op.indices[i].disp;
			}
			if (!naked || (i != 0))
			{
				out << ']';
			}
		}

		if (op.comps)
		{
			switch (op.mode)
			{
			case SOSM_MASK://xy，xyz，xyzw
				out << (sm_dump_short_syntax ? '.' : '!');
				for (uint32_t i = 0; i < op.comps; ++ i)
				{
					if (op.mask & (1 << i))
					{
						out << "xyzw"[i];
					}
				}
				break;

			case SOSM_SWIZZLE://xxyy，xxxx
				out << '.';
				for (uint32_t i = 0; i < op.comps; ++ i)
				{
					out << "xyzw"[op.swizzle[i]];
				}
				break;

			case SOSM_SCALAR:
				out << (sm_dump_short_syntax ? '.' : ':');
				out << "xyzw"[op.swizzle[0]];
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}
	}//end else
	if (op.abs)
	{
		out << '|';
	}
}

//disasm declarations
void ASMGen::Disasm(std::ostream& out, ShaderDecl const & dcl)
{
	out << ShaderOpcodeName(dcl.opcode);
	switch (dcl.opcode)
	{
	case SO_DCL_GLOBAL_FLAGS:
		{
			bool first_flag = true;
			if (dcl.dcl_global_flags.allow_refactoring)
			{
				out << " refactoringAllowed";
				first_flag = false;
			}
			if (dcl.dcl_global_flags.early_depth_stencil)
			{
				if (!first_flag)
				{
					out << " |";
				}
				out << " forceEarlyDepthStencil";
				first_flag = false;
			}
			if (dcl.dcl_global_flags.fp64)
			{
				if (!first_flag)
				{
					out << " |";
				}
				out << " enableDoublePrecisionFloatOps";
				first_flag = false;
			}
			if (dcl.dcl_global_flags.enable_raw_and_structured_in_non_cs)
			{
				if (!first_flag)
				{
					out << " |";
				}
				out << " enableRawAndStructuredBuffers";
				first_flag = false;
			}
			if (dcl.dcl_global_flags.skip_optimization)
			{
				if (!first_flag)
				{
					out << " |";
				}
				out << " skipOptimization";
				first_flag = false;
			}
			if (dcl.dcl_global_flags.enable_minimum_precision)
			{
				if (!first_flag)
				{
					out << " |";
				}
				out << " enableMinimumPrecision";
				first_flag = false;
			}
			if (dcl.dcl_global_flags.enable_raw_and_structured_in_non_cs)
			{
				if (!first_flag)
				{
					out << " |";
				}
				out << " enableDoubleExtensions";
				first_flag = false;
			}
			if (dcl.dcl_global_flags.enable_raw_and_structured_in_non_cs)
			{
				if (!first_flag)
				{
					out << " |";
				}
				out << " enableShaderExtensions";
				first_flag = false;
			}
		}
		break;

	case SO_DCL_INPUT_PS:
	case SO_DCL_INPUT_PS_SIV:
	case SO_DCL_INPUT_PS_SGV:
		out << ' ' << ShaderInterpolationModeName(dcl.dcl_input_ps.interpolation);
		break;

	case SO_DCL_TEMPS:
		out << ' ' << dcl.num;
		break;

	case SO_DCL_INDEXABLE_TEMP:
		dcl.op->type = SOT_INDEXABLE_TEMP;
		break;

	case SO_DCL_RESOURCE:
		out << "_" << ShaderResourceDimensionName(dcl.dcl_resource.target);
		if ((SRD_TEXTURE2DMS == dcl.dcl_resource.target) || (SRD_TEXTURE2DMSARRAY == dcl.dcl_resource.target))
		{
			if (dcl.dcl_resource.nr_samples)
			{
				out << " (" << dcl.dcl_resource.nr_samples << ")";
			}
			out << " (" << ShaderResourceReturnTypeName(dcl.rrt.x)
				<< "," << ShaderResourceReturnTypeName(dcl.rrt.y)
				<< "," << ShaderResourceReturnTypeName(dcl.rrt.z)
				<< "," << ShaderResourceReturnTypeName(dcl.rrt.w)
				<< ")";
		}
		break;

	case SO_DCL_UNORDERED_ACCESS_VIEW_TYPED:
		out << "_" << ShaderResourceDimensionName(dcl.dcl_resource.target);
		out << " (" << ShaderResourceReturnTypeName(dcl.rrt.x)
			<< "," << ShaderResourceReturnTypeName(dcl.rrt.y)
			<< "," << ShaderResourceReturnTypeName(dcl.rrt.z)
			<< "," << ShaderResourceReturnTypeName(dcl.rrt.w)
			<< ")";
		break;

	case SO_IMMEDIATE_CONSTANT_BUFFER:
		{
			float const * data = reinterpret_cast<float const *>(&dcl.data[0]);
			out << "{\n";
			uint32_t vector_num = dcl.num / 4;
			for (uint32_t i = 0; i < vector_num; ++ i)
			{
				if (i != 0)
				{
					out << ",\n";
				}
				out << "{";
				for (int j = 0; j < 4; ++ j)
				{
					// Normalized float test
					if (ValidFloat(data[i * 4 + j]))
					{
						out << data[i * 4 + j];
					}
					else
					{
						out << *reinterpret_cast<int const *>(&data[i * 4 + j]);
					}
					if (j != 3)
					{
						out << ",";
					}
				}
				out << "}";
			}
			out << "\n}\n";
		}
		break;

	default:
		break;
	}
	if (dcl.op)
	{
		out << ' ';
		this->Disasm(out, *dcl.op, GetOpInType(dcl.opcode));
	}
	switch (dcl.opcode)
	{
	case SO_DCL_INDEX_RANGE:
		out << ' ' << dcl.num;
		break;

	case SO_DCL_INDEXABLE_TEMP:
		out << dcl.op->indices[0].disp << "[" << dcl.indexable_temp.num << "]" << ", " << dcl.indexable_temp.comps;
		break;

	case SO_DCL_CONSTANT_BUFFER:
		out << ", " << (dcl.dcl_constant_buffer.dynamic ? "dynamicIndexed" : "immediateIndexed");
		break;

	case SO_DCL_INPUT_SIV:
	case SO_DCL_INPUT_SGV:
	case SO_DCL_OUTPUT_SIV:
	case SO_DCL_OUTPUT_SGV:
	case SO_DCL_INPUT_PS_SIV:
	case SO_DCL_INPUT_PS_SGV:
		out << ", " << ShaderSystemValueName(static_cast<ShaderSystemValue>(dcl.num));
		break;

	case SO_DCL_SAMPLER:
		//out<<", "<<dcl.dcl_sampler.
		if (dcl.dcl_sampler.mono)
		{
			out << ", mode_mono";
		}
		else if (dcl.dcl_sampler.shadow)
		{
			out << ", mode_comparison";
		}
		else
		{
			out << ", mode_default";
		}
		break;

	case SO_DCL_GS_INPUT_PRIMITIVE:
		out << ' ' << ShaderPrimitiveName(dcl.dcl_gs_input_primitive.primitive);
		break;

	case SO_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
		out << ' ' << ShaderPrimitiveTopologyName(dcl.dcl_gs_output_primitive_topology.primitive_topology);
		break;

	case SO_DCL_MAX_OUTPUT_VERTEX_COUNT:
	case SO_DCL_GS_INSTANCE_COUNT:	
		out << ' ' << dcl.num;
		break;

	case SO_DCL_INPUT_CONTROL_POINT_COUNT:
		out << ' ' << dcl.dcl_input_control_point_count.control_points;
		break;

	case SO_DCL_OUTPUT_CONTROL_POINT_COUNT:
		out << ' ' << dcl.dcl_output_control_point_count.control_points;
		break;

	case SO_DCL_TESS_DOMAIN:
		out << ' ' << ShaderTessellatorDomainName(dcl.dcl_tess_domain.domain);
		break;

	case SO_DCL_TESS_PARTITIONING:
		out << ' ' << ShaderTessellatorPartitioningName(dcl.dcl_tess_partitioning.partitioning);
		break;

	case SO_DCL_TESS_OUTPUT_PRIMITIVE:
		out << ' ' << ShaderTessellatorOutputPrimitiveName(dcl.dcl_tess_output_primitive.primitive);
		break;

	case SO_DCL_HS_MAX_TESSFACTOR:
		out << ' ' << dcl.f32;
		break;

	case SO_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
		out << ' ' << dcl.num;
		break;

	case SO_DCL_RESOURCE_STRUCTURED:
	case SO_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
		out << ' ' << dcl.structured.stride;
		break;

	case SO_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED:
		out << ' ' << dcl.structured.stride
			<< ' ' << dcl.structured.count;
		break;

	case SO_DCL_THREAD_GROUP:
		out << ' ' << dcl.thread_group_size[0]
			<< ',' << dcl.thread_group_size[1]
			<< ',' << dcl.thread_group_size[2];
		break;

	default:
		break;
	}
}

//disasm instructions
void ASMGen::Disasm(std::ostream& out, ShaderInstruction const & insn)
{
	ShaderImmType sit = GetOpInType(insn.opcode);

	out << ShaderOpcodeName(insn.opcode);
	if (insn.insn.sat)
	{
		out << "_sat";
	}
	if (SO_SYNC == insn.opcode)
	{
		if (insn.sync.uav_global)
		{
			out << "_uglobal";
		}
		if (insn.sync.uav_group)
		{
			out << "_ugroup";
		}
		if (insn.sync.shared_memory)
		{
			out << "_g";
		}
		if (insn.sync.threads_in_group)
		{
			out << "_t";
		}

	}
	if ((SO_BREAKC == insn.opcode) || (SO_IF == insn.opcode) || (SO_CONTINUEC == insn.opcode)
		|| (SO_RETC == insn.opcode) || (SO_DISCARD == insn.opcode))
	{
		if (insn.insn.test_nz)
		{
			out << "_nz";
		}
		else
		{
			out << "_z";
		}
	}
	if (insn.extended)
	{
		out << " (" << static_cast<int>(insn.sample_offset[0]) << "," << static_cast<int>(insn.sample_offset[1])
			<< "," << static_cast<int>(insn.sample_offset[2]) << ")";
		out << " (" << ShaderResourceDimensionName(static_cast<ShaderResourceDimension>(insn.resource_target)) << ")";
		out << " (" << ShaderResourceReturnTypeName(static_cast<ShaderResourceReturnType>(insn.resource_return_type[0]))
			<< "," << ShaderResourceReturnTypeName(static_cast<ShaderResourceReturnType>(insn.resource_return_type[1]))
			<< "," << ShaderResourceReturnTypeName(static_cast<ShaderResourceReturnType>(insn.resource_return_type[2]))
			<< "," << ShaderResourceReturnTypeName(static_cast<ShaderResourceReturnType>(insn.resource_return_type[3]))
			<< ")";
	}
	if ((SO_RESINFO == insn.opcode) && insn.insn.resinfo_return_type)
	{
		out << "_" << ShaderResInfoReturnTypeName(static_cast<ShaderResInfoReturnType>(insn.insn.resinfo_return_type));
	}
	if (SO_SAMPLE_INFO == insn.opcode)
	{
		out << ShaderSampleInfoReturnTypeName(static_cast<ShaderSampleInfoReturnType>(insn.insn.resinfo_return_type));
	}

	for (uint32_t i = 0; i < insn.num_ops; ++ i)
	{
		if (i != 0)
		{
			out << ',';
		}
		out << ' ';
		this->Disasm(out, *insn.ops[i], sit);
	}
}

void ASMGen::ToASM(std::ostream& out)
{
	//diasm constant buffers
	this->Disasm(out, program_->cbuffers);
	//disasm resource bindings
	this->Disasm(out, program_->resource_bindings);
	//disasm patch constant signature
	this->Disasm(out, program_->params_patch, FOURCC_PCSG);
	//disasm input signature
	this->Disasm(out, program_->params_in, FOURCC_ISGN);
	//disasm output signature
	this->Disasm(out, program_->params_out, FOURCC_OSGN);
	
	out << "pvghdc"[program_->version.type] << "s_" << program_->version.major << "_" << program_->version.minor << "\n";
	for (auto const & dcl : program_->dcls)
	{
		this->Disasm(out, *dcl);
		out << "\n";
	}

	for (auto const & insn : program_->insns)
	{
		this->Disasm(out, *insn);
		out << "\n";
	}
}

void ASMGen::Disasm(std::ostream& out, std::vector<DXBCSignatureParamDesc> const & signature, uint32_t fourcc)
{
	if (signature.empty())return;
	uint32_t count = static_cast<uint32_t>(signature.size());
	switch (fourcc)
	{
	case FOURCC_ISGN:
		out << "//\n//Input Signature:\n//\n";
		break;

	case FOURCC_OSGN:
		out << "//\n//Output Signature:\n//\n";
		break;

	case FOURCC_PCSG:
		out << "//\n//Patch Constant Signature:\n//\n";
		break;

	default:
		BOOST_ASSERT(false);
		break;
	}
	out << "//" << std::setw(15) << "Name"
		<< std::setw(6) << "Index"
		<< std::setw(7) << "Mask"
		<< std::setw(9) << "Register"
		<< std::setw(15) << "ValueType"
		<< std::setw(15) << "ComponentType"
		<< std::setw(10) << "Used"
		<< "\n";
	out << "//" << std::setw(15) << "-------------"
		<< std::setw(6) << "-----"
		<< std::setw(7) << "------"
		<< std::setw(9) << "--------"
		<< std::setw(15) << "-------------"
		<< std::setw(15) << "-------------"
		<< std::setw(10) << "-------"
		<< "\n";
	for (uint32_t i = 0; i < count; ++ i)
	{
		out << "//"
			<< std::setw(15) << signature[i].semantic_name
			<< std::setw(6) << signature[i].semantic_index;
		out << std::setw(4);
		for (uint32_t j = 0; j < 4; ++ j)
		{
			if (signature[i].mask & (1 << j))
			{
				out << "xyzw"[j];
			}
			else
			{
				out << " ";
			}
		}
		out << std::setw(9) << signature[i].register_index
			<< std::setw(15) << ShaderSystemValueName(static_cast<ShaderSystemValue>(signature[i].system_value_type))
			<< std::setw(15) << ShaderRegisterComponentTypeName(signature[i].component_type);
		uint32_t used;
		if (fourcc == FOURCC_OSGN)
		{
			used = ~static_cast<uint32_t>(signature[i].read_write_mask) & 0xF;
		}
		else
		{
			used = static_cast<uint32_t>(signature[i].read_write_mask);
		}
		out << std::setw(7);
		for (uint32_t j = 0; j < 4; ++ j)
		{
			if (used & (1 << j))
			{
				out << "xyzw"[j];
			}
			else
			{
				out << " ";
			}
		}
		out << "\n";
	}
	out << "//\n";
}

void ASMGen::Disasm(std::ostream& out, std::vector<DXBCConstantBuffer> const & cbs)
{
	if (cbs.empty())
	{
		return;
	}
	out << "//\n//Buffer Definitions:\n"
		<< "//\n";
	for (auto const & cb : cbs)
	{
		out << "//" << ShaderCBufferTypeName(cb.desc.type) << " "
			<< cb.desc.name << "\n"
			<< "//{\n";
		
		for (auto const & var : cb.vars)
		{
			BOOST_ASSERT_MSG(var.has_type_desc, "CB member must have type desc");

			//array element count,0 if not a array
			uint32_t element_count = var.type_desc.elements;
			out << "// ";
			switch (var.type_desc.var_class)
			{
			case SVC_MATRIX_ROWS:
				out << "row_major "
					<< std::setw(5) << var.type_desc.name
					<< var.type_desc.rows << "x"
					<< var.type_desc.columns;
				break;

			case SVC_MATRIX_COLUMNS:
				out << std::setw(15) << var.type_desc.name
					<< var.type_desc.rows << "x"
					<< var.type_desc.columns;
				break;

			case SVC_VECTOR:
				out << std::setw(17) << var.type_desc.name
					<< var.type_desc.columns;
				break;

			case SVC_SCALAR:
				out << std::setw(18) << var.type_desc.name;
				break;

				// TODO: to be fixed here
			case SVC_STRUCT:
				out << var.type_desc.name << " " << var.type_desc.offset;
				break;

			default:
				BOOST_ASSERT_MSG(false, "Unhandled type");
				break;
			}
			out << std::setw(20) << var.var_desc.name;
			if (element_count)
			{
				out << "[" << element_count << "]";
			}
			out << ";"
				<< " //" << "Offset: " << std::setw(5)
				<< var.var_desc.start_offset
				<< " Size: " << std::setw(5)
				<< var.var_desc.size;
			if (!var.var_desc.flags)
			{
				out << " [unused]";
			}
			out << "\n";

			// cb default value
			if (var.var_desc.default_val)
			{
				out << "//=";
				this->Disasm(out, var);
				out << "\n";
			}
		}
		out << "//}\n";
	}
}

void ASMGen::Disasm(std::ostream& out, std::vector<DXBCInputBindDesc> const & bindings)
{
	if (bindings.empty())
	{
		return;
	}

	out << "//\n//Resource Definitions:\n"
		<< "//\n";
	out << "//" << std::setw(20) << "Name"
		<< std::setw(30) << "Type"
		<< std::setw(10) << "Slot\n";
	out << "//" << std::setw(20) << "--------------"
		<< std::setw(30) << "--------------------------"
		<< std::setw(10) << "--------\n";
	
	for (auto const & ibd : bindings)
	{
		out << "//";
		out << std::setw(20) << ibd.name
			<< std::setw(30) << ShaderInputTypeName(ibd.type)
			<< std::setw(10) << ibd.bind_point
			<< "\n";
	}
}

void ASMGen::Disasm(std::ostream& out, DXBCShaderVariable const & var,uint32_t offset)
{
	char const * p_base = static_cast<char const *>(var.var_desc.default_val) + offset;
	switch (var.type_desc.var_class)
	{
	case SVC_MATRIX_ROWS:
	case SVC_MATRIX_COLUMNS:
		if (var.type_desc.type != SVT_FLOAT)
		{
			BOOST_ASSERT_MSG(false, "Only support float matrix.");
		}
		if (var.type_desc.rows != var.type_desc.columns)
		{
			BOOST_ASSERT_MSG(false, "Only support square matrix's default value for now.");
		}
		out << "mat" << var.type_desc.rows << "(";
		for (uint32_t column = 0; column < var.type_desc.rows; ++ column)
		{
			for (uint32_t row = 0; row < var.type_desc.rows; ++ row)
			{
				if ((row != 0) || (column != 0))
				{
					out << ",";
				}
				char const * p = p_base;
				if (SVC_MATRIX_COLUMNS == var.type_desc.var_class)
				{
					p += row * 16 + column * 4;
				}
				else
				{
					p += column * 16 + row * 4;
				}
				this->Disasm(out, p, var.type_desc.type);
			}
		}
		out<<" )";
		break;

	case SVC_VECTOR:
		{
			switch(var.type_desc.type)
			{
			case SVT_INT:
				out << "i";
				break;

			case SVT_UINT:
				out << "u";
				break;

			case SVT_FLOAT:
				break;

			default:
				BOOST_ASSERT_MSG(false, "Unhandled type.");
				break;
			}
			out << "vec" << var.type_desc.columns << "(";
			char const * p = p_base;
			for (uint32_t i = 0; i < var.type_desc.columns; ++ i)
			{
				if (i != 0)
				{
					out << ",";
				}
				this->Disasm(out, p, var.type_desc.type);
				p += 4;
			}
			out << ")";
		}
		break;

	case SVC_SCALAR:
		this->Disasm(out, p_base, var.type_desc.type);
		break;

	default:
		BOOST_ASSERT_MSG(false, "Unhandled type");
		break;
	}
}

void ASMGen::Disasm(std::ostream& out, char const * value, ShaderVariableType type)
{
	switch (type)
	{
	case SVT_INT:
		{
			int32_t const * p = reinterpret_cast<int32_t const *>(value);
			out << *p;
		}
		break;

	case SVT_UINT:
		{
			uint32_t const * p = reinterpret_cast<uint32_t const *>(value);
			out << *p;
		}
		break;

	case SVT_FLOAT:
		{
			float const * p = reinterpret_cast<float const *>(value);
			out << *p;
		}
		break;

	default:
		BOOST_ASSERT_MSG(false, "Unhandled type.");
		break;
	}
}

void ASMGen::Disasm(std::ostream& out, DXBCShaderVariable const & var)
{
	if (0 == var.type_desc.elements)
	{
		this->Disasm(out, var, 0);
	}
	else
	{
		uint32_t stride = 0;
		switch (var.type_desc.var_class)
		{
		case SVC_SCALAR:
			stride = 4;
			break;
					
		case SVC_VECTOR:
			stride = 16;
			break;

		case SVC_MATRIX_ROWS:
		case SVC_MATRIX_COLUMNS:
			stride = var.type_desc.columns * 16;
			break;

		default:
			BOOST_ASSERT_MSG(false, "Unhandled type.");
			break;
		}
		out << "{";
		for (uint32_t i = 0; i < var.type_desc.elements; ++ i)
		{
			if (i != 0)
			{
				out << ",";
			}
			this->Disasm(out, var, stride * i);
						
		}
		out<<"}";
	}
}
