// D3D11RenderFactory.hpp
// KlayGE D3D11��Ⱦ������󹤳� ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef KLAYGE_PLUGINS_D3D11_RENDER_FACTORY_HPP
#define KLAYGE_PLUGINS_D3D11_RENDER_FACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

namespace KlayGE
{
	class D3D11RenderFactory final : public RenderFactory
	{
	public:
		D3D11RenderFactory();

		std::wstring const & Name() const override;

		TexturePtr MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
		TexturePtr MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
		TexturePtr MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
		TexturePtr MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;

		FrameBufferPtr MakeFrameBuffer() override;

		RenderLayoutPtr MakeRenderLayout() override;

		GraphicsBufferPtr MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride = 0) override;
		GraphicsBufferPtr MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride = 0) override;
		GraphicsBufferPtr MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride = 0) override;

		QueryPtr MakeOcclusionQuery() override;
		QueryPtr MakeConditionalRender() override;
		QueryPtr MakeTimerQuery() override;
		QueryPtr MakeSOStatisticsQuery() override;

		virtual FencePtr MakeFence() override;

		ShaderResourceViewPtr MakeTextureSrv(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
			uint32_t first_level, uint32_t num_levels) override;
		ShaderResourceViewPtr MakeTexture2DSrv(TexturePtr const& texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
			uint32_t first_level, uint32_t num_levels) override;
		ShaderResourceViewPtr MakeBufferSrv(GraphicsBufferPtr const & gbuffer, ElementFormat pf, uint32_t first_elem,
			uint32_t num_elems) override;

		RenderTargetViewPtr Make1DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) override;
		RenderTargetViewPtr Make2DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) override;
		RenderTargetViewPtr Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level) override;
		RenderTargetViewPtr Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
			int level) override;
		RenderTargetViewPtr Make3DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level) override;
		RenderTargetViewPtr MakeCubeRtv(TexturePtr const & texture, ElementFormat pf, int array_index, int level) override;
		RenderTargetViewPtr MakeBufferRtv(GraphicsBufferPtr const & gbuffer, ElementFormat pf, uint32_t first_elem,
			uint32_t num_elems) override;

		DepthStencilViewPtr Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf,
			uint32_t sample_count, uint32_t sample_quality) override;
		DepthStencilViewPtr Make1DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) override;
		DepthStencilViewPtr Make2DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) override;
		DepthStencilViewPtr Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level) override;
		DepthStencilViewPtr Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
			int level) override;
		DepthStencilViewPtr Make3DDsv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level) override;
		DepthStencilViewPtr MakeCubeDsv(TexturePtr const & texture, ElementFormat pf, int array_index, int level) override;

		UnorderedAccessViewPtr Make1DUav(TexturePtr const & texture, ElementFormat pf, int first_array_index,
			int array_size, int level) override;
		UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, ElementFormat pf, int first_array_index,
			int array_size, int level) override;
		UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
			Texture::CubeFaces face, int level) override;
		UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
			int level) override;
		UnorderedAccessViewPtr Make3DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
			uint32_t first_slice, uint32_t num_slices, int level) override;
		UnorderedAccessViewPtr MakeCubeUav(TexturePtr const & texture, ElementFormat pf, int array_index,
			int level) override;
		UnorderedAccessViewPtr MakeBufferUav(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
			uint32_t first_elem, uint32_t num_elems) override;

		ShaderObjectPtr MakeShaderObject() override;
		ShaderStageObjectPtr MakeShaderStageObject(ShaderStage stage) override;

	private:
		std::unique_ptr<RenderEngine> DoMakeRenderEngine() override;

		RenderStateObjectPtr DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc) override;
		SamplerStateObjectPtr DoMakeSamplerStateObject(SamplerStateDesc const & desc) override;

		void DoSuspend() override;
		void DoResume() override;

	private:
		D3D11RenderFactory(D3D11RenderFactory const & rhs);
		D3D11RenderFactory& operator=(D3D11RenderFactory const & rhs);
	};
}

#endif			// KLAYGE_PLUGINS_D3D11_RENDER_FACTORY_HPP
