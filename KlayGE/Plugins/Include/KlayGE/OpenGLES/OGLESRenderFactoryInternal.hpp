// OGLESRenderFactoryInternal.h
// KlayGE OpenGL ES��Ⱦ������ ͷ�ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERFACTORYINTERNAL_HPP
#define _OGLESRENDERFACTORYINTERNAL_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

namespace KlayGE
{
	class OGLESRenderFactory : public RenderFactory
	{
	public:
		OGLESRenderFactory();

		std::wstring const & Name() const;

		virtual TexturePtr MakeDelayCreationTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
		virtual TexturePtr MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
		virtual TexturePtr MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t array_size,
				uint32_t numMipMaps, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
		virtual TexturePtr MakeDelayCreationTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;

		FrameBufferPtr MakeFrameBuffer();

		RenderLayoutPtr MakeRenderLayout();
		virtual GraphicsBufferPtr MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt = EF_Unknown) override;
		virtual GraphicsBufferPtr MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt = EF_Unknown) override;
		virtual GraphicsBufferPtr MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt = EF_Unknown) override;

		QueryPtr MakeOcclusionQuery();
		QueryPtr MakeConditionalRender();
		QueryPtr MakeTimerQuery();

		virtual FencePtr MakeFence() override;

		RenderViewPtr Make1DRenderView(Texture& texture, int first_array_index, int array_size, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, int first_array_index, int array_size, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, int array_index, uint32_t slice, int level);
		RenderViewPtr MakeCubeRenderView(Texture& texture, int array_index, int level);
		RenderViewPtr Make3DRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height, ElementFormat pf);
		RenderViewPtr Make2DDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf,
			uint32_t sample_count, uint32_t sample_quality);
		RenderViewPtr Make1DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level);
		RenderViewPtr Make2DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level);
		RenderViewPtr Make2DDepthStencilRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level);
		RenderViewPtr Make2DDepthStencilRenderView(Texture& texture, int array_index, uint32_t slice, int level);
		RenderViewPtr MakeCubeDepthStencilRenderView(Texture& texture, int array_index, int level);
		RenderViewPtr Make3DDepthStencilRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level);

		UnorderedAccessViewPtr Make1DUnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level);
		UnorderedAccessViewPtr Make2DUnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level);
		UnorderedAccessViewPtr Make2DUnorderedAccessView(Texture& texture, int array_index, Texture::CubeFaces face, int level);
		UnorderedAccessViewPtr Make2DUnorderedAccessView(Texture& texture, int array_index, uint32_t slice, int level);
		UnorderedAccessViewPtr MakeCubeUnorderedAccessView(Texture& texture, int array_index, int level);
		UnorderedAccessViewPtr Make3DUnorderedAccessView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		UnorderedAccessViewPtr MakeGraphicsBufferUnorderedAccessView(GraphicsBuffer& gbuffer, ElementFormat pf);

		ShaderObjectPtr MakeShaderObject();

	private:
		virtual std::unique_ptr<RenderEngine> DoMakeRenderEngine() override;

		RasterizerStateObjectPtr DoMakeRasterizerStateObject(RasterizerStateDesc const & desc);
		DepthStencilStateObjectPtr DoMakeDepthStencilStateObject(DepthStencilStateDesc const & desc);
		BlendStateObjectPtr DoMakeBlendStateObject(BlendStateDesc const & desc);
		SamplerStateObjectPtr DoMakeSamplerStateObject(SamplerStateDesc const & desc);

		virtual void DoSuspend() override;
		virtual void DoResume() override;

	private:
		OGLESRenderFactory(OGLESRenderFactory const &);
		OGLESRenderFactory& operator=(OGLESRenderFactory const &);
	};
}

#endif			// _OGLESRENDERFACTORYINTERNAL_HPP
