// RenderTexture.cpp
// KlayGE 渲染到纹理类 实现文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.1.0
// 初次建立 (2005.10.29)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderTexture.hpp>

namespace KlayGE
{
	class NullRenderTexture : public RenderTexture
	{
	public:
		void AttachTexture2D(TexturePtr texture2D)
		{
		}
		void AttachTextureCube(TexturePtr textureCube, Texture::CubeFaces face)
		{
		}
		void DetachTexture()
		{
		}

		void CustomAttribute(std::string const & name, void* pData)
		{
		}
		bool RequiresTextureFlipping() const
		{
			return true;
		}
	};

	RenderTexturePtr RenderTexture::NullObject()
	{
		static RenderTexturePtr obj(new NullRenderTexture);
		return obj;
	}
}
