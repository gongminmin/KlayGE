// RenderVertexStream.hpp
// KlayGE 渲染到顶点流类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.19)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERVERTEXSTREAM_HPP
#define _RENDERVERTEXSTREAM_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderTarget.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{	
	class RenderVertexStream : public RenderTarget
	{
	public:
		virtual void Attach(VertexStreamPtr vs) = 0;
		virtual void Detach() = 0;

		void SwapBuffers()
		{
		}

		bool IsTexture() const
		{
			return false;
		}

	protected:
		VertexStreamPtr vs_;
	};
}

#endif			// _RENDERVERTEXSTREAM_HPP