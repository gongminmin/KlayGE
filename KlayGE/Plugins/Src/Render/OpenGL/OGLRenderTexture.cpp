#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderTexture.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#include <gl/gl.h>

#include <KlayGE/OpenGL/OGLRenderTexture.hpp>

namespace KlayGE
{
	OGLRenderTexture::OGLRenderTexture(uint32_t width, uint32_t height)
	{
		left_ = 0;
		top_ = 0;
		privateTex_ = Context::Instance().RenderFactoryInstance().MakeTexture(width, height, 0, PF_X8R8G8B8, Texture::TU_RenderTarget);
		width_ = privateTex_->Width();
		height_ = privateTex_->Height();
	}

	void OGLRenderTexture::CustomAttribute(std::string const & name, void* pData)
	{
		if ("IsTexture" == name)
		{
			bool* b = reinterpret_cast<bool*>(pData);
			*b = true;

			return;
		}
	}
}
