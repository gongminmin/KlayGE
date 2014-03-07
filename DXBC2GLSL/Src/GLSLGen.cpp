//--------------------------------------------------------------------
//this converter can convert HLSL5 to GLSL 2.1 3.x 4.x
//for now, only support HLSL5->GLSL4.3
//log:
//dcl_immediateConstantBuffer unfound in disassembler.
//sampleinfo samplepos hasn't corresponding functions in GLSL
//ddiv dfma drcp in DX11.1 unfound in disassembler.
//--------------------------------------------------------------------

#include <DXBC2GLSL/GLSLGen.hpp>

#include <string>
#include <ostream>

namespace
{
	char const * GLSLVersionStr[] = 
	{
		"110",
		"120",
		"130",
		"140",
		"150",
		"330",
		"400",
		"410",
		"420",
		"430",
		"440"
	};

	struct RegisterDesc
	{
		uint32_t index;
		ShaderRegisterComponentType type;
		ShaderInterpolationMode interpolation;
	};

	uint32_t bitcount32(uint32_t x)
	{
		x -= ((x >> 1) & 0x55555555);
		x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
		x = (((x >> 4) + x) & 0x0F0F0F0F);
		x += (x >> 8);
		x += (x >> 16);
		return x & 0x0000003F;
	}

	uint32_t PrimitiveNumVertices(ShaderPrimitive primitive)
	{
		uint32_t num_vertices;
		switch (primitive)
		{
		case SP_Point:
			num_vertices = 1;
			break;

		case SP_Line:
			num_vertices = 2;
			break;

		case SP_Triangle:
			num_vertices = 3;
			break;

		case SP_LineAdj:
			num_vertices = 4;
			break;

		case SP_TriangleAdj:
			num_vertices = 6;
			break;

		default:
			BOOST_ASSERT(false);
			num_vertices = 0;
			break;
		}

		return num_vertices;
	}
}

uint32_t GLSLGen::DefaultRules(GLSLVersion version)
{
	uint32_t rules = 0;
	if (version >= GSV_110)
	{
	}
	if (version >= GSV_120)
	{
	}
	if (version >= GSV_130)
	{
		rules |= GSR_UIntType;
		rules |= GSR_GenericTexture;
		rules |= GSR_PSInterpolation;
		rules |= GSR_InOutPrefix;
		rules |= GSR_TextureGrad;
		rules |= GSR_BitwiseOp;
	}
	if (version >= GSV_140)
	{
		rules |= GSR_GlobalUniformsInUBO;
		rules |= GSR_UseUBO;
	}
	if (version >= GSV_150)
	{
		rules |= GSR_CoreGS;
	}
	if (version >= GSV_330)
	{
		rules |= GSR_UniformBlockBinding;
		rules |= GSR_ExplicitPSOutputLayout;
		rules |= GSR_ExplicitInputLayout;
	}
	if (version >= GSV_400)
	{
		rules |= GSR_Int64Type;
		rules |= GSR_MultiStreamGS;
	}
	if (version >= GSV_410)
	{
	}
	if (version >= GSV_420)
	{
	}
	if (version >= GSV_430)
	{
	}
	if (version >= GSV_440)
	{
	}

	return rules;
}

void GLSLGen::FeedDXBC(boost::shared_ptr<ShaderProgram> const & program, bool has_gs, GLSLVersion version, uint32_t glsl_rules)
{
	program_ = program;
	shader_type_ = program_->version.type;
	has_gs_ = has_gs;
	glsl_version_ = version;
	glsl_rules_ = glsl_rules;

	if (!(glsl_rules_ & GSR_UseUBO))
	{
		glsl_rules_ &= ~GSR_UniformBlockBinding;
		glsl_rules_ &= ~GSR_GlobalUniformsInUBO;
	}

	this->LinkCFInsns();
	this->FindLabels();
	this->FindEndOfProgram();
	this->FindDclIndexRange();
	this->FindSamplers();
	this->FindTempDcls();
}

void GLSLGen::ToGLSL(std::ostream& out)
{
	//out << "//" << "pvghdc"[program_->version.type] << "s_" << program_->version.major << "_" << program_->version.minor << "\n";

	out << "#version " << GLSLVersionStr[glsl_version_] << "\n";
	if ((ST_GS == shader_type_) && !(glsl_rules_ & GSR_CoreGS))
	{
		out << "#extension GL_EXT_geometry_shader4 : enable\n";
	}
	out << "\n";

	if ((ST_GS == shader_type_) && (glsl_rules_ & GSR_CoreGS))
	{
		out << "layout(";
		switch (program_->gs_input_primitive)
		{
		case SP_Point:
			out << "points";
			break;

		case SP_Line:
			out << "lines";
			break;

		case SP_Triangle:
			out << "triangles";
			break;

		case SP_LineAdj:
			out << "lines_adjacency";
			break;

		case SP_TriangleAdj:
			out << "triangles_adjacency";
			break;

		default:
			BOOST_ASSERT_MSG(false, "Wrong input primitive name.");
			break;
		}
		if (program_->gs_instance_count > 0)
		{
			out << ", invocation = " << program_->gs_instance_count;
		}
		out << ") in;\n";

		out << "layout(";
		switch (program_->gs_output_topology[0])
		{
		case SPT_PointList:
			out << "points";
			break;

		case SPT_LineStrip:
			out << "line_strip";
			break;

		case SPT_TriangleStrip:
			out << "triangle_strip";
			break;

		default:
			BOOST_ASSERT_MSG(false, "Wrong output primitive topology name.");
			break;
		}
		out << ", max_vertices = " << program_->max_gs_output_vertex << ") out;\n\n";
	}

	this->ToDeclarations(out);

	out << "\nvoid main()" << "\n" << "{" << "\n";

	this->ToDeclInterShaderInputRegisters(out);
	this->ToCopyToInterShaderInputRegisters(out);
	this->ToDeclInterShaderOutputRegisters(out);

	for (std::vector<boost::shared_ptr<ShaderDecl> >::const_iterator iter = temp_dcls_.begin();
		iter != temp_dcls_.end(); ++ iter)
	{
		this->ToTemps(out, **iter);
	}
	out << "ivec4 iTempX[2];\n";
	if (glsl_rules_ & GSR_UIntType)
	{
		out << "u";
	}
	else
	{
		out << "i";
	}
	out << "vec4 uTempX[2];\n";
	out << "\n";
	for (size_t i = 0; i < program_->insns.size(); ++ i)
	{
		current_insn_ = *program_->insns[i];
		this->ToInstruction(out, *program_->insns[i]);
		out << "\n";
		if (i == end_of_program_)
		{
			break;
		}
	}
	out << "}" << "\n";
}

void GLSLGen::ToDeclarations(std::ostream& out)
{
	for (size_t i = 0; i < program_->params_out.size(); ++ i)
	{
		if ((SN_RENDER_TARGET_ARRAY_INDEX == program_->params_out[i].system_value_type)
			|| (SN_VIEWPORT_ARRAY_INDEX == program_->params_out[i].system_value_type))
		{
			program_->params_out[i].component_type = SRCT_SINT32;
		}
	}

	this->ToDclInterShaderInputRecords(out);
	this->ToDclInterShaderOutputRecords(out);

	for (size_t i = 0; i < program_->dcls.size(); ++ i)
	{
		this->ToDeclaration(out, *program_->dcls[i]);
	}
}

