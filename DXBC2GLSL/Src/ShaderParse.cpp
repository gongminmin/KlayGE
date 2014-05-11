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

#include <DXBC2GLSL/Shader.hpp>
#include <DXBC2GLSL/Utils.hpp>

namespace
{
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
	struct DXBCSignatureParameterD3D10
	{
		uint32_t name_offset;
		uint32_t semantic_index;
		uint32_t system_value_type;
		uint32_t component_type;
		uint32_t register_num;
		uint8_t mask;
		uint8_t read_write_mask;
		uint8_t padding[2];
	};

	struct DXBCSignatureParameterD3D11
	{
		uint32_t stream;
		uint32_t name_offset;
		uint32_t semantic_index;
		uint32_t system_value_type;
		uint32_t component_type;
		uint32_t register_num;
		uint8_t mask;
		uint8_t read_write_mask;
		uint8_t padding[2];
	};

	struct DXBCSignatureParameterD3D11_1
	{
		uint32_t stream;
		uint32_t name_offset;
		uint32_t semantic_index;
		uint32_t system_value_type;
		uint32_t component_type;
		uint32_t register_num;
		uint8_t mask;
		uint8_t read_write_mask;
		uint8_t padding[2];
		uint32_t min_precision;
	};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

	bool SortFuncLess(DXBCShaderVariable const & lh, DXBCShaderVariable const & rh)
	{
		return lh.var_desc.start_offset < rh.var_desc.start_offset;
	}
}

struct ShaderParser
{
	uint32_t const * tokens;//shader tokens
	uint32_t const * tokens_end;
	DXBCChunkHeader const * resource_chunk;//resource definition and constant buffer chunk
	DXBCChunkSignatureHeader const * input_signature;
	DXBCChunkSignatureHeader const * output_signature;
	DXBCChunkSignatureHeader const * patch_constant_signature;
	KlayGE::shared_ptr<ShaderProgram> program;

	ShaderParser(const DXBCContainer& dxbc, KlayGE::shared_ptr<ShaderProgram> const & program)
		: program(program)
	{
		resource_chunk = dxbc.resource_chunk;
		input_signature = reinterpret_cast<DXBCChunkSignatureHeader const *>(dxbc.input_signature);
		output_signature = reinterpret_cast<DXBCChunkSignatureHeader const *>(dxbc.output_signature);
		patch_constant_signature = reinterpret_cast<DXBCChunkSignatureHeader const *>(dxbc.patch_constant_signature);
		uint32_t size = dxbc.shader_chunk->size;
		KlayGE::LittleEndianToNative<sizeof(size)>(&size);
		tokens = reinterpret_cast<uint32_t const *>(dxbc.shader_chunk + 1);
		tokens_end = reinterpret_cast<uint32_t const *>(reinterpret_cast<char const *>(tokens) + size);
	}

	uint32_t Read32()
	{
		BOOST_ASSERT(tokens < tokens_end);
		uint32_t cur_token = *tokens;
		KlayGE::LittleEndianToNative<sizeof(cur_token)>(&cur_token);
		++ tokens; 
		return cur_token;
	}

	template <typename T>
	void ReadToken(T* tok)
	{
		*reinterpret_cast<uint32_t*>(tok) = this->Read32();
	}

	uint64_t Read64()
	{
		uint32_t a = this->Read32();
		uint32_t b = this->Read32();
		return static_cast<uint64_t>(a) | (static_cast<uint64_t>(b) << 32);
	}

	void Skip(uint32_t toskip)
	{
		tokens += toskip;
	}

