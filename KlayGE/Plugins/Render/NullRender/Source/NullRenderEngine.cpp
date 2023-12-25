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
#include <KFL/CXX23/utility.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Hash.hpp>

#include "NullRenderEngine.hpp"

namespace KlayGE
{
	NullRenderEngine::NullRenderEngine() = default;

	NullRenderEngine::~NullRenderEngine()
	{
		this->Destroy();
	}

	std::wstring const & NullRenderEngine::Name() const
	{
		static std::wstring const name(L"Null Render Engine");
		return name;
	}

	void NullRenderEngine::DoCreateRenderWindow([[maybe_unused]] std::string const & name, [[maybe_unused]] RenderSettings const & settings)
	{
	}

	void NullRenderEngine::ForceFlush()
	{
	}

	TexturePtr const & NullRenderEngine::ScreenDepthStencilTexture() const
	{
		static TexturePtr ret;
		return ret;
	}

	void NullRenderEngine::ScissorRect(
		[[maybe_unused]] uint32_t x, [[maybe_unused]] uint32_t y, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
	{
	}

	void NullRenderEngine::GetCustomAttrib(std::string_view name, void* value) const
	{
		size_t const name_hash = HashValue(std::move(name));
		if (CtHash("MAJOR_VERSION") == name_hash)
		{
			*static_cast<uint32_t*>(value) = major_version_;
		}
		else if (CtHash("MINOR_VERSION") == name_hash)
		{
			*static_cast<uint32_t*>(value) = minor_version_;
		}
		else if (CtHash("FRAG_DEPTH_SUPPORT") == name_hash)
		{
			*static_cast<bool*>(value) = frag_depth_support_;
		}
	}

	void NullRenderEngine::SetCustomAttrib(std::string_view name, void* value)
	{
		size_t const name_hash = HashValue(std::move(name));
		if (CtHash("PLATFORM") == name_hash)
		{
			native_shader_platform_name_ = *static_cast<std::string*>(value);

			if (native_shader_platform_name_.find("d3d_12") == 0)
			{
				shader_profiles_[std::to_underlying(ShaderStage::Vertex)] = "vs_5_1";
				shader_profiles_[std::to_underlying(ShaderStage::Pixel)] = "ps_5_1";
				shader_profiles_[std::to_underlying(ShaderStage::Geometry)] = "gs_5_1";
				shader_profiles_[std::to_underlying(ShaderStage::Compute)] = "cs_5_1";
				shader_profiles_[std::to_underlying(ShaderStage::Hull)] = "hs_5_1";
				shader_profiles_[std::to_underlying(ShaderStage::Domain)] = "ds_5_1";
			}
			else
			{
				shader_profiles_[std::to_underlying(ShaderStage::Vertex)] = "vs_5_0";
				shader_profiles_[std::to_underlying(ShaderStage::Pixel)] = "ps_5_0";
				shader_profiles_[std::to_underlying(ShaderStage::Geometry)] = "gs_5_0";
				shader_profiles_[std::to_underlying(ShaderStage::Compute)] = "cs_5_0";
				shader_profiles_[std::to_underlying(ShaderStage::Hull)] = "hs_5_0";
				shader_profiles_[std::to_underlying(ShaderStage::Domain)] = "ds_5_0";
			}
		}
		else if (CtHash("MAJOR_VERSION") == name_hash)
		{
			major_version_ = static_cast<uint8_t>(*static_cast<uint32_t*>(value));
		}
		else if (CtHash("MINOR_VERSION") == name_hash)
		{
			minor_version_ = static_cast<uint8_t>(*static_cast<uint32_t*>(value));
		}
		else if (CtHash("REQUIRES_FLIPPING") == name_hash)
		{
			requires_flipping_ = *static_cast<bool*>(value);
		}
		else if (CtHash("NATIVE_SHADER_FOURCC") == name_hash)
		{
			native_shader_fourcc_ = *static_cast<uint32_t*>(value);
		}
		else if (CtHash("NATIVE_SHADER_VERSION") == name_hash)
		{
			native_shader_version_ = *static_cast<uint32_t*>(value);
		}
		else if (CtHash("DEVICE_CAPS") == name_hash)
		{
			caps_ = *static_cast<RenderDeviceCaps*>(value);
		}
		else if (CtHash("FRAG_DEPTH_SUPPORT") == name_hash)
		{
			frag_depth_support_ = *static_cast<bool*>(value);
		}
	}

	void NullRenderEngine::DoBindFrameBuffer([[maybe_unused]] FrameBufferPtr const & fb)
	{
	}

	void NullRenderEngine::DoBindSOBuffers([[maybe_unused]] RenderLayoutPtr const & rl)
	{
	}

	void NullRenderEngine::DoRender(
		[[maybe_unused]] RenderEffect const& effect, [[maybe_unused]] RenderTechnique const& tech, [[maybe_unused]] RenderLayout const& rl)
	{
	}

	void NullRenderEngine::DoDispatch([[maybe_unused]] RenderEffect const& effect, [[maybe_unused]] RenderTechnique const& tech,
		[[maybe_unused]] uint32_t tgx, [[maybe_unused]] uint32_t tgy, [[maybe_unused]] uint32_t tgz)
	{
	}

	void NullRenderEngine::DoDispatchIndirect([[maybe_unused]] RenderEffect const & effect, [[maybe_unused]] RenderTechnique const & tech,
		[[maybe_unused]] GraphicsBufferPtr const & buff_args, [[maybe_unused]] uint32_t offset)
	{
	}

	void NullRenderEngine::DoResize([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
	{
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

	void NullRenderEngine::FullScreen([[maybe_unused]] bool fs)
	{
	}
}
