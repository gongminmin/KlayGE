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

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		OGLTexture1D& texture_1d_;
		int level_;
	};

	typedef boost::shared_ptr<OGLTexture1DRenderView> OGLTexture1DRenderViewPtr;


	class OGLTexture2DRenderView : public OGLRenderView, boost::noncopyable
	{
	public:
		OGLTexture2DRenderView(Texture& texture_2d, int level);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		OGLTexture2D& texture_2d_;
		int level_;
	};

	typedef boost::shared_ptr<OGLTexture2DRenderView> OGLTexture2DRenderViewPtr;


	class OGLTexture3DRenderView : public OGLRenderView, boost::noncopyable
	{
	public:
		OGLTexture3DRenderView(Texture& texture_3d, uint32_t slice, int level);
		~OGLTexture3DRenderView();

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		void OnUnbind(FrameBuffer& fb, uint32_t att);

	private:
		void CopyToSlice(uint32_t att);

	private:
		OGLTexture3D& texture_3d_;
		uint32_t slice_;
		int level_;
		int copy_to_tex_;
		GLuint tex_2d_;
	};

	typedef boost::shared_ptr<OGLTexture3DRenderView> OGLTexture3DRenderViewPtr;


	class OGLTextureCubeRenderView : public OGLRenderView, boost::noncopyable
	{
	public:
		OGLTextureCubeRenderView(Texture& texture_cube, Texture::CubeFaces face, int level);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

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
		~OGLGraphicsBufferRenderView();

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		void OnUnbind(FrameBuffer& fb, uint32_t att);

	private:
		void CopyToGB(uint32_t att);

	private:
		GraphicsBuffer& gbuffer_;
	};

	typedef boost::shared_ptr<OGLGraphicsBufferRenderView> OGLGraphicsBufferRenderViewPtr;


	class OGLDepthStencilRenderView : public OGLRenderView, boost::noncopyable
	{
	public:
		OGLDepthStencilRenderView(uint32_t width, uint32_t height, PixelFormat pf, uint32_t multi_sample);
		~OGLDepthStencilRenderView();

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		uint32_t multi_sample_;
		GLuint rbo_;
	};

	typedef boost::shared_ptr<OGLDepthStencilRenderView> OGLDepthStencilRenderViewPtr;
}

#endif			// _OGLRENDERVIEW_HPP
