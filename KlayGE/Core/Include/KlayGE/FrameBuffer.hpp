// FrameBuffer.hpp
// KlayGE 渲染到纹理类 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 改为FrameBuffer (2006.5.30)
//
// 2.8.0
// 去掉了GetTexture (2005.7.19)
//
// 2.4.0
// 增加了IsTexture和SwapBuffers (2005.3.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _FRAMEBUFFER_HPP
#define _FRAMEBUFFER_HPP

#include <KlayGE/PreDeclare.hpp>

#include <vector>

#include <KlayGE/RenderView.hpp>
#include <KlayGE/RenderTarget.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class FrameBuffer : public RenderTarget
	{
	public:
		enum ATTACHMENT
		{
			ATT_DepthStencil,
			ATT_Color0,
			ATT_Color1,
			ATT_Color2,
			ATT_Color3
		};

	public:
		virtual ~FrameBuffer() = 0;

		static FrameBufferPtr NullObject();

		void Attach(uint32_t att, RenderViewPtr view);
		void Detach(uint32_t att);

		void OnBind();
		void OnUnbind();

		void SwapBuffers()
		{
		}

	protected:
		std::vector<RenderViewPtr> clr_views_;
		RenderViewPtr rs_view_;
	};
}

#endif			// _FRAMEBUFFER_HPP
