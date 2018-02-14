/**
 * @file NullRenderFactory.cpp
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

#include <KlayGE/NullRender/NullRenderEngine.hpp>
#include <KlayGE/NullRender/NullRenderStateObject.hpp>
#include <KlayGE/NullRender/NullShaderObject.hpp>
#include <KlayGE/NullRender/NullTexture.hpp>

#include <KlayGE/NullRender/NullRenderFactory.hpp>

namespace KlayGE
{
	NullRenderFactory::NullRenderFactory()
	{
	}

	std::wstring const & NullRenderFactory::Name() const
	{
		static std::wstring const name(L"Null Render Factory");
		return name;
	}

	TexturePtr NullRenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		KFL_UNUSED(width);
		KFL_UNUSED(num_mip_maps);
		KFL_UNUSED(array_size);
		KFL_UNUSED(format);
		return MakeSharedPtr<NullTexture>(Texture::TT_1D, sample_count, sample_quality, access_hint);
	}
	TexturePtr NullRenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(num_mip_maps);
		KFL_UNUSED(array_size);
		KFL_UNUSED(format);
		return MakeSharedPtr<NullTexture>(Texture::TT_2D, sample_count, sample_quality, access_hint);
	}
	TexturePtr NullRenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(depth);
		KFL_UNUSED(num_mip_maps);
		KFL_UNUSED(array_size);
		KFL_UNUSED(format);
		return MakeSharedPtr<NullTexture>(Texture::TT_3D, sample_count, sample_quality, access_hint);
	}
	TexturePtr NullRenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		KFL_UNUSED(size);
		KFL_UNUSED(num_mip_maps);
		KFL_UNUSED(array_size);
		KFL_UNUSED(format);
		return MakeSharedPtr<NullTexture>(Texture::TT_Cube, sample_count, sample_quality, access_hint);
	}

	FrameBufferPtr NullRenderFactory::MakeFrameBuffer()
	{
		return FrameBufferPtr();
	}

	RenderLayoutPtr NullRenderFactory::MakeRenderLayout()
	{
		return RenderLayoutPtr();
	}

	GraphicsBufferPtr NullRenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		KFL_UNUSED(usage);
		KFL_UNUSED(access_hint);
		KFL_UNUSED(size_in_byte);
		KFL_UNUSED(fmt);
		return GraphicsBufferPtr();
	}

	GraphicsBufferPtr NullRenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		KFL_UNUSED(usage);
		KFL_UNUSED(access_hint);
		KFL_UNUSED(size_in_byte);
		KFL_UNUSED(fmt);
		return GraphicsBufferPtr();
	}

	GraphicsBufferPtr NullRenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		KFL_UNUSED(usage);
		KFL_UNUSED(access_hint);
		KFL_UNUSED(size_in_byte);
		KFL_UNUSED(fmt);
		return GraphicsBufferPtr();
	}

	QueryPtr NullRenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr();
	}

	QueryPtr NullRenderFactory::MakeConditionalRender()
	{
		return QueryPtr();
	}

	QueryPtr NullRenderFactory::MakeTimerQuery()
	{
		return QueryPtr();
	}

	QueryPtr NullRenderFactory::MakeSOStatisticsQuery()
	{
		return QueryPtr();
	}

	FencePtr NullRenderFactory::MakeFence()
	{
		return FencePtr();
	}

	RenderViewPtr NullRenderFactory::Make1DRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::Make2DRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::Make2DRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::Make2DRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(slice);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::MakeCubeRenderView(Texture& texture, int array_index, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::Make3DRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(first_slice);
		KFL_UNUSED(num_slices);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer,
		uint32_t width, uint32_t height, ElementFormat pf)
	{
		KFL_UNUSED(gbuffer);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(pf);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::Make2DDepthStencilRenderView(uint32_t width, uint32_t height,
		ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(pf);
		KFL_UNUSED(sample_count);
		KFL_UNUSED(sample_quality);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::Make1DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::Make2DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}
	
	RenderViewPtr NullRenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(slice);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	RenderViewPtr NullRenderFactory::MakeCubeDepthStencilRenderView(Texture& texture, int array_index, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}
	
	RenderViewPtr NullRenderFactory::Make3DDepthStencilRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(first_slice);
		KFL_UNUSED(num_slices);
		KFL_UNUSED(level);
		return RenderViewPtr();
	}

	UnorderedAccessViewPtr NullRenderFactory::Make1DUnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr NullRenderFactory::Make2DUnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr NullRenderFactory::Make2DUnorderedAccessView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr NullRenderFactory::Make2DUnorderedAccessView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(slice);
		KFL_UNUSED(level);
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr NullRenderFactory::MakeCubeUnorderedAccessView(Texture& texture, int array_index, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr NullRenderFactory::Make3DUnorderedAccessView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		KFL_UNUSED(texture);
		KFL_UNUSED(array_index);
		KFL_UNUSED(first_slice);
		KFL_UNUSED(num_slices);
		KFL_UNUSED(level);
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr NullRenderFactory::MakeGraphicsBufferUnorderedAccessView(GraphicsBuffer& gbuffer, ElementFormat pf)
	{
		KFL_UNUSED(gbuffer);
		KFL_UNUSED(pf);
		return UnorderedAccessViewPtr();
	}

	ShaderObjectPtr NullRenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<NullShaderObject>();
	}

	std::unique_ptr<RenderEngine> NullRenderFactory::DoMakeRenderEngine()
	{
		return MakeUniquePtr<NullRenderEngine>();
	}

	RenderStateObjectPtr NullRenderFactory::DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc,
		DepthStencilStateDesc const & dss_desc, BlendStateDesc const & bs_desc)
	{
		return MakeSharedPtr<NullRenderStateObject>(rs_desc, dss_desc, bs_desc);
	}

	SamplerStateObjectPtr NullRenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<NullSamplerStateObject>(desc);
	}

	void NullRenderFactory::DoSuspend()
	{
	}

	void NullRenderFactory::DoResume()
	{
	}
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::NullRenderFactory>();
	}
}
