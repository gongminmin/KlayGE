#ifndef _OGLRENDERFACTORY_HPP
#define _OGLRENDERFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

#define NOMINMAX
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")

namespace KlayGE
{
	class OGLRenderFactory : public RenderFactory
	{
	public:
		const WString& Name() const;

		RenderEngine& RenderEngineInstance();

		TexturePtr MakeTexture(U32 width, U32 height, U16 mipMapsNum,
			PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default);
		RenderTexturePtr MakeRenderTexture(U32 width, U32 height);

		FontPtr MakeFont(const WString& fontName, U32 fontHeight = 12, U32 flags = 0);

		RenderEffectPtr MakeRenderEffect(const String& srcData, UINT flags = 0);
	};
}

#endif			// _OGLRENDERFACTORY_HPP
