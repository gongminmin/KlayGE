// OGLES2RenderView.hpp
// KlayGE OpenGL ES 2渲染视图类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLES2RENDERVIEW_HPP
#define _OGLES2RENDERVIEW_HPP

#pragma once

#include <KlayGE/RenderView.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/OpenGLES2/OGLES2Texture.hpp>

namespace KlayGE
{
	class OGLES2RenderView : public RenderView
	{
	public:
		OGLES2RenderView();
		virtual ~OGLES2RenderView();

		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		GLuint OGLTexture() const
		{
			return tex_;
		}

	protected:
		void DoClear(uint32_t flags, Color const & clr, float depth, int32_t stencil);

	protected:
		GLuint tex_;
		GLuint fbo_;
		GLuint index_;
	};

	typedef boost::shared_ptr<OGLES2RenderView> OGLES2RenderViewPtr;


	class OGLES2ScreenColorRenderView : public OGLES2RenderView, boost::noncopyable
	{
	public:
		OGLES2ScreenColorRenderView(uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);
	};

	typedef boost::shared_ptr<OGLES2ScreenColorRenderView> OGLES2ScreenColorRenderViewPtr;


	class OGLES2ScreenDepthStencilRenderView : public OGLES2RenderView, boost::noncopyable
	{
	public:
		OGLES2ScreenDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);
	};

	typedef boost::shared_ptr<OGLES2ScreenDepthStencilRenderView> OGLES2ScreenDepthStencilRenderViewPtr;


	class OGLES2Texture1DRenderView : public OGLES2RenderView, boost::noncopyable
	{
	public:
		OGLES2Texture1DRenderView(Texture& texture_1d, int array_index, int level);

		void ClearColor(Color const & clr);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		OGLES2Texture1D& texture_1d_;
		int array_index_;
		int level_;
	};

	typedef boost::shared_ptr<OGLES2Texture1DRenderView> OGLES2Texture1DRenderViewPtr;


	class OGLES2Texture2DRenderView : public OGLES2RenderView, boost::noncopyable
	{
	public:
		OGLES2Texture2DRenderView(Texture& texture_2d, int array_index, int level);

		void ClearColor(Color const & clr);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		OGLES2Texture2D& texture_2d_;
		int array_index_;
		int level_;
	};

	typedef boost::shared_ptr<OGLES2Texture2DRenderView> OGLES2Texture2DRenderViewPtr;


	class OGLES2Texture3DRenderView : public OGLES2RenderView, boost::noncopyable
	{
	public:
		OGLES2Texture3DRenderView(Texture& texture_3d, int array_index, uint32_t slice, int level);
		~OGLES2Texture3DRenderView();

		void ClearColor(Color const & clr);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		void OnUnbind(FrameBuffer& fb, uint32_t att);

	private:
		void CopyToSlice(uint32_t att);

	private:
		OGLES2Texture3D& texture_3d_;
		uint32_t slice_;
		int level_;
		int copy_to_tex_;
		GLuint tex_2d_;
	};

	typedef boost::shared_ptr<OGLES2Texture3DRenderView> OGLES2Texture3DRenderViewPtr;


	class OGLES2TextureCubeRenderView : public OGLES2RenderView, boost::noncopyable
	{
	public:
		OGLES2TextureCubeRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);

		void ClearColor(Color const & clr);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		OGLES2TextureCube& texture_cube_;
		Texture::CubeFaces face_;
		int level_;
	};

	typedef boost::shared_ptr<OGLES2Texture2DRenderView> OGLES2Texture2DRenderViewPtr;


	class OGLES2DepthStencilRenderView : public OGLES2RenderView, boost::noncopyable
	{
	public:
		OGLES2DepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);
		OGLES2DepthStencilRenderView(Texture& texture, int array_index, int level);
		~OGLES2DepthStencilRenderView();

		void ClearColor(Color const & clr);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		GLenum target_type_;
		int array_index_;
		int level_;
		uint32_t sample_count_, sample_quality_;
		GLuint rbo_;
	};

	typedef boost::shared_ptr<OGLES2DepthStencilRenderView> OGLES2DepthStencilRenderViewPtr;
}

#endif			// _OGLES2RENDERVIEW_HPP
