// RenderTexture.hpp
// KlayGE 渲染纹理类 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 增加了IsTexture和SwapBuffers (2005.3.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERTEXTURE_HPP
#define _RENDERTEXTURE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderTarget.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{	
	class RenderTexture : public RenderTarget
	{
	public:
		TexturePtr const & GetTexture() const
			{ return privateTex_; }
		TexturePtr& GetTexture()
			{ return privateTex_; }

		void SwapBuffers()
		{
		}

		bool IsTexture() const
		{
			return true;
		}

	protected:
		TexturePtr privateTex_;
	};
}

#endif			// _RENDERTEXTURE_HPP