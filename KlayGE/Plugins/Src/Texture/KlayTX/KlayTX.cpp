// KlayTX.hpp
// KlayGE .klaytx纹理类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.9.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/VFile.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/KlayTX/KlayTX.hpp>

namespace KlayGE
{
	// 构造函数
	//////////////////////////////////////////////////////////////////////////////////
	KlayTX::KlayTX()
	{
	}

	// 载入纹理
	//////////////////////////////////////////////////////////////////////////////////
	TexturePtr KlayTX::Load(VFile& file, U16 mipmapsNum)
	{
		file.Rewind();
		file.Read(&header_, sizeof(header_));
		Verify(header_.id == MakeFourCC<'K', 'l', 'T', 'X'>::value);

		file.Seek(header_.offset, VFile::SM_Current);

		data_.resize(this->Width() * this->Height() * PixelFormatBits(this->Format()) / 8);
		file.Read(&data_[0], data_.size());

		TexturePtr tex(Engine::RenderFactoryInstance().MakeTexture(this->Width(), this->Height(), mipmapsNum,
			this->Format(), Texture::TU_Default));

		tex->CopyMemoryToTexture(&data_[0], this->Format());

		return tex;
	}

	// 图片宽度
	//////////////////////////////////////////////////////////////////////////////////
	U32 KlayTX::Width() const
	{
		return header_.width;
	}

	// 图片高度
	//////////////////////////////////////////////////////////////////////////////////
	U32 KlayTX::Height() const
	{
		return header_.height;
	}

	// 图片格式
	//////////////////////////////////////////////////////////////////////////////////
	PixelFormat KlayTX::Format() const
	{
		return header_.format;
	}
}
