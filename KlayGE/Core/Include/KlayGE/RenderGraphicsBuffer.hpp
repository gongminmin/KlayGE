// RenderGraphicsBuffer.hpp
// KlayGE 渲染图形缓冲区类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERGRAPHICSBUFFER_HPP
#define _RENDERGRAPHICSBUFFER_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderTarget.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{	
	class RenderGraphicsBuffer : public RenderTarget
	{
	public:
		virtual ~RenderGraphicsBuffer()
		{
		}

		static RenderGraphicsBufferPtr NullObject();

		virtual void Attach(GraphicsBufferPtr gb) = 0;
		virtual void Detach() = 0;

		void SwapBuffers()
		{
		}

		bool IsTexture() const
		{
			return false;
		}

	protected:
		GraphicsBufferPtr vs_;
	};
}

#endif			// _RENDERGRAPHICSBUFFER_HPP