// D3D9FrameBuffer.hpp
// KlayGE D3D9渲染纹理类 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 改为FrameBuffer (2006.5.30)
//
// 2.4.0
// 去掉了OnLostDevice和OnResetDevice，改由texture管理 (2005.3.3)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9FRAMEBUFFER_HPP
#define _D3D9FRAMEBUFFER_HPP

#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

namespace KlayGE
{
	class D3D9FrameBuffer : public FrameBuffer, public D3D9Resource
	{
	public:
		D3D9FrameBuffer();
		virtual ~D3D9FrameBuffer();

		ID3D9SurfacePtr D3DRenderSurface(uint32_t n) const;
		ID3D9SurfacePtr D3DRenderZBuffer() const;

		virtual std::wstring const & Description() const;

		virtual void OnBind();

		bool RequiresFlipping() const
		{
			return true;
		}

		void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();
	};

	typedef boost::shared_ptr<D3D9FrameBuffer> D3D9FrameBufferPtr;
}

#endif			// _D3D9RENDERTEXTURE_HPP