void GLSLGen::ToDclInterShaderInputRecords(std::ostream& out)
{
	for (size_t i = 0; i < program_->params_in.size(); ++ i)
	{
		if ((program_->params_in[i].read_write_mask != 0)
			&& (SN_UNDEFINED == program_->params_in[i].system_value_type))
		{
			ShaderRegisterComponentType type = program_->params_in[i].component_type;
			uint32_t register_index = program_->params_in[i].register_index;

			ShaderInterpolationMode interpolation = SIM_Undefined;
			if (ST_PS == shader_type_)
			{
				for (size_t i = 0; i < program_->dcls.size(); ++ i)
				{
					if ((SO_DCL_INPUT_PS == program_->dcls[i]->opcode)
						&& (program_->dcls[i]->op->indices[0].disp == register_index))
					{
						interpolation = program_->dcls[i]->dcl_input_ps.interpolation;
						break;
					}
				}
			}

			if (glsl_rules_ & GSR_PSInterpolation)
			{
				switch (interpolation)
				{
				case SIM_Undefined:
					break;

				case SIM_Constant:
					out << "flat ";
					break;

				case SIM_Linear:
					out << "smooth ";
					break;

				case SIM_LinearCentroid:
					out << "smooth centroid ";
					break;

				case SIM_LinearNoPerspective:
					out << "noperspective ";
					break;

				case SIM_LinearNoPerspectiveCentroid:
					out << "noperspective centroid ";
					break;

				case SIM_LinearSample:
					out << "smooth sample ";
					break;

				case SIM_LinearNoPerspectiveSample:
					out << "noperspective sample ";
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}

			if (ST_VS == shader_type_)
			{
				if (glsl_rules_ & GSR_ExplicitInputLayout)
				{
					// layout(location=N)
					out << "layout(location=" << i << ") ";
				}
			}

			// No layout qualifier here see dcl_output
			if (glsl_rules_ & GSR_InOutPrefix)
			{
				out << "in ";
			}
			else
			{
				if (ST_VS == shader_type_)
				{
					out << "attribute ";
				}
				else
				{
					out << "varying ";
				}
			}
			switch (type)
			{
			case SRCT_UINT32:
				if (glsl_rules_ & GSR_UIntType)
				{
					out << "u";
				}
				else
				{
					out << "i";
				}
				break;

			case SRCT_SINT32:
				out << "i";
				break;

			case SRCT_FLOAT32:
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			out << "vec4 ";
			if (shader_type_ != ST_VS)
			{
				out << "v_";
			}
			out << program_->params_in[i].semantic_name << program_->params_in[i].semantic_index;
			if (ST_GS == shader_type_)
			{
				out << "In" << '[' << PrimitiveNumVertices(program_->gs_input_primitive) << ']';
			}
			out << ";\n";
		}
	}

	if (!program_->params_in.empty())
	{
		out << "\n";
	}
}

void GLSLGen::ToDclInterShaderOutputRecords(std::ostream& out)
{
	for (size_t i = 0; i < program_->params_out.size(); ++ i)
	{
		if ((program_->params_out[i].read_write_mask != 0xF)
			&& (SN_UNDEFINED == program_->params_out[i].system_value_type))
		{
			if (ST_PS == shader_type_)
			{
				if (glsl_rules_ & GSR_ExplicitPSOutputLayout)
				{
					out << "layout(location=" << i << ") ";
				}
			}

			if (glsl_rules_ & GSR_InOutPrefix)
			{
				out << "out ";
			}
			else
			{
				out << "varying ";
			}

			int num_comps = 4;
			if (ST_PS == shader_type_)
			{
				num_comps = bitcount32(program_->params_out[i].mask);
			}

			if (1 == num_comps)
			{
				switch (program_->params_out[i].component_type)
				{
				case SRCT_UINT32:
					if (glsl_rules_ & GSR_UIntType)
					{
						out << "u";
					}
					out << "int";
					break;

				case SRCT_SINT32:
					out << "int";
					break;

				case SRCT_FLOAT32:
					out << "float";
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				switch (program_->params_out[i].component_type)
				{
				case SRCT_UINT32:
					if (glsl_rules_ & GSR_UIntType)
					{
						out << "u";
					}
					else
					{
						out << "i";
					}
					break;

				case SRCT_SINT32:
					out << "i";
					break;

				case SRCT_FLOAT32:
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
				out << "vec" << num_comps;
			}
			
			out << " v_" << program_->params_out[i].semantic_name
				<< program_->params_out[i].semantic_index;
			if ((ST_VS == shader_type_) && has_gs_)
			{
				out << "In";
			}
			out << ";\n";
		}
	}

	if (!program_->params_out.empty())
	{
		out << "\n";
	}
}

void GLSLGen::ToDeclInterShaderInputRegisters(std::ostream& out) const
{
	std::vector<RegisterDesc> input_registers;
	for (size_t i = 0; i < program_->params_in.size(); ++ i)
	{
		DXBCSignatureParamDesc const & sig_desc = program_->params_in[i];
		if (sig_desc.read_write_mask != 0)
		{
			uint32_t register_index = sig_desc.register_index;
			if (register_index != 0xFFFFFFFF)
			{
				ShaderRegisterComponentType type = sig_desc.component_type;
				bool found = false;
				for (std::vector<RegisterDesc>::iterator iter = input_registers.begin();
					iter != input_registers.end(); ++ iter)
				{
					if (iter->index == register_index)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					RegisterDesc desc;
					desc.index = register_index;
					desc.type = type;
					desc.interpolation = SIM_Undefined;
					input_registers.push_back(desc);
				}
			}
		}
	}

	for (size_t i = 0; i < input_registers.size(); ++ i)
	{
		switch (input_registers[i].type)
		{
		case SRCT_UINT32:
			if (glsl_rules_ & GSR_UIntType)
			{
				out << "u";
			}
			else
			{
				out << "i";
			}
			break;

		case SRCT_SINT32:
			out << "i";
			break;

		case SRCT_FLOAT32:
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		out << "vec4 i_REGISTER" << input_registers[i].index;
		if (ST_GS == shader_type_)
		{
			out << '[' << PrimitiveNumVertices(program_->gs_input_primitive) << ']';
		}
		out << ";\n";
	}
}

void GLSLGen::ToCopyToInterShaderInputRegisters(std::ostream& out) const
{
	uint32_t num_vertices = 1;
	if (ST_GS == shader_type_)
	{
		num_vertices = PrimitiveNumVertices(program_->gs_input_primitive);
	}

	for (size_t i = 0; i < program_->params_in.size(); ++ i)
	{
		DXBCSignatureParamDesc const & sig_desc = program_->params_in[i];
		if ((sig_desc.read_write_mask != 0) && (sig_desc.register_index != 0xFFFFFFFF))
		{
			for (uint32_t v = 0; v < num_vertices; ++ v)
			{
				out << "i_REGISTER" << sig_desc.register_index;
				if (ST_GS == shader_type_)
				{
					out << '[' << v << ']';
				}

				uint32_t mask = sig_desc.mask;
				out << '.';
				this->ToComponentSelector(out, this->ComponentSelectorFromCount(bitcount32(mask)));

				out << " = ";

				bool need_comps = true;
				if (ST_GS == shader_type_)
				{
					switch (sig_desc.system_value_type)
					{
					case SN_POSITION:
						if (glsl_rules_ & GSR_CoreGS)
						{
							out << "gl_in" << '[' << v << "].gl_Position";
						}
						else
						{
							out << "gl_PositionIn";
						}
						need_comps = true;
						break;

					case SN_CLIP_DISTANCE:
						if (glsl_rules_ & GSR_CoreGS)
						{
							out << "gl_in" << '[' << v << "].gl_ClipDistance[" << sig_desc.semantic_index << "]";
						}
						else
						{
							out << "gl_ClipDistanceIn[" << sig_desc.semantic_index << "]";
						}
						need_comps = false;
						break;

					case SN_PRIMITIVE_ID:
						out << "gl_PrimitiveIDIn";
						need_comps = false;
						break;

					case SN_UNDEFINED:
						out << "v_" << sig_desc.semantic_name << sig_desc.semantic_index << "In"
							<< '[' << v << ']';
						need_comps = true;
						break;
					}
				}
				else
				{
					switch (sig_desc.system_value_type)
					{
					case SN_POSITION:
						out << "gl_Position";
						need_comps = true;
						break;

					case SN_RENDER_TARGET_ARRAY_INDEX:
						out << "gl_Layer";
						need_comps = false;
						break;

					case SN_VERTEX_ID:
						out << "gl_VertexID";
						need_comps = false;
						break;

					case SN_INSTANCE_ID:
						out << "gl_InstanceID";
						need_comps = false;
						break;

					case SN_PRIMITIVE_ID:
						out << "gl_PrimitiveID";
						need_comps = false;
						break;

					case SN_VIEWPORT_ARRAY_INDEX:
						out << "gl_ViewportIndex";
						need_comps = false;
						break;

					case SN_UNDEFINED:
						if (shader_type_ != ST_VS)
						{
							out << "v_";
						}
						out << sig_desc.semantic_name << sig_desc.semantic_index;
						need_comps = true;
						break;
					}
				}
				if (need_comps)
				{
					out << ".";
					this->ToComponentSelector(out, this->ComponentSelectorFromMask(mask, 4));
				}
				out << ";\n";
			}
		}
	}

	if (!program_->params_in.empty())
	{
		out << "\n";
	}
}

void GLSLGen::ToDeclInterShaderOutputRegisters(std::ostream& out) const
{
	std::vector<RegisterDesc> output_dcl_record;

	for (size_t i = 0; i < program_->params_out.size(); ++ i)
	{
		DXBCSignatureParamDesc const & sig_desc = program_->params_out[i];
		if (sig_desc.read_write_mask != 0xF)
		{
			ShaderRegisterComponentType type = sig_desc.component_type;
			uint32_t register_index = sig_desc.register_index;
			bool found = false;
			for (std::vector<RegisterDesc>::iterator iter = output_dcl_record.begin();
				iter != output_dcl_record.end(); ++ iter)
			{
				if (iter->index == register_index)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				RegisterDesc desc;
				desc.index = register_index;
				desc.type = type;
				desc.interpolation = SIM_Undefined;
				output_dcl_record.push_back(desc);
			}
		}
	}

	for (size_t i = 0; i < output_dcl_record.size(); ++ i)
	{
		switch (output_dcl_record[i].type)
		{
		case SRCT_UINT32:
			if (glsl_rules_ & GSR_UIntType)
			{
				out << "u";
			}
			else
			{
				out << "i";
			}
			break;

		case SRCT_SINT32:
			out << "i";
			break;

		case SRCT_FLOAT32:
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		out << "vec4 o_REGISTER" << output_dcl_record[i].index << ";\n";
	}

	if (!output_dcl_record.empty())
	{
		out << "\n";
	}
}

void GLSLGen::ToCopyToInterShaderOutputRegisters(std::ostream& out) const
{
	for (size_t i = 0; i < program_->params_out.size(); ++ i)
	{
		DXBCSignatureParamDesc const & sig_desc = program_->params_out[i];
		if (sig_desc.read_write_mask != 0xF)
		{
			uint32_t mask = sig_desc.mask;
			bool need_comps = true;
			switch (sig_desc.system_value_type)
			{
			case SN_POSITION:
				out << "gl_Position";
				need_comps = true;
				break;

			case SN_RENDER_TARGET_ARRAY_INDEX:
				out << "gl_Layer";
				need_comps = false;
				break;

			case SN_PRIMITIVE_ID:
				out << "gl_PrimitiveID";
				need_comps = false;
				break;

			case SN_VIEWPORT_ARRAY_INDEX:
				out << "gl_ViewportIndex";
				need_comps = false;
				break;

			case SN_UNDEFINED:
				out << "v_" << sig_desc.semantic_name << sig_desc.semantic_index;
				if ((ST_VS == shader_type_) && has_gs_)
				{
					out << "In";
				}
				need_comps = true;
				break;
			}
			if (need_comps)
			{
				out << '.';
				this->ToComponentSelector(out, this->ComponentSelectorFromCount(bitcount32(mask)));
			}
			out << " = ";
			out << "o_REGISTER" << sig_desc.register_index << '.';
			this->ToComponentSelector(out, this->ComponentSelectorFromMask(mask, 4));
			out << ";\n";
		}
	}
}

void GLSLGen::ToDeclaration(std::ostream& out, ShaderDecl const & dcl)
{
	ShaderImmType sit = GetOpInType(dcl.opcode);
	switch (dcl.opcode)
	{
	case SO_DCL_INPUT:
	case SO_DCL_INPUT_SGV:
	case SO_DCL_INPUT_SIV:
	case SO_DCL_OUTPUT:
	case SO_DCL_OUTPUT_SIV:
	case SO_DCL_OUTPUT_SGV:
	case SO_DCL_INPUT_PS_SGV:
	case SO_DCL_INPUT_PS_SIV:
		break;

	case SO_DCL_CONSTANT_BUFFER:
		{
			cb_index_mode_[dcl.op->indices[0].disp] = dcl.dcl_constant_buffer.dynamic;

			if (glsl_rules_ & GSR_UniformBlockBinding)
			{
				out<<"layout(binding="
					<<dcl.op->indices[0].disp
					<<") ";
			}

			// Find the cb corresponding to bind_point
			for (std::vector<DXBCConstantBuffer>::const_iterator cb_iter = program_->cbuffers.begin();
				cb_iter != program_->cbuffers.end(); ++ cb_iter)
			{
				if ((SCBT_CBUFFER == cb_iter->desc.type) && (cb_iter->bind_point == dcl.op->indices[0].disp))
				{
					// If this cb has a member with default value ,then treat all members of this cb as constant
					// variables with intialization value in glsl.
					// e.g.
					/***********************************
					cbuffer Immutable
					{
						int a = 5;
						float b = 3.0f;
					}
					is converted to:
					const int a = 5;
					const float b = 3.0f;
					In this case, uniform block is not used.
					*************************************/
						
					bool has_default_value = false;
					for (std::vector<DXBCShaderVariable>::const_iterator var_iter = cb_iter->vars.begin();
						var_iter != cb_iter->vars.end(); ++ var_iter)
					{
						if (var_iter->var_desc.default_val)
						{
							has_default_value = true;
							break;
						}
					}
					if ((glsl_rules_ & GSR_UseUBO) && ((glsl_rules_ & GSR_GlobalUniformsInUBO) || (cb_iter->desc.name[0] != '$'))
						&& (!has_default_value))
					{
						out << "uniform ";
						out << cb_iter->desc.name << "\n{\n";
					}
					char const * uniform = "";
					if (!(glsl_rules_ & GSR_GlobalUniformsInUBO))
					{
						uniform = "uniform ";
					}
						
					for (std::vector<DXBCShaderVariable>::const_iterator var_iter = cb_iter->vars.begin();
						var_iter != cb_iter->vars.end(); ++ var_iter)
					{
						if (var_iter->has_type_desc)
						{
							// Array element count, 0 if not a array
							uint32_t element_count = var_iter->type_desc.elements;
							if (has_default_value)
							{
								out << "const ";
							}
							switch (var_iter->type_desc.var_class)
							{
							case SVC_SCALAR:
								out << uniform << var_iter->type_desc.name;
								out << " " << var_iter->var_desc.name;
								if (element_count)
								{
									out << "[" << element_count << "]";
								}
								break;

							case SVC_VECTOR:
								out << uniform;
								if (1 == var_iter->type_desc.columns)
								{
									out << var_iter->type_desc.name;
								}
								else
								{
									switch (var_iter->type_desc.type)
									{
									case SVT_INT:
										out << "i";
										break;

									case SVT_FLOAT:
										break;

									case SVT_UINT:
										out << "u";
										break;

									default:
										BOOST_ASSERT_MSG(false, "unexpected vector type");
										break;
									}
									out << "vec";
								}
								out << var_iter->type_desc.columns << " " << var_iter->var_desc.name;
								if (element_count)
								{
									out << "[" << element_count << "]";
								}
								break;

							case SVC_MATRIX_COLUMNS:
								// In glsl mat3x2 means 3 columns 2 rows, which is opposite to hlsl
								out << uniform << "mat" << var_iter->type_desc.columns << "x" << var_iter->type_desc.rows
									<< " " << var_iter->var_desc.name;
								if (element_count)
								{
									out << "[" << element_count << "]";
								}
								break;

							case SVC_MATRIX_ROWS:
								// In glsl mat3x2 means 3 columns 2 rows, which is opposite to hlsl
								out << "layout(row_major) "
									<< uniform << "mat" << var_iter->type_desc.columns << "x" << var_iter->type_desc.rows
									<< " " << var_iter->var_desc.name;
								if (element_count)
								{
									out << "[" << element_count << "]";
								}
								break;

							default:
								BOOST_ASSERT_MSG(false, "Unhandled type,when converting dcl_constant_buffer");
								break;
							}
							if (has_default_value)
							{
								out << " = ";
								this->ToDefaultValue(out, *var_iter);
							}
							out << ";\n";
						}
					}
					if ((glsl_rules_ & GSR_UseUBO) && ((glsl_rules_ & GSR_GlobalUniformsInUBO) || (cb_iter->desc.name[0] != '$')))
					{
						out << "};\n";
					}
					out << "\n";

					break;
				}
			}
		}
		break;

	case SO_IMMEDIATE_CONSTANT_BUFFER:
		{
			uint32_t vector_num = dcl.num / 4;
			float const * data = reinterpret_cast<float const *>(&dcl.data[0]);
			BOOST_ASSERT_MSG(vector_num != 0, "immediate cb size can't be 0");
			out << "const vec4 icb[" << vector_num << "] = vec4[" << vector_num  << "](\n";
			for (uint32_t i = 0; i < vector_num; ++ i)
			{
				if (i != 0)
				{
					out << ",\n";
				}

				out << "vec4(";
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
						out << ", ";
					}
				}
				out << ")";
			}
			out << "\n);\n";
		}
		break;

	case SO_DCL_TEMPS:
	case SO_DCL_INDEXABLE_TEMP:
		// Moved to GLSLGen::ToTemps();
		break;

	case SO_DCL_INPUT_PS:
		// Moved to GLSLGen::ToInterShaderInputRecords();
		break;

	case SO_DCL_RESOURCE:
		{
			for (std::vector<TextureSamplerInfo>::const_iterator iter = textures_.begin(); iter != textures_.end(); ++ iter)
			{
				if (iter->tex_index == dcl.op->indices[0].disp)
				{
					for (std::vector<SamplerInfo>::const_iterator sampler_iter = iter->samplers.begin();
						sampler_iter != iter->samplers.end(); ++ sampler_iter)
					{
						out << "uniform ";
						switch (dcl.rrt.x)
						{
						case SRRT_UNORM:
						case SRRT_SNORM:
						case SRRT_FLOAT:
							break;

						case SRRT_SINT:
							out << "i";
							break;

						case SRRT_UINT:
							out << "u";
							break;

						default:
							BOOST_ASSERT_MSG(false, "Unsupported resource return type");
							break;
						}
						out << "sampler";
						switch (dcl.dcl_resource.target)
						{
						case SRD_BUFFER:
							out << "Buffer";
							break;

						case SRD_TEXTURE1D:
							out << "1D";
							break;

						case SRD_TEXTURE2D:
							out << "2D";
							break;

						case SRD_TEXTURE2DMS:
							out << "2DMS";
							break;

						case SRD_TEXTURE3D:
							out << "3D";
							break;

						case SRD_TEXTURECUBE:
							out << "Cube";
							break;

						case SRD_TEXTURE1DARRAY:
							out << "1DArray";
							break;

						case SRD_TEXTURE2DARRAY:
							out << "2DArray";
							break;

						case SRD_TEXTURE2DMSARRAY:
							out << "2DMSArray";
							break;

						case SRD_TEXTURECUBEARRAY:
							out << "CubeArray";
							break;

						default:
							BOOST_ASSERT_MSG(false, "Unexpected resource target type");
							break;
						}
						if (!iter->samplers.empty() && sampler_iter->shadow)
						{
							out << "Shadow ";
						}
						else
						{
							out << " ";
						}
						this->ToOperands(out, *dcl.op, sit, true, false, false, true, true);
						DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_SAMPLER, static_cast<uint32_t>(sampler_iter->index));
						out << "_" << desc.name;
						out << ";\n";
					}
					if (iter->samplers.empty())
					{
						out << "uniform ";
						switch (dcl.rrt.x)
						{
						case SRRT_UNORM:
						case SRRT_SNORM:
						case SRRT_FLOAT:
							break;

						case SRRT_SINT:
							out << "i";
							break;

						case SRRT_UINT:
							out << "u";
							break;

						default:
							BOOST_ASSERT_MSG(false, "Unsupported resource return type");
							break;
						}
						out << "sampler";
						switch (dcl.dcl_resource.target)
						{
						case SRD_BUFFER:
							out << "Buffer ";
							break;

						case SRD_TEXTURE1D:
							out << "1D ";
							break;

						case SRD_TEXTURE2D:
							out << "2D ";
							break;

						case SRD_TEXTURE2DMS:
							out << "2DMS ";
							break;

						case SRD_TEXTURE3D:
							out << "3D ";
							break;

						case SRD_TEXTURECUBE:
							out << "Cube ";
							break;

						case SRD_TEXTURE1DARRAY:
							out << "1DArray ";
							break;

						case SRD_TEXTURE2DARRAY:
							out << "2DArray ";
							break;

						case SRD_TEXTURE2DMSARRAY:
							out << "2DMSArray ";
							break;

						case SRD_TEXTURECUBEARRAY:
							out << "CubeArray ";
							break;

						default:
							BOOST_ASSERT_MSG(false, "Unexpected resource target type");
							break;
						}
						this->ToOperands(out, *dcl.op, sit, true, false, false, true, true);
						out << ";\n";
					}
					break;
				}
			}
		}
		break;

	case SO_DCL_GLOBAL_FLAGS:
		// TODO: ignore it for now
		break;

	case SO_DCL_SAMPLER:
		// TODO: ignore it for now
		break;

	case SO_DCL_UNORDERED_ACCESS_VIEW_TYPED:
		{
			out << "layout(binding=" << dcl.op->indices[0].disp << ") uniform ";
			switch (dcl.rrt.x)
			{
			case SRRT_UNORM:
			case SRRT_SNORM:
			case SRRT_FLOAT:
				break;

			case SRRT_SINT:
				out << "i";
				break;

			case SRRT_UINT:
				out << "u";
				break;

			default:
				BOOST_ASSERT_MSG(false, "Unsupported resource return type");
				break;
			}
			out << "image";
			switch (dcl.dcl_resource.target)
			{
			case SRD_TEXTURE1D:
				out << "1D";
				break;

			case SRD_TEXTURE2D:
				out << "2D";
				break;

			case SRD_TEXTURE3D:
				out << "3D";
				break;

			case SRD_TEXTURE1DARRAY:
				out << "1DArray";
				break;

			case SRD_TEXTURE2DARRAY:
				out << "2DArray";
				break;

			case SRD_BUFFER:
				out << "Buffer";
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_UAV_RWTYPED, static_cast<uint32_t>(dcl.op->indices[0].disp));
			out << " " << desc.name << ";\n";
		}
		break;

	case SO_DCL_STREAM:
	case SO_DCL_GS_INPUT_PRIMITIVE:
	case SO_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
	case SO_DCL_MAX_OUTPUT_VERTEX_COUNT:
	case SO_DCL_GS_INSTANCE_COUNT:
		break;

	default:
		BOOST_ASSERT_MSG(false, "Unhandled declarations");
		break;
	}
}

void GLSLGen::ToInstruction(std::ostream& out, ShaderInstruction const & insn) const
{
	int selector[4] = { 0 };
	ShaderImmType oit = GetOpInType(insn.opcode);
	ShaderImmType oot = GetOpOutType(insn.opcode);
	bool dual_output = false;
	int num_comps = 0;
	switch (insn.opcode)
	{
		//-----------------------------------------------------------------------------------------
		//common single-precision float instructions
		//-----------------------------------------------------------------------------------------
	case SO_MOV:
		if (SOT_OUTPUT == insn.ops[0]->type)
		{
			oot = this->OperandAsType(*insn.ops[0], oit);
			oit = oot;
		}
		else
		{
			oot = this->OperandAsType(*insn.ops[1], oit);
			oit = oot;
		}
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ";
		switch (oot)
		{
		case SIT_UInt:
			if (SOT_TEMP == insn.ops[0]->type)
			{
				out << "i";
			}
			else
			{
				if (glsl_rules_ & GSR_UIntType)
				{
					out << "u";
				}
				else
				{
					out << "i";
				}
			}
			break;

		case SIT_Int:
			out << "i";
			break;

		case SIT_Double:
			out << "d";
			break;

		case SIT_Float:
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
		out << "vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_MUL:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << " * ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_MAD:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << " * ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << " + ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_IMAD:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << " * ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << " + ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_UMAD:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ";
		if (glsl_rules_ & GSR_UIntType)
		{
			out << "u";
		}
		else
		{
			out << "i";
		}
		out << "vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << " * ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << " + ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_DP4:
		// Notice: dot()only support float vec2/3/4 type. In glsl, ivec and uvec are converted to vec implicitly.
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(dot(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_DP3:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(dot(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ".xyz, ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ".xyz))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_DP2:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(dot(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ".xy, ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ".xy))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_EXP:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(exp2(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_SQRT:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(sqrt(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_RSQ:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(inversesqrt(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_DIV:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << " / ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_ADD:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << " + ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_IADD:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << " + ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_FRC:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(fract(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_LOG:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(log2(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_EQ:
		// Need more process to the return value of equal(): true->0xFFFFFFFF, false->0x00000000
		// e.g.
		// r0.xy=equal();
		// r0.x=r0.x?-1:0;
		// r0.y=r0.y?-1:0;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " == ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = ivec4(ivec4(equal(vec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), vec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_IEQ:
		// Need more process to the return value of equal(): true->0xFFFFFFFF, false->0x00000000
		// e.g.
		// r0.xy=equal();
		// r0.x=r0.x?-1:0;
		// r0.y=r0.y?-1:0;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " == ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = ivec4(ivec4(equal(ivec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}		
		break;

	case SO_NE:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " != ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = ivec4(ivec4(notEqual(vec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_INE:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " != ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = ivec4(ivec4(notEqual(ivec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_LT:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " < ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = ivec4(ivec4(lessThan(vec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), vec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_ILT:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " < ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = ivec4(ivec4(lessThan(ivec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_ULT:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " < ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = ivec4(";
			if (glsl_rules_ & GSR_UIntType)
			{
				out << "uvec4(lessThan(uvec4(";
				this->ToOperands(out, *insn.ops[1], oit);
				out << "), uvec4(";
				this->ToOperands(out, *insn.ops[2], oit);
			}
			else
			{
				out << "ivec4(lessThan(ivec4(";
				this->ToOperands(out, *insn.ops[1], oit);
				out << "), ivec4(";
				this->ToOperands(out, *insn.ops[2], oit);
			}
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_GE:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " >= ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = ivec4(ivec4(greaterThanEqual(vec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), vec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_IGE:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " >= ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = ivec4(ivec4(greaterThanEqual(ivec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_UGE:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		if (1 == this->GetOperandComponentNum(*insn.ops[1]))
		{
			out << " = (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << " < ";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") ? -1 : 0;";
		}
		else
		{
			out << " = uvec4(";
			if (glsl_rules_ & GSR_UIntType)
			{
				out << "uvec4(greaterThanEqual(uvec4(";
				this->ToOperands(out, *insn.ops[1], oit);
				out << "), uvec4(";
				this->ToOperands(out, *insn.ops[2], oit);
			}
			else
			{
				out << "ivec4(greaterThanEqual(ivec4(";
				this->ToOperands(out, *insn.ops[1], oit);
				out << "), ivec4(";
				this->ToOperands(out, *insn.ops[2], oit);
			}
			out << "))) * -1)";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_AND:
		{
			bool op1_is_float = (SIT_Float == this->OperandAsType(*insn.ops[1], oit));
			bool op2_is_float = (SIT_Float == this->OperandAsType(*insn.ops[2], oit));
			if (op1_is_float || op2_is_float)
			{
				oot = SIT_Float;
				this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
				out << " = vec4(vec4(";
				if (op1_is_float)
				{
					this->ToOperands(out, *insn.ops[1], SIT_Float);
				}
				else
				{
					out << "bvec4(";
					this->ToOperands(out, *insn.ops[1], oit);
					out << ")";
				}
				out << ") * vec4(";
				if (op2_is_float)
				{
					this->ToOperands(out, *insn.ops[2], SIT_Float);
				}
				else
				{
					out << "bvec4(";
					this->ToOperands(out, *insn.ops[2], oit);
					out << ")";
				}
				out << ")";
			}
			else
			{
				oot = SIT_Int;
				this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
				out << " = ivec4(";
				if (glsl_rules_ & GSR_BitwiseOp)
				{
					out << "ivec4(";
					this->ToOperands(out, *insn.ops[1], oit);
					out << ") & ivec4(";
					this->ToOperands(out, *insn.ops[2], oit);
					out << ")";
				}
				else
				{
					out << "bvec4(";
					this->ToOperands(out, *insn.ops[1], oit);
					out << ") && bvec4(";
					this->ToOperands(out, *insn.ops[2], oit);
					out << ")";
				}
			}
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_OR:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(";
		if (glsl_rules_ & GSR_BitwiseOp)
		{
			out << "ivec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ") | ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ")";
		}
		else
		{
			out << "bvec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ") || bvec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ")";
		}
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_XOR:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(ivec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ") ^ ivec4(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_NOT:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(";
		if (glsl_rules_ & GSR_BitwiseOp)
		{
			out << "~i";
		}
		else
		{
			out << "!b";
		}
		out << "vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_MOVC:
		if (SOT_OUTPUT == insn.ops[0]->type)
		{
			oot = this->OperandAsType(*insn.ops[0], oit);
			oit = oot;
		}
		else
		{
			oot = this->OperandAsType(*insn.ops[2], oit);
			oit = oot;
		}
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; ++ i)
		{
			selector[i] = this->ToSingleComponentSelector(out, *insn.ops[0], i, 0 == i);
		}
		out << " = ";
		if (num_comps > 1)
		{
			out << "vec" << num_comps << "(";
		}
		for (int i = 0; i < num_comps; ++ i)
		{
			if (i != 0)
			{
				out << ", ";
			}

			out << "bool(";
			this->ToOperands(out, *insn.ops[1], this->OperandAsType(*insn.ops[1], oit), false);
			this->ToSingleComponentSelector(out, *insn.ops[1], selector[i]);
			out << ")";
			out << " ? ";
			this->ToOperands(out, *insn.ops[2], oit, false, false, true);
			this->ToSingleComponentSelector(out, *insn.ops[2], selector[i]);
			out << " : ";
			this->ToOperands(out, *insn.ops[3], oit, false, false, true);
			this->ToSingleComponentSelector(out, *insn.ops[3], selector[i]);
		}
		if (num_comps > 1)
		{
			out << ")";
		}
		out << ";";
		break;

	case SO_MAX:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(max(vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "), vec4(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_IMAX:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(max(ivec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "), ivec4(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_UMAX:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ";
		if (glsl_rules_ & GSR_UIntType)
		{
			out << "uvec4(max(uvec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), uvec4(";
		}
		else
		{
			out << "ivec4(max(ivec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), ivec4(";
		}
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_MIN:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(min(vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "), vec4(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_IMIN:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(min(ivec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "), ivec4(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_UMIN:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ";
		if (glsl_rules_ & GSR_UIntType)
		{
			out << "uvec4(min(uvec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), uvec4(";
		}
		else
		{
			out << "ivec4(min(ivec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << "), ivec4(";
		}
		this->ToOperands(out, *insn.ops[2], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_FTOI:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_ITOF:
	case SO_UTOF:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_FTOU:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ";
		if (glsl_rules_ & GSR_UIntType)
		{
			out << "u";
		}
		else
		{
			out << "i";
		}
		out << "vec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_ROUND_NI:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(floor(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_ROUND_PI:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(ceil(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_ROUND_NE:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(roundEven(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_ROUND_Z:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		num_comps = this->GetOperandComponentNum(*insn.ops[1]);
		out << " = vec4(";
		if (1 == num_comps)
		{
			out << "int";
		}
		else
		{
			out << "ivec" << num_comps;
		}
		out << "(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_SINCOS:
		if (insn.ops[0]->type != SOT_NULL)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(sin(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		if (insn.ops[1]->type != SOT_NULL)
		{
			if (insn.ops[0]->type != NULL)
			{
				out << "\n";
			}
			this->ToOperands(out, *insn.ops[1], oot | (oot << 8));
			out << " = vec4(cos(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "))";
			this->ToComponentSelectors(out, *insn.ops[1]);
			out << ";";
		}
		break;

	case SO_RCP:
		// TODO: need more test
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(1.0f / ";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_SWAPC:
		// swapc 0dest0 1dest1 2src0 3src1 4src2
		// can be converted to following two instructions:
		// movc dest0 src0 src2 src1
		// movc dest1 src0 src1 src2

		// First movc
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			int j = this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = bool(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], j);
			out << ") ? ";
			this->ToOperands(out, *insn.ops[4], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[4], j);
			out << " : ";
			this->ToOperands(out, *insn.ops[3], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[3], j);
			out << ";";
			if (i != num_comps - 1)
			{
				out << "\n";
			}
		}
		// Second movc
		num_comps = this->GetOperandComponentNum(*insn.ops[1]);
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[1], oot | (oot << 8), false);
			int j = this->ToSingleComponentSelector(out, *insn.ops[1], i);
			out << " = bool(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], j);
			out << ") ? ";
			this->ToOperands(out, *insn.ops[3], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[3], j);
			out << " : ";
			this->ToOperands(out, *insn.ops[4], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[4], j);
			out << ";";
			if (i != num_comps - 1)
			{
				out << "\n";
			}
		}
		break;

		//-------------------------------------------------------------------------------------
		//integer instructions
		//---------------------------------------------------------------------------------------

	case SO_INEG:
		// TODO: to be tested,is that true?
		// dest.mask = ivec4(-src0).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(-";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_ISHL:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(ivec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ") ";
		if (glsl_rules_ & GSR_BitwiseOp)
		{
			out << "<< ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ")";
		}
		else
		{
			out << "* ivec4(pow(vec4(2), vec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ")))";
		}
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_ISHR:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(ivec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ") ";
		if (glsl_rules_ & GSR_BitwiseOp)
		{
			out << ">> ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ")";
		}
		else
		{
			out << "/ ivec4(pow(vec4(2), vec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ")))";
		}
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_IMUL:
		//imul destHI[.mask], destLO[.mask], [-]src0[.swizzle], [-]src1[.swizzle]
		if (glsl_rules_ & GSR_Int64Type)
		{
			out << "imulExtended(ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "), ivec4(";
			this->ToOperands(out, *insn.ops[3], oit);
			out << ")";
			out << ", ";
			out << "iTempX[0]";
			out << ", ";
			out << "iTempX[1]";
		}
		else
		{
			out << "iTempX[0] = ivec4(0);\n";
			out << "iTempX[1] = ";
			out << "ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") * ivec4(";
			this->ToOperands(out, *insn.ops[3], oit);
		}
		out << ");\n";
		//destHI.xz=vec4(destHI).xz;
		if (insn.ops[0]->type != SOT_NULL)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = iTempX[0]";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		//destLO.xz=vec4(destLO).xz;
		if (insn.ops[1]->type != SOT_NULL)
		{
			if (insn.ops[0]->type != SOT_NULL)
			{
				out << "\n";
			}
			this->ToOperands(out, *insn.ops[1], oot | (oot << 8));
			out << " = iTempX[1]";
			this->ToComponentSelectors(out, *insn.ops[1]);
			out << ";";

			dual_output = true;
		}
		break;

		//--------------------------------------------------------------------------------------
		// unsigned integer instructions
		//---------------------------------------------------------------------------------------

	case SO_UADDC:
		// gl:uaddCarry(x,y,out carry)
		// uaddc dest0[.mask], dest1[.mask], src0[.swizzle], src1[.swizzle]
		// dest0.xz=uaddCarry(src0.xxxx,src1.xxxx,dest1).xz;
		// TODO: to be tested
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(uaddCarry(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		if (SOT_NULL == insn.ops[1]->type)
		{
			out << ", ";
			this->ToOperands(out, *insn.ops[1], oit, false);
		}
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		////dest1.xz=uvec4(dest1).xz;
		//if(sm_shortfile_names[insn.ops[1]->file]!="null")
		//{
		//	out<<"\n";
		//	this->ToOperands(out,*insn.ops[1],oit);
		//	out<<" = uvec4(";
		//	this->ToOperands(out,*insn.ops[1],oit,false);
		//	out<<")";
		//	this->ToComponentSelectors(out,*insn.ops[0]);
		//	out<<";";
		//}
		break;

	case SO_UDIV:
		//dest0.mask= uvec4(src0 / src1).mask;
		//dest1 = uvec4(src0 % src1).mask;
		if (insn.ops[0]->type != SOT_NULL)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = ivec4(ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") / ivec4(";
			this->ToOperands(out, *insn.ops[3], oit);
			out << "))";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		if (insn.ops[1]->type != SOT_NULL)
		{
			if (insn.ops[0]->type != SOT_NULL)
			{
				out << "\n";
			}
			this->ToOperands(out, *insn.ops[1], oot | (oot << 8));
			out << " = ivec4(ivec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") % ivec4(";
			this->ToOperands(out, *insn.ops[3], oit);
			out << "))";
			this->ToComponentSelectors(out, *insn.ops[1]);
			out << ";";

			dual_output = true;
		}
		break;

		//umul destHI[.mask], destLO[.mask], src0[.swizzle], src1[.swizzle]
		//umulExtended(src0.xxxx,src1.xxxx,destHI,destLO);
	case SO_UMUL:
		// TODO: to be tested
		if (glsl_rules_ & GSR_Int64Type)
		{
			out << "umulExtended(uvec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << "), uvec4(";
			this->ToOperands(out, *insn.ops[3], oit);
			out << ")";
			out << ", ";
			out << "uTempX[0]";
			out << ", ";
			out << "uTempX[1]";
		}
		else
		{
			out << "uTempX[0] = uvec4(0);";
			out << "uTempX[1] = ";
			if (glsl_rules_ & GSR_UIntType)
			{
				out << "u";
			}
			else
			{
				out << "i";
			}
			out << "vec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ") * ";
			if (glsl_rules_ & GSR_UIntType)
			{
				out << "u";
			}
			else
			{
				out << "i";
			}
			out << "vec4(";
			this->ToOperands(out, *insn.ops[3], oit);
		}
		out << ");";
		//destHI.xz=vec4(destHI).xz;
		if (insn.ops[0]->type != SOT_NULL)
		{
			out << "\n";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = uTempX[0]";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		//destLO.xz=vec4(destLO).xz;
		if (insn.ops[1]->type != SOT_NULL)
		{
			if (insn.ops[0]->type != SOT_NULL)
			{
				out << "\n";
			}
			this->ToOperands(out, *insn.ops[1], oot | (oot << 8));
			out << " = uTempX[1]";
			this->ToComponentSelectors(out, *insn.ops[1]);
			out << ";";

			dual_output = true;
		}
		break;

	case SO_USHR:
		//dest.mask = uvec4(src0 >> src1).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ";
		if (glsl_rules_ & GSR_UIntType)
		{
			out << "uvec4(uvec4(";
		}
		else
		{
			out << "ivec4(ivec4(";
		}
		this->ToOperands(out, *insn.ops[1], oit);
		out << ") ";
		if (glsl_rules_ & GSR_BitwiseOp)
		{
			out << ">> ";
			if (glsl_rules_ & GSR_UIntType)
			{
				out << "u";
			}
			else
			{
				out << "i";
			}
			out << "vec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ")";
		}
		else
		{
			out << "/ ";
			if (glsl_rules_ & GSR_UIntType)
			{
				out << "u";
			}
			else
			{
				out << "i";
			}
			out << "vec4(pow(vec4(2), vec4(";
			this->ToOperands(out, *insn.ops[2], oit);
			out << ")))";
		}
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_USUBB:
		//usubb result borrow src0 src1
		//result.xz=uvec4(usubBorrow(src0,src1,borrow)).xz;
		//差为负数时存在问题，见hlsl msdn 
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = uvec4(usubBorrow(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ", ";
		out << "uTempX[0]";
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		//borrow.xz=vec4(borrow).xz;
		if (insn.ops[1]->type != SOT_NULL)
		{
			this->ToOperands(out, *insn.ops[1], oot | (oot << 8));
			out << " = uTempX[0]";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";

			dual_output = true;
		}
		break;

		//---------------------------------------------------------------------------
		//bit instructions
		//----------------------------------------------------------------------------

	case SO_COUNTBITS:
		//the number of bits set in src0
		//dest.mask= uvec4(bitCount(src0)).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = uvec4(bitCount(uvec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_BFI:
		//bfi dest, src0, src1, src2, src3
		//dest : The address of the results.
		//src0 : The bitfield width to take from src2.
		//src1 : The bitfield offset for replacing bits in src3
		//src2 : The number the bits are taken from.
		//src3 : The number with bits to be replaced
		//this instruction is used for packing integers or flags
		//dest.mask = bitfieldInsert(uvec4(src3),uvec4(src2),int(src1),int(src0)).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = uvec4(bitfieldInsert(uvec4(";
		this->ToOperands(out, *insn.ops[4], oit);
		out << "), uvec4(";
		this->ToOperands(out, *insn.ops[3], oit);
		out << "), int(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << "), int(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_BFREV:
		//bfrev dest,src0
		//reverse bits of src0
		//dest.mask = bitfieldReverse(src0).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = uvec4(bitfieldReverse(uvec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << " ;";
		break;

	case SO_UBFE:
		//ubfe dest,src0,src1,src2
		//Given a range of bits in a number, shift those bits to the LSB and set remaining bits to 0.
		//dest : Contains the results of the instruction.
		//src0 : The LSB 5 bits provide the bitfield width (0-31).
		//src1 : The LSB 5 bits of src1 provide the bitfield offset (0-31).
		//src2 : The number to shift.
		//set all bits of dest to zero
		//dest.mask = uvec4(0x00000000).mask;
		//dest.mask = bitfieldExtract(src2,src1,src0).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = uvec4(0x00000000)";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";\n";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = uvec4(bitfieldExtract(uvec4(";
		this->ToOperands(out, *insn.ops[3], oit);
		out << "), int(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << "),int(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << " ;";
		break;

	case SO_IBFE:
		//set all bits of dest to zero
		//dest.mask= uvec4(0x00000000).mask;
		//dest.mask= bitfieldExtract(src2,src1,src0).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = uvec4(0x00000000)";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";\n";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(bitfieldExtract(ivec4(";
		this->ToOperands(out, *insn.ops[3], oit);
		out << "), int(";
		this->ToOperands(out, *insn.ops[2], oit);
		out << "), int(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_FIRSTBIT_LO:
		//dest.mask =findLSB(src0).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = uvec4(findLSB(uvec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_FIRSTBIT_HI:
		//dest.mask = findMSB(src0).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = uvec4(findMSB(uvec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_FIRSTBIT_SHI:
		//dest.mask =findMSB(src0).mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ivec4(findMSB(ivec4(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ")))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_F16TOF32:
		//获取dest mask的个数
		//for each select component
		//dest.select_component=unpackHalf2x16(bitfieldExtract(src.select_component,0,16)).x;
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = unpackHalf2x16(bitfieldExtract(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i);
			out << ",0,16)).x;";
			if (i < num_comps - 1)
			{
				out << "\n";
			}
		}
		break;

	case SO_F32TOF16:
		//获取dest mask的个数
		//for each component
		//dest.select_component=bitfieldExtract(packHalf2x16(vec2(src.comp)),0,16);
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = bitfieldExtract(packHalf2x16(vec2(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i);
			out << ")),0,16);";
			if (i < num_comps - 1)
			{
				out << "\n";
			}
		}
		break;

		//----------------------------------------------------------------------------
		//flow control instructions
		//------------------------------------------------------------------------------

	case SO_RET:
		this->ToCopyToInterShaderOutputRegisters(out);
		out << "return;";
		break;

	case SO_NOP:
		break;

	case SO_LOOP:
		out << "while(true)\n";
		out << "{";
		break;

	case SO_ENDLOOP:
		out << "}";
		break;

	case SO_BREAK:
		out << "break;";
		break;

	case SO_CONTINUE:
		out << "continue;";
		break;

	case SO_SWITCH:
		out << "switch(";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << ")\n{";
		break;

	case SO_DEFAULT:
		out << "default:";
		break;

	case SO_ENDSWITCH:
		out << "}";
		break;

	case SO_CASE:
		out << "case ";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " :";
		break;

	case SO_BREAKC:
		//break if any bit is nonzero
		if (insn.insn.test_nz)
		{
			out << "if(bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))break;";
		}
		else//if all bits are zero
		{
			out << "if(!bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))break;";
		}
		break;

	case SO_IF:
		//if any bit is nonzero
		if (insn.insn.test_nz)
		{
			out << "if(bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))\n{";
		}
		else//if all bits are zero
		{
			out << "if(!bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))\n{";
		}
		break;

	case SO_ENDIF:
		out << "}";
		break;

	case SO_ELSE:
		out << "}\nelse\n{";
		break;

	case SO_CONTINUEC:
		//continue if any bit is nonzero
		if (insn.insn.test_nz)
		{
			out << "if(bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))continue;";
		}
		else//continue if all bits are zero
		{
			out << "if(!bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))continue;";
		}
		break;

	case SO_RETC:
		//return if any bit is nonzero
		if (insn.insn.test_nz)
		{
			out << "if(bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))return;";
		}
		else//return if all bits are zero
		{
			out << "if(!bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))return;";
		}
		break;

	case SO_DISCARD:
		if (insn.insn.test_nz)
		{
			// Discard if any bit is none zero
			out << "if(bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))discard;";
		}
		else
		{
			// Discard if all bits are zero
			out << "if(!bool(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << "))discard;";
		}
		break;

	case SO_CALL:
	case SO_CALLC:
		// TODO: l# #=?
		// to be tested, not sure where to get #
		{
			uint32_t label_value;
			if (SO_CALLC == insn.opcode)
			{
				label_value = static_cast<uint32_t>(insn.ops[1]->imm_values[0].u32);
				if (insn.insn.test_nz)
				{
					out << "if(bool(";
					this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
					out << ")){\n";
				}
				else// if all bits are zero
				{
					out << "if(!bool(";
					this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
					out << ")){\n";
				}
			}
			else
			{
				label_value = static_cast<uint32_t>(insn.ops[0]->imm_values[0].u32);
			}
			for (uint32_t i = label_to_insn_num_[label_value].start_num; i < label_to_insn_num_[label_value].end_num; i++)
			{
				this->ToInstruction(out, *program_->insns[i]);
			}
			if (SO_CALLC == insn.opcode)
			{
				out << "\n}";
			}
		}
		break;

		//----------------------------------------------------------------------------------------
		//double instructions
		//----------------------------------------------------------------------------------------

	case SO_FTOD:
		//ftod dest.mask src0.swwizle swwizle can only be xy or x or y
		//获取mask的个数，只能为2或4（xy zw xyzw)
		num_comps = this->GetOperandComponentNum(*insn.ops[0]) / 2;
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2 + 1, false);
			out << "= uintBitsToFloat(unpackDouble2x32(double(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i);
			out << ")));";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DTOF:
		//获取mask的个数，只能为1或2（x y z w xy zx xw yz yw zw)
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << "= float(packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2 + 1, false);
			out << ")));";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DADD:
		// TODO: to be tested
		// dest mask: xy zw xyzw, 2 or 4
		num_comps = this->GetOperandComponentNum(*insn.ops[0]) / 2;
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2 + 1, false);
			out << "= uintBitsToFloat(unpackDouble2x32(packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2 + 1, false);
			out << ")) + packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2 + 1, false);
			out << "))));";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DMOV:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = ";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ";";
		break;

	case SO_DMOVC:
		// TODO: to be tested
		// 获取dest mask的个数，只能为2或4（xy zw xyzw)
		num_comps = this->GetOperandComponentNum(*insn.ops[0]) / 2;
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2 + 1, false);
			out << " = ";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i);
			out << " ? ";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2 + 1, false);
			out << " : ";
			this->ToOperands(out, *insn.ops[3], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[3], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[3], i * 2 + 1, false);
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DLT:
		// 获取mask的个数，只能为1或2（x y z w xy zx xw yz yw zw)
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2 + 1, false);
			out << ")) < ";
			out << " = packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2 + 1, false);
			out << "));\n";
			this->ToOperands(out, *insn.ops[0], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = ";
			this->ToOperands(out, *insn.ops[0], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " ? uintBitsToFloat(0xffffffff) : uintBitsToFloat(0x00000000)";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DGE:
		// 获取mask的个数，只能为1或2（x y z w xy zx xw yz yw zw)
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2 + 1, false);
			out << ")) >= ";
			out << " = packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2 + 1, false);
			out << "));\n";
			this->ToOperands(out, *insn.ops[0], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = ";
			this->ToOperands(out, *insn.ops[0], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " ? uintBitsToFloat(0xffffffff) : uintBitsToFloat(0x00000000)";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DEQ:
		// 获取mask的个数，只能为1或2（x y z w xy zx xw yz yw zw)
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2 + 1, false);
			out << ")) == ";
			out << " = packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2 + 1, false);
			out << "));\n";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = ";
			this->ToOperands(out, *insn.ops[0], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " ? uintBitsToFloat(0xffffffff) : uintBitsToFloat(0x00000000)";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DNE:
		// 获取mask的个数，只能为1或2（x y z w xy zx xw yz yw zw)
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2 + 1, false);
			out << ")) != ";
			out << " = packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2 + 1, false);
			out << "));\n";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " = ";
			this->ToOperands(out, *insn.ops[0], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i);
			out << " ? uintBitsToFloat(0xffffffff) : uintBitsToFloat(0x00000000)";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DMAX:
		// 获取dest mask的个数，只能为2或4（xy zw xyzw)
		num_comps = this->GetOperandComponentNum(*insn.ops[0]) / 2;
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2 + 1, false);
			out << " = uintBitsToFloat(unpackDouble2x32(max(packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2 + 1, false);
			out << ")), ";
			out << "packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2 + 1, false);
			out << ")))));";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DMIN:
		// 获取dest mask的个数，只能为2或4（xy zw xyzw)
		num_comps = this->GetOperandComponentNum(*insn.ops[0]) / 2;
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2 + 1, false);
			out << " = uintBitsToFloat(unpackDouble2x32(min(packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2 + 1, false);
			out << ")), ";
			out << "packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2 + 1, false);
			out << ")))));";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

	case SO_DMUL:
		// dest mask:xy zw xyzw 个数2或4
		num_comps = this->GetOperandComponentNum(*insn.ops[0]) / 2;
		for (int i = 0; i < num_comps; i++)
		{
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[0], i * 2 + 1, false);
			out << "= uintBitsToFloat(unpackDouble2x32(packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[1], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[1], i * 2 + 1, false);
			out << ")) * packDouble2x32(floatBitsToUint(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2);
			this->ToSingleComponentSelector(out, *insn.ops[2], i * 2 + 1, false);
			out << "))));";
			if ((2 == num_comps) && (0 == i))
			{
				out << "\n";
			}
		}
		break;

		//following three instrucions:dfma ddiv drcp need special conditions(all met):
		//The system supports DirectX 11.1.
		//The system includes a WDDM 1.2 driver.
		//The driver reports support for this instruction via D3D11_FEATURE_DATA_D3D11_OPTIONS.ExtendedDoublesShaderInstructions set to TRUE.
		//see http://msdn.microsoft.com/en-us/library/hh920927(v=vs.85).aspx
		//case SO_DRCP:
		//break;

		//----------------------------------------------------------------------------
		//PS instructions
		//----------------------------------------------------------------------------

	case SO_RESINFO:
		//resinfo[_uint|_rcpFloat] dest[.mask], srcMipLevel.select_component, srcResource[.swizzle]
		{
			//ignore _uint suffix
			//process _rcpFloat suffix
			char const * c = (SRIRT_RCPFLOAT == insn.insn.resinfo_return_type) ? "1.0/" : "";
			for (std::vector<TextureSamplerInfo>::const_iterator iter = textures_.begin();
				iter != textures_.end(); ++ iter)
			{
				if (iter->tex_index == insn.ops[2]->indices[0].disp)
				{
					std::string s;
					if (!iter->samplers.empty())
					{
						DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_SAMPLER, static_cast<uint32_t>(iter->samplers[0].index));
						s = std::string("_") + desc.name;
					}
					switch (insn.resource_target)
					{
					case SRD_TEXTURE1D:
						if (SRIRT_RCPFLOAT == insn.insn.resinfo_return_type)
						{
							//dest.x=float(textureSize(src0,src1));
							this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						}
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << "));";
						if (this->GetOperandComponentNum(*insn.ops[0]) == 2)
						{
							out << "\n";
							//dest.y=float(textureQueryLevels(src1));
							this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
							this->ToSingleComponentSelector(out, *insn.ops[0], 1);
							out << " = float(textureQueryLevels(";
							this->ToOperands(out, *insn.ops[2], oit, false);
							out << s;
							out << "));";
						}
						break;

					case SRD_TEXTURE2D:
						//dest.x=float(textureSize(src0,src1).x);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << ");\n";
						//dest.y=float(textureSize(src0,src1).y);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << ");";
						if (3 == this->GetOperandComponentNum(*insn.ops[0]))
						{
							out << "\n";
							//dest.z=float(textureQueryLevels(src1));
							this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
							this->ToSingleComponentSelector(out, *insn.ops[0], 2);
							out << " = float(textureQueryLevels(";
							this->ToOperands(out, *insn.ops[2], oit, false);
							out << s;
							out << "));";
						}
						break;

					case SRD_TEXTURE2DMS:
						//dest.x=float(textureSize(src0,src1).x);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << ");\n";
						//dest.y=float(textureSize(src0,src1).y);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << ");";
						break;

					case SRD_TEXTURE3D:
						//dest.x=float(textureSize(src0,src1).x);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << ");\n";
						//dest.y=float(textureSize(src0,src1).y);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << ");\n";
						//dest.z=float(textureSize(src0,src1).z);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 2);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 2);
						out << ");";
						if (4 == this->GetOperandComponentNum(*insn.ops[0]))
						{
							out << "\n";
							//dest.w=float(textureQueryLevels(src1));
							this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
							this->ToSingleComponentSelector(out, *insn.ops[0], 3);
							out << " = float(textureQueryLevels(";
							this->ToOperands(out, *insn.ops[2], oit, false);
							out << s;
							out << "));";
						}
						break;

					case SRD_TEXTURECUBE:
						//dest.x=float(textureSize(src0,src1).x);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << ");\n";
						//dest.y=float(textureSize(src0,src1).y);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << ");";
						if (3 == this->GetOperandComponentNum(*insn.ops[0]))
						{
							out << "\n";
							//dest.z=float(textureQueryLevels(src1));
							this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
							this->ToSingleComponentSelector(out, *insn.ops[0], 2);
							out << " = float(textureQueryLevels(";
							this->ToOperands(out, *insn.ops[2], oit, false);
							out << s;
							out << "));";
						}
						break;

					case SRD_TEXTURE1DARRAY:
						//dest.x=float(textureSize(src0,src1).x);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << ");\n";
						//dest.y=float(textureSize(src0,src1).y);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << " = float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << ");";
						if (3 == this->GetOperandComponentNum(*insn.ops[0]))
						{
							out << "\n";
							//dest.z=float(textureQueryLevels(src1));
							this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
							this->ToSingleComponentSelector(out, *insn.ops[0], 2);
							out << " = float(textureQueryLevels(";
							this->ToOperands(out, *insn.ops[2], oit, false);
							out << s;
							out << "));";
						}
						break;

					case SRD_TEXTURE2DARRAY:
						//dest.x=float(textureSize(src0,src1).x);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << ");\n";
						//dest.y=float(textureSize(src0,src1).y);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << ");\n";
						//dest.z=float(textureSize(src0,src1).z);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 2);
						out << " = float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 2);
						out << ");";
						if (4 == this->GetOperandComponentNum(*insn.ops[0]))
						{
							out << "\n";
							//dest.w=float(textureQueryLevels(src1));
							this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
							this->ToSingleComponentSelector(out, *insn.ops[0], 3);
							out << " = float(textureQueryLevels(";
							this->ToOperands(out, *insn.ops[2], oit, false);
							out << s;
							out << "));";
						}
						break;

						//-----------------------------------------------------------------------------------------
						//there seems to be problems with dx sdk reference about texture2dmsarray.getDimesnsions()
						//the document says 
						//void GetDimensions(
						//out  UINT Width,
						//out  UINT Height,
						//out  UINT Elements,
						//out  UINT NumberOfSamples
						//);
						//but according to assemble instructions,parameters should be width,height,samples,elements
						//-------------------------------------------------------------------------------------------
					case SRD_TEXTURE2DMSARRAY:
						//dest.x=float(textureSize(src0,src1).x);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 0);
						out << ");\n";
						//dest.y=float(textureSize(src0,src1).y);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << " = " << c << "float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 1);
						out << ");\n";
						//dest.z=float(textureSize(src0,src1).z);
						this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
						this->ToSingleComponentSelector(out, *insn.ops[0], 2);
						out << " = float(textureSize(";
						this->ToOperands(out, *insn.ops[2], oit, false);
						out << s;
						out << ", ";
						this->ToOperands(out, *insn.ops[1], oit);
						out << ")";
						this->ToSingleComponentSelector(out, *insn.ops[0], 2);
						out << ");";
						break;

						//SM5 does not support query element count of cube array
						//case SRD_TEXTURECUBEARRAY:
						//break;
					default:
						BOOST_ASSERT_MSG(false, "Unexpected resource target type");
						break;
					}

					break;
				}
			}
		}
		break;

	case SO_SAMPLE_INFO:
		//dest.mask=?
		BOOST_ASSERT_MSG(false, "for sampleinfo,there's no corresponding instruction in glsl");
		break;

	case SO_BUFINFO:
		//dest.mask=uint(textureSize(src0));
		{
			for (std::vector<TextureSamplerInfo>::const_iterator iter = textures_.begin();
				iter != textures_.end(); ++ iter)
			{
				if (iter->tex_index == insn.ops[2]->indices[0].disp)
				{
					std::string s;
					if (!iter->samplers.empty())
					{
						DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_SAMPLER, static_cast<uint32_t>(iter->samplers[0].index));
						s = std::string("_") + desc.name;
						//sprintf(s,"_%d",iter->samplers[0].index);
					}
					this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
					out << " = uint(textureSize(";
					this->ToOperands(out, *insn.ops[1], oit, false);
					out << s;
					out << "));";

					break;
				}
			}
		}
		break;

	case SO_SAMPLE:
		//sample_indexable(0,0,0)(texture2d)(float,float,float,float) dest,src0(coord),src1(t),src2(s)
		//dest.mask=textureOffset(src1,src0,offset).mask;
		{
			//cube and cubearray doesnt support tex-coord offset
			char const * coord_mask = "";
			char const * offset_mask = "";
			char const * texture_type = "";
			bool offset = true;
			switch (insn.resource_target)
			{
			case SRD_TEXTURE1D:
				coord_mask = ".x";
				offset_mask = ".x";
				texture_type = "1D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2D:
				coord_mask = ".xy";
				offset_mask = ".xy";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURE3D:
				coord_mask = ".xyz";
				offset_mask = ".xyz";
				texture_type = "3D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0)
					|| (insn.sample_offset[2] != 0);
				break;

			case SRD_TEXTURE1DARRAY:
				coord_mask = ".xy";
				offset_mask = ".x";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2DARRAY:
				coord_mask = ".xyz";
				offset_mask = ".xy";
				texture_type = "3D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURECUBE:
				coord_mask = ".xyz";
				texture_type = "Cube";
				offset = false;
				break;

			case SRD_TEXTURECUBEARRAY:
				coord_mask = ".xyzw";
				offset = false;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(texture" << ((glsl_rules_ & GSR_GenericTexture) ? "" : texture_type)
				<< (offset ? "Offset" : "") << "(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[3], oit, false);
			out << ", vec4(";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ")" << coord_mask;
			if (offset)
			{
				out << ", ivec3(";
				out << static_cast<int>(insn.sample_offset[0]) << ", "
					<< static_cast<int>(insn.sample_offset[1]) << ", "
					<< static_cast<int>(insn.sample_offset[2]) << ")";
				out << offset_mask;
			}
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[2]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_SAMPLE_B:
		//sample_b_indexable(0,0,0)(texture2d)(float,float,float,float) dest,src0(coord),src1(t),src2(s),src3(bias)
		//dest.mask=textureOffset(src1,src0,offset,src3).mask;
		{
			char const * coord_mask = "";
			char const * offset_mask = "";
			char const * texture_type = "";
			bool offset = true;
			switch (insn.resource_target)
			{
			case SRD_TEXTURE1D:
				coord_mask = ".x";
				offset_mask = ".x";
				texture_type = "1D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2D:
				coord_mask = ".xy";
				offset_mask = ".xy";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURE3D:
				coord_mask = ".xyz";
				offset_mask = ".xyz";
				texture_type = "3D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0)
					|| (insn.sample_offset[2] != 0);
				break;

			case SRD_TEXTURE1DARRAY:
				coord_mask = ".xy";
				offset_mask = ".x";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2DARRAY:
				coord_mask = ".xyz";
				offset_mask = ".xy";
				texture_type = "3D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURECUBE:
				coord_mask = ".xyz";
				texture_type = "Cube";
				offset = false;
				break;

			case SRD_TEXTURECUBEARRAY:
				coord_mask = ".xyzw";
				offset = false;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(texture" << ((glsl_rules_ & GSR_GenericTexture) ? "" : texture_type)
				<< (offset ? "Offset" : "") << "(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[3], oit, false);
			out << ", (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ")" << coord_mask;
			if (offset)
			{
				out << ", ivec3(";
				out << static_cast<int>(insn.sample_offset[0]) << ", "
					<< static_cast<int>(insn.sample_offset[1]) << ", "
					<< static_cast<int>(insn.sample_offset[2]) << ")";
				out << offset_mask;
			}
			out << ", ";
			this->ToOperands(out, *insn.ops[4], oit);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[2]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_SAMPLE_C:
		{
			char const * offset_mask = "";
			char const * texture_type = "";
			bool offset = true;
			switch (insn.resource_target)
			{
			case SRD_TEXTURE1D:
				offset_mask = ".x";
				texture_type = "1D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE1DARRAY:
				offset_mask = ".x";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2D:
				offset_mask = ".xy";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURE2DARRAY:
				offset_mask = ".xy";
				texture_type = "3D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURECUBE:
			case SRD_TEXTURECUBEARRAY:
				texture_type = "Cube";
				offset = false;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(texture" << ((glsl_rules_ & GSR_GenericTexture) ? "" : texture_type)
				<< (offset ? "Offset" : "") << "(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[3], oit, false);
			switch (insn.resource_target)
			{
			case SRD_TEXTURE1D:
				out << ", vec3((";
				this->ToOperands(out, *insn.ops[1], oit);
				out << ").x,0,";
				this->ToOperands(out, *insn.ops[4], oit);
				out << ")";
				break;

			case SRD_TEXTURE1DARRAY:
			case SRD_TEXTURE2D:
				out << ", vec3((";
				this->ToOperands(out, *insn.ops[1], oit);
				out << ").xy,";
				this->ToOperands(out, *insn.ops[4], oit);
				out << ")";
				break;

			case SRD_TEXTURECUBE:
			case SRD_TEXTURE2DARRAY:
				out << ", vec4((";
				this->ToOperands(out, *insn.ops[1], oit);
				out << ").xyz, ";
				this->ToOperands(out, *insn.ops[4], oit);
				out << ")";
				break;

			case SRD_TEXTURECUBEARRAY:
				out << ", (";
				this->ToOperands(out, *insn.ops[1], oit);
				out << ").xyzw, ";
				this->ToOperands(out, *insn.ops[4], oit);
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			if (offset)
			{
				out << ", ivec3(";
				out << static_cast<int>(insn.sample_offset[0]) << ", "
					<< static_cast<int>(insn.sample_offset[1]) << ", "
					<< static_cast<int>(insn.sample_offset[2]) << ")";
				out << offset_mask;
			}
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[2]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_SAMPLE_C_LZ:
		{
			char const * offset_mask = "";
			char const * texture_type = "";
			bool offset = true;
			switch (insn.resource_target)
			{
			case SRD_TEXTURE1D:
				offset_mask = ".x";
				texture_type = "1D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE1DARRAY:
				offset_mask = ".x";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2D:
				offset_mask = ".xy";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(texture" << ((glsl_rules_ & GSR_GenericTexture) ? "" : texture_type)
				<< "Lod" << (offset ? "Offset" : "") << "(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[3], oit, false);
			switch (insn.resource_target)
			{
			case SRD_TEXTURE1D:
				out << ", vec3((";
				this->ToOperands(out, *insn.ops[1], oit);
				out << ").x,0,";
				this->ToOperands(out, *insn.ops[4], oit);
				out << ")";
				break;

			case SRD_TEXTURE1DARRAY:
			case SRD_TEXTURE2D:
				out << ", vec3((";
				this->ToOperands(out, *insn.ops[1], oit);
				out << ").xy,";
				this->ToOperands(out, *insn.ops[4], oit);
				out << ")";
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			out << ", 0";
			if (offset)
			{
				out << ", ivec3(";
				out << static_cast<int>(insn.sample_offset[0]) << ", "
					<< static_cast<int>(insn.sample_offset[1]) << ", "
					<< static_cast<int>(insn.sample_offset[2]) << ")";
				out << offset_mask;
			}
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[2]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_SAMPLE_L:
		{
			char const * coord_mask = "";
			char const * offset_mask = "";
			char const * texture_type = "";
			bool offset = true;
			switch (insn.resource_target)
			{
			case SRD_TEXTURE1D:
				coord_mask = ".x";
				offset_mask = ".x";
				texture_type = "1D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2D:
				coord_mask = ".xy";
				offset_mask = ".xy";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURE3D:
				coord_mask = ".xyz";
				offset_mask = ".xyz";
				texture_type = "3D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0)
					|| (insn.sample_offset[2] != 0);
				break;

			case SRD_TEXTURE1DARRAY:
				coord_mask = ".xy";
				offset_mask = ".x";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2DARRAY:
				coord_mask = ".xyz";
				offset_mask = ".xy";
				texture_type = "3D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURECUBE:
				coord_mask = ".xyz";
				texture_type = "Cube";
				offset = false;
				break;

			case SRD_TEXTURECUBEARRAY:
				coord_mask = ".xyzw";
				offset = false;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(texture" << ((glsl_rules_ & GSR_GenericTexture) ? "" : texture_type)
				<< "Lod" << (offset ? "Offset" : "") << "(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[3], oit, false);
			out << ", (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ")" << coord_mask;
			out << ", ";
			this->ToOperands(out, *insn.ops[4], oit);
			if (offset)
			{
				out << ", ivec3(";
				out << static_cast<int>(insn.sample_offset[0]) << ", "
					<< static_cast<int>(insn.sample_offset[1]) << ", "
					<< static_cast<int>(insn.sample_offset[2]) << ")";
				out << offset_mask;
			}
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[2]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_SAMPLE_D:
		{
			char const * deriv_mask = "";
			char const * coord_mask = "";
			char const * offset_mask = "";
			char const * texture_type = "";
			bool offset = true;
			switch (insn.resource_target)
			{
			case SRD_TEXTURE1D:
				coord_mask = ".x";
				offset_mask = ".x";
				deriv_mask = ".x";
				texture_type = "1D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2D:
				coord_mask = ".xy";
				offset_mask = ".xy";
				deriv_mask = ".xy";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURE3D:
				coord_mask = ".xyz";
				offset_mask = ".xyz";
				deriv_mask = ".xyz";
				texture_type = "3D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0)
					|| (insn.sample_offset[2] != 0);
				break;

			case SRD_TEXTURE1DARRAY:
				coord_mask = ".xy";
				offset_mask = ".x";
				deriv_mask = ".x";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0);
				break;

			case SRD_TEXTURE2DARRAY:
				coord_mask = ".xyz";
				offset_mask = ".xy";
				deriv_mask = ".xy";
				texture_type = "2D";
				offset = (insn.sample_offset[0] != 0)
					|| (insn.sample_offset[1] != 0);
				break;

			case SRD_TEXTURECUBE:
				coord_mask = ".xyz";
				deriv_mask = ".xyz";
				texture_type = "Cube";
				offset = false;
				break;

			case SRD_TEXTURECUBEARRAY:
				coord_mask = ".xyzw";
				deriv_mask = ".xyz";
				offset = false;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(texture" << ((glsl_rules_ & GSR_GenericTexture) ? "" : texture_type)
				<< ((glsl_rules_ & GSR_TextureGrad) ? "Grad"  : "") << (offset ? "Offset" : "") << "(";
			this->ToOperands(out, *insn.ops[2], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[3], oit, false);
			out << ", (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ")" << coord_mask;
			if (glsl_rules_ & GSR_TextureGrad)
			{
				out << ", (";
				this->ToOperands(out, *insn.ops[4], oit);//ddx
				out << ")" << deriv_mask;
				out << ", (";
				this->ToOperands(out, *insn.ops[5], oit);//ddy
				out << ")" << deriv_mask;
			}
			if (offset)
			{
				out << ", ivec3(";
				out << static_cast<int>(insn.sample_offset[0]) << ", "
					<< static_cast<int>(insn.sample_offset[1]) << ", "
					<< static_cast<int>(insn.sample_offset[2]) << ")";
				out << offset_mask;
			}
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[2]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_LOD:
		//lod dest[.mask], srcAddress[.swizzle], srcResource[.swizzle], srcSampler
		//dest.mask=textureQueryLod(src2,src0).y;
		{
			char const * mask = "";
			for (std::vector<TextureSamplerInfo>::const_iterator iter = textures_.begin();
				iter != textures_.end(); ++ iter)
			{
				if (insn.ops[2]->indices[0].disp == iter->tex_index)
				{
					switch (iter->type)
					{
					case SRD_TEXTURE1D:
						mask = ".x";
						break;

					case SRD_TEXTURE2D:
						mask = ".xy";
						break;

					case SRD_TEXTURE3D:
						mask = ".xyz";
						break;

					case SRD_TEXTURE1DARRAY:
						mask = ".x";
						break;

					case SRD_TEXTURE2DARRAY:
						mask = ".xy";
						break;

					case SRD_TEXTURECUBE:
						mask = ".xyz";
						break;

					case SRD_TEXTURECUBEARRAY:
						mask = ".xyz";
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
					BOOST_ASSERT(1 == this->GetOperandComponentNum(*insn.ops[0]));
					this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
					out << " = textureQueryLod(";
					this->ToOperands(out, *insn.ops[2], oit, false);
					out << "_";
					this->ToOperands(out, *insn.ops[3], oit, false);
					out << ", (";
					this->ToOperands(out, *insn.ops[1], oit);
					out << ")" << mask;
					out << ").y;";
					break;
				}
			}
		}
		break;

	case SO_LD:
		{
			//find a name of texutre:eg.t0_s0 or t0
			for (std::vector<TextureSamplerInfo>::const_iterator iter = textures_.begin();
				iter != textures_.end(); ++ iter)
			{
				if (insn.ops[2]->indices[0].disp == iter->tex_index)
				{
					std::string s;
					if (!iter->samplers.empty())
					{
						DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_SAMPLER, static_cast<uint32_t>(iter->samplers[0].index));
						s = std::string("_") + desc.name;
					}
					char const * mask = "";
					char const * lod_mask = "";
					char const * offset_mask = "";
					bool lod = true;
					switch (insn.resource_target)
					{
					case SRD_TEXTURE1D:
						mask = ".x";
						lod_mask = ".y";
						offset_mask = ".x";
						break;

					case SRD_TEXTURE2D:
						mask = ".xy";
						lod_mask = ".z";
						offset_mask = ".xy";
						break;

					case SRD_TEXTURE3D:
						mask = ".xyz";
						lod_mask = ".w";
						offset_mask = ".xyz";
						break;

					case SRD_TEXTURE1DARRAY:
						mask = ".xy";
						lod_mask = ".z";
						offset_mask = ".x";
						break;

					case SRD_TEXTURE2DARRAY:
						mask = ".xyz";
						lod_mask = ".w";
						offset_mask = ".xy";
						break;

					case SRD_BUFFER:
						mask = ".x";
						lod = false;
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
					this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
					out << " = vec4(texelFetch";
					if (lod)
					{
						out << "Offset";
					}
					out << "(";
					this->ToOperands(out, *insn.ops[2], oit, false);
					out << s;
					out << ", ivec4(";
					this->ToOperands(out, *insn.ops[1], SIT_Int, false);
					out << ")" << mask;
					if (lod)//lod and offset
					{
						out << ", ivec4(";
						this->ToOperands(out, *insn.ops[1], SIT_Int, false);
						out << ")" << lod_mask;
						out << ", ivec3(";
						out << static_cast<int>(insn.sample_offset[0]) << ", "
							<< static_cast<int>(insn.sample_offset[1]) << ", "
							<< static_cast<int>(insn.sample_offset[2]) << ")";
						out << offset_mask;
					}
					out << ")";
					this->ToComponentSelectors(out, *insn.ops[2]);
					out << ")";
					this->ToComponentSelectors(out, *insn.ops[0]);
					out << ";";
					break;
				}
			}
		}
		break;

	case SO_LD_MS:
		{
			//find a name of texutre:eg.t0_s0 or t0
			for (std::vector<TextureSamplerInfo>::const_iterator iter = textures_.begin();
				iter != textures_.end(); ++ iter)
			{
				if (insn.ops[2]->indices[0].disp == iter->tex_index)
				{
					std::string s;
					if (!iter->samplers.empty())
					{
						DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_SAMPLER, static_cast<uint32_t>(iter->samplers[0].index));
						s = std::string("_") + desc.name;
					}
					char const * mask = "";
					switch (insn.resource_target)
					{
					case SRD_TEXTURE2DMS:
						mask = ".xy";
						break;

					case SRD_TEXTURE2DMSARRAY:
						mask = ".xyz";
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
					this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
					out << " = vec4(texelFetch(";
					this->ToOperands(out, *insn.ops[2], oit, false);
					out << s;
					out << ", ivec4(";
					this->ToOperands(out, *insn.ops[1], SIT_Int);
					out << ")" << mask << ", int(";
					this->ToOperands(out, *insn.ops[3], SIT_Int);
					out << ")" << ")";
					this->ToComponentSelectors(out, *insn.ops[2]);
					out << ")";
					this->ToComponentSelectors(out, *insn.ops[0]);
					out << ";";
					break;
				}
			}
		}
		break;

	case SO_GATHER4:
		//gather4_indexable(u,v,w)(texture2d)(float,float,float,float) dest.mask, src0(coord),src1(resource), src2(sampler)
		//dest.mask=(textureGather[Offset](src1,src0[,offset],src2.comp).src1_mask).dest_mask;
		{
			char const * coord_mask = "";
			char const * offset_mask = "";
			bool offset = true;
			switch (insn.resource_target)
			{
			case SRD_TEXTURE2D:
				coord_mask = ".xy";
				offset_mask = ".xy";
				break;

			case SRD_TEXTURE2DARRAY:
				coord_mask = ".xyz";
				offset_mask = ".xy";
				break;

			case SRD_TEXTURECUBE:
				coord_mask = ".xyz";
				offset = false;
				break;

			case SRD_TEXTURECUBEARRAY:
				coord_mask = ".xyzw";
				offset = false;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(textureGather";
			if (offset)
			{
				out << "Offset(";
			}
			this->ToOperands(out, *insn.ops[2], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[3], oit, false);
			out << ", (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ")" << coord_mask << ", ";
			if (offset)
			{
				out << "ivec3(";
				out << static_cast<int>(insn.sample_offset[0]) << ", "
					<< static_cast<int>(insn.sample_offset[1]) << ", "
					<< static_cast<int>(insn.sample_offset[2]) << ")";
				out << offset_mask;
			}
			out << ", " << this->GetComponentSelector(*insn.ops[3], 0);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[2]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_GATHER4_C:
		//gather4_indexable(u,v,w)(target)(comp_type) dest.mask, src0(coord),src1(resource), src2(sampler),src3 cmp_ref
		//dest.mask=(textureGather[Offset](src1,src0[,offset],src2.comp).src1_mask).dest_mask;
		{
			char const * coord_mask = "";
			char const * offset_mask = "";
			bool offset = true;
			switch (insn.resource_target)
			{
			case SRD_TEXTURE2D:
				coord_mask = ".xy";
				offset_mask = ".xy";
				break;

			case SRD_TEXTURE2DARRAY:
				coord_mask = ".xyz";
				offset_mask = ".xy";
				break;

			case SRD_TEXTURECUBE:
				coord_mask = ".xyz";
				offset = false;
				break;

			case SRD_TEXTURECUBEARRAY:
				coord_mask = ".xyzw";
				offset = false;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(textureGather";
			if (offset)
			{
				out << "Offset(";
			}
			this->ToOperands(out, *insn.ops[2], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[3], oit, false);
			out << ", (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ")" << coord_mask << ", ";
			this->ToOperands(out, *insn.ops[4], oit);
			if (offset)
			{
				out << ", ivec3(";
				out << static_cast<int>(insn.sample_offset[0]) << ", "
					<< static_cast<int>(insn.sample_offset[1]) << ", "
					<< static_cast<int>(insn.sample_offset[2]) << ")";
				out << offset_mask;
			}
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[2]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_GATHER4_PO:
		//gather4_po_indexable(target)(comp_type) dest.mask, src0(coord),src1(offset),src2(resource),src3(sampler)
		//dest.mask=(textureGatherOffset(src2,src0,src1,src3.comp).src2_mask).dest_mask;
		{
			char const * coord_mask = "";
			char const * offset_mask = "";
			switch (insn.resource_target)
			{
			case SRD_TEXTURE2D:
				coord_mask = ".xy";
				offset_mask = ".xy";
				break;

			case SRD_TEXTURE2DARRAY:
				coord_mask = ".xyz";
				offset_mask = ".xy";
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(textureGatherOffset(";
			this->ToOperands(out, *insn.ops[3], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[4], oit, false);
			out << ", (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ")" << coord_mask << ", ";
			this->ToOperands(out, *insn.ops[2], SIT_Int);
			out << offset_mask;
			out << ", " << this->GetComponentSelector(*insn.ops[3], 0);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[3]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_GATHER4_PO_C:
		//gather4_po_c_indexable(target)(comp_type) dest.mask, src0(coord),src1(offset),src2(resource),src3(sampler),scr4(cmp_ref)
		//dest.mask=(textureGatherOffset(src2,src0,src1,src3.comp).src2_mask).dest_mask;
		{
			char const * coord_mask = "";
			char const * offset_mask = "";
			switch (insn.resource_target)
			{
			case SRD_TEXTURE2D:
				coord_mask = ".xy";
				offset_mask = ".xy";
				break;

			case SRD_TEXTURE2DARRAY:
				coord_mask = ".xyz";
				offset_mask = ".xy";
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = vec4(textureGatherOffset(";
			this->ToOperands(out, *insn.ops[3], oit, false);
			out << "_";
			this->ToOperands(out, *insn.ops[4], oit, false);
			out << ", (";
			this->ToOperands(out, *insn.ops[1], oit);
			out << ")" << coord_mask << ", ";
			this->ToOperands(out, *insn.ops[2], SIT_Int);
			out << offset_mask << ", " << this->GetComponentSelector(*insn.ops[4], 0) << ")";
			this->ToComponentSelectors(out, *insn.ops[3]);
			out << ")";
			this->ToComponentSelectors(out, *insn.ops[0]);
			out << ";";
		}
		break;

	case SO_DERIV_RTX_COARSE:
	case SO_DERIV_RTX_FINE:
		//deriv_rtx_coarse[_sat] dest[.mask], [-]src0[_abs][.swizzle]
		//dest.mask=ddx(src0).mask
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(dFdx(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_DERIV_RTY_COARSE:
	case SO_DERIV_RTY_FINE:
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = vec4(dFdy(";
		this->ToOperands(out, *insn.ops[1], oit);
		out << "))";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_LD_UAV_TYPED:
		//ld_uav_typed dst0[.mask], srcAddress[.swizzle], srcUAV[.swizzle]
		//dst0.mask=(imageLoad(srcUAV,srcAddress).srcUAV_mask).dst_mask;
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
		out << " = vec4(imageLoad(";
		this->ToOperands(out, *insn.ops[2], oit, false);
		out << ", ";
		this->ToOperands(out, *insn.ops[1], SIT_UInt);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[2]);
		out << ")";
		this->ToComponentSelectors(out, *insn.ops[0]);
		out << ";";
		break;

	case SO_STORE_UAV_TYPED:
		{
			//store_uav_typed dstUAV.xyzw, dstAddress[.swizzle](uint), src0[.swizzle]
			//imageStore(dstUAV,dstAddress,src0);

			//get component type of UAV
			DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_UAV_RWTYPED, static_cast<uint32_t>(insn.ops[0]->indices[0].disp));
			ShaderImmType data_sit;
			switch (desc.return_type)
			{
			case SRRT_UNORM:
			case SRRT_SNORM:
			case SRRT_FLOAT:
				data_sit = SIT_Float;
				break;

			case SRRT_SINT:
				data_sit = SIT_Int;
				break;

			case SRRT_UINT:
				data_sit = SIT_UInt;
				break;

			default:
				BOOST_ASSERT_MSG(false, "Unsupported resource return type");
				data_sit = SIT_Float;
				break;
			}
			out << "imageStore(";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
			out << ", ";
			this->ToOperands(out, *insn.ops[1], SIT_UInt);
			out << ", ";
			this->ToOperands(out, *insn.ops[2], data_sit);
			out << ");";
		}
		break;

		//------------------------------------------------------------------
		//atomic operations
		//-------------------------------------------------------------------

	case SO_ATOMIC_AND:
		//atomic_and dst, dstAddress[.swizzle], src0[.select_component]
		//imageAtomicAnd(dst,dstAddress[.swizzle],src0);
		out << "imageAtomicAnd(";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
		out << ", ";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ");";
		break;

	case SO_ATOMIC_OR:
		//atomic_or dst, dstAddress[.swizzle], src0[.select_component]
		//imageAtomicOr(dst,dstAddress[.swizzle],src0);
		out << "imageAtomicOr(";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
		out << ", ";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ");";
		break;

	case SO_ATOMIC_XOR:
		//atomic_xor dst, dstAddress[.swizzle], src0[.select_component]
		//imageAtomicXor(dst,dstAddress[.swizzle],src0);
		out << "imageAtomicXor(";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
		out << ", ";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ");";
		break;

	case SO_ATOMIC_CMP_STORE:
		//atomic_cmp_store dst, dstAddress[.swizzle], src0[.select_component], src1[.select_component]
		//imageAtomicCompSwap(dst,dstAddress[.swizzle],src0[.select_component], src1[.select_component]);
		out << "imageAtomicCompSwap(";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
		out << ", ";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ");";
		break;

	case SO_ATOMIC_IADD:
		//atomic_iadd dst, dstAddress[.swizzle], src0[.select_component]
		//imageAtomicAdd(dst,dstAddress[.swizzle],src0);
		out << "imageAtomicAdd(";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
		out << ", ";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ");";
		break;

	case SO_ATOMIC_IMAX:
	case SO_ATOMIC_UMAX:
		//atomic_imax dst, dstAddress[.swizzle], src0[.select_component]
		//imageAtomicMax(dst,dstAddress[.swizzle],src0);
		out << "imageAtomicMax(";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
		out << ", ";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ");";
		break;

	case SO_ATOMIC_IMIN:
	case SO_ATOMIC_UMIN:
		//atomic_imin dst, dstAddress[.swizzle], src0[.select_component]
		//imageAtomicMin(dst,dstAddress[.swizzle],src0);
		out << "imageAtomicMin(";
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
		out << ", ";
		this->ToOperands(out, *insn.ops[1], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ");";
		break;

	case SO_IMM_ATOMIC_IADD:
		//imm_atomic_iadd dst0[.single_component_mask], dst1, dstAddress[.swizzle], src0[.select_component]
		//dst0[.single_component_mask]=imageAtomicAdd(dst1,dstAddress[.swizzle],src0);
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = imageAtomicAdd(";
		this->ToOperands(out, *insn.ops[1], oit, false);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ");";
		break;

	case SO_IMM_ATOMIC_AND:
		//imm_atomic_and dst0[.single_component_mask], dst1, dstAddress[.swizzle], src0[.select_component]
		//dst0[.single_component_mask]=imageAtomicAnd(dst1,dstAddress[.swizzle],src0);
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = imageAtomicAnd(";
		this->ToOperands(out, *insn.ops[1], oit, false);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ");";
		break;

	case SO_IMM_ATOMIC_OR:
		//imm_atomic_or dst0[.single_component_mask], dst1, dstAddress[.swizzle], src0[.select_component]
		//dst0[.single_component_mask]=imageAtomicOr(dst1,dstAddress[.swizzle],src0);
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = imageAtomicOr(";
		this->ToOperands(out, *insn.ops[1], oit, false);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ");";
		break;

	case SO_IMM_ATOMIC_XOR:
		//imm_atomic_xor dst0[.single_component_mask], dst1, dstAddress[.swizzle], src0[.select_component]
		//dst0[.single_component_mask]=imageAtomicXor(dst1,dstAddress[.swizzle],src0);
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = imageAtomicXor(";
		this->ToOperands(out, *insn.ops[1], oit, false);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ");";
		break;

	case SO_IMM_ATOMIC_IMAX:
	case SO_IMM_ATOMIC_UMAX:
		//imm_atomic_imax dst0[.single_component_mask], dst1, dstAddress[.swizzle], src0[.select_component]
		//dst0[.single_component_mask]=imageAtomicMax(dst1,dstAddress[.swizzle],src0);
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = imageAtomicMax(";
		this->ToOperands(out, *insn.ops[1], oit, false);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ");";
		break;

	case SO_IMM_ATOMIC_IMIN:
	case SO_IMM_ATOMIC_UMIN:
		//imm_atomic_umin dst0[.single_component_mask], dst1, dstAddress[.swizzle], src0[.select_component]
		//dst0[.single_component_mask]=imageAtomicMin(dst1,dstAddress[.swizzle],src0);
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << " = imageAtomicMin(";
		this->ToOperands(out, *insn.ops[1], oit, false);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ");";
		break;

	case SO_IMM_ATOMIC_CMP_EXCH:
		//imm_atomic_cmp_exch dst0[.single_component_mask], dst1, dstAddress[.swizzle], src0[.select_component], src1[.select_component]
		//imageAtomicCompSwap(dst,dstAddress[.swizzle],src0[.select_component], src1[.select_component]);
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << "= imageAtomicCompSwap(";
		this->ToOperands(out, *insn.ops[1], oit, false);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[4], oit);
		out << ");";
		break;

	case SO_IMM_ATOMIC_EXCH:
		//imm_atomic_exch dst0[.single_component_mask], dst1, dstAddress[.swizzle], src0[.select_component]
		//dst0[.single_component_mask]=imageAtomicExchange(dst1,dstAddress[.swizzle], src0[.select_component]);
		this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
		out << "= imageAtomicExchange(";
		this->ToOperands(out, *insn.ops[1], oit, false);
		out << ", ";
		this->ToOperands(out, *insn.ops[2], oit);
		out << ", ";
		this->ToOperands(out, *insn.ops[3], oit);
		out << ");";
		break;

		//------------------------------------------------------------------
		// Geometry shader operations
		//-------------------------------------------------------------------
	case SO_EMIT:
		this->ToCopyToInterShaderOutputRegisters(out);
		out << "EmitVertex();";
		break;

	case SO_EMITTHENCUT:
		out << "EndPrimitive();";
		break;

	case SO_EMIT_STREAM:
		this->ToCopyToInterShaderOutputRegisters(out);
		if (glsl_rules_ & GSR_MultiStreamGS)
		{
			out << "EmitStreamVertex(" << insn.ops[0]->indices[0].disp << ");";
		}
		else
		{
			out << "EmitVertex();";
		}
		break;

	case SO_EMITTHENCUT_STREAM:
		this->ToCopyToInterShaderOutputRegisters(out);
		if (glsl_rules_ & GSR_MultiStreamGS)
		{
			out << "EmitStreamVertex(" << insn.ops[0]->indices[0].disp << ");\n";
			out << "EndStreamPrimitive(" << insn.ops[0]->indices[0].disp << ");";
		}
		else
		{
			out << "EmitVertex();\n";
			out << "EndPrimitive();";
		}
		break;

	case SO_CUT_STREAM:
		if (glsl_rules_ & GSR_MultiStreamGS)
		{
			out << "EndStreamPrimitive(" << insn.ops[0]->indices[0].disp << ");";
		}
		else
		{
			out << "EndPrimitive();";
		}
		break;

	default:
		BOOST_ASSERT_MSG(false, "Unhandled instructions");
		break;
	}

	if (insn.insn.sat)
	{
		// process _sat instruction modifier

		if (SIT_Float == oit)
		{
			out << "\n";
			this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
			out << " = clamp(";
			this->ToOperands(out, *insn.ops[0], oit);
			out << ", 0.0f, 1.0f);";
		}
		else if (SIT_Double == oit)
		{
			switch (insn.opcode)
			{
			case SO_DLT:
			case SO_DNE:
			case SO_DGE:
			case SO_DEQ:
				// TODO: to be tested
				// 这四个指令dest mask 为1个或2个
				out << "\n";
				this->ToOperands(out, *insn.ops[0], oot | (oot << 8));
				out << " = clamp(";
				this->ToOperands(out, *insn.ops[0], oit);
				out << ", 0.0f, 1.0f);";
				break;

			default:
				//其余指令dest mask为2个或4个
				//dest.xy=uintBitsToFloat(unpackDouble2x32(clamp(packDouble2x32(floatBitsToUint(dest.xy)))));
				//获取dest mask的个数，只能为2或4（xy zw xyzw)
				num_comps = this->GetOperandComponentNum(*insn.ops[0]) / 2;
				for (int i = 0; i < num_comps; i++)
				{
					this->ToOperands(out, *insn.ops[0], oot | (oot << 8), false);
					this->ToSingleComponentSelector(out, *insn.ops[0], i * 2);
					this->ToSingleComponentSelector(out, *insn.ops[0], i * 2 + 1, false);
					out << " = uintBitsToFloat(unpackDouble2x32(clamp(packDouble2x32(floatBitsToUint(";
					this->ToOperands(out, *insn.ops[0], oit, false);
					this->ToSingleComponentSelector(out, *insn.ops[0], i * 2);
					this->ToSingleComponentSelector(out, *insn.ops[0], i * 2 + 1, false);
					out << ")))));";
					if ((2 == num_comps) && (0 == i))
					{
						out << "\n";
					}
				}
				break;
			}
		}
	}

	if ((insn.num_ops > 0) && (SOT_TEMP == insn.ops[0]->type))
	{
		num_comps = this->GetOperandComponentNum(*insn.ops[0]);
		for (int i = 0; i < num_comps; ++ i)
		{
			temp_as_type_[static_cast<size_t>(insn.ops[0]->indices[0].disp) * 4
				+ this->GetComponentSelector(*insn.ops[0], i)] = static_cast<uint8_t>(oot);
		}
	}
	if (dual_output && (insn.num_ops > 1) && (SOT_TEMP == insn.ops[1]->type))
	{
		num_comps = this->GetOperandComponentNum(*insn.ops[1]);
		for (int i = 0; i < num_comps; ++ i)
		{
			temp_as_type_[static_cast<size_t>(insn.ops[1]->indices[0].disp) * 4
				+ this->GetComponentSelector(*insn.ops[1], i)] = static_cast<uint8_t>(oot);
		}
	}
}

void GLSLGen::ToOperands(std::ostream& out, ShaderOperand const & op, uint32_t imm_as_type,
		bool mask, bool dcl_array, bool no_swizzle, bool no_idx, bool no_cast) const
{
	ShaderImmType imm_type = static_cast<ShaderImmType>(imm_as_type & 0xFF);
	ShaderImmType as_type = static_cast<ShaderImmType>(imm_as_type >> 8);

	if (this->IsImmediateNumber(op) && (SO_MOV == current_insn_.opcode))
	{
		ShaderRegisterComponentType srct = this->GetOutputParamType(*current_insn_.ops[0]);
		switch (srct)
		{
		case SRCT_UINT32:
			imm_type = SIT_UInt;
			break;

		case SRCT_SINT32:
			imm_type = SIT_Int;
			break;

		case SRCT_FLOAT32:
			imm_type = SIT_Float;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	if (op.neg)
	{
		out << '-';
	}
	if (op.abs)
	{
		out << "abs(";
	}
	// 3.0 3 3U, etc
	if (SOT_IMMEDIATE32 == op.type)
	{
		if (1 == op.comps)
		{
			switch (imm_type)
			{
			case SIT_Float:
				// Normalized float test
				if (ValidFloat(op.imm_values[0].f32))
				{
					out << op.imm_values[0].f32;
				}
				else
				{
					out << op.imm_values[0].i32;
				}
				break;

			case SIT_Int:
				out << op.imm_values[0].i32;
				break;

			case SIT_UInt:
				if (0xC0490FDB == op.imm_values[0].u32)
				{
					// Hack for predefined magic value
					out << op.imm_values[0].f32;
				}
				else
				{
					out << op.imm_values[0].u32;
				}
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}
		else
		{
			if (SIT_Int == imm_type)
			{
				out << "i";
			}
			else if (SIT_UInt == imm_type)
			{
				if (glsl_rules_ & GSR_UIntType)
				{
					out << "u";
				}
				else
				{
					out << "i";
				}
			}
			out << "vec" << static_cast<int>(op.comps) << "(";
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
	}
	else if (SOT_IMMEDIATE64 == op.type)
	{
		// Only 4.3+ support double type

		if (1 == op.comps)
		{
			out << op.imm_values[0].f64;
		}
		else
		{
			out << "dvec" << static_cast<int>(op.comps) << "(";
			for (uint32_t i = 0; i < op.comps; ++ i)
			{
				if (i != 0)
				{
					out << ", ";
				}
				out << op.imm_values[i].f64;
			}
			out << ")";
		}
	}
	else
	{
		// 应该是类似v1 v2[2] o0 cb0[1]的这种变量名形式

		int flag = 0;
		int64_t start = 0;
		int64_t index = 0;
		int num = 0;
		bool whether_output_comps = true;//for matrixs,do not output comps
		bool whether_output_idx = true;//for cb member and vs input,no index
		std::vector<DclIndexRangeInfo>::const_iterator itr = idx_range_info_.begin();
		for (; itr != idx_range_info_.end(); ++ itr)
		{
			if (itr->op_type == op.type)
			{
				if (itr->start == op.indices[0].disp)
				{
					flag = 1;
					start = itr->start;
					num = itr->num;
					break;
				}
				else if ((op.indices[0].disp > itr->start) && (op.indices[0].disp < itr->start + itr->num))
				{
					flag = 2;
					index = op.indices[0].disp - itr->start;
					start = itr->start;
					break;
				}
			}
		}

		bool naked = false;
		//ajudge whether it is naked
		//naked为true表示有常数后缀如cb0[数字或表达式]
		//naked为false表示没有常数后缀如v[数字或表达式],用于array input variable
		switch (op.type)
		{
		case SOT_TEMP:
		case SOT_INPUT:
		case SOT_OUTPUT:
		case SOT_CONSTANT_BUFFER:
		case SOT_INDEXABLE_TEMP:
		case SOT_RESOURCE:
		case SOT_SAMPLER:
		case SOT_UNORDERED_ACCESS_VIEW:
		case SOT_THREAD_GROUP_SHARED_MEMORY:
			naked = true;
			break;

		default:
			naked = false;
			break;
		}
		if (op.indices[0].reg)
		{
			naked = false;
		}

		bool cast = false;
		ShaderImmType op_as_type;
		if (imm_type != SIT_Unknown)
		{
			op_as_type = imm_type;
		}
		else
		{
			op_as_type = this->OperandAsType(op, imm_as_type);
		}
		if (!no_cast)
		{
			if (((SIT_Int == op_as_type) || (SIT_UInt == op_as_type))
				&& ((SIT_Float == imm_type) || (SIT_Double == imm_type)))
			{
				if (SIT_UInt == op_as_type)
				{
					out << "uintBitsToFloat";
				}
				else
				{
					out << "intBitsToFloat";
				}
				out << "(";
				cast = true;
			}
			else if (((SIT_Float == op_as_type) || (SIT_Double == op_as_type))
				&& ((SIT_Int == imm_type) || (SIT_UInt == imm_type)))
			{
				if (SIT_UInt == imm_type)
				{
					out << "floatBitsToUint";
				}
				else
				{
					out << "floatBitsToInt";
				}
				out << "(";
				cast = true;
			}
		}

		// output operand name 
		this->ToOperandName(out, op, as_type, &whether_output_idx, &whether_output_comps, no_swizzle, no_idx);

		// output index
		// constant buffer is mapped to variable name,does not need index.
		// vs input is mapped to semantic,does not need index.
		if (whether_output_idx)
		{
			for (uint32_t i = 0; i < op.num_indices; ++ i)
			{
				//第一层索引不需要[]，如cb0[22]中0为第一层,naked==false需要[]
				if (!naked || i != 0)
				{
					out << '[';
				}
				if (op.indices[i].reg)
				{
					out << "int(";
					this->ToOperands(out, *op.indices[i].reg, imm_type, true, false, false, false, true);
					if (op.indices[i].disp)
					{
						out << "+" << op.indices[i].disp;
					}
					out << ")";
				}
				else
				{
					if (i == 0)
					{
						if (!dcl_array)
						{
							// Use an array
							if (flag == 0)
							{
								out << op.indices[i].disp;
							}
							else
							{
								out << start << "[" << index << "]";
							}
						}
						else
						{
							// Declare an array
							out << op.indices[i].disp;
							if (flag == 1)
							{
								out << "[" << num << "]";
							}
						}
					}
					else
					{
						out << op.indices[i].disp;
					}
				}
				if (!naked || i != 0)
				{
					out << ']';
				}
			}
		}
		//output component selectors
		//matrix has been converted to mat[2][3]-like form,do not need ouput comps
		if (whether_output_comps)
		{
			if (op.comps && mask)
			{
				switch (op.mode)
				{
				case SOSM_MASK:
					out << '.';
					this->ToComponentSelector(out, this->ComponentSelectorFromMask(op.mask, op.comps));
					break;

				case SOSM_SWIZZLE:
					out << '.';
					this->ToComponentSelector(out, this->ComponentSelectorFromSwizzle(op.swizzle, op.comps));
					break;

				case SOSM_SCALAR:
					out << '.';
					this->ToComponentSelector(out, this->ComponentSelectorFromScalar(op.swizzle[0]));
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}

		if (cast)
		{
			out << ")";
		}
	}

	if (op.abs)
	{
		out << ")";
	}
}

ShaderImmType GLSLGen::OperandAsType(ShaderOperand const & op, uint32_t imm_as_type) const
{
	ShaderImmType imm_type = static_cast<ShaderImmType>(imm_as_type & 0xFF);
	ShaderImmType as_type = static_cast<ShaderImmType>(imm_as_type >> 8);

	if (this->IsImmediateNumber(op) && (SO_MOV == current_insn_.opcode))
	{
		ShaderRegisterComponentType srct = this->GetOutputParamType(*current_insn_.ops[0]);
		switch (srct)
		{
		case SRCT_UINT32:
			imm_type = SIT_UInt;
			break;

		case SRCT_SINT32:
			imm_type = SIT_Int;
			break;

		case SRCT_FLOAT32:
			imm_type = SIT_Float;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	if (SOT_IMMEDIATE32 == op.type)
	{
		as_type = SIT_Unknown;
		for (uint32_t i = 0; i < op.comps; ++ i)
		{
			switch (imm_type)
			{
			case SIT_Float:
				// Normalized float test
				if (ValidFloat(op.imm_values[i].f32))
				{
					as_type = std::max(as_type, SIT_Float);
				}
				else
				{
					as_type = std::max(as_type, SIT_Int);
				}
				break;

			case SIT_Int:
				as_type = std::max(as_type, SIT_Int);
				break;

			case SIT_UInt:
				if (0xC0490FDB == op.imm_values[0].u32)
				{
					// Hack for predefined magic value
					as_type = std::max(as_type, SIT_Float);
				}
				else
				{
					as_type = std::max(as_type, SIT_UInt);
				}
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}
	}
	else if (SOT_IMMEDIATE64 == op.type)
	{
		as_type = SIT_Double;
	}
	else
	{
		if (SIT_Unknown == as_type)
		{
			if (SOT_TEMP == op.type)
			{
				as_type = SIT_Unknown;
				for (uint32_t i = 0; i < op.comps; ++ i)
				{
					as_type = std::max(as_type, static_cast<ShaderImmType>(temp_as_type_[
						static_cast<size_t>(op.indices[0].disp) * 4 + this->GetComponentSelector(op, i)]));
				}
			}
			else if (SOT_INPUT == op.type)
			{
				switch (this->GetInputParamDesc(op, (ST_GS == shader_type_) ? 1 : 0).component_type)
				{
				case SRCT_UINT32:
					as_type = SIT_UInt;
					break;

				case SRCT_SINT32:
					as_type = SIT_Int;
					break;

				case SRCT_FLOAT32:
					as_type = SIT_Float;
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else if (SOT_OUTPUT == op.type)
			{
				switch (this->GetOutputParamType(op))
				{
				case SRCT_UINT32:
					as_type = SIT_UInt;
					break;

				case SRCT_SINT32:
					as_type = SIT_Int;
					break;

				case SRCT_FLOAT32:
					as_type = SIT_Float;
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else if (SOT_CONSTANT_BUFFER == op.type)
			{
				uint32_t bind_point = static_cast<uint32_t>(op.indices[0].disp);
				uint32_t register_index = static_cast<uint32_t>(op.indices[1].disp);
				uint32_t min_selector = this->GetMinComponentSelector(op);
				uint32_t offset = 16 * register_index + min_selector * 4;

				for (std::vector<DXBCConstantBuffer>::const_iterator cb_iter = program_->cbuffers.begin();
					cb_iter != program_->cbuffers.end(); ++ cb_iter)
				{
					if ((SCBT_CBUFFER == cb_iter->desc.type) && (cb_iter->bind_point == bind_point))
					{
						// find which cb member current cb# array index is located in
						for (std::vector<DXBCShaderVariable>::const_iterator var_iter = cb_iter->vars.begin();
							var_iter != cb_iter->vars.end(); ++ var_iter)
						{
							if ((offset >= var_iter->var_desc.start_offset)
								&& (var_iter->var_desc.start_offset + var_iter->var_desc.size > offset))
							{
								// indicate if a register contains more than one variables because of register packing
								bool contain_multi_var = false;
								uint32_t max_selector = this->GetMaxComponentSelector(op);
								if ((offset + (max_selector - min_selector + 1) * 4) > (var_iter->var_desc.start_offset + var_iter->var_desc.size))
								{
									contain_multi_var = true;
								}
								BOOST_ASSERT_MSG(var_iter->has_type_desc, "Constant buffer should have type desc");

								switch (var_iter->type_desc.type)
								{
								case SVT_INT:
									as_type = SIT_Int;
									break;

								case SVT_UINT:
									as_type = SIT_UInt;
									break;

								case SVT_FLOAT:
									as_type = SIT_Float;
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				as_type = imm_type;
			}
		}
	}

	return as_type;
}

void GLSLGen::ToOperandName(std::ostream& out, ShaderOperand const & op, ShaderImmType as_type,
		bool* need_idx, bool* need_comps, bool no_swizzle, bool no_idx) const
{
	*need_comps = true;
	*need_idx = true;
	if ((ST_PS == shader_type_) && ((SOT_INPUT == op.type) || (SOT_OUTPUT == op.type)))
	{
		if (SOT_INPUT == op.type)
		{
			DXBCSignatureParamDesc const & param_desc = this->GetInputParamDesc(op);
			if (0 == strcmp(param_desc.semantic_name, "SV_Position"))
			{
				*need_idx = false;
				out << "gl_FragCoord";
			}
			else if (0 == strcmp(param_desc.semantic_name, "SV_SampleIndex"))
			{
				*need_idx = false;
				out << "gl_SampleID";
			}
			else if (0 == strcmp(param_desc.semantic_name, "SV_PrimitiveID"))
			{
				*need_idx = false;
				out << "gl_PrimitiveID";
			}
			else
			{
				out << "i_REGISTER";
			}
		}
		else
		{
			BOOST_ASSERT(SOT_OUTPUT == op.type);

			out << "o_REGISTER";
		}
	}
	else if ((ST_VS == shader_type_) && ((SOT_INPUT == op.type) || (SOT_OUTPUT == op.type)))
	{
		if (SOT_INPUT == op.type)
		{
			DXBCSignatureParamDesc const & param_desc = this->GetInputParamDesc(op);
			if (0 == strcmp(param_desc.semantic_name, "SV_VertexID"))
			{
				*need_idx = false;
				out << "gl_VertexID";
			}
			else if (0 == strcmp(param_desc.semantic_name, "SV_InstanceID"))
			{
				*need_idx = false;
				out << "gl_InstanceID";
			}
			else
			{
				*need_idx = true;
				out << "i_REGISTER";
			}
		}
		else
		{
			BOOST_ASSERT(SOT_OUTPUT == op.type);

			DXBCSignatureParamDesc const & param_desc = this->GetOutputParamDesc(op);
			if (0 == strcmp(param_desc.semantic_name, "SV_ClipDistance"))
			{
				*need_idx = false;
				out << "gl_ClipDistance[" << param_desc.semantic_index << "]";
			}
			else
			{
				*need_idx = true;
				out << "o_REGISTER";
			}
		}
	}
	else if ((ST_GS == shader_type_) && ((SOT_INPUT == op.type) || (SOT_OUTPUT == op.type)))
	{
		if (SOT_INPUT == op.type)
		{
			*need_idx = false;
			out << "i_REGISTER" << op.indices[1].disp;
			if (!no_idx)
			{
				out << '[' << op.indices[0].disp << ']';
			}
		}
		else
		{
			BOOST_ASSERT(SOT_OUTPUT == op.type);

			*need_idx = true;
			out << "o_REGISTER";
		}
	}
	else if (SOT_OUTPUT_DEPTH == op.type)
	{
		out << "gl_FragDepth";
		*need_comps = false;
		*need_idx = false;
	}
	else if (SOT_TEMP == op.type)
	{
		if (SIT_Unknown == as_type)
		{
			as_type = static_cast<ShaderImmType>(temp_as_type_[static_cast<size_t>(op.indices[0].disp) * 4
				+ this->GetComponentSelector(op, 0)]);
		}

		out << 't';
		switch (as_type)
		{
		case SIT_Float:
		case SIT_Double:
			out << 'f';
			break;

		case SIT_Int:
		case SIT_UInt:
			out << 'i';
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}
	else if (SOT_SAMPLER == op.type)
	{
		DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_SAMPLER, static_cast<uint32_t>(op.indices[0].disp));
		out << desc.name;
		*need_comps = false;
		*need_idx = false;
	}
	else if (SOT_RESOURCE == op.type)
	{
		DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_TEXTURE, static_cast<uint32_t>(op.indices[0].disp));
		out << desc.name;
		*need_comps = false;
		*need_idx = false;
	}
	else if (SOT_UNORDERED_ACCESS_VIEW == op.type)
	{
		DXBCInputBindDesc const & desc = this->GetResourceDesc(SIT_UAV_RWTYPED, static_cast<uint32_t>(op.indices[0].disp));
		out << desc.name;
		*need_comps = false;
		*need_idx = false;
	}
	else if (SOT_INDEXABLE_TEMP == op.type)
	{
		out << "IndexableTemp";
	}
	else if ((SOT_INPUT == op.type) && (ST_VS == shader_type_))
	{
		*need_idx = false;

		DXBCSignatureParamDesc const & param_desc = this->GetInputParamDesc(op);
		*need_comps = (param_desc.mask > 1);
		out << param_desc.semantic_name << param_desc.semantic_index;
	}
	else if (SOT_CONSTANT_BUFFER == op.type)
	{
		*need_idx = false;
		bool dynamic_indexed = cb_index_mode_.find(op.indices[0].disp)->second;

		// map cb array element to cb member names(with array index if it's a array)

		// map cb#[i] to a cb member name
		uint32_t bind_point = static_cast<uint32_t>(op.indices[0].disp);
		uint32_t register_index = static_cast<uint32_t>(op.indices[1].disp);
		uint32_t min_selector = this->GetMinComponentSelector(op);
		uint32_t offset = 16 * register_index + min_selector * 4;
		uint32_t num_selectors = this->GetOperandComponentNum(op);

		// find cb corresponding to bind_point
		for (std::vector<DXBCConstantBuffer>::const_iterator cb_iter = program_->cbuffers.begin();
			cb_iter != program_->cbuffers.end(); ++ cb_iter)
		{
			if ((SCBT_CBUFFER == cb_iter->desc.type) && (cb_iter->bind_point == bind_point))
			{
				// find which cb member current cb# array index is located in
				for (std::vector<DXBCShaderVariable>::const_iterator var_iter = cb_iter->vars.begin();
					var_iter != cb_iter->vars.end(); ++ var_iter)
				{
					if ((offset >= var_iter->var_desc.start_offset)
						&& (var_iter->var_desc.start_offset + var_iter->var_desc.size > offset))
					{
						// indicate if a register contains more than one variables because of register packing
						bool contain_multi_var = false;
						uint32_t max_selector = this->GetMaxComponentSelector(op);
						if ((offset + (max_selector - min_selector + 1) * 4) > (var_iter->var_desc.start_offset + var_iter->var_desc.size))
						{
							contain_multi_var = true;
						}
						BOOST_ASSERT_MSG(var_iter->has_type_desc, "Constant buffer should have type desc");

						// if cb member is a array,this is element count, 0 if not a array
						uint32_t element_count = var_iter->type_desc.elements;
						// find corresponding cb member array element index if it's a array
						uint32_t element_index = 0;

						switch (var_iter->type_desc.var_class)
						{
						case SVC_VECTOR:
						case SVC_SCALAR:
							if (!contain_multi_var)
							{
								out << var_iter->var_desc.name;
								if (element_count != 0)
								{
									out << "[";
									if (dynamic_indexed && op.indices[1].reg)
									{
										this->ToOperands(out, *op.indices[1].reg, SIT_Int);
									}
									else
									{
										element_index = (16 * register_index - var_iter->var_desc.start_offset) / 16;
										out << element_index;
									}
									out << "]";
								}
								else
								{
									// array is not packed, so doesn't need remap component_selector
									// if not, because of register packing, we need to remap it.
									// see: http://msdn.microsoft.com/zh-cn/library/windows/desktop/bb509632

									*need_comps = false;
									if ((SVC_VECTOR == var_iter->type_desc.var_class) && !no_swizzle)
									{
										// remap register component to the right cb member variable component
										// example case:
										// |a  |b  |c.x|c.y|->this is a register,a b c is three variables
										// |.x |.y |.z |.w |->this is register component
										// so we need to remap .zw to .xy
										for (uint32_t i = 0; i < num_selectors; ++ i)
										{
											if (i == 0)
											{
												out << ".";
											}
											uint32_t variable_offset = var_iter->var_desc.start_offset;
											uint32_t register_component_offset = 16 * register_index + 4 * this->GetComponentSelector(op, i);
											uint32_t remapped_component = (register_component_offset - variable_offset) / 4;
											out << "xyzw"[remapped_component];
										}
									}
								}

								if (SVC_SCALAR == var_iter->type_desc.var_class)
								{
									*need_comps = false;
								}
							}
							else
							{
								//if a current register references more than one variable,things become more
								//complex:
								//e.g.
								//|a[3].x|a[3].y|b   |c   |->this is a register,a b c are variables,a is an array
								//|.x    |.y    |.z  |.w  |->this is register component
								//current register reference is .yzw
								//so we should convert it to :
								//[i|u]vec{2,3,4}(a[3].y,b.x,c.x)
								*need_comps = false;
								switch (var_iter->type_desc.type)
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
									BOOST_ASSERT(false);
									break;
								}
								out << "vec" << num_selectors;
								out << "(";
								for (uint32_t i = 0; i < num_selectors; ++ i)
								{
									if (i != 0)
									{
										out << ", ";
									}

									uint32_t register_selector = this->GetComponentSelector(op, i);
									// find the cb member this register selector correspond to
									// TODO: Check the O(N^2) here
									uint32_t offset = 16 * register_index + register_selector * 4;
									for (std::vector<DXBCShaderVariable>::const_iterator var_iter2 = cb_iter->vars.begin();
										var_iter2 != cb_iter->vars.end(); ++ var_iter2)
									{
										if ((offset >= var_iter2->var_desc.start_offset)
											&& (var_iter2->var_desc.start_offset + var_iter2->var_desc.size > offset))
										{
											out << var_iter2->var_desc.name;
											uint32_t element_count = var_iter2->type_desc.elements;
											// ajudge which cb member array element it's located in
											if (element_count)
											{
												element_index = (16 * register_index - var_iter2->var_desc.start_offset) / 16;
												out << "[" << element_index << "]";
											}

											if ((SVC_VECTOR == var_iter2->type_desc.var_class) && !no_swizzle)
											{
												// remap register selector to the right cb member variable component
												out << ".";
												// for array,doesn't need to remap because array element is always at the start of a register,since
												// array is not packed.
												if (element_count)
												{
													out << "xyzw"[register_selector];
												}
												else
												{
													// remap
													uint32_t variable_offset = var_iter2->var_desc.start_offset;
													uint32_t register_component_offset = 16 * register_index + 4 * this->GetComponentSelector(op, i);
													uint32_t remapped_component = (register_component_offset - variable_offset) / 4;
													out << "xyzw"[remapped_component];
												}
											}

											break;
										}
									}
								}
								out << ")";
							}
							break;

						case SVC_MATRIX_ROWS:
						case SVC_MATRIX_COLUMNS:
							{
								//hlsl matrix subscript is opposite to glsl
								//e.g.in hlsl mat[2][3] <=>in glsl mat[3][2]
								//so mat[2].xyzw in hlsl<=>vec4(mat[0][2],mat[1][2],mat[2][2],mat[3][2]) in glsl

								BOOST_ASSERT_MSG(!contain_multi_var, "Matrix will not be packed?");

								//indicate how many registers a matrix array element occupies
								uint32_t register_stride;
								if (SVC_MATRIX_ROWS == var_iter->type_desc.var_class)
								{
									register_stride = var_iter->type_desc.rows;
								}
								else
								{
									register_stride = var_iter->type_desc.columns;
								}
								if (element_count != 0)
								{
									//identify which matrix array element it's loacated in
									element_index = (16 * register_index - var_iter->var_desc.start_offset) / 16 / register_stride;
								}
								uint32_t row = (16 * register_index - var_iter->var_desc.start_offset) / 16 - element_index * register_stride;
								uint32_t column = 0;
								uint32_t count = this->GetOperandComponentNum(op);
								if (count == 1)
								{
									out << "float";
								}
								else out << "vec" << count;//glsl only support float or double matrix 
								out << "(";
								for (uint32_t i = 0; i < count; ++ i)
								{
									if (i > 0)
									{
										out << ", ";
									}
									//convert component selector to column number
									//x y z w--> 0 1 2 3
									column = this->GetComponentSelector(op, i);
									out << var_iter->var_desc.name;
									if (element_count)
									{
										out << "[";
										if (dynamic_indexed && op.indices[1].reg)
										{
											this->ToOperands(out, *op.indices[1].reg, SIT_Int);
										}
										else
										{
											out << element_index;
										}
										out << "]";
									}
									if (SVC_MATRIX_ROWS == var_iter->type_desc.var_class)
									{
										out << "[" << row << "]" << "[" << column << "]";
									}
									else
									{
										out << "[" << column << "]" << "[" << row << "]";
									}
								}
								out << ")";
								*need_comps = false;
								}
							break;

						default:
							BOOST_ASSERT_MSG(false, "Unhandled type");
							break;
						}

						break;
					}
				}

				break;
			}
		}
	}
	else if (SOT_INPUT_PRIMITIVEID == op.type)
	{
		out << "gl_PrimitiveIDIn";
		*need_comps = false;
		*need_idx = false;
	}
	else if (SOT_INPUT_GS_INSTANCE_ID == op.type)
	{
		out << "gl_InvocationID";
		*need_comps = false;
		*need_idx = false;
	}
	else
	{
		out << ShaderOperandTypeShortName(op.type);
	}
}

int GLSLGen::ToSingleComponentSelector(std::ostream& out, ShaderOperand const & op, int i, bool dot) const
{
	if ((SOT_IMMEDIATE32 == op.type) || (SOT_IMMEDIATE64 == op.type))
	{
		if ((op.comps > 1) && (i < op.comps))
		{
			if (dot)
			{
				out << ".";
			}
			out << "xyzw"[i];
		}
	}
	else
	{
		if (dot)
		{
			out << '.';
		}
		uint32_t comp = this->GetComponentSelector(op, i);
		out << "xyzw"[comp];
		return comp;
	}

	return 0;
}

//param i:the component selector to get
//return:the idx of selector:0 1 2 3 stand for x y z w
//for .xw i=0 return 0 i=1 return 3
int GLSLGen::GetComponentSelector(ShaderOperand const & op, int i) const
{
	uint32_t comps_index;
	switch (op.mode)
	{
	case SOSM_MASK:
		comps_index = this->ComponentSelectorFromMask(op.mask, op.comps);
		break;

	case SOSM_SWIZZLE:
		comps_index = this->ComponentSelectorFromSwizzle(op.swizzle, op.comps);
		break;

	case SOSM_SCALAR:
		comps_index = this->ComponentSelectorFromScalar(op.swizzle[0]);
		i = 0;
		break;

	default:
		BOOST_ASSERT(false);
		comps_index = 0;
		break;
	}

	uint32_t comp = (comps_index >> (i * 8)) & 0xFF;
	BOOST_ASSERT(comp != 0xFF);
	return comp;
}

void GLSLGen::ToComponentSelectors(std::ostream& out, ShaderOperand const & op, bool dot) const
{
	if ((op.type != SOT_IMMEDIATE32) && (op.type != SOT_IMMEDIATE64))
	{
		if (op.comps)
		{
			switch (op.mode)
			{
			case SOSM_MASK:
				if (dot)
				{
					out << '.';
				}
				this->ToComponentSelector(out, this->ComponentSelectorFromMask(op.mask, op.comps));
				break;

			case SOSM_SWIZZLE:
				out << '.';
				this->ToComponentSelector(out, this->ComponentSelectorFromSwizzle(op.swizzle, op.comps));
				break;

			case SOSM_SCALAR:
				out << '.';
				this->ToComponentSelector(out, this->ComponentSelectorFromScalar(op.swizzle[0]));
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}
	}
}

bool GLSLGen::IsImmediateNumber(ShaderOperand const & op) const
{
	return ((SOT_IMMEDIATE32 == op.type) || (SOT_IMMEDIATE64 == op.type));
}

int GLSLGen::GetOperandComponentNum(ShaderOperand const & op) const
{
	int num_comps = 0;
	if (op.comps)
	{
		switch (op.mode)
		{
		case SOSM_MASK:
			for (uint32_t i = 0; i < op.comps; ++ i)
			{
				if (op.mask & (1 << i))
				{
					++ num_comps;
				}
			}
			break;

		case SOSM_SWIZZLE:
			num_comps = op.comps;
			break;

		case SOSM_SCALAR:
			num_comps = 1;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}
	return num_comps;
}

ShaderRegisterComponentType GLSLGen::GetOutputParamType(ShaderOperand const & op) const
{
	if (SOT_OUTPUT == op.type)
	{
		for (std::vector<DXBCSignatureParamDesc>::const_iterator iter = program_->params_out.begin();
			iter != program_->params_out.end(); ++ iter)
		{
			if (iter->register_index == op.indices[0].disp)
			{
				return iter->component_type;
			}
		}
		BOOST_ASSERT(false);
		return SRCT_FLOAT32;
	}
	else if (SOT_OUTPUT_DEPTH == op.type)
	{
		for (std::vector<DXBCSignatureParamDesc>::const_iterator iter = program_->params_out.begin();
			iter != program_->params_out.end(); ++ iter)
		{
			if (0 == strcmp("SV_Depth", iter->semantic_name))
			{
				return iter->component_type;
			}
		}
		BOOST_ASSERT(false);
		return SRCT_FLOAT32;
	}
	else if ((SOT_TEMP == op.type) || (SOT_INDEXABLE_TEMP == op.type))
	{
		return SRCT_FLOAT32;
	}
	else
	{
		BOOST_ASSERT(false);
		return SRCT_FLOAT32;
	}
}

DXBCSignatureParamDesc const & GLSLGen::GetOutputParamDesc(ShaderOperand const & op, uint32_t index) const
{
	for (std::vector<DXBCSignatureParamDesc>::const_iterator iter = program_->params_out.begin();
		iter != program_->params_out.end(); ++ iter)
	{
		if (iter->register_index == op.indices[index].disp)
		{
			return *iter;
		}
	}

	BOOST_ASSERT(false);
	static DXBCSignatureParamDesc invalid;
	return invalid;
}

DXBCSignatureParamDesc const & GLSLGen::GetInputParamDesc(ShaderOperand const & op, uint32_t index) const
{
	BOOST_ASSERT(SOT_INPUT == op.type);
	for (std::vector<DXBCSignatureParamDesc>::const_iterator iter = program_->params_in.begin();
		iter != program_->params_in.end(); ++ iter)
	{
		if (iter->register_index == op.indices[index].disp)
		{
			return *iter;
		}
	}

	BOOST_ASSERT(false);
	static DXBCSignatureParamDesc invalid;
	return invalid;
}

void GLSLGen::FindDclIndexRange()
{
	for (size_t i = 0; i < program_->dcls.size(); ++ i)
	{
		if (SO_DCL_INDEX_RANGE == program_->dcls[i]->opcode)
		{
			DclIndexRangeInfo mInfo;
			mInfo.op_type = program_->dcls[i]->op->type;
			mInfo.start = program_->dcls[i]->op->indices[0].disp;
			mInfo.num = program_->dcls[i]->num;
			idx_range_info_.push_back(mInfo);
		}
	}
}

void GLSLGen::FindSamplers()
{
	for (size_t i = 0; i < program_->dcls.size(); ++ i)
	{
		if (SO_DCL_RESOURCE == program_->dcls[i]->opcode)
		{
			TextureSamplerInfo tex;
			tex.type = program_->dcls[i]->dcl_resource.target;
			tex.tex_index = program_->dcls[i]->op->indices[0].disp;
			for (size_t i = 0; i < program_->insns.size(); ++ i)
			{
				if ((SO_SAMPLE == program_->insns[i]->opcode)
					|| (SO_SAMPLE_C == program_->insns[i]->opcode)
					|| (SO_SAMPLE_C_LZ == program_->insns[i]->opcode)
					|| (SO_SAMPLE_L == program_->insns[i]->opcode)
					|| (SO_SAMPLE_D == program_->insns[i]->opcode)
					|| (SO_SAMPLE_B == program_->insns[i]->opcode)
					|| (SO_LOD == program_->insns[i]->opcode)
					|| (SO_GATHER4 == program_->insns[i]->opcode)
					|| (SO_GATHER4_C == program_->insns[i]->opcode)
					|| (SO_GATHER4_PO == program_->insns[i]->opcode)
					|| (SO_GATHER4_PO_C == program_->insns[i]->opcode))
				{
					// 3:sampler, 2:resource
					int i_tex = 2;
					int i_sam = 3;
					if ((SO_GATHER4_PO == program_->insns[i]->opcode)
						|| (SO_GATHER4_PO_C == program_->insns[i]->opcode))
					{
						i_tex = 3;
						i_sam = 4;
					}
					if (tex.tex_index == program_->insns[i]->ops[i_tex]->indices[0].disp)
					{
						SamplerInfo sam;
						sam.index = program_->insns[i]->ops[i_sam]->indices[0].disp;
						bool found = false;
						for (std::vector<SamplerInfo>::const_iterator iter = tex.samplers.begin();
							iter != tex.samplers.end(); ++ iter)
						{
							if (iter->index == sam.index)
							{
								found = true;
								break;
							}
						}
						if (!found)
						{
							// search sampler dcls
							for (size_t i = 0; i < program_->dcls.size(); ++ i)
							{
								if ((SO_DCL_SAMPLER == program_->dcls[i]->opcode)
									&& (sam.index == program_->dcls[i]->op->indices[0].disp))
								{
									if (program_->dcls[i]->dcl_sampler.shadow)
									{
										sam.shadow = true;
									}
									else
									{
										sam.shadow = false;
									}
								}
							}
							tex.samplers.push_back(sam);
						}
					}
				}
			}
			textures_.push_back(tex);
		}
	}
}

void GLSLGen::LinkCFInsns()
{
	if (!cf_insn_linked_.empty())
	{
		return;
	}

	std::vector<uint32_t> kcf_insn_linked(program_->insns.size(), static_cast<uint32_t>(-1));
	std::vector<uint32_t> cf_stack;
	for (size_t insn_num = 0; insn_num < program_->insns.size(); ++ insn_num)
	{
		uint32_t v;
		switch (program_->insns[insn_num]->opcode)
		{
		case SO_LOOP:
			cf_stack.push_back(insn_num);
			break;

		case SO_ENDLOOP:
			BOOST_ASSERT(!cf_stack.empty());
			v = cf_stack.back();
			BOOST_ASSERT(SO_LOOP == program_->insns[v]->opcode);
			kcf_insn_linked[v] = insn_num;
			kcf_insn_linked[insn_num] = v;
			cf_stack.pop_back();
			break;

		case SO_IF:
		case SO_SWITCH:
			kcf_insn_linked[insn_num] = insn_num; // later changed
			cf_stack.push_back(insn_num);
			break;

		case SO_ELSE:
		case SO_CASE:
			BOOST_ASSERT(!cf_stack.empty());
			v = cf_stack.back();
			if (SO_ELSE == program_->insns[insn_num]->opcode)
			{
				BOOST_ASSERT(SO_IF == program_->insns[v]->opcode);
			}
			else
			{
				BOOST_ASSERT((SO_SWITCH == program_->insns[v]->opcode) || (SO_CASE == program_->insns[v]->opcode));
			}
			kcf_insn_linked[insn_num] = kcf_insn_linked[v]; // later changed
			kcf_insn_linked[v] = insn_num;
			cf_stack.back() = insn_num;
			break;

		case SO_ENDSWITCH:
		case SO_ENDIF:
			BOOST_ASSERT(!cf_stack.empty());
			v = cf_stack.back();
			if (SO_ENDIF == program_->insns[insn_num]->opcode)
			{
				BOOST_ASSERT((SO_IF == program_->insns[v]->opcode) || (SO_ELSE == program_->insns[v]->opcode));
			}
			else
			{
				BOOST_ASSERT((SO_SWITCH == program_->insns[v]->opcode) || (SO_CASE == program_->insns[v]->opcode));
			}
			kcf_insn_linked[insn_num] = kcf_insn_linked[v];
			kcf_insn_linked[v] = insn_num;
			cf_stack.pop_back();
			break;

		default:
			break;
		}
	}
	BOOST_ASSERT(cf_stack.empty());
	cf_insn_linked_.swap(kcf_insn_linked);
	return;
}

void GLSLGen::FindLabels()
{
	if (labels_found_)
	{
		return;
	}

	std::vector<LabelInfo> labels;
	for (size_t insn_num = 0; insn_num < program_->insns.size(); ++ insn_num)
	{
		switch (program_->insns[insn_num]->opcode)
		{
		case SO_LABEL:
			if (program_->insns[insn_num]->num_ops > 0)
			{
				ShaderOperand& op = *program_->insns[insn_num]->ops[0];
				LabelInfo info;
				if ((SOT_LABEL == op.type) && op.HasSimpleIndex())
				{
					uint32_t idx = static_cast<uint32_t>(op.indices[0].disp);
					info.start_num = idx + 1;
					// find the last insn in the label code.
					uint32_t threshold = idx;
					for (uint32_t i = info.start_num; ; ++ i)
					{
						if ((SO_RET == program_->insns[i]->opcode) && (i > threshold))
						{
							info.end_num = i;
							break;
						}
						if ((SO_IF == program_->insns[i]->opcode)
							|| (SO_SWITCH == program_->insns[i]->opcode)
							|| (SO_ELSE == program_->insns[i]->opcode)
							|| (SO_LOOP == program_->insns[i]->opcode)
							|| (SO_CASE == program_->insns[i]->opcode))
						{
							if (cf_insn_linked_[i] > threshold)
							{
								threshold = cf_insn_linked_[i];
							}
						}

					}
					if (idx >= labels.size())
					{
						labels.resize(idx + 1);
					}
					labels[idx] = info;
				}
			}
			break;

		default:
			break;
		}
	}

	label_to_insn_num_.swap(labels);
	labels_found_ = true;
}

void GLSLGen::FindEndOfProgram()
{
	uint32_t threshold = 0;
	for (uint32_t i = 0;; i++)
	{
		if (program_->insns[i]->opcode == SO_RET&&i >= threshold)
		{
			end_of_program_ = i;
			break;
		}
		if ((SO_IF == program_->insns[i]->opcode)
			|| (SO_SWITCH == program_->insns[i]->opcode)
			|| (SO_ELSE == program_->insns[i]->opcode)
			|| (SO_LOOP == program_->insns[i]->opcode)
			|| (SO_CASE == program_->insns[i]->opcode))
		{
			if (cf_insn_linked_[i] > threshold)
			{
				threshold = cf_insn_linked_[i];
			}
		}
	}
}

DXBCInputBindDesc const & GLSLGen::GetResourceDesc(ShaderInputType type, uint32_t bind_point) const
{
	for (std::vector<DXBCInputBindDesc>::const_iterator iter = program_->resource_bindings.begin();
		iter != program_->resource_bindings.end(); ++ iter)
	{
		if ((iter->type == type) && (iter->bind_point == bind_point))
		{
			return *iter;
		}
	}

	BOOST_ASSERT(false);
	static DXBCInputBindDesc ret;
	return ret;
}

void GLSLGen::FindTempDcls()
{
	for (size_t i = 0; i < program_->dcls.size(); ++ i)
	{
		if ((SO_DCL_TEMPS == program_->dcls[i]->opcode)
			|| (SO_DCL_INDEXABLE_TEMP == program_->dcls[i]->opcode))
		{
			temp_dcls_.push_back(program_->dcls[i]);
		}
	}
}

void GLSLGen::ToTemps(std::ostream& out, ShaderDecl const & dcl)
{
	switch (dcl.opcode)
	{
	case SO_DCL_TEMPS:
		{
			for (uint32_t i = 0; i < dcl.num; ++ i)
			{
				out << "vec4 " << "tf" << i << ";\n";
				out << "ivec4 " << "ti" << i << ";\n";
			}

			temp_as_type_.assign(dcl.num * 4, SIT_Float);
		}
		break;

	case SO_DCL_INDEXABLE_TEMP:
		// Temp array
		out << "vec" << dcl.indexable_temp.comps << " IndexableTemp"
			<< dcl.op->indices[0].disp << "[" << dcl.indexable_temp.num << "];\n";
		break;

	default:
		BOOST_ASSERT(false);
		break;
	}
}

uint32_t GLSLGen::GetMaxComponentSelector(ShaderOperand const & op) const
{
	uint32_t num = this->GetOperandComponentNum(op);
	uint32_t max_idx = 0;
	for (uint32_t i = 0; i < num; ++ i)
	{
		uint32_t idx = this->GetComponentSelector(op, i);
		max_idx = std::max(max_idx, idx);
	}
	return max_idx;
}

uint32_t GLSLGen::GetMinComponentSelector(ShaderOperand const & op) const
{
	uint32_t num = this->GetOperandComponentNum(op);
	uint32_t min_idx = 3;
	for (uint32_t i = 0; i < num; ++ i)
	{
		uint32_t idx = this->GetComponentSelector(op, i);
		min_idx = std::min(min_idx, idx);
	}
	return min_idx;
}

void GLSLGen::ToDefaultValue(std::ostream& out, DXBCShaderVariable const & var, uint32_t offset)
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
				this->ToDefaultValue(out, p, var.type_desc.type);
			}
		}
		out << ")";
		break;

	case SVC_VECTOR:
		{
			switch (var.type_desc.type)
			{
			case SVT_INT:
				out << "i";
				break;

			case SVT_UINT:
				if (glsl_rules_ & GSR_UIntType)
				{
					out << "u";
				}
				else
				{
					out << "i";
				}
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
				this->ToDefaultValue(out, p, var.type_desc.type);
				p += 4;
			}
			out << ")";
		}
		break;

	case SVC_SCALAR:
		this->ToDefaultValue(out, p_base, var.type_desc.type);
		break;

	default:
		BOOST_ASSERT_MSG(false, "Unhandled type");
		break;
	}
}

void GLSLGen::ToDefaultValue(std::ostream& out, char const * value, ShaderVariableType type)
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

void GLSLGen::ToDefaultValue(std::ostream& out, DXBCShaderVariable const & var)
{
	if (0 == var.type_desc.elements)
	{
		this->ToDefaultValue(out, var, 0);
	}
	else
	{
		uint32_t stride = 0;
		switch (var.type_desc.type)
		{
		case SVT_INT:
			out << "i";
			break;

		case SVT_UINT:
			if (glsl_rules_ & GSR_UIntType)
			{
				out << "u";
			}
			else
			{
				out << "i";
			}
			break;

		case SVT_FLOAT:
			break;

		default:
			BOOST_ASSERT_MSG(false, "Unhandled type.");
			break;
		}
		switch (var.type_desc.var_class)
		{
		case SVC_SCALAR:
			stride = 4;
			break;
					
		case SVC_VECTOR:
			stride = 16;
			out << "vec" << var.type_desc.columns << "[]";
			break;

		case SVC_MATRIX_ROWS:
		case SVC_MATRIX_COLUMNS:
			stride = var.type_desc.columns * 16;
			out << "mat" << var.type_desc.columns << "x" << var.type_desc.rows << "[]";
			break;

		default:
			BOOST_ASSERT_MSG(false, "Unhandled type");
			break;
		}
		out << "(";
		for (uint32_t i = 0; i < var.type_desc.elements; ++ i)
		{
			if (i != 0)
			{
				out << ",";
			}
			this->ToDefaultValue(out, var, stride * i);
		}
		out << ")";
	}
}

uint32_t GLSLGen::ComponentSelectorFromMask(uint32_t mask, uint32_t comps) const
{
	uint32_t comps_index = 0;
	uint32_t count = 0;
	for (uint32_t i = 0; i < comps; ++ i)
	{
		if (mask & (1 << i))
		{
			comps_index |= i << (count * 8);
			++ count;
		}
	}
	for (; count < 4; ++ count)
	{
		comps_index |= 0xFF << (count * 8);
	}
	return comps_index;
}

uint32_t GLSLGen::ComponentSelectorFromSwizzle(uint8_t const swizzle[4], uint32_t comps) const
{
	uint32_t comps_index = 0;
	for (uint32_t i = 0; i < comps; ++ i)
	{
		comps_index |= swizzle[i] << (i * 8);
	}
	for (; comps < 4; ++ comps)
	{
		comps_index |= 0xFF << (comps * 8);
	}
	return comps_index;
}

uint32_t GLSLGen::ComponentSelectorFromScalar(uint8_t scalar) const
{
	uint32_t comps_index = 0xFFFFFF00;
	comps_index |= scalar;
	return comps_index;
}

uint32_t GLSLGen::ComponentSelectorFromCount(uint32_t count) const
{
	uint32_t comps_index = 0;
	for (uint32_t i = 0; i < count; ++ i)
	{
		comps_index |= i << (i * 8);
	}
	for (; count < 4; ++ count)
	{
		comps_index |= 0xFF << (count * 8);
	}
	return comps_index;
}

void GLSLGen::ToComponentSelector(std::ostream& out, uint32_t comps) const
{
	for (int i = 0; i < 4; ++ i)
	{
		uint32_t comp = (comps >> (i * 8)) & 0xFF;
		if (comp != 0xFF)
		{
			out << "xyzw"[comp];
		}
	}
}
