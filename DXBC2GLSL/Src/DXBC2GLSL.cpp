/**************************************************************************
 *
 * Copyright 2013 Shenghua Lin, Minmin Gong
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

#include <DXBC2GLSL/DXBC2GLSL.hpp>
#include <DXBC2GLSL/DXBC.hpp>
#include <DXBC2GLSL/GLSLGen.hpp>
#include <sstream>

namespace DXBC2GLSL
{
	uint32_t DXBC2GLSL::DefaultRules(GLSLVersion version)
	{
		return GLSLGen::DefaultRules(version);
	}

	void DXBC2GLSL::FeedDXBC(void const * dxbc_data, bool has_gs, GLSLVersion version)
	{
		this->FeedDXBC(dxbc_data, has_gs, version, this->DefaultRules(version));
	}

	void DXBC2GLSL::FeedDXBC(void const * dxbc_data, bool has_gs, GLSLVersion version, uint32_t glsl_rules)
	{
		dxbc_ = DXBCParse(dxbc_data);
		if (dxbc_)
		{
			if (dxbc_->shader_chunk)
			{
				shader_ = ShaderParse(*dxbc_);

				std::stringstream ss;

				GLSLGen converter;
				converter.FeedDXBC(shader_, has_gs, version, glsl_rules);
				converter.ToGLSL(ss);

				glsl_ = ss.str();
			}
		}
	}

	std::string const & DXBC2GLSL::GLSLString() const
	{
		return glsl_;
	}

	uint32_t DXBC2GLSL::NumInputParams() const
	{
		return static_cast<uint32_t>(shader_->params_in.size());
	}

	DXBCSignatureParamDesc const & DXBC2GLSL::InputParam(uint32_t index) const
	{
		BOOST_ASSERT(index < shader_->params_in.size());
		return shader_->params_in[index];
	}

	uint32_t DXBC2GLSL::NumOutputParams() const
	{
		return static_cast<uint32_t>(shader_->params_out.size());
	}

	DXBCSignatureParamDesc const & DXBC2GLSL::OutputParam(uint32_t index) const
	{
		BOOST_ASSERT(index < shader_->params_out.size());
		return shader_->params_out[index];
	}

	uint32_t DXBC2GLSL::NumCBuffers() const
	{
		return static_cast<uint32_t>(shader_->cbuffers.size());
	}

	uint32_t DXBC2GLSL::NumVariables(uint32_t cb_index) const
	{
		BOOST_ASSERT(cb_index < shader_->cbuffers.size());
		return static_cast<uint32_t>(shader_->cbuffers[cb_index].vars.size());
	}

	char const * DXBC2GLSL::VariableName(uint32_t cb_index, uint32_t var_index) const
	{
		BOOST_ASSERT(cb_index < shader_->cbuffers.size());
		BOOST_ASSERT(var_index < shader_->cbuffers[cb_index].vars.size());
		return shader_->cbuffers[cb_index].vars[var_index].var_desc.name;
	}

	bool DXBC2GLSL::VariableUsed(uint32_t cb_index, uint32_t var_index) const
	{
		BOOST_ASSERT(cb_index < shader_->cbuffers.size());
		BOOST_ASSERT(var_index < shader_->cbuffers[cb_index].vars.size());
		return shader_->cbuffers[cb_index].vars[var_index].var_desc.flags ? true : false;
	}

	uint32_t DXBC2GLSL::NumResources() const
	{
		return static_cast<uint32_t>(shader_->resource_bindings.size());
	}

	char const * DXBC2GLSL::ResourceName(uint32_t index) const
	{
		BOOST_ASSERT(index < shader_->resource_bindings.size());
		return shader_->resource_bindings[index].name;
	}

	uint32_t DXBC2GLSL::ResourceBindPoint(uint32_t index) const
	{
		BOOST_ASSERT(index < shader_->resource_bindings.size());
		return shader_->resource_bindings[index].bind_point;
	}

	ShaderInputType DXBC2GLSL::ResourceType(uint32_t index) const
	{
		BOOST_ASSERT(index < shader_->resource_bindings.size());
		return shader_->resource_bindings[index].type;
	}

	bool DXBC2GLSL::ResourceUsed(uint32_t index) const
	{
		BOOST_ASSERT(index < shader_->resource_bindings.size());
		return !(shader_->resource_bindings[index].flags & DSIF_Unused);
	}

	ShaderPrimitive DXBC2GLSL::GSInputPrimitive() const
	{
		return shader_->gs_input_primitive;
	}

	uint32_t DXBC2GLSL::NumGSOutputTopology() const
	{
		return static_cast<uint32_t>(shader_->gs_output_topology.size());
	}

	ShaderPrimitiveTopology DXBC2GLSL::GSOutputTopology(uint32_t index) const
	{
		BOOST_ASSERT(index < shader_->gs_output_topology.size());
		return shader_->gs_output_topology[index];
	}

	uint32_t DXBC2GLSL::MaxGSOutputVertex() const
	{
		return shader_->max_gs_output_vertex;
	}
}
