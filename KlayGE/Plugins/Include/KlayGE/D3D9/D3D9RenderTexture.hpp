#ifndef _D3D9RENDERTEXTURE_HPP
#define _D3D9RENDERTEXTURE_HPP

#include <KlayGE/RenderTexture.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	class D3D9RenderFactory;

	class D3D9RenderTexture : public RenderTexture
	{
		friend class D3D9RenderFactory;

	public:
		virtual void CustomAttribute(const String& name, void* pData);

		bool RequiresTextureFlipping() const
			{ return true; }

	protected:
		D3D9RenderTexture(U32 width, U32 height);
	};
}

#endif			// _D3D9RENDERTEXTURE_HPP