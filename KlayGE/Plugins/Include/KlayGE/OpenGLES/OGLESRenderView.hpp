// OGLESRenderView.hpp
// KlayGE OpenGL ES渲染视图类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERVIEW_HPP
#define _OGLESRENDERVIEW_HPP

#pragma once

#include <KlayGE/RenderView.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/OpenGLES/OGLESTexture.hpp>

namespace KlayGE
{
	class OGLESShaderResourceView : public ShaderResourceView
	{
	public:
		virtual void RetrieveGLTargetTexture(GLuint& target, GLuint& tex) const = 0;

	protected:
		mutable GLuint gl_target_;
		mutable GLuint gl_tex_;
	};

	class OGLESTextureShaderResourceView final : public OGLESShaderResourceView
	{
	public:
		explicit OGLESTextureShaderResourceView(TexturePtr const & texture);

		void RetrieveGLTargetTexture(GLuint& target, GLuint& tex) const override;
	};

	class OGLESBufferShaderResourceView final : public OGLESShaderResourceView
	{
	public:
		OGLESBufferShaderResourceView(GraphicsBufferPtr const & gbuffer, ElementFormat pf);

		void RetrieveGLTargetTexture(GLuint& target, GLuint& tex) const override;
	};

	class OGLESRenderTargetView : public RenderTargetView
	{
	public:
		virtual GLuint RetrieveGLTexture() const;

	protected:
		void DoClearColor(Color const & clr);
		void DoDiscardColor();

	protected:
		mutable GLuint gl_tex_ = 0;
		GLuint gl_fbo_ = 0;
		GLuint index_ = 0;
	};
	typedef std::shared_ptr<OGLESRenderTargetView> OGLESRenderTargetViewPtr;

	class OGLESDepthStencilView : public DepthStencilView
	{
	public:
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		virtual GLuint RetrieveGLTexture() const;

	protected:
		void DoClearDepthStencil(uint32_t flags, float depth, int32_t stencil);
		void DoDiscardDepthStencil();

	protected:
		mutable GLuint gl_tex_ = 0;
		GLuint gl_fbo_ = 0;
	};
	typedef std::shared_ptr<OGLESDepthStencilView> OGLESDepthStencilViewPtr;

	class OGLESScreenRenderTargetView final : public OGLESRenderTargetView
	{
	public:
		OGLESScreenRenderTargetView(uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
		void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;

		GLuint RetrieveGLTexture() const override;
	};


	class OGLESScreenDepthStencilView final : public OGLESDepthStencilView
	{
	public:
		OGLESScreenDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf);

		void Discard() override;

		void OnAttached(FrameBuffer& fb) override;
		void OnDetached(FrameBuffer& fb) override;

		GLuint RetrieveGLTexture() const override;
	};


	class OGLESTexture1DRenderTargetView final : public OGLESRenderTargetView
	{
	public:
		OGLESTexture1DRenderTargetView(TexturePtr const & texture_1d, ElementFormat pf, int array_index, int array_size, int level);

		void ClearColor(Color const & clr) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
		void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;

	private:
		int array_index_;
		int array_size_;
		int level_;
	};


	class OGLESTexture2DRenderTargetView final : public OGLESRenderTargetView
	{
	public:
		OGLESTexture2DRenderTargetView(TexturePtr const & texture_2d, ElementFormat pf, int array_index, int array_size, int level);

		void ClearColor(Color const & clr) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
		void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;

	private:
		int array_index_;
		int array_size_;
		int level_;
	};


	class OGLESTexture3DRenderTargetView final : public OGLESRenderTargetView
	{
	public:
		OGLESTexture3DRenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t slice, int level);
		~OGLESTexture3DRenderTargetView();

		void ClearColor(Color const & clr) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
		void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;

		void OnUnbind(FrameBuffer& fb, FrameBuffer::Attachment att);

	private:
		void CopyToSlice(FrameBuffer::Attachment att);

	private:
		uint32_t slice_;
		int level_;
		int copy_to_tex_;
		GLuint gl_tex_2d_;
	};


	class OGLESTextureCubeRenderTargetView final : public OGLESRenderTargetView
	{
	public:
		OGLESTextureCubeRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level);
		OGLESTextureCubeRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, int level);

		void ClearColor(Color const & clr) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
		void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;

	private:
		Texture::CubeFaces face_;
		int level_;
	};


	class OGLESTextureDepthStencilView final : public OGLESDepthStencilView
	{
	public:
		OGLESTextureDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);
		OGLESTextureDepthStencilView(TexturePtr const & texture, ElementFormat pf, int array_index, int array_size, int level);
		~OGLESTextureDepthStencilView();

		void ClearColor(Color const & clr);

		void Discard() override;

		void OnAttached(FrameBuffer& fb) override;
		void OnDetached(FrameBuffer& fb) override;

	private:
		GLenum target_type_;
		int array_index_;
		int array_size_;
		int level_;
		uint32_t sample_count_, sample_quality_;
		GLuint gl_rbos_[2];
	};


	class OGLESTextureCubeFaceDepthStencilView final : public OGLESDepthStencilView
	{
	public:
		OGLESTextureCubeFaceDepthStencilView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level);

		void ClearColor(Color const & clr);

		void Discard() override;

		void OnAttached(FrameBuffer& fb) override;
		void OnDetached(FrameBuffer& fb) override;

	private:
		Texture::CubeFaces face_;
		int level_;
	};


#if defined(KLAYGE_PLATFORM_IOS)
	class OGLESEAGLRenderView final : public OGLESRenderTargetView
	{
	public:
		explicit OGLESEAGLRenderView(ElementFormat pf);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att);
		void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att);

		void BindRenderBuffer();

		GLuint RetrieveGLTexture() const override;

	protected:
		GLuint gl_rf_;
	};
#endif
}

#endif			// _OGLESRENDERVIEW_HPP
