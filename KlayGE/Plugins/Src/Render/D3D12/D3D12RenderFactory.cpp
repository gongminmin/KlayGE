/**
 * @file D3D12RenderFactory.cpp
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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12FrameBuffer.hpp>
#include <KlayGE/D3D12/D3D12RenderLayout.hpp>
#include <KlayGE/D3D12/D3D12GraphicsBuffer.hpp>
#include <KlayGE/D3D12/D3D12Query.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>
#include <KlayGE/D3D12/D3D12RenderStateObject.hpp>
#include <KlayGE/D3D12/D3D12ShaderObject.hpp>
#include <KlayGE/D3D12/D3D12Fence.hpp>

#include <KlayGE/D3D12/D3D12RenderFactory.hpp>

namespace KlayGE
{
	D3D12RenderFactory::D3D12RenderFactory()
	{
	}

	std::wstring const & D3D12RenderFactory::Name() const
	{
		static std::wstring const name(L"Direct3D12 Render Factory");
		return name;
	}

	TexturePtr D3D12RenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D12Texture1D>(width, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}
	TexturePtr D3D12RenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D12Texture2D>(width, height, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}
	TexturePtr D3D12RenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D12Texture3D>(width, height, depth, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}
	TexturePtr D3D12RenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D12TextureCube>(size, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}

	FrameBufferPtr D3D12RenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<D3D12FrameBuffer>();
	}

	RenderLayoutPtr D3D12RenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<D3D12RenderLayout>();
	}

	GraphicsBufferPtr D3D12RenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint,
			size_in_byte, fmt);
	}

	GraphicsBufferPtr D3D12RenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint,
			size_in_byte, fmt);
	}

	GraphicsBufferPtr D3D12RenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint,
			size_in_byte, fmt);
	}

	QueryPtr D3D12RenderFactory::MakeOcclusionQuery()
	{
		return MakeSharedPtr<D3D12OcclusionQuery>();
	}

	QueryPtr D3D12RenderFactory::MakeConditionalRender()
	{
		return MakeSharedPtr<D3D12ConditionalRender>();
	}

	QueryPtr D3D12RenderFactory::MakeTimerQuery()
	{
		return MakeSharedPtr<D3D12TimerQuery>();
	}

	QueryPtr D3D12RenderFactory::MakeSOStatisticsQuery()
	{
		return MakeSharedPtr<D3D12SOStatisticsQuery>();
	}

	FencePtr D3D12RenderFactory::MakeFence()
	{
		return MakeSharedPtr<D3D12Fence>();
	}

	RenderViewPtr D3D12RenderFactory::Make1DRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D12RenderTargetRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr D3D12RenderFactory::Make2DRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D12RenderTargetRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr D3D12RenderFactory::Make2DRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D12RenderTargetRenderView>(texture, array_index, face, level);
	}

	RenderViewPtr D3D12RenderFactory::Make2DRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return this->Make3DRenderView(texture, array_index, slice, 1, level);
	}

	RenderViewPtr D3D12RenderFactory::MakeCubeRenderView(Texture& texture, int array_index, int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D12RenderTargetRenderView>(texture, array_index, array_size, level);
	}

	RenderViewPtr D3D12RenderFactory::Make3DRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D12RenderTargetRenderView>(texture, array_index, first_slice, num_slices, level);
	}

	RenderViewPtr D3D12RenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer,
		uint32_t width, uint32_t height, ElementFormat pf)
	{
		return MakeSharedPtr<D3D12RenderTargetRenderView>(gbuffer, width, height, pf);
	}

	RenderViewPtr D3D12RenderFactory::Make2DDepthStencilRenderView(uint32_t width, uint32_t height,
		ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		return MakeSharedPtr<D3D12DepthStencilRenderView>(width, height, pf, sample_count, sample_quality);
	}

	RenderViewPtr D3D12RenderFactory::Make1DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D12DepthStencilRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr D3D12RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D12DepthStencilRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr D3D12RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D12DepthStencilRenderView>(texture, array_index, face, level);
	}
	
	RenderViewPtr D3D12RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return this->Make3DDepthStencilRenderView(texture, array_index, slice, 1, level);
	}

	RenderViewPtr D3D12RenderFactory::MakeCubeDepthStencilRenderView(Texture& texture, int array_index, int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D12DepthStencilRenderView>(texture, array_index, array_size, level);
	}
	
	RenderViewPtr D3D12RenderFactory::Make3DDepthStencilRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D12DepthStencilRenderView>(texture, array_index, first_slice, num_slices, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make1DUnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D12UnorderedAccessView>(texture, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make2DUnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D12UnorderedAccessView>(texture, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make2DUnorderedAccessView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D12UnorderedAccessView>(texture, array_index, face, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make2DUnorderedAccessView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return MakeSharedPtr<D3D12UnorderedAccessView>(texture, array_index, slice, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::MakeCubeUnorderedAccessView(Texture& texture, int array_index, int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D12UnorderedAccessView>(texture, array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make3DUnorderedAccessView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D12UnorderedAccessView>(texture, array_index, first_slice, num_slices, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::MakeGraphicsBufferUnorderedAccessView(GraphicsBuffer& gbuffer, ElementFormat pf)
	{
		return MakeSharedPtr<D3D12UnorderedAccessView>(gbuffer, pf);
	}

	ShaderObjectPtr D3D12RenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<D3D12ShaderObject>();
	}

	std::unique_ptr<RenderEngine> D3D12RenderFactory::DoMakeRenderEngine()
	{
		return MakeUniquePtr<D3D12RenderEngine>();
	}

	RenderStateObjectPtr D3D12RenderFactory::DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc,
		DepthStencilStateDesc const & dss_desc, BlendStateDesc const & bs_desc)
	{
		return MakeSharedPtr<D3D12RenderStateObject>(rs_desc, dss_desc, bs_desc);
	}

	SamplerStateObjectPtr D3D12RenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<D3D12SamplerStateObject>(desc);
	}

	void D3D12RenderFactory::DoSuspend()
	{
		// TODO
	}

	void D3D12RenderFactory::DoResume()
	{
		// TODO
	}
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::D3D12RenderFactory>();
	}
}
