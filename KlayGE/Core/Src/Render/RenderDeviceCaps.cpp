/**
 * @file RenderDeviceCaps.cpp
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
#include <KlayGE/RenderDeviceCaps.hpp>

namespace KlayGE
{
	bool RenderDeviceCaps::VertexFormatSupport(ElementFormat format) const
	{
		auto iter = std::lower_bound(vertex_formats_.begin(), vertex_formats_.end(), format);
		return (iter != vertex_formats_.end()) && (*iter == format);
	}

	bool RenderDeviceCaps::TextureFormatSupport(ElementFormat format) const
	{
		auto iter = std::lower_bound(texture_formats_.begin(), texture_formats_.end(), format);
		return (iter != texture_formats_.end()) && (*iter == format);
	}

	bool RenderDeviceCaps::RenderTargetFormatSupport(ElementFormat format, uint32_t sample_count, uint32_t sample_quality) const
	{
		auto iter = render_target_formats_.find(format);
		if (iter != render_target_formats_.end())
		{
			for (auto const & p : iter->second)
			{
				if ((sample_count == this->DecodeSampleCount(p)) && (sample_quality < this->DecodeSampleQuality(p)))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool RenderDeviceCaps::TextureRenderTargetFormatSupport(ElementFormat format, uint32_t sample_count, uint32_t sample_quality) const
	{
		return this->TextureFormatSupport(format) && this->RenderTargetFormatSupport(format, sample_count, sample_quality);
	}

	bool RenderDeviceCaps::UavFormatSupport(ElementFormat format) const
	{
		auto iter = std::lower_bound(uav_formats_.begin(), uav_formats_.end(), format);
		return (iter != uav_formats_.end()) && (*iter == format);
	}

	ElementFormat RenderDeviceCaps::BestMatchVertexFormat(std::span<ElementFormat const> formats) const
	{
		ElementFormat ret = EF_Unknown;
		for (auto fmt : formats)
		{
			if (this->VertexFormatSupport(fmt))
			{
				ret = fmt;
				break;
			}
		}

		return ret;
	}

	ElementFormat RenderDeviceCaps::BestMatchTextureFormat(std::span<ElementFormat const> formats) const
	{
		ElementFormat ret = EF_Unknown;
		for (auto fmt : formats)
		{
			if (this->TextureFormatSupport(fmt))
			{
				ret = fmt;
				break;
			}
		}

		return ret;
	}

	ElementFormat RenderDeviceCaps::BestMatchRenderTargetFormat(std::span<ElementFormat const> formats,
		uint32_t sample_count, uint32_t sample_quality) const
	{
		ElementFormat ret = EF_Unknown;
		for (auto fmt : formats)
		{
			if (this->RenderTargetFormatSupport(fmt, sample_count, sample_quality))
			{
				ret = fmt;
				break;
			}
		}

		return ret;
	}

	ElementFormat RenderDeviceCaps::BestMatchTextureRenderTargetFormat(std::span<ElementFormat const> formats,
		uint32_t sample_count, uint32_t sample_quality) const
	{
		ElementFormat ret = EF_Unknown;
		for (auto fmt : formats)
		{
			if (this->TextureRenderTargetFormatSupport(fmt, sample_count, sample_quality))
			{
				ret = fmt;
				break;
			}
		}

		return ret;
	}

	ElementFormat RenderDeviceCaps::BestMatchUavFormat(std::span<ElementFormat const> formats) const
	{
		ElementFormat ret = EF_Unknown;
		for (auto fmt : formats)
		{
			if (this->UavFormatSupport(fmt))
			{
				ret = fmt;
				break;
			}
		}

		return ret;
	}

	void RenderDeviceCaps::AssignVertexFormats(std::vector<ElementFormat> vertex_formats)
	{
		std::sort(vertex_formats.begin(), vertex_formats.end());
		vertex_formats.erase(std::unique(vertex_formats.begin(), vertex_formats.end()), vertex_formats.end());

		vertex_formats_ = std::move(vertex_formats);
	}

	void RenderDeviceCaps::AssignTextureFormats(std::vector<ElementFormat> texture_formats)
	{
		std::sort(texture_formats.begin(), texture_formats.end());
		texture_formats.erase(std::unique(texture_formats.begin(), texture_formats.end()), texture_formats.end());

		texture_formats_ = std::move(texture_formats);

		this->UpdateSupportBits();
	}

	void RenderDeviceCaps::AssignRenderTargetFormats(std::map<ElementFormat, std::vector<uint32_t>> render_target_formats)
	{
		render_target_formats_.clear();
		for (auto const & item : render_target_formats)
		{
			render_target_formats_.emplace(item.first, item.second);
		}

		this->UpdateSupportBits();
	}

	void RenderDeviceCaps::AssignUavFormats(std::vector<ElementFormat> uav_formats)
	{
		std::sort(uav_formats.begin(), uav_formats.end());
		uav_formats.erase(std::unique(uav_formats.begin(), uav_formats.end()), uav_formats.end());

		uav_formats_ = std::move(uav_formats);
	}

	void RenderDeviceCaps::UpdateSupportBits()
	{
		depth_texture_support = (this->TextureFormatSupport(EF_D24S8) || this->TextureFormatSupport(EF_D16));
		fp_color_support = ((this->TextureFormatSupport(EF_B10G11R11F) && this->RenderTargetFormatSupport(EF_B10G11R11F, 1, 0))
			|| (this->TextureFormatSupport(EF_ABGR16F) && this->RenderTargetFormatSupport(EF_ABGR16F, 1, 0)));
		pack_to_rgba_required = !(this->TextureFormatSupport(EF_R16F) && this->RenderTargetFormatSupport(EF_R16F, 1, 0)
			&& this->TextureFormatSupport(EF_R32F) && this->RenderTargetFormatSupport(EF_R32F, 1, 0));
	}
}
