/**
 * @file DXBC2GLSL.cpp
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

#include <DXBC2GLSL/DXBC2GLSL.hpp>
#include <KFL/CustomizedStreamBuf.hpp>
#include <DXBC2GLSL/DXBC.hpp>
#include <DXBC2GLSL/GLSLGen.hpp>
#include <sstream>

namespace DXBC2GLSL
{
	uint32_t DXBC2GLSL::DefaultRules(GLSLVersion version)
	{
		return GLSLGen::DefaultRules(version);
	}

	void DXBC2GLSL::FeedDXBC(void const * dxbc_data,
			bool has_gs, bool has_ps, ShaderTessellatorPartitioning ds_partitioning, ShaderTessellatorOutputPrimitive ds_output_primitive,
			GLSLVersion version)
	{
		this->FeedDXBC(dxbc_data, has_gs, has_ps, ds_partitioning, ds_output_primitive, version, this->DefaultRules(version));
	}

	void DXBC2GLSL::FeedDXBC(void const * dxbc_data,
			bool has_gs, bool has_ps, ShaderTessellatorPartitioning ds_partitioning, ShaderTessellatorOutputPrimitive ds_output_primitive,
			GLSLVersion version, uint32_t glsl_rules)
	{
		dxbc_ = DXBCParse(dxbc_data);
		if (dxbc_)
		{
			if (dxbc_->shader_chunk)
			{
				shader_ = ShaderParse(*dxbc_);

				KlayGE::StringOutputStreamBuf glsl_buff(glsl_);
				std::ostream ss(&glsl_buff);

				GLSLGen converter;
				converter.FeedDXBC(shader_, has_gs, has_ps, ds_partitioning, ds_output_primitive, version, glsl_rules);
				converter.ToGLSL(ss);
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

	ShaderSRVDimension DXBC2GLSL::ResourceDimension(uint32_t index) const
	{
		BOOST_ASSERT(index < shader_->resource_bindings.size());
		return shader_->resource_bindings[index].dimension;
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

	uint32_t DXBC2GLSL::GSInstanceCount() const
	{
		return shader_->gs_instance_count;
	}

	ShaderTessellatorPartitioning DXBC2GLSL::DSPartitioning() const
	{
		return shader_->ds_tessellator_partitioning;
	}

	ShaderTessellatorOutputPrimitive DXBC2GLSL::DSOutputPrimitive() const
	{
		return shader_->ds_tessellator_output_primitive;
	}
}
