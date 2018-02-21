/**
 * @file NullRenderEngine.cpp
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

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Hash.hpp>

#include <KlayGE/NullRender/NullRenderEngine.hpp>

namespace KlayGE
{
	NullRenderEngine::NullRenderEngine()
	{
	}

	NullRenderEngine::~NullRenderEngine()
	{
		this->Destroy();
	}

	std::wstring const & NullRenderEngine::Name() const
	{
		static std::wstring const name(L"Null Render Engine");
		return name;
	}

	void NullRenderEngine::DoCreateRenderWindow(std::string const & name, RenderSettings const & settings)
	{
		KFL_UNUSED(name);
		KFL_UNUSED(settings);
	}

	void NullRenderEngine::ForceFlush()
	{
	}

	TexturePtr const & NullRenderEngine::ScreenDepthStencilTexture() const
	{
		static TexturePtr ret;
		return ret;
	}

	void NullRenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		KFL_UNUSED(x);
		KFL_UNUSED(y);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
	}

	void NullRenderEngine::GetCustomAttrib(std::string_view name, void* value) const
	{
		size_t const name_hash = HashRange(name.begin(), name.end());
		if (CT_HASH("MAJOR_VERSION") == name_hash)
		{
			*static_cast<uint32_t*>(value) = major_version_;
		}
		else if (CT_HASH("MINOR_VERSION") == name_hash)
		{
			*static_cast<uint32_t*>(value) = minor_version_;
		}
		else if (CT_HASH("FRAG_DEPTH_SUPPORT") == name_hash)
		{
			*static_cast<bool*>(value) = frag_depth_support_;
		}
	}

	void NullRenderEngine::SetCustomAttrib(std::string_view name, void* value)
	{
		size_t const name_hash = HashRange(name.begin(), name.end());
		if (CT_HASH("PLATFORM") == name_hash)
		{
			native_shader_platform_name_ = *static_cast<std::string*>(value);

			if (native_shader_platform_name_.find("d3d_11") == 0)
			{
				vs_profile_ = "vs_5_0";
				ps_profile_ = "ps_5_0";
				gs_profile_ = "gs_5_0";
				cs_profile_ = "cs_5_0";
				hs_profile_ = "hs_5_0";
				ds_profile_ = "ds_5_0";
			}
			else if (native_shader_platform_name_.find("d3d_12") == 0)
			{
				vs_profile_ = "vs_5_1";
				ps_profile_ = "ps_5_1";
				gs_profile_ = "gs_5_1";
				cs_profile_ = "cs_5_1";
				hs_profile_ = "hs_5_1";
				ds_profile_ = "ds_5_1";
			}
			else
			{
				KFL_UNREACHABLE("Invalid feature level");
			}
		}
		else if (CT_HASH("MAJOR_VERSION") == name_hash)
		{
			major_version_ = static_cast<uint8_t>(*static_cast<uint32_t*>(value));
		}
		else if (CT_HASH("MINOR_VERSION") == name_hash)
		{
			minor_version_ = static_cast<uint8_t>(*static_cast<uint32_t*>(value));
		}
		else if (CT_HASH("REQUIRES_FLIPPING") == name_hash)
		{
			requires_flipping_ = *static_cast<bool*>(value);
		}
		else if (CT_HASH("NATIVE_SHADER_FOURCC") == name_hash)
		{
			native_shader_fourcc_ = *static_cast<uint32_t*>(value);
		}
		else if (CT_HASH("NATIVE_SHADER_VERSION") == name_hash)
		{
			native_shader_version_ = *static_cast<uint32_t*>(value);
		}
		else if (CT_HASH("VERTEX_FORMAT") == name_hash)
		{
			vertex_format_ = *static_cast<std::vector<ElementFormat>*>(value);
			this->FillRenderDeviceCaps();
		}
		else if (CT_HASH("TEXTURE_FORMAT") == name_hash)
		{
			texture_format_ = *static_cast<std::vector<ElementFormat>*>(value);
			this->FillRenderDeviceCaps();
		}
		else if (CT_HASH("RENDER_TARGET_FORMAT") == name_hash)
		{
			rendertarget_format_ = *static_cast<std::map<ElementFormat, std::vector<std::pair<uint32_t, uint32_t>>>*>(value);
			this->FillRenderDeviceCaps();
		}
		else if (CT_HASH("UAV_FORMAT") == name_hash)
		{
			uav_format_ = *static_cast<std::vector<ElementFormat>*>(value);
			this->FillRenderDeviceCaps();
		}
		else if (CT_HASH("DEVICE_CAPS") == name_hash)
		{
			caps_ = *static_cast<RenderDeviceCaps*>(value);
		}
		else if (CT_HASH("FRAG_DEPTH_SUPPORT") == name_hash)
		{
			frag_depth_support_ = *static_cast<bool*>(value);
		}
	}

	void NullRenderEngine::DoBindFrameBuffer(FrameBufferPtr const & fb)
	{
		KFL_UNUSED(fb);
	}

	void NullRenderEngine::DoBindSOBuffers(RenderLayoutPtr const & rl)
	{
		KFL_UNUSED(rl);
	}

	void NullRenderEngine::DoRender(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl)
	{
		KFL_UNUSED(effect);
		KFL_UNUSED(tech);
		KFL_UNUSED(rl);
	}

	void NullRenderEngine::DoDispatch(RenderEffect const & effect, RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		KFL_UNUSED(effect);
		KFL_UNUSED(tech);
		KFL_UNUSED(tgx);
		KFL_UNUSED(tgy);
		KFL_UNUSED(tgz);
	}

	void NullRenderEngine::DoDispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
		GraphicsBufferPtr const & buff_args, uint32_t offset)
	{
		KFL_UNUSED(effect);
		KFL_UNUSED(tech);
		KFL_UNUSED(buff_args);
		KFL_UNUSED(offset);
	}

	void NullRenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		KFL_UNUSED(width);
		KFL_UNUSED(height);
	}

	void NullRenderEngine::DoDestroy()
	{
	}

	void NullRenderEngine::DoSuspend()
	{
	}

	void NullRenderEngine::DoResume()
	{
	}

	bool NullRenderEngine::FullScreen() const
	{
		return false;
	}

	void NullRenderEngine::FullScreen(bool fs)
	{
		KFL_UNUSED(fs);
	}

	bool NullRenderEngine::VertexFormatSupport(ElementFormat elem_fmt)
	{
		auto iter = std::lower_bound(vertex_format_.begin(), vertex_format_.end(), elem_fmt);
		return (iter != vertex_format_.end()) && (*iter == elem_fmt);
	}

	bool NullRenderEngine::TextureFormatSupport(ElementFormat elem_fmt)
	{
		auto iter = std::lower_bound(texture_format_.begin(), texture_format_.end(), elem_fmt);
		return (iter != texture_format_.end()) && (*iter == elem_fmt);
	}

	bool NullRenderEngine::RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality)
	{
		auto iter = rendertarget_format_.find(elem_fmt);
		if (iter != rendertarget_format_.end())
		{
			for (auto const & p : iter->second)
			{
				if ((sample_count == p.first) && (sample_quality < p.second))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool NullRenderEngine::UAVFormatSupport(ElementFormat elem_fmt)
	{
		auto iter = std::lower_bound(uav_format_.begin(), uav_format_.end(), elem_fmt);
		return (iter != uav_format_.end()) && (*iter == elem_fmt);
	}

	void NullRenderEngine::FillRenderDeviceCaps()
	{
		std::sort(vertex_format_.begin(), vertex_format_.end());
		vertex_format_.erase(std::unique(vertex_format_.begin(), vertex_format_.end()), vertex_format_.end());
		std::sort(texture_format_.begin(), texture_format_.end());
		texture_format_.erase(std::unique(texture_format_.begin(), texture_format_.end()), texture_format_.end());
		std::sort(uav_format_.begin(), uav_format_.end());
		uav_format_.erase(std::unique(uav_format_.begin(), uav_format_.end()), uav_format_.end());

		caps_.vertex_format_support = [this](ElementFormat elem_fmt)
			{
				return this->VertexFormatSupport(elem_fmt);
			};
		caps_.texture_format_support = [this](ElementFormat elem_fmt)
			{
				return this->TextureFormatSupport(elem_fmt);
			};
		caps_.rendertarget_format_support = [this](ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality)
			{
				return this->RenderTargetFormatSupport(elem_fmt, sample_count, sample_quality);
			};
		caps_.uav_format_support = [this](ElementFormat elem_fmt)
			{
				return this->UAVFormatSupport(elem_fmt);
			};
	}
}
