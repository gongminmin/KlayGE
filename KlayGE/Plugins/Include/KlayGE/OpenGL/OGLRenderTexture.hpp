#ifndef _OGLRENDERTEXTURE_HPP
#define _OGLRENDERTEXTURE_HPP

#include <KlayGE/RenderTexture.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")

namespace KlayGE
{
	class OGLRenderTexture : public RenderTexture
	{
	public:
		OGLRenderTexture(U32 width, U32 height);

		virtual void CustomAttribute(std::string const & name, void* pData);

		bool RequiresTextureFlipping() const
			{ return true; }
	};

	typedef boost::shared_ptr<OGLRenderTexture> OGLRenderTexturePtr;
}

#endif			// _OGLRENDERTEXTURE_HPP
