// RenderFactory.hpp
// KlayGE 渲染工厂类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 支持Cube和3D RenderView (2010.4.24)
//
// 3.8.0
// 增加了MakeSamplerStateObject (2008.9.21)
// 增加了MakeConditionalRender (2008.10.11)
// 支持depth texture (2008.11.6)
//
// 3.0.0
// 增加了MakeVertexBuffer (2005.9.7)
//
// 2.8.0
// 增加了LoadEffect (2005.7.25)
//
// 2.4.0
// 增加了1D/2D/3D/cube的支持 (2005.3.8)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERFACTORY_HPP
#define _RENDERFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KFL/ArrayRef.hpp>

#include <string>
#include <unordered_map>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderFactory : boost::noncopyable
	{
	public:
		virtual ~RenderFactory();

		virtual std::wstring const & Name() const = 0;

		RenderEngine& RenderEngineInstance();

		void Suspend();
		void Resume();

		virtual TexturePtr MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;
		virtual TexturePtr MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;
		virtual TexturePtr MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;
		virtual TexturePtr MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;

		TexturePtr MakeTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
			ArrayRef<ElementInitData> init_data = {}, float4 const * clear_value_hint = nullptr);
		TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
			ArrayRef<ElementInitData> init_data = {}, float4 const * clear_value_hint = nullptr);
		TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
			ArrayRef<ElementInitData> init_data = {}, float4 const * clear_value_hint = nullptr);
		TexturePtr MakeTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
			ArrayRef<ElementInitData> init_data = {}, float4 const * clear_value_hint = nullptr);

		virtual FrameBufferPtr MakeFrameBuffer() = 0;

		virtual RenderLayoutPtr MakeRenderLayout() = 0;

		virtual GraphicsBufferPtr MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte,
			uint32_t structure_byte_stride = 0) = 0;
		virtual GraphicsBufferPtr MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte,
			uint32_t structure_byte_stride = 0) = 0;
		virtual GraphicsBufferPtr MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte,
			uint32_t structure_byte_stride = 0) = 0;

		GraphicsBufferPtr MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
			uint32_t structure_byte_stride = 0);
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
			uint32_t structure_byte_stride = 0);
		GraphicsBufferPtr MakeConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
			uint32_t structure_byte_stride = 0);

		virtual QueryPtr MakeOcclusionQuery() = 0;
		virtual QueryPtr MakeConditionalRender() = 0;
		virtual QueryPtr MakeTimerQuery() = 0;
		virtual QueryPtr MakeSOStatisticsQuery() = 0;

		virtual FencePtr MakeFence() = 0;

		ShaderResourceViewPtr MakeTextureSrv(TexturePtr const & texture, uint32_t first_array_index, uint32_t array_size,
			uint32_t first_level, uint32_t num_levels);
		virtual ShaderResourceViewPtr MakeTextureSrv(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index,
			uint32_t array_size, uint32_t first_level, uint32_t num_levels) = 0;
		virtual ShaderResourceViewPtr MakeBufferSrv(GraphicsBufferPtr const & gbuffer, ElementFormat pf, uint32_t first_elem,
			uint32_t num_elems) = 0;
		ShaderResourceViewPtr MakeTextureSrv(TexturePtr const & texture);
		ShaderResourceViewPtr MakeTextureSrv(TexturePtr const & texture, ElementFormat pf);
		ShaderResourceViewPtr MakeBufferSrv(GraphicsBufferPtr const & gbuffer, ElementFormat pf);

		RenderTargetViewPtr Make1DRtv(TexturePtr const & texture, int first_array_index, int array_size, int level);
		virtual RenderTargetViewPtr Make1DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) = 0;
		RenderTargetViewPtr Make2DRtv(TexturePtr const & texture, int first_array_index, int array_size, int level);
		virtual RenderTargetViewPtr Make2DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) = 0;
		RenderTargetViewPtr Make2DRtv(TexturePtr const & texture, int array_index, Texture::CubeFaces face, int level);
		virtual RenderTargetViewPtr Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level) = 0;
		RenderTargetViewPtr Make2DRtv(TexturePtr const & texture, int array_index, uint32_t slice, int level);
		virtual RenderTargetViewPtr Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice, int level) = 0;
		RenderTargetViewPtr Make3DRtv(TexturePtr const & texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		virtual RenderTargetViewPtr Make3DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level) = 0;
		RenderTargetViewPtr MakeCubeRtv(TexturePtr const & texture, int array_index, int level);
		virtual RenderTargetViewPtr MakeCubeRtv(TexturePtr const & texture, ElementFormat pf, int array_index, int level) = 0;
		virtual RenderTargetViewPtr MakeBufferRtv(GraphicsBufferPtr const & gbuffer, ElementFormat pf, uint32_t first_elem,
			uint32_t num_elems) = 0;
		RenderTargetViewPtr MakeTextureRtv(TexturePtr const & texture, uint32_t level);
		RenderTargetViewPtr MakeBufferRtv(GraphicsBufferPtr const & gbuffer, ElementFormat pf);

		virtual DepthStencilViewPtr Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
			uint32_t sample_quality) = 0;
		DepthStencilViewPtr Make1DDsv(TexturePtr const & texture, int first_array_index, int array_size, int level);
		virtual DepthStencilViewPtr Make1DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) = 0;
		DepthStencilViewPtr Make2DDsv(TexturePtr const & texture, int first_array_index, int array_size, int level);
		virtual DepthStencilViewPtr Make2DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) = 0;
		DepthStencilViewPtr Make2DDsv(TexturePtr const & texture, int array_index, Texture::CubeFaces face, int level);
		virtual DepthStencilViewPtr Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level) = 0;
		DepthStencilViewPtr Make2DDsv(TexturePtr const & texture, int array_index, uint32_t slice, int level);
		virtual DepthStencilViewPtr Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice, int level) = 0;
		DepthStencilViewPtr Make3DDsv(TexturePtr const & texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		virtual DepthStencilViewPtr Make3DDsv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level) = 0;
		DepthStencilViewPtr MakeCubeDsv(TexturePtr const & texture, int array_index, int level);
		virtual DepthStencilViewPtr MakeCubeDsv(TexturePtr const & texture, ElementFormat pf, int array_index, int level) = 0;
		DepthStencilViewPtr MakeTextureDsv(TexturePtr const & texture, uint32_t level);

		UnorderedAccessViewPtr Make1DUav(TexturePtr const & texture, int first_array_index, int array_size, int level);
		virtual UnorderedAccessViewPtr Make1DUav(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) = 0;
		UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, int first_array_index, int array_size, int level);
		virtual UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
			int level) = 0;
		UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, int array_index, Texture::CubeFaces face, int level);
		virtual UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level) = 0;
		UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, int array_index, uint32_t slice, int level);
		virtual UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
			int level) = 0;
		UnorderedAccessViewPtr Make3DUav(TexturePtr const & texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		virtual UnorderedAccessViewPtr Make3DUav(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level) = 0;
		UnorderedAccessViewPtr MakeCubeUav(TexturePtr const & texture, int array_index, int level);
		virtual UnorderedAccessViewPtr MakeCubeUav(TexturePtr const & texture, ElementFormat pf, int array_index, int level) = 0;
		virtual UnorderedAccessViewPtr MakeBufferUav(GraphicsBufferPtr const & gbuffer, ElementFormat pf, uint32_t first_elem,
			uint32_t num_elems) = 0;
		UnorderedAccessViewPtr MakeTextureUav(TexturePtr const & texture, uint32_t level);
		UnorderedAccessViewPtr MakeBufferUav(GraphicsBufferPtr const & gbuffer, ElementFormat pf);

		RenderStateObjectPtr MakeRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc);
		SamplerStateObjectPtr MakeSamplerStateObject(SamplerStateDesc const & desc);
		virtual ShaderObjectPtr MakeShaderObject() = 0;
		virtual ShaderStageObjectPtr MakeShaderStageObject(ShaderStage stage) = 0;

	private:
		virtual std::unique_ptr<RenderEngine> DoMakeRenderEngine() = 0;

		virtual RenderStateObjectPtr DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc) = 0;
		virtual SamplerStateObjectPtr DoMakeSamplerStateObject(SamplerStateDesc const & desc) = 0;

		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

	protected:
		std::unique_ptr<RenderEngine> re_;

		std::unordered_map<size_t, RenderStateObjectPtr> rs_pool_;
		std::unordered_map<size_t, SamplerStateObjectPtr> ss_pool_;
	};
}

#endif			// _RENDERFACTORY_HPP
