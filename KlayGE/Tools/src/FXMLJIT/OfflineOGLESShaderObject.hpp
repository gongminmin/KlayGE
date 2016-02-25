/**
* @file OfflineOGLESShaderObject.hpp
* @author Minmin Gong
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

#ifndef _OFFLINEOGLESSHADEROBJECT_HPP
#define _OFFLINEOGLESSHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include "OfflineShaderObject.hpp"

namespace KlayGE
{
	namespace Offline
	{
		class OGLESShaderObject : public ShaderObject
		{
		public:
			explicit OGLESShaderObject(OfflineRenderDeviceCaps const & caps);

			virtual void StreamOut(std::ostream& os, ShaderType type) override;

			void AttachShader(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids);
			void AttachShader(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so);
			void LinkShaders(RenderEffect const & effect);

		private:
			std::shared_ptr<std::array<std::string, ST_NumShaderTypes>> shader_func_names_;
			std::shared_ptr<std::array<std::shared_ptr<std::string>, ST_NumShaderTypes>> glsl_srcs_;
			std::shared_ptr<std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes>> pnames_;
			std::shared_ptr<std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes>> glsl_res_names_;
			std::shared_ptr<std::vector<VertexElementUsage>> vs_usages_;
			std::shared_ptr<std::vector<uint8_t>> vs_usage_indices_;
			std::shared_ptr<std::vector<std::string>> glsl_vs_attrib_names_;
			uint32_t ds_partitioning_, ds_output_primitive_;

			std::vector<std::tuple<std::string, RenderEffectParameterPtr, RenderEffectParameterPtr, uint32_t>> tex_sampler_binds_;
		};

		typedef std::shared_ptr<OGLESShaderObject> OGLESShaderObjectPtr;
	}
}

#endif			// _OFFLINEOGLESSHADEROBJECT_HPP
