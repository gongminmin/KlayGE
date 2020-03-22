/**
 * @file DXBC2GLSL.hpp
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

#ifndef _DXBC2GLSL_HPP
#define _DXBC2GLSL_HPP

#pragma once

#include <KFL/KFL.hpp>

#ifndef DXBC2GLSL_SOURCE
	#define KLAYGE_LIB_NAME DXBC2GLSLLib
	#include <KFL/Detail/AutoLink.hpp>
#endif	// DXBC2GLSL_SOURCE

#include <DXBC2GLSL/DXBC.hpp>
#include <DXBC2GLSL/Shader.hpp>
#include <DXBC2GLSL/GLSLGen.hpp>

namespace DXBC2GLSL
{
	class DXBC2GLSL final
	{
	public:
		static uint32_t DefaultRules(GLSLVersion version);

		void FeedDXBC(void const * dxbc_data,
			bool has_gs, bool has_ps, ShaderTessellatorPartitioning ds_partitioning, ShaderTessellatorOutputPrimitive ds_output_primitive,
			GLSLVersion version);
		void FeedDXBC(void const * dxbc_data,
			bool has_gs, bool has_ps, ShaderTessellatorPartitioning ds_partitioning, ShaderTessellatorOutputPrimitive ds_output_primitive,
			GLSLVersion version, uint32_t glsl_rules);

		std::string const & GLSLString() const;

		uint32_t NumInputParams() const;
		DXBCSignatureParamDesc const & InputParam(uint32_t index) const;

		uint32_t NumOutputParams() const;
		DXBCSignatureParamDesc const & OutputParam(uint32_t index) const;

		uint32_t NumCBuffers() const;
		uint32_t NumVariables(uint32_t cb_index) const;
		char const * VariableName(uint32_t cb_index, uint32_t var_index) const;
		bool VariableUsed(uint32_t cb_index, uint32_t var_index) const;

		uint32_t NumResources() const;
		char const * ResourceName(uint32_t index) const;
		uint32_t ResourceBindPoint(uint32_t index) const;
		ShaderInputType ResourceType(uint32_t index) const;
		ShaderSRVDimension ResourceDimension(uint32_t index) const;
		bool ResourceUsed(uint32_t index) const;

		ShaderPrimitive GSInputPrimitive() const;
		uint32_t NumGSOutputTopology() const;
		ShaderPrimitiveTopology GSOutputTopology(uint32_t index) const;
		uint32_t MaxGSOutputVertex() const;
		uint32_t GSInstanceCount() const;

		ShaderTessellatorPartitioning DSPartitioning() const;
		ShaderTessellatorOutputPrimitive DSOutputPrimitive() const;

	private:
		std::shared_ptr<DXBCContainer> dxbc_;
		std::shared_ptr<ShaderProgram> shader_;
		std::string glsl_;
	};
}

#endif		// _DXBC2GLSL_HPP