	void ReadOp(ShaderOperand& op)
	{
		TokenizedShaderOperand optok;
		this->ReadToken(&optok);
		BOOST_ASSERT(optok.op_type < SOT_COUNT);
		op.swizzle[0] = 0;
		op.swizzle[1] = 1;
		op.swizzle[2] = 2;
		op.swizzle[3] = 3;
		op.mask = 0xF;
		switch (optok.comps_enum)
		{
		case SONC_0:
			op.comps = 0;
			break;

		case SONC_1:
			op.comps = 1;
			op.swizzle[1] = op.swizzle[2] = op.swizzle[3] = 0;
			break;

		case SONC_4:
			op.comps = 4;
			op.mode = optok.mode;
			switch (optok.mode)
			{
			case SOSM_MASK:
				op.mask = SM_OPERAND_SEL_MASK(optok.sel);
				break;

			case SOSM_SWIZZLE:
				op.swizzle[0] = SM_OPERAND_SEL_SWZ(optok.sel, 0);
				op.swizzle[1] = SM_OPERAND_SEL_SWZ(optok.sel, 1);
				op.swizzle[2] = SM_OPERAND_SEL_SWZ(optok.sel, 2);
				op.swizzle[3] = SM_OPERAND_SEL_SWZ(optok.sel, 3);
				break;

			case SOSM_SCALAR:
				op.swizzle[0] = op.swizzle[1] = op.swizzle[2] = op.swizzle[3] = SM_OPERAND_SEL_SCALAR(optok.sel);
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			break;

		default:
			BOOST_ASSERT_MSG(false, "Unhandled operand component type");
			break;
		}
		op.type = static_cast<ShaderOperandType>(optok.op_type);
		op.num_indices = optok.num_indices;

		if (optok.extended)
		{
			TokenizedShaderOperandExtended optokext;
			this->ReadToken(&optokext);
			if (0 == optokext.type)
			{
			}
			else if (1 == optokext.type)
			{
				op.neg = optokext.neg;
				op.abs = optokext.abs;
			}
			else
			{
				BOOST_ASSERT_MSG(false, "Unhandled extended operand token type");
			}
		}

		for (uint32_t i = 0; i < op.num_indices; ++ i)
		{
			uint32_t repr;
			if (0 == i)
			{
				repr = optok.index0_repr;
			}
			else if (1 == i)
			{
				repr = optok.index1_repr;
			}
			else if (2 == i)
			{
				repr = optok.index2_repr;
			}
			else
			{
				BOOST_ASSERT_MSG(false, "Unhandled operand index representation");
				repr = 0;
			}
			op.indices[i].disp = 0;
			// TODO: is disp supposed to be signed here??
			switch (repr)
			{
			case SOIP_IMM32:
				op.indices[i].disp = static_cast<int32_t>(this->Read32());
				break;

			case SOIP_IMM64:
				op.indices[i].disp = this->Read64();
				break;

			case SOIP_RELATIVE:
				op.indices[i].reg = KlayGE::MakeSharedPtr<ShaderOperand>();
				this->ReadOp(*op.indices[i].reg);
				break;

			case SOIP_IMM32_PLUS_RELATIVE:
				op.indices[i].disp = static_cast<int32_t>(this->Read32());
				op.indices[i].reg = KlayGE::MakeSharedPtr<ShaderOperand>();
				this->ReadOp(*op.indices[i].reg);
				break;

			case SOIP_IMM64_PLUS_RELATIVE:
				op.indices[i].disp = this->Read64();
				op.indices[i].reg = KlayGE::MakeSharedPtr<ShaderOperand>();
				this->ReadOp(*op.indices[i].reg);
				break;
			}
		}

		if (SOT_IMMEDIATE32 == op.type)
		{
			for (uint32_t i = 0; i < op.comps; ++ i)
			{
				op.imm_values[i].i32 = this->Read32();
			}
		}
		else if (SOT_IMMEDIATE64 == op.type)
		{
			for (uint32_t i = 0; i < op.comps; ++ i)
			{
				op.imm_values[i].i64 = this->Read64();
			}
		}
	}

	void ParseShader()
	{
		this->ReadToken(&program->version);

		uint32_t lentok = this->Read32();
		tokens_end = tokens - 2 + lentok;

		uint32_t cur_gs_stream = 0;

		while (tokens != tokens_end)
		{
			TokenizedShaderInstruction insntok;
			this->ReadToken(&insntok);
			uint32_t const * insn_end = tokens - 1 + insntok.length;
			ShaderOpcode opcode = insntok.opcode;
			BOOST_ASSERT(opcode < SO_COUNT);

			if (SO_IMMEDIATE_CONSTANT_BUFFER == opcode)
			{
				// immediate constant buffer data
				uint32_t customlen = this->Read32() - 2;

				KlayGE::shared_ptr<ShaderDecl> dcl = KlayGE::MakeSharedPtr<ShaderDecl>();
				program->dcls.push_back(dcl);

				dcl->opcode = SO_IMMEDIATE_CONSTANT_BUFFER;
				dcl->num = customlen;
				dcl->data.resize(customlen * sizeof(tokens[0]));

				memcpy(&dcl->data[0], &tokens[0], customlen * sizeof(tokens[0]));

				this->Skip(customlen);
				continue;
			}

			if ((SO_HS_FORK_PHASE == opcode) || (SO_HS_JOIN_PHASE == opcode) || (SO_HS_CONTROL_POINT_PHASE == opcode) || (SO_HS_DECLS == opcode ))
			{
				// need to interleave these with the declarations or we cannot
				// assign fork/join phase instance counts to phases
				KlayGE::shared_ptr<ShaderDecl> dcl = KlayGE::MakeSharedPtr<ShaderDecl>();
				program->dcls.push_back(dcl);
				dcl->opcode = opcode;
			}

			if (((opcode >= SO_DCL_RESOURCE) && (opcode <= SO_DCL_GLOBAL_FLAGS))
				|| ((opcode >= SO_DCL_STREAM) && (opcode <= SO_DCL_RESOURCE_STRUCTURED))
				|| (SO_DCL_GS_INSTANCE_COUNT == opcode))
			{
				KlayGE::shared_ptr<ShaderDecl> dcl = KlayGE::MakeSharedPtr<ShaderDecl>();
				program->dcls.push_back(dcl);
				reinterpret_cast<TokenizedShaderInstruction&>(*dcl) = insntok;

				TokenizedShaderInstructionExtended exttok;
				memcpy(&exttok, &insntok, sizeof(exttok));
				while(exttok.extended)
				{
					this->ReadToken(&exttok);
				}

#define READ_OP_ANY dcl->op = KlayGE::MakeSharedPtr<ShaderOperand>(); this->ReadOp(*dcl->op);
#define READ_OP(FILE) READ_OP_ANY
				//check(dcl->op->file == SOT_##FILE);

				switch (opcode)
				{
				case SO_DCL_GLOBAL_FLAGS:
					break;

				case SO_DCL_RESOURCE:
					READ_OP(RESOURCE);
					this->ReadToken(&dcl->rrt);
					break;

				case SO_DCL_SAMPLER:
					READ_OP(SAMPLER);
					break;

				case SO_DCL_INPUT:
				case SO_DCL_INPUT_PS:
					READ_OP(INPUT);
					break;

				case SO_DCL_INPUT_SIV:
				case SO_DCL_INPUT_SGV:
				case SO_DCL_INPUT_PS_SIV:
				case SO_DCL_INPUT_PS_SGV:
					READ_OP(INPUT);
					dcl->sv = static_cast<ShaderSystemValue>(static_cast<uint16_t>(this->Read32()));
					break;

				case SO_DCL_OUTPUT:
					READ_OP(OUTPUT);
					break;

				case SO_DCL_OUTPUT_SIV:
				case SO_DCL_OUTPUT_SGV:
					READ_OP(OUTPUT);
					dcl->sv = static_cast<ShaderSystemValue>(static_cast<uint16_t>(this->Read32()));
					break;

				case SO_DCL_INDEX_RANGE:
					READ_OP_ANY;
					BOOST_ASSERT((SOT_INPUT == dcl->op->type) || (SOT_OUTPUT == dcl->op->type));
					dcl->num = this->Read32();
					break;

				case SO_DCL_TEMPS:
					dcl->num = this->Read32();
					break;

				case SO_DCL_INDEXABLE_TEMP:
					dcl->op = KlayGE::MakeSharedPtr<ShaderOperand>();
					dcl->op->indices[0].disp = this->Read32();
					dcl->indexable_temp.num = this->Read32();
					dcl->indexable_temp.comps = this->Read32();
					break;

				case SO_DCL_CONSTANT_BUFFER:
					READ_OP(CONSTANT_BUFFER);
					break;

				case SO_DCL_GS_INPUT_PRIMITIVE:
					program->gs_input_primitive = dcl->dcl_gs_input_primitive.primitive;
					break;

				case SO_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
					program->gs_output_topology[cur_gs_stream]
						= dcl->dcl_gs_output_primitive_topology.primitive_topology;
					break;

				case SO_DCL_MAX_OUTPUT_VERTEX_COUNT:
					dcl->num = this->Read32();
					program->max_gs_output_vertex = dcl->num;
					break;

				case SO_DCL_GS_INSTANCE_COUNT:
					dcl->num = this->Read32();
					program->gs_instance_count = dcl->num;
					break;
				
				case SO_DCL_TESS_OUTPUT_PRIMITIVE:
					program->ds_tessellator_output_primitive = dcl->dcl_tess_output_primitive.primitive;
					break;

				case SO_DCL_TESS_PARTITIONING:
					program->ds_tessellator_partitioning = dcl->dcl_tess_partitioning.partitioning;
					break;

				case SO_DCL_TESS_DOMAIN:
					program->ds_tessellator_domain = dcl->dcl_tess_domain.domain;
					break;

				case SO_DCL_OUTPUT_CONTROL_POINT_COUNT:
					program->hs_output_control_point_count = dcl->dcl_output_control_point_count.control_points;
					break;

				case SO_DCL_INPUT_CONTROL_POINT_COUNT:
					program->hs_input_control_point_count = dcl->dcl_input_control_point_count.control_points;
					break;

				case SO_DCL_HS_MAX_TESSFACTOR:
					{
					uint32_t ret=this->Read32();
					float* pf=reinterpret_cast<float*>(&ret);
					dcl->f32 = *pf;
					}
					break;

				case SO_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
					dcl->num = this->Read32();
					break;

				case SO_DCL_FUNCTION_BODY:
					dcl->num = this->Read32();
					break;

				case SO_DCL_FUNCTION_TABLE:
					dcl->num = this->Read32();
					dcl->data.resize(dcl->num * sizeof(uint32_t));
					for (uint32_t i = 0; i < dcl->num; ++ i)
					{
						(reinterpret_cast<uint32_t*>(&dcl->data[0]))[i] = this->Read32();
					}
					break;

				case SO_DCL_INTERFACE:
					dcl->intf.id = this->Read32();
					dcl->intf.expected_function_table_length = this->Read32();
					{
						uint32_t v = this->Read32();
						dcl->intf.table_length = v & 0xffff;
						dcl->intf.array_length = v >> 16;
					}
					dcl->data.resize(dcl->intf.table_length * sizeof(uint32_t));
					for (uint32_t i = 0; i < dcl->intf.table_length; ++ i)
					{
						(reinterpret_cast<uint32_t*>(&dcl->data[0]))[i] = this->Read32();
					}
					break;

				case SO_DCL_THREAD_GROUP:
					dcl->thread_group_size[0] = this->Read32();
					dcl->thread_group_size[1] = this->Read32();
					dcl->thread_group_size[2] = this->Read32();
					program->cs_thread_group_size[0] = dcl->thread_group_size[0];
					program->cs_thread_group_size[1] = dcl->thread_group_size[1];
					program->cs_thread_group_size[2] = dcl->thread_group_size[2];
					break;

				case SO_DCL_UNORDERED_ACCESS_VIEW_TYPED:
					READ_OP(UNORDERED_ACCESS_VIEW);
					this->ReadToken(&dcl->rrt);
					break;

				case SO_DCL_UNORDERED_ACCESS_VIEW_RAW:
					READ_OP(UNORDERED_ACCESS_VIEW);
					break;

				case SO_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
					READ_OP(UNORDERED_ACCESS_VIEW);
					dcl->structured.stride = this->Read32();
					break;

				case SO_DCL_THREAD_GROUP_SHARED_MEMORY_RAW:
					READ_OP(THREAD_GROUP_SHARED_MEMORY);
					dcl->num = this->Read32();
					break;

				case SO_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED:
					READ_OP(THREAD_GROUP_SHARED_MEMORY);
					dcl->structured.stride = this->Read32();
					dcl->structured.count = this->Read32();
					break;

				case SO_DCL_RESOURCE_RAW:
					READ_OP(RESOURCE);
					break;

				case SO_DCL_RESOURCE_STRUCTURED:
					READ_OP(RESOURCE);
					dcl->structured.stride = this->Read32();
					break;

				case SO_DCL_STREAM:
					READ_OP(STREAM);
					cur_gs_stream = static_cast<uint32_t>(dcl->op->indices[0].disp);
					program->gs_output_topology.push_back(SPT_Undefined);
					break;

				default:
					BOOST_ASSERT_MSG(false, "Unhandled declaration");
					break;
				}

				BOOST_ASSERT(tokens == insn_end);
			}
			else
			{
				if(SO_HS_DECLS == opcode)continue;
				KlayGE::shared_ptr<ShaderInstruction> insn = KlayGE::MakeSharedPtr<ShaderInstruction>();
				program->insns.push_back(insn);
				reinterpret_cast<TokenizedShaderInstruction&>(*insn) = insntok;

				TokenizedShaderInstructionExtended exttok;
				memcpy(&exttok, &insntok, sizeof(exttok));
				while (exttok.extended)
				{
					this->ReadToken(&exttok);
					if (SEOP_SAMPLE_CONTROLS == exttok.type)
					{
						insn->sample_offset[0] = exttok.sample_controls.offset_u;
						insn->sample_offset[1] = exttok.sample_controls.offset_v;
						insn->sample_offset[2] = exttok.sample_controls.offset_w;
					}
					else if (SEOP_RESOURCE_DIM == exttok.type)
					{
						insn->resource_target = exttok.resource_target.target;
					}
					else if (SEOP_RESOURCE_RETURN_TYPE == exttok.type)
					{
						insn->resource_return_type[0] = exttok.resource_return_type.x;
						insn->resource_return_type[1] = exttok.resource_return_type.y;
						insn->resource_return_type[2] = exttok.resource_return_type.z;
						insn->resource_return_type[3] = exttok.resource_return_type.w;
					}
				}

				switch (opcode)
				{
				case SO_INTERFACE_CALL:
					insn->num = this->Read32();
					break;
		
				default:
					break;
				}

				uint32_t op_num = 0;
				while (tokens != insn_end)
				{
					BOOST_ASSERT(tokens < insn_end);
					BOOST_ASSERT(op_num < SM_MAX_OPS);
					insn->ops[op_num] = KlayGE::MakeSharedPtr<ShaderOperand>();
					this->ReadOp(*insn->ops[op_num]);
					++ op_num;
				}
				insn->num_ops = op_num;
			}
		}
	}

	char const * Parse()
	{
		this->ParseShader();
		
		if (resource_chunk)
		{
			this->ParseCBAndResourceBinding();
			this->SortCBVars();
		}
		
		if (input_signature)
		{
			if (FOURCC_ISG1 == input_signature->fourcc)
			{
				this->ParseSignature(input_signature, FOURCC_ISG1);
			}
			else
			{
				BOOST_ASSERT(FOURCC_ISGN == input_signature->fourcc);
				this->ParseSignature(input_signature, FOURCC_ISGN);
			}
		}

		if (output_signature)
		{
			if (FOURCC_OSG1 == output_signature->fourcc)
			{
				this->ParseSignature(output_signature, FOURCC_OSG1);
			}
			else if (FOURCC_OSG5 == output_signature->fourcc)
			{
				this->ParseSignature(output_signature, FOURCC_OSG5);
			}
			else
			{
				BOOST_ASSERT(FOURCC_OSGN == output_signature->fourcc);
				this->ParseSignature(output_signature, FOURCC_OSGN);
			}
		}

		if(patch_constant_signature)
		{
			this->ParseSignature(patch_constant_signature, FOURCC_PCSG);
		}

		return nullptr;
	}

	uint32_t ParseCBAndResourceBinding() const
	{
		BOOST_ASSERT_MSG(FOURCC_RDEF == resource_chunk->fourcc, "parameter chunk is not a resource chunk,parse_constant_buffer()");

		uint32_t const * tokens = reinterpret_cast<uint32_t const *>(resource_chunk + 1);
		uint32_t const * first_token = tokens;
		uint32_t num_cb = *tokens;
		KlayGE::LittleEndianToNative<sizeof(num_cb)>(&num_cb);
		++ tokens;
		uint32_t cb_offset = *tokens;
		KlayGE::LittleEndianToNative<sizeof(cb_offset)>(&cb_offset);
		++ tokens;

		uint32_t num_resource_bindings = *tokens;
		KlayGE::LittleEndianToNative<sizeof(num_resource_bindings)>(&num_resource_bindings);
		++ tokens;
		uint32_t resource_binding_offset = *tokens;
		KlayGE::LittleEndianToNative<sizeof(resource_binding_offset)>(&resource_binding_offset);
		++ tokens;
		uint32_t shader_model = *tokens;
		KlayGE::LittleEndianToNative<sizeof(shader_model)>(&shader_model);
		++ tokens;
		// TODO: check here, shader_model is unused.
		shader_model = shader_model;
		uint32_t compile_flags = *tokens;
		KlayGE::LittleEndianToNative<sizeof(compile_flags)>(&compile_flags);
		++ tokens;
		// TODO: check here, compile_flags is unused.
		compile_flags = compile_flags;

		uint32_t const * resource_binding_tokens = reinterpret_cast<uint32_t const *>(reinterpret_cast<char const *>(first_token) + resource_binding_offset);
		program->resource_bindings.resize(num_resource_bindings);
		for (uint32_t i = 0; i < num_resource_bindings; ++ i)
		{
			DXBCInputBindDesc& bind = program->resource_bindings[i];
			uint32_t name_offset = *resource_binding_tokens;
			KlayGE::LittleEndianToNative<sizeof(name_offset)>(&name_offset);
			++ resource_binding_tokens;
			bind.name = reinterpret_cast<char const *>(first_token) + name_offset;
			bind.type = static_cast<ShaderInputType>(*resource_binding_tokens);
			KlayGE::LittleEndianToNative<sizeof(bind.type)>(&bind.type);
			++ resource_binding_tokens;
			bind.return_type = static_cast<ShaderResourceReturnType>(*resource_binding_tokens);
			KlayGE::LittleEndianToNative<sizeof(bind.return_type)>(&bind.return_type);
			++ resource_binding_tokens;
			bind.dimension = static_cast<ShaderSRVDimension>(*resource_binding_tokens);
			KlayGE::LittleEndianToNative<sizeof(bind.dimension)>(&bind.dimension);
			++ resource_binding_tokens;
			bind.num_samples = *resource_binding_tokens;
			KlayGE::LittleEndianToNative<sizeof(bind.num_samples)>(&bind.num_samples);
			++ resource_binding_tokens;
			bind.bind_point = *resource_binding_tokens;
			KlayGE::LittleEndianToNative<sizeof(bind.bind_point)>(&bind.bind_point);
			++ resource_binding_tokens;
			bind.bind_count = *resource_binding_tokens;
			KlayGE::LittleEndianToNative<sizeof(bind.bind_count)>(&bind.bind_count);
			++ resource_binding_tokens;
			bind.flags = *resource_binding_tokens;
			KlayGE::LittleEndianToNative<sizeof(bind.flags)>(&bind.flags);
			++ resource_binding_tokens;
		}

		uint32_t const * cb_tokens = reinterpret_cast<uint32_t const *>(reinterpret_cast<char const *>(first_token) + cb_offset);
		program->cbuffers.resize(num_cb);

		for (uint32_t i = 0; i < num_cb; ++ i)
		{
			DXBCConstantBuffer& cb=program->cbuffers[i];
			uint32_t name_offset = *cb_tokens;
			KlayGE::LittleEndianToNative<sizeof(name_offset)>(&name_offset);
			++ cb_tokens;
			uint32_t var_count = *cb_tokens;
			KlayGE::LittleEndianToNative<sizeof(var_count)>(&var_count);
			++ cb_tokens;
			uint32_t var_offset = *cb_tokens;
			KlayGE::LittleEndianToNative<sizeof(var_offset)>(&var_offset);
			++ cb_tokens;
			const uint32_t* var_token = reinterpret_cast<uint32_t const *>(reinterpret_cast<char const *>(first_token) + var_offset);
			cb.vars.resize(var_count);
			for (uint32_t j = 0; j < var_count; ++ j)
			{
				DXBCShaderVariable& var = cb.vars[j];
				uint32_t name_offset = *var_token;
				KlayGE::LittleEndianToNative<sizeof(name_offset)>(&name_offset);
				++ var_token;
				var.var_desc.name = reinterpret_cast<char const *>(first_token) + name_offset;
				var.var_desc.start_offset = *var_token;
				KlayGE::LittleEndianToNative<sizeof(var.var_desc.start_offset)>(&var.var_desc.start_offset);
				++ var_token;
				var.var_desc.size = *var_token;
				KlayGE::LittleEndianToNative<sizeof(var.var_desc.size)>(&var.var_desc.size);
				++ var_token;
				var.var_desc.flags = *var_token;
				KlayGE::LittleEndianToNative<sizeof(var.var_desc.flags)>(&var.var_desc.flags);
				++ var_token;
				uint32_t type_offset = *var_token;
				KlayGE::LittleEndianToNative<sizeof(type_offset)>(&type_offset);
				++ var_token;
				uint32_t default_value_offset = *var_token;
				KlayGE::LittleEndianToNative<sizeof(default_value_offset)>(&default_value_offset);
				++ var_token;

				if (program->version.major >= 5)
				{
					var.var_desc.start_texture = *var_token;
					++ var_token;
					var.var_desc.texture_size = *var_token;
					++ var_token;
					var.var_desc.start_sampler = *var_token;
					++ var_token;
					var.var_desc.sampler_size = *var_token;
					++ var_token;

					KlayGE::LittleEndianToNative<sizeof(var.var_desc.start_texture)>(&var.var_desc.start_texture);
					KlayGE::LittleEndianToNative<sizeof(var.var_desc.texture_size)>(&var.var_desc.texture_size);
					KlayGE::LittleEndianToNative<sizeof(var.var_desc.start_sampler)>(&var.var_desc.start_sampler);
					KlayGE::LittleEndianToNative<sizeof(var.var_desc.sampler_size)>(&var.var_desc.sampler_size);

				}
				if (default_value_offset)
				{
					var.var_desc.default_val = reinterpret_cast<char const *>(first_token) + default_value_offset;
				}
				else
				{
					var.var_desc.default_val = nullptr;
				}
				if (type_offset)
				{
					var.has_type_desc = true;
					uint16_t const * type_token = reinterpret_cast<uint16_t const *>(reinterpret_cast<char const *>(first_token) + type_offset);

					uint16_t tmp;

					tmp = *type_token;
					KlayGE::LittleEndianToNative<sizeof(tmp)>(&tmp);
					var.type_desc.var_class = static_cast<ShaderVariableClass>(tmp);
					++ type_token;

					tmp = *type_token;
					KlayGE::LittleEndianToNative<sizeof(tmp)>(&tmp);
					var.type_desc.type = static_cast<ShaderVariableType>(tmp);
					++ type_token;

					tmp = *type_token;
					KlayGE::LittleEndianToNative<sizeof(tmp)>(&tmp);
					var.type_desc.rows = tmp;
					++ type_token;
					tmp = *type_token;
					KlayGE::LittleEndianToNative<sizeof(tmp)>(&tmp);
					var.type_desc.columns = tmp;
					++ type_token;

					tmp = *type_token;
					KlayGE::LittleEndianToNative<sizeof(tmp)>(&tmp);
					var.type_desc.elements = tmp;
					++ type_token;
					tmp = *type_token;
					KlayGE::LittleEndianToNative<sizeof(tmp)>(&tmp);
					var.type_desc.members = tmp;
					++ type_token;

					tmp = *type_token;
					KlayGE::LittleEndianToNative<sizeof(tmp)>(&tmp);
					uint32_t var_member_offset = tmp << 16;
					++ type_token;
					tmp = *type_token;
					KlayGE::LittleEndianToNative<sizeof(tmp)>(&tmp);
					var_member_offset |= tmp;
					++ type_token;
					
					var.type_desc.offset = var_member_offset;
					var.type_desc.name = ShaderVariableTypeName(var.type_desc.type);
				}
				else
				{
					var.has_type_desc = false;
				}
			}

			cb.desc.name = reinterpret_cast<char const *>(first_token) + name_offset;
			cb.desc.size = *cb_tokens;
			++ cb_tokens;
			cb.desc.flags = *cb_tokens;
			++ cb_tokens;
			cb.desc.type = static_cast<ShaderCBufferType>(*cb_tokens);
			++ cb_tokens;
			cb.desc.variables = var_count;
			cb.bind_point = this->GetCBBindPoint(cb.desc.name);
		}

		return num_cb;
	}

	uint32_t GetCBBindPoint(char const * name) const
	{
		for (std::vector<DXBCInputBindDesc>::const_iterator itr = program->resource_bindings.begin();
			itr != program->resource_bindings.end(); ++ itr)
		{
			if (0 == strcmp(itr->name, name))
			{
				return itr->bind_point;
			}
		}

		BOOST_ASSERT(false);
		return static_cast<uint32_t>(-1);
	}

	uint32_t ParseSignature(DXBCChunkSignatureHeader const * sig, uint32_t fourcc) const
	{
		std::vector<DXBCSignatureParamDesc>* params = nullptr;
		switch (fourcc)
		{
		case FOURCC_ISG1:
		case FOURCC_ISGN:
			params = &program->params_in;
			break;

		case FOURCC_OSG1:
		case FOURCC_OSG5:
		case FOURCC_OSGN:
			params = &program->params_out;
			break;

		case FOURCC_PCSG:
			params = &program->params_patch;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		uint32_t offset = sig->offset;
		KlayGE::LittleEndianToNative<sizeof(offset)>(&offset);
		void const * elements_base
			= static_cast<void const *>(reinterpret_cast<uint8_t const *>(sig)
				+ sizeof(DXBCChunkHeader) + offset);

		uint32_t count = sig->count;
		KlayGE::LittleEndianToNative<sizeof(count)>(&count);
		params->resize(count);

		if ((FOURCC_ISG1 == fourcc) || (FOURCC_OSG1 == fourcc))
		{
			DXBCSignatureParameterD3D11_1 const * elements
				= reinterpret_cast<DXBCSignatureParameterD3D11_1 const *>(elements_base);
			for (uint32_t i = 0; i < count; ++ i)
			{
				DXBCSignatureParamDesc& param = (*params)[i];
				uint32_t name_offset = elements[i].name_offset;
				KlayGE::LittleEndianToNative<sizeof(name_offset)>(&name_offset);
				param.semantic_name = reinterpret_cast<char const *>(sig) + sizeof(DXBCChunkHeader) + name_offset;
				param.semantic_index = elements[i].semantic_index;
				param.system_value_type = static_cast<ShaderName>(elements[i].system_value_type);
				param.component_type = static_cast<ShaderRegisterComponentType>(elements[i].component_type);
				param.register_index = elements[i].register_num;
				param.mask = elements[i].mask;
				param.read_write_mask = elements[i].read_write_mask;
				param.stream = elements[i].stream;
				param.min_precision = elements[i].min_precision;

				KlayGE::LittleEndianToNative<sizeof(param.semantic_index)>(&param.semantic_index);
				KlayGE::LittleEndianToNative<sizeof(param.system_value_type)>(&param.system_value_type);
				KlayGE::LittleEndianToNative<sizeof(param.component_type)>(&param.component_type);
				KlayGE::LittleEndianToNative<sizeof(param.register_index)>(&param.register_index);
				KlayGE::LittleEndianToNative<sizeof(param.stream)>(&param.stream);
				KlayGE::LittleEndianToNative<sizeof(param.min_precision)>(&param.min_precision);
			}
		}
		else if (FOURCC_OSG5 == fourcc)
		{
			DXBCSignatureParameterD3D11 const * elements
				= reinterpret_cast<DXBCSignatureParameterD3D11 const *>(elements_base);
			for (uint32_t i = 0; i < count; ++ i)
			{
				DXBCSignatureParamDesc& param = (*params)[i];
				uint32_t name_offset = elements[i].name_offset;
				KlayGE::LittleEndianToNative<sizeof(name_offset)>(&name_offset);
				param.semantic_name = reinterpret_cast<char const *>(sig) + sizeof(DXBCChunkHeader) + name_offset;
				param.semantic_index = elements[i].semantic_index;
				param.system_value_type = static_cast<ShaderName>(elements[i].system_value_type);
				param.component_type = static_cast<ShaderRegisterComponentType>(elements[i].component_type);
				param.register_index = elements[i].register_num;
				param.mask = elements[i].mask;
				param.read_write_mask = elements[i].read_write_mask;
				param.stream = elements[i].stream;
				param.min_precision = 0;

				KlayGE::LittleEndianToNative<sizeof(param.semantic_index)>(&param.semantic_index);
				KlayGE::LittleEndianToNative<sizeof(param.system_value_type)>(&param.system_value_type);
				KlayGE::LittleEndianToNative<sizeof(param.component_type)>(&param.component_type);
				KlayGE::LittleEndianToNative<sizeof(param.register_index)>(&param.register_index);
				KlayGE::LittleEndianToNative<sizeof(param.stream)>(&param.stream);
			}
		}
		else
		{
			BOOST_ASSERT((FOURCC_ISGN == fourcc) || (FOURCC_OSGN == fourcc) || (FOURCC_PCSG == fourcc));

			DXBCSignatureParameterD3D10 const * elements
				= reinterpret_cast<DXBCSignatureParameterD3D10 const *>(elements_base);
			for (uint32_t i = 0; i < count; ++ i)
			{
				DXBCSignatureParamDesc& param = (*params)[i];
				uint32_t name_offset = elements[i].name_offset;
				KlayGE::LittleEndianToNative<sizeof(name_offset)>(&name_offset);
				param.semantic_name = reinterpret_cast<char const *>(sig) + sizeof(DXBCChunkHeader) + name_offset;
				param.semantic_index = elements[i].semantic_index;
				param.system_value_type = static_cast<ShaderName>(elements[i].system_value_type);
				param.component_type = static_cast<ShaderRegisterComponentType>(elements[i].component_type);
				param.register_index = elements[i].register_num;
				param.mask = elements[i].mask;
				param.read_write_mask = elements[i].read_write_mask;
				param.stream = 0;
				param.min_precision = 0;

				KlayGE::LittleEndianToNative<sizeof(param.semantic_index)>(&param.semantic_index);
				KlayGE::LittleEndianToNative<sizeof(param.system_value_type)>(&param.system_value_type);
				KlayGE::LittleEndianToNative<sizeof(param.component_type)>(&param.component_type);
				KlayGE::LittleEndianToNative<sizeof(param.register_index)>(&param.register_index);
			}
		}
		
		return count;
	}

	void SortCBVars()
	{
		for (std::vector<DXBCConstantBuffer>::iterator itr = program->cbuffers.begin(); itr != program->cbuffers.end(); ++ itr)
		{
			if (SCBT_CBUFFER == itr->desc.type)
			{
				std::sort(itr->vars.begin(), itr->vars.end(), SortFuncLess);
			}
		}
	}

};

KlayGE::shared_ptr<ShaderProgram> ShaderParse(DXBCContainer const & dxbc)
{
	KlayGE::shared_ptr<ShaderProgram> program = KlayGE::MakeSharedPtr<ShaderProgram>();
	ShaderParser parser(dxbc, program);
	if (!parser.Parse())
	{
		return program;
	}
	
	return KlayGE::shared_ptr<ShaderProgram>();
}
