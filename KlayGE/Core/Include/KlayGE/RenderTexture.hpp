#ifndef _RENDERTEXTURE_HPP
#define _RENDERTEXTURE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderTarget.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{	
	class RenderTexture : public RenderTarget
	{
	public:
		const Texture& GetTexture() const
			{ return *privateTex_; }
		Texture& GetTexture()
			{ return *privateTex_; }

	protected:
		TexturePtr privateTex_;
	};
}

#endif			// _RENDERTEXTURE_HPP