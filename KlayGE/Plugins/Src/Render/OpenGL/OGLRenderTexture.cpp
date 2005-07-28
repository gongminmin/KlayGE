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
		BOOST_ASSERT(false);
	}

	void OGLRenderTexture::AttachTexture2D(TexturePtr texture2D)
	{
		BOOST_ASSERT(false);
	}
	
	void OGLRenderTexture::AttachTextureCube(TexturePtr textureCube, Texture::CubeFaces face)
	{
		BOOST_ASSERT(false);
	}

	void OGLRenderTexture::DetachTexture()
	{
		BOOST_ASSERT(false);
	}
}
