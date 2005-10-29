// RenderTexture.hpp
// KlayGE 渲染到纹理类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 去掉了GetTexture (2005.7.19)
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
		static RenderTexturePtr NullObject();

		virtual void AttachTexture2D(TexturePtr texture2D) = 0;
		virtual void AttachTextureCube(TexturePtr textureCube, Texture::CubeFaces face) = 0;
		virtual void DetachTexture() = 0;

		void SwapBuffers()
		{
		}

		bool IsTexture() const
		{
			return true;
		}

	protected:
		TexturePtr privateTex_;
		Texture::CubeFaces face_;
	};
}

#endif			// _RENDERTEXTURE_HPP