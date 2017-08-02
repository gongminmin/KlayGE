/**
 * @file NullRenderEngine.hpp
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

#ifndef KLAYGE_PLUGINS_NULL_RENDER_ENGINE_HPP
#define KLAYGE_PLUGINS_NULL_RENDER_ENGINE_HPP

#pragma once

#include <KFL/Vector.hpp>
#include <KFL/Color.hpp>

#include <vector>
#include <map>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>

namespace KlayGE
{
	class NullRenderEngine : public RenderEngine
	{
	public:
		NullRenderEngine();
		~NullRenderEngine() override;

		std::wstring const & Name() const override;

		bool RequiresFlipping() const override
		{
			return requires_flipping_;
		}

		void ForceFlush() override;

		TexturePtr const & ScreenDepthStencilTexture() const override;

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void GetCustomAttrib(std::string_view name, void* value) const override;
		void SetCustomAttrib(std::string_view name, void* value) override;

		bool FullScreen() const override;
		void FullScreen(bool fs) override;

		char const * VertexShaderProfile() const
		{
			return vs_profile_.c_str();
		}
		char const * PixelShaderProfile() const
		{
			return ps_profile_.c_str();
		}
		char const * GeometryShaderProfile() const
		{
			return gs_profile_.c_str();
		}
		char const * ComputeShaderProfile() const
		{
			return cs_profile_.c_str();
		}
		char const * HullShaderProfile() const
		{
			return hs_profile_.c_str();
		}
		char const * DomainShaderProfile() const
		{
			return ds_profile_.c_str();
		}

	private:
		void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) override;
		void DoBindFrameBuffer(FrameBufferPtr const & fb) override;
		void DoBindSOBuffers(RenderLayoutPtr const & rl) override;
		void DoRender(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl) override;
		void DoDispatch(RenderEffect const & effect, RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz) override;
		void DoDispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset) override;
		void DoResize(uint32_t width, uint32_t height) override;
		void DoDestroy() override;

		void DoSuspend() override;
		void DoResume() override;

		void FillRenderDeviceCaps();

		bool VertexFormatSupport(ElementFormat elem_fmt);
		bool TextureFormatSupport(ElementFormat elem_fmt);
		bool RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality);
		bool UAVFormatSupport(ElementFormat elem_fmt);

	private:
		uint8_t major_version_;
		uint8_t minor_version_;
		bool requires_flipping_;
		bool frag_depth_support_;

		std::vector<ElementFormat> vertex_format_;
		std::vector<ElementFormat> texture_format_;
		std::map<ElementFormat, std::vector<std::pair<uint32_t, uint32_t>>> rendertarget_format_;
		std::vector<ElementFormat> uav_format_;

		std::string vs_profile_;
		std::string ps_profile_;
		std::string gs_profile_;
		std::string cs_profile_;
		std::string hs_profile_;
		std::string ds_profile_;
	};
}

#endif			// KLAYGE_PLUGINS_NULL_RENDER_ENGINE_HPP
