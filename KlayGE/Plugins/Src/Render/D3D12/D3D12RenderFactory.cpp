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
#include <KFL/ErrorHandling.hpp>
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
	D3D12RenderFactory::D3D12RenderFactory() = default;

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
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint, size_in_byte, structure_byte_stride);
	}

	GraphicsBufferPtr D3D12RenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint, size_in_byte, structure_byte_stride);
	}

	GraphicsBufferPtr D3D12RenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint, size_in_byte, structure_byte_stride);
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

	ShaderResourceViewPtr D3D12RenderFactory::MakeTextureSrv(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t first_level, uint32_t num_levels)
	{
		return MakeSharedPtr<D3D12TextureShaderResourceView>(texture, pf, first_array_index, array_size, first_level, num_levels);
	}

	ShaderResourceViewPtr D3D12RenderFactory::MakeTexture2DSrv(
		TexturePtr const& texture, ElementFormat pf, int array_index, Texture::CubeFaces face, uint32_t first_level, uint32_t num_levels)
	{
		return MakeSharedPtr<D3D12TextureShaderResourceView>(texture, pf, array_index, face, first_level, num_levels);
	}

	ShaderResourceViewPtr D3D12RenderFactory::MakeBufferSrv(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		return MakeSharedPtr<D3D12BufferShaderResourceView>(gbuffer, pf, first_elem, num_elems);
	}

	RenderTargetViewPtr D3D12RenderFactory::Make1DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<D3D12Texture1D2DCubeRenderTargetView>(texture, pf, first_array_index, array_size, level);
	}

	RenderTargetViewPtr D3D12RenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<D3D12Texture1D2DCubeRenderTargetView>(texture, pf, first_array_index, array_size, level);
	}

	RenderTargetViewPtr D3D12RenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D12TextureCubeFaceRenderTargetView>(texture, pf, array_index, face, level);
	}

	RenderTargetViewPtr D3D12RenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
		int level)
	{
		return this->Make3DRtv(texture, pf, array_index, slice, 1, level);
	}

	RenderTargetViewPtr D3D12RenderFactory::Make3DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t first_slice,
		uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D12Texture3DRenderTargetView>(texture, pf, array_index, first_slice, num_slices, level);
	}

	RenderTargetViewPtr D3D12RenderFactory::MakeCubeRtv(TexturePtr const & texture, ElementFormat pf, int array_index, int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D12Texture1D2DCubeRenderTargetView>(texture, pf, array_index, array_size, level);
	}

	RenderTargetViewPtr D3D12RenderFactory::MakeBufferRtv(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		return MakeSharedPtr<D3D12BufferRenderTargetView>(gbuffer, pf, first_elem, num_elems);
	}

	DepthStencilViewPtr D3D12RenderFactory::Make2DDsv(uint32_t width, uint32_t height,
		ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		return MakeSharedPtr<D3D12Texture1D2DCubeDepthStencilView>(width, height, pf, sample_count, sample_quality);
	}

	DepthStencilViewPtr D3D12RenderFactory::Make1DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<D3D12Texture1D2DCubeDepthStencilView>(texture, pf, first_array_index, array_size, level);
	}

	DepthStencilViewPtr D3D12RenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<D3D12Texture1D2DCubeDepthStencilView>(texture, pf, first_array_index, array_size, level);
	}

	DepthStencilViewPtr D3D12RenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D12TextureCubeFaceDepthStencilView>(texture, pf, array_index, face, level);
	}
	
	DepthStencilViewPtr D3D12RenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
		int level)
	{
		return this->Make3DDsv(texture, pf, array_index, slice, 1, level);
	}
	
	DepthStencilViewPtr D3D12RenderFactory::Make3DDsv(TexturePtr const & texture, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D12Texture3DDepthStencilView>(texture, pf, array_index, first_slice, num_slices, level);
	}

	DepthStencilViewPtr D3D12RenderFactory::MakeCubeDsv(TexturePtr const & texture, ElementFormat pf, int array_index, int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D12Texture1D2DCubeDepthStencilView>(texture, pf, array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make1DUav(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D12Texture1D2DCubeUnorderedAccessView>(texture, pf, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make2DUav(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D12Texture1D2DCubeUnorderedAccessView>(texture, pf, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D12TextureCubeFaceUnorderedAccessView>(texture, pf, array_index, face, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
		uint32_t slice, int level)
	{
		return this->Make3DUav(texture, pf, array_index, slice, 1, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::Make3DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D12Texture3DUnorderedAccessView>(texture, pf, array_index, first_slice, num_slices, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::MakeCubeUav(TexturePtr const & texture, ElementFormat pf, int array_index,
		int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D12Texture1D2DCubeUnorderedAccessView>(texture, pf, array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D12RenderFactory::MakeBufferUav(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		return MakeSharedPtr<D3D12BufferUnorderedAccessView>(gbuffer, pf, first_elem, num_elems);
	}

	ShaderObjectPtr D3D12RenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<D3D12ShaderObject>();
	}

	ShaderStageObjectPtr D3D12RenderFactory::MakeShaderStageObject(ShaderStage stage)
	{
		ShaderStageObjectPtr ret;
		switch (stage)
		{
		case ShaderStage::Vertex:
			ret = MakeSharedPtr<D3D12VertexShaderStageObject>();
			break;

		case ShaderStage::Pixel:
			ret = MakeSharedPtr<D3D12PixelShaderStageObject>();
			break;

		case ShaderStage::Geometry:
			ret = MakeSharedPtr<D3D12GeometryShaderStageObject>();
			break;

		case ShaderStage::Compute:
			ret = MakeSharedPtr<D3D12ComputeShaderStageObject>();
			break;

		case ShaderStage::Hull:
			ret = MakeSharedPtr<D3D12HullShaderStageObject>();
			break;

		case ShaderStage::Domain:
			ret = MakeSharedPtr<D3D12DomainShaderStageObject>();
			break;

		default:
			KFL_UNREACHABLE("Invalid shader stage");
		}
		return ret;
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
