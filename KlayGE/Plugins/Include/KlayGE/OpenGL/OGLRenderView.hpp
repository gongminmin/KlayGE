// OGLRenderView.hpp
// KlayGE OpenGL渲染视图类 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERVIEW_HPP
#define _OGLRENDERVIEW_HPP

#include <KlayGE/RenderView.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/OpenGL/OGLTexture.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLRenderView : public RenderView
	{
	public:
		virtual ~OGLRenderView();

		GLuint OGLTexture() const
		{
			return tex_;
		}

	protected:
		GLuint tex_;
	};

	typedef boost::shared_ptr<OGLRenderView> OGLRenderViewPtr;


	class OGLTexture1DRenderView : public OGLRenderView, boost::noncopyable
	{
	public:
		OGLTexture1DRenderView(Texture& texture_1d, int level);

		void OnAttached(FrameBuffer& fb, uint32_t n);
		void OnDetached(FrameBuffer& fb, uint32_t n);

	private:
		OGLTexture1D& texture_1d_;
		int level_;
	};

	typedef boost::shared_ptr<OGLTexture1DRenderView> OGLTexture1DRenderViewPtr;


	class OGLTexture2DRenderView : public OGLRenderView, boost::noncopyable
	{
	public:
		OGLTexture2DRenderView(Texture& texture_2d, int level);

		void OnAttached(FrameBuffer& fb, uint32_t n);
		void OnDetached(FrameBuffer& fb, uint32_t n);

	private:
		OGLTexture2D& texture_2d_;
		int level_;
	};

	typedef boost::shared_ptr<OGLTexture2DRenderView> OGLTexture2DRenderViewPtr;


	class OGLTextureCubeRenderView : public OGLRenderView, boost::noncopyable
	{
	public:
		OGLTextureCubeRenderView(Texture& texture_cube, Texture::CubeFaces face, int level);

		void OnAttached(FrameBuffer& fb, uint32_t n);
		void OnDetached(FrameBuffer& fb, uint32_t n);

	private:
		OGLTextureCube& texture_cube_;
		Texture::CubeFaces face_;
		int level_;
	};

	typedef boost::shared_ptr<OGLTexture2DRenderView> OGLTexture2DRenderViewPtr;


	class OGLGraphicsBufferRenderView : public OGLRenderView, boost::noncopyable
	{
	public:
		OGLGraphicsBufferRenderView(GraphicsBuffer& gb,
							uint32_t width, uint32_t height, PixelFormat pf);

		void OnAttached(FrameBuffer& fb, uint32_t n);
		void OnDetached(FrameBuffer& fb, uint32_t n);

	private:
		GraphicsBuffer& gbuffer_;
		PixelFormat pf_;
	};

	typedef boost::shared_ptr<OGLGraphicsBufferRenderView> OGLGraphicsBufferRenderViewPtr;
}

#endif			// _OGLRENDERVIEW_HPP
