#ifndef _OGLRENDERTEXTURE_HPP
#define _OGLRENDERTEXTURE_HPP

#include <KlayGE/RenderTexture.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLRenderTexture : public RenderTexture
	{
	public:
		OGLRenderTexture(uint32_t width, uint32_t height);

		virtual void CustomAttribute(std::string const & name, void* pData);

		bool RequiresTextureFlipping() const
			{ return true; }
	};

	typedef boost::shared_ptr<OGLRenderTexture> OGLRenderTexturePtr;
}

#endif			// _OGLRENDERTEXTURE_HPP
