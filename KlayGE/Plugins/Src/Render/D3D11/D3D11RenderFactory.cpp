// D3D11RenderFactory.cpp
// KlayGE D3D11渲染引擎抽象工厂 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Query.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>
#include <KlayGE/D3D11/D3D11Fence.hpp>

#include <KlayGE/D3D11/D3D11RenderFactory.hpp>

namespace KlayGE
{
	D3D11RenderFactory::D3D11RenderFactory() = default;

	std::wstring const & D3D11RenderFactory::Name() const
	{
		static std::wstring const name(L"Direct3D11 Render Factory");
		return name;
	}

	TexturePtr D3D11RenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D11Texture1D>(width, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}
	TexturePtr D3D11RenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D11Texture2D>(width, height, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}
	TexturePtr D3D11RenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D11Texture3D>(width, height, depth, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}
	TexturePtr D3D11RenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D11TextureCube>(size, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}

	FrameBufferPtr D3D11RenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<D3D11FrameBuffer>();
	}

	RenderLayoutPtr D3D11RenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<D3D11RenderLayout>();
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_VERTEX_BUFFER, size_in_byte, structure_byte_stride);
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_INDEX_BUFFER, size_in_byte, structure_byte_stride);
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_CONSTANT_BUFFER, size_in_byte, structure_byte_stride);
	}

	QueryPtr D3D11RenderFactory::MakeOcclusionQuery()
	{
		return MakeSharedPtr<D3D11OcclusionQuery>();
	}

	QueryPtr D3D11RenderFactory::MakeConditionalRender()
	{
		return MakeSharedPtr<D3D11ConditionalRender>();
	}

	QueryPtr D3D11RenderFactory::MakeTimerQuery()
	{
		return MakeSharedPtr<D3D11TimerQuery>();
	}

	QueryPtr D3D11RenderFactory::MakeSOStatisticsQuery()
	{
		return MakeSharedPtr<D3D11SOStatisticsQuery>();
	}

	FencePtr D3D11RenderFactory::MakeFence()
	{
		FencePtr ret;

		auto* d3d_device_5 = checked_cast<D3D11RenderEngine&>(*re_).D3DDevice5();
		if (d3d_device_5 != nullptr)
		{
			ret = MakeSharedPtr<D3D11_4Fence>();
		}
		else
		{
			ret = MakeSharedPtr<D3D11Fence>();
		}

		return ret;
	}

	ShaderResourceViewPtr D3D11RenderFactory::MakeTextureSrv(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t first_level, uint32_t num_levels)
	{
		return MakeSharedPtr<D3D11TextureShaderResourceView>(texture, pf, first_array_index, array_size, first_level, num_levels);
	}

	ShaderResourceViewPtr D3D11RenderFactory::MakeTexture2DSrv(
		TexturePtr const& texture, ElementFormat pf, int array_index, Texture::CubeFaces face, uint32_t first_level, uint32_t num_levels)
	{
		return MakeSharedPtr<D3D11CubeTextureFaceShaderResourceView>(texture, pf, array_index, face, first_level, num_levels);
	}

	ShaderResourceViewPtr D3D11RenderFactory::MakeBufferSrv(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		return MakeSharedPtr<D3D11BufferShaderResourceView>(gbuffer, pf, first_elem, num_elems);
	}

	RenderTargetViewPtr D3D11RenderFactory::Make1DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
		int level)
	{
		return MakeSharedPtr<D3D11Texture1D2DCubeRenderTargetView>(texture, pf, first_array_index, array_size, level);
	}

	RenderTargetViewPtr D3D11RenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
		int level)
	{
		return MakeSharedPtr<D3D11Texture1D2DCubeRenderTargetView>(texture, pf, first_array_index, array_size, level);
	}

	RenderTargetViewPtr D3D11RenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D11TextureCubeFaceRenderTargetView>(texture, pf, array_index, face, level);
	}

	RenderTargetViewPtr D3D11RenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
		int level)
	{
		return this->Make3DRtv(texture, pf, array_index, slice, 1, level);
	}

	RenderTargetViewPtr D3D11RenderFactory::Make3DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t first_slice,
		uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D11Texture3DRenderTargetView>(texture, pf, array_index, first_slice, num_slices, level);
	}

	RenderTargetViewPtr D3D11RenderFactory::MakeCubeRtv(TexturePtr const & texture, ElementFormat pf, int array_index, int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D11Texture1D2DCubeRenderTargetView>(texture, pf, array_index, array_size, level);
	}

	RenderTargetViewPtr D3D11RenderFactory::MakeBufferRtv(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		return MakeSharedPtr<D3D11BufferRenderTargetView>(gbuffer, pf, first_elem, num_elems);
	}

	DepthStencilViewPtr D3D11RenderFactory::Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
		uint32_t sample_quality)
	{
		return MakeSharedPtr<D3D11Texture1D2DCubeDepthStencilView>(width, height, pf, sample_count, sample_quality);
	}

	DepthStencilViewPtr D3D11RenderFactory::Make1DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<D3D11Texture1D2DCubeDepthStencilView>(texture, pf, first_array_index, array_size, level);
	}

	DepthStencilViewPtr D3D11RenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
		int level)
	{
		return MakeSharedPtr<D3D11Texture1D2DCubeDepthStencilView>(texture, pf, first_array_index, array_size, level);
	}

	DepthStencilViewPtr D3D11RenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D11TextureCubeFaceDepthStencilView>(texture, pf, array_index, face, level);
	}
	
	DepthStencilViewPtr D3D11RenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
		int level)
	{
		return this->Make3DDsv(texture, pf, array_index, slice, 1, level);
	}
	
	DepthStencilViewPtr D3D11RenderFactory::Make3DDsv(TexturePtr const & texture, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D11Texture3DDepthStencilView>(texture, pf, array_index, first_slice, num_slices, level);
	}

	DepthStencilViewPtr D3D11RenderFactory::MakeCubeDsv(TexturePtr const & texture, ElementFormat pf, int array_index,
		int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D11Texture1D2DCubeDepthStencilView>(texture, pf, array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make1DUav(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D11Texture1D2DCubeUnorderedAccessView>(texture, pf, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make2DUav(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D11Texture1D2DCubeUnorderedAccessView>(texture, pf, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D11TextureCubeFaceUnorderedAccessView>(texture, pf, array_index, face, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
		uint32_t slice, int level)
	{
		return this->Make3DUav(texture, pf, array_index, slice, 1, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make3DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D11Texture3DUnorderedAccessView>(texture, pf, array_index, first_slice, num_slices, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::MakeCubeUav(TexturePtr const & texture, ElementFormat pf, int array_index,
		int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D11Texture1D2DCubeUnorderedAccessView>(texture, pf, array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::MakeBufferUav(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		return MakeSharedPtr<D3D11BufferUnorderedAccessView>(gbuffer, pf, first_elem, num_elems);
	}

	ShaderObjectPtr D3D11RenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<D3D11ShaderObject>();
	}

	ShaderStageObjectPtr D3D11RenderFactory::MakeShaderStageObject(ShaderStage stage)
	{
		ShaderStageObjectPtr ret;
		switch (stage)
		{
		case ShaderStage::Vertex:
			ret = MakeSharedPtr<D3D11VertexShaderStageObject>();
			break;

		case ShaderStage::Pixel:
			ret = MakeSharedPtr<D3D11PixelShaderStageObject>();
			break;

		case ShaderStage::Geometry:
			ret = MakeSharedPtr<D3D11GeometryShaderStageObject>();
			break;

		case ShaderStage::Compute:
			ret = MakeSharedPtr<D3D11ComputeShaderStageObject>();
			break;

		case ShaderStage::Hull:
			ret = MakeSharedPtr<D3D11HullShaderStageObject>();
			break;

		case ShaderStage::Domain:
			ret = MakeSharedPtr<D3D11DomainShaderStageObject>();
			break;

		default:
			KFL_UNREACHABLE("Invalid shader stage");
		}
		return ret;
	}

	std::unique_ptr<RenderEngine> D3D11RenderFactory::DoMakeRenderEngine()
	{
		return MakeUniquePtr<D3D11RenderEngine>();
	}

	RenderStateObjectPtr D3D11RenderFactory::DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc,
		DepthStencilStateDesc const & dss_desc, BlendStateDesc const & bs_desc)
	{
		return MakeSharedPtr<D3D11RenderStateObject>(rs_desc, dss_desc, bs_desc);
	}

	SamplerStateObjectPtr D3D11RenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11SamplerStateObject>(desc);
	}

	void D3D11RenderFactory::DoSuspend()
	{
		// TODO
	}

	void D3D11RenderFactory::DoResume()
	{
		// TODO
	}
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::D3D11RenderFactory>();
	}
}
