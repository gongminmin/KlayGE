#ifndef _D3D9RENDERTEXTURE_HPP
#define _D3D9RENDERTEXTURE_HPP

#include <KlayGE/RenderTexture.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	class D3D9RenderTexture : public RenderTexture
	{
	public:
		D3D9RenderTexture(uint32 width, uint32 height);

		virtual void CustomAttribute(std::string const & name, void* pData);

		bool RequiresTextureFlipping() const
			{ return true; }
	};

	typedef boost::shared_ptr<D3D9RenderTexture> D3D9RenderTexturePtr;
}

#endif			// _D3D9RENDERTEXTURE_HPP
