// D3D9RenderView.hpp
// KlayGE D3D9渲染视图类 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDERVIEW_HPP
#define _D3D9RENDERVIEW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/Texture.hpp>

#include <boost/noncopyable.hpp>

#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

namespace KlayGE
{
	class D3D9Texture1D;
	class D3D9Texture2D;
	class D3D9Texture3D;
	class D3D9TextureCube;
	class D3D9GraphicsBuffer;

	class D3D9RenderView : public RenderView, public D3D9Resource
	{
	public:
		virtual ~D3D9RenderView();

		void Clear(Color const & clr);
		void Clear(float depth);
		void Clear(int32_t stencil);
		void Clear(float depth, int32_t stencil);

		ID3D9SurfacePtr D3DRenderSurface() const
		{
			return surface_;
		}

	protected:
		ID3D9SurfacePtr surface_;
	};
	typedef boost::shared_ptr<D3D9RenderView> D3D9RenderViewPtr;

	class D3D9SurfaceRenderView : public D3D9RenderView, boost::noncopyable
	{
	public:
		explicit D3D9SurfaceRenderView(ID3D9SurfacePtr surf);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();
	};
	typedef boost::shared_ptr<D3D9SurfaceRenderView> D3D9SurfaceRenderViewPtr;

	class D3D9Texture1DRenderView : public D3D9RenderView, boost::noncopyable
	{
	public:
		D3D9Texture1DRenderView(Texture& texture_1d, int array_index, int level);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		D3D9Texture1D& texture_1d_;
		int level_;
	};
	typedef boost::shared_ptr<D3D9Texture1DRenderView> D3D9Texture1DRenderViewPtr;

	class D3D9Texture2DRenderView : public D3D9RenderView, boost::noncopyable
	{
	public:
		D3D9Texture2DRenderView(Texture& texture_2d, int array_index, int level);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		D3D9Texture2D& texture_2d_;
		int level_;
	};
	typedef boost::shared_ptr<D3D9Texture2DRenderView> D3D9Texture2DRenderViewPtr;
	
	class D3D9Texture3DRenderView : public D3D9RenderView, boost::noncopyable
	{
	public:
		D3D9Texture3DRenderView(Texture& texture_3d, int array_index, uint32_t slice, int level);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		void OnUnbind(FrameBuffer& fb, uint32_t att);

	private:
		ID3D9SurfacePtr CreateSurface(D3DPOOL pool);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		D3D9Texture3D& texture_3d_;
		uint32_t slice_;
		int level_;
	};
	typedef boost::shared_ptr<D3D9Texture3DRenderView> D3D9Texture3DRenderViewPtr;

	class D3D9TextureCubeRenderView : public D3D9RenderView, boost::noncopyable
	{
	public:
		D3D9TextureCubeRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		D3D9TextureCube& texture_cube_;
		Texture::CubeFaces face_;
		int level_;
	};
	typedef boost::shared_ptr<D3D9TextureCubeRenderView> D3D9TextureCubeRenderViewPtr;

	class D3D9GraphicsBufferRenderView : public D3D9RenderView, boost::noncopyable
	{
	public:
		D3D9GraphicsBufferRenderView(GraphicsBuffer& gb,
							uint32_t width, uint32_t height, ElementFormat pf);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		void OnUnbind(FrameBuffer& fb, uint32_t att);

	private:
		ID3D9SurfacePtr CreateGBSurface(D3DPOOL pool);
		void CopyToGB();

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		GraphicsBuffer& gbuffer_;
	};
	typedef boost::shared_ptr<D3D9GraphicsBufferRenderView> D3D9GraphicsBufferRenderViewPtr;

	class D3D9DepthStencilRenderView : public D3D9RenderView, boost::noncopyable
	{
	public:
		D3D9DepthStencilRenderView(uint32_t width, uint32_t height,
			ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);
		D3D9DepthStencilRenderView(Texture& texture, int array_index, int level);
		D3D9DepthStencilRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		ID3D9SurfacePtr CreateSurface();

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		D3DMULTISAMPLE_TYPE multi_sample_;
		uint32_t multi_sample_quality_;
	};
	typedef boost::shared_ptr<D3D9DepthStencilRenderView> D3D9DepthStencilRenderViewPtr;
}

#endif			// _D3D9RENDERVIEW_HPP
