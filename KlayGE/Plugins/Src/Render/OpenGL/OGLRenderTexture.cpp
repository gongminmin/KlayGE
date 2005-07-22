#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderTexture.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderTexture.hpp>

namespace KlayGE
{
	OGLRenderTexture::OGLRenderTexture()
	{
		left_ = 0;
		top_ = 0;
	}

	void OGLRenderTexture::CustomAttribute(std::string const & name, void* pData)
	{
		assert(false);
	}

	void OGLRenderTexture::AttachTexture2D(TexturePtr texture2D)
	{
		assert(false);
	}
	
	void OGLRenderTexture::AttachTextureCube(TexturePtr textureCube, Texture::CubeFaces face)
	{
		assert(false);
	}

	void OGLRenderTexture::DetachTexture()
	{
		assert(false);
	}
}
