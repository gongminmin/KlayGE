// D3D9Texture3D.cpp
// KlayGE D3D9 3D纹理类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <KlayGE/D3D9/D3D9MinGWDefs.hpp>

#include <d3d9.h>
#include <d3dx9.h>

#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx9d.lib")
#else
	#pragma comment(lib, "d3dx9.lib")
#endif
#endif

namespace KlayGE
{
	D3D9Texture3D::D3D9Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: D3D9Texture(TT_3D, sample_count, sample_quality, access_hint),
						auto_gen_mipmaps_(false)
	{
		if (array_size > 1)
		{
			THR(boost::system::posix_error::not_supported);
		}

		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3dDevice_ = renderEngine.D3DDevice();

		numMipMaps_ = numMipMaps;
		array_size_ = 1;
		format_		= format;
		widths_.assign(1, width);
		heights_.assign(1, height);
		depths_.assign(1, depth);

		bpp_ = NumFormatBits(format);

		uint32_t usage;
		D3DPOOL pool;
		if (access_hint & EAH_GPU_Write)
		{
			usage = D3DUSAGE_RENDERTARGET;
			pool = D3DPOOL_DEFAULT;
		}
		else
		{
			usage = 0;
			pool = D3DPOOL_MANAGED;
		}
		d3dTexture3D_ = this->CreateTexture3D(usage, pool);

		this->QueryBaseTexture();
		this->UpdateParams();

		if (init_data != NULL)
		{
			if (access_hint & EAH_GPU_Write)
			{
				TexturePtr sys_mem = Context::Instance().RenderFactoryInstance().MakeTexture3D(widths_[0],
					heights_[0], depths_[0], numMipMaps_, array_size_, format_, sample_count, sample_quality, EAH_CPU_Write, init_data);
				sys_mem->CopyToTexture(*this);
			}
			else
			{
				for (uint32_t level = 0; level < numMipMaps_; ++ level)
				{
					Texture::Mapper mapper(*this, level, TMA_Write_Only, 0, 0, 0, widths_[level], heights_[level], depths_[level]);

					if (IsCompressedFormat(format_))
					{
						int block_size;
						if (EF_BC1 == format)
						{
							block_size = 8;
						}
						else
						{
							block_size = 16;
						}

						memcpy(mapper.Pointer<uint8_t>(), init_data[level].data,
							((widths_[level] + 3) / 4) * ((heights_[level] + 3) / 4) * depth * block_size);
					}
					else
					{
						for (uint32_t d = 0; d < depths_[level]; ++ d)
						{
							for (uint32_t h = 0; h < heights_[level]; ++ h)
							{
								memcpy(mapper.Pointer<uint8_t>() + mapper.SlicePitch() * d + mapper.RowPitch() * h,
									static_cast<uint8_t const *>(init_data[level].data) + init_data[level].slice_pitch * d + init_data[level].row_pitch * h,
									std::min(mapper.RowPitch(), init_data[level].row_pitch));
							}
						}
					}
				}
			}
		}
	}

	uint32_t D3D9Texture3D::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return widths_[level];
	}

	uint32_t D3D9Texture3D::Height(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return heights_[level];
	}

	uint32_t D3D9Texture3D::Depth(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return depths_[level];
	}

	void D3D9Texture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());
		BOOST_ASSERT(depths_[0] == target.Depth(0));

		D3D9Texture3D& other(*checked_cast<D3D9Texture3D*>(&target));

		uint32_t maxLevel = 1;
		if (this->NumMipMaps() == target.NumMipMaps())
		{
			maxLevel = this->NumMipMaps();
		}

		DWORD filter = D3DX_FILTER_LINEAR;
		if (IsSRGB(format_))
		{
			filter |= D3DX_FILTER_SRGB_IN;
		}
		if (IsSRGB(target.Format()))
		{
			filter |= D3DX_FILTER_SRGB_OUT;
		}

		for (uint32_t level = 0; level < maxLevel; ++ level)
		{
			IDirect3DVolume9* src;
			TIF(d3dTexture3D_->GetVolumeLevel(level, &src));

			IDirect3DVolume9* dst;
			TIF(other.d3dTexture3D_->GetVolumeLevel(level, &dst));

			TIF(D3DXLoadVolumeFromVolume(dst, NULL, NULL, src, NULL, NULL, filter, 0));

			src->Release();
			dst->Release();
		}

		if (this->NumMipMaps() != target.NumMipMaps())
		{
			target.BuildMipSubLevels();
		}
	}

	void D3D9Texture3D::CopyToTexture3D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			uint32_t src_xOffset, uint32_t src_yOffset, uint32_t src_zOffset)
	{
		BOOST_ASSERT(type_ == target.Type());
		BOOST_ASSERT(depths_[0] == target.Depth(0));

		D3D9Texture3D& other(*checked_cast<D3D9Texture3D*>(&target));

		DWORD filter = D3DX_FILTER_LINEAR;
		if (IsSRGB(format_))
		{
			filter |= D3DX_FILTER_SRGB_IN;
		}
		if (IsSRGB(target.Format()))
		{
			filter |= D3DX_FILTER_SRGB_OUT;
		}

		{
			IDirect3DVolume9* src;
			TIF(d3dTexture3D_->GetVolumeLevel(level, &src));

			IDirect3DVolume9* dst;
			TIF(other.d3dTexture3D_->GetVolumeLevel(level, &dst));

			D3DBOX srcBox = { src_xOffset, src_yOffset, src_xOffset + src_width, src_yOffset + src_height,
				src_zOffset, src_zOffset + src_depth };
			D3DBOX dstBox = { dst_xOffset, dst_yOffset, dst_xOffset + dst_width, dst_yOffset + dst_height,
				dst_zOffset, dst_zOffset + dst_depth };
			TIF(D3DXLoadVolumeFromVolume(dst, NULL, &dstBox, src, NULL, &srcBox, filter, 0));

			src->Release();
			dst->Release();
		}
	}

	void D3D9Texture3D::Map3D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		D3DBOX box = { x_offset, y_offset, x_offset + width, y_offset + height, z_offset, z_offset + depth };
		D3DLOCKED_BOX locked_box;
		d3dTexture3D_->LockBox(level, &locked_box, &box, TMA_Read_Only == tma ? D3DLOCK_READONLY : 0);
		data = locked_box.pBits;
		row_pitch = locked_box.RowPitch;
		slice_pitch = locked_box.SlicePitch;
	}

	void D3D9Texture3D::Unmap3D(int level)
	{
		d3dTexture3D_->UnlockBox(level);
	}

	void D3D9Texture3D::BuildMipSubLevels()
	{
		if (auto_gen_mipmaps_)
		{
			d3dTexture3D_->GenerateMipSubLevels();
		}
		else
		{
			DWORD filter = D3DX_FILTER_NONE;
			if (IsSRGB(format_))
			{
				filter |= D3DX_FILTER_SRGB;
			}

			if (this->AccessHint() & EAH_GPU_Write)
			{
				ID3D9VolumeTexturePtr d3dTexture3D = this->CreateTexture3D(D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);

				IDirect3DVolume9* src;
				TIF(d3dTexture3D_->GetVolumeLevel(0, &src));

				IDirect3DVolume9* dst;
				TIF(d3dTexture3D->GetVolumeLevel(0, &dst));

				TIF(D3DXLoadVolumeFromVolume(dst, NULL, NULL, src, NULL, NULL, filter, 0));

				d3dTexture3D->GenerateMipSubLevels();
				d3dTexture3D_ = d3dTexture3D;

				auto_gen_mipmaps_ = true;

				src->Release();
				dst->Release();
			}
			else
			{
				ID3D9VolumeTexturePtr d3dTexture3D = this->CreateTexture3D(0, D3DPOOL_SYSTEMMEM);

				IDirect3DBaseTexture9* base;
				d3dTexture3D->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&base));

				IDirect3DVolume9* src;
				TIF(d3dTexture3D_->GetVolumeLevel(0, &src));

				IDirect3DVolume9* dst;
				TIF(d3dTexture3D->GetVolumeLevel(0, &dst));

				TIF(D3DXLoadVolumeFromVolume(dst, NULL, NULL, src, NULL, NULL, filter, 0));

				TIF(D3DXFilterTexture(base, NULL, 0, filter));
				TIF(d3dDevice_->UpdateTexture(base, d3dBaseTexture_.get()));

				base->Release();
				src->Release();
				dst->Release();
			}
		}
	}

	void D3D9Texture3D::DoOnLostDevice()
	{
		if (this->AccessHint() & EAH_GPU_Write)
		{
			d3dBaseTexture_.reset();
			d3dTexture3D_.reset();
		}
	}

	void D3D9Texture3D::DoOnResetDevice()
	{
		if (this->AccessHint() & EAH_GPU_Write)
		{
			d3dTexture3D_ = this->CreateTexture3D(D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);
			this->QueryBaseTexture();
		}
	}

	ID3D9VolumeTexturePtr D3D9Texture3D::CreateTexture3D(uint32_t usage, D3DPOOL pool)
	{
		if (IsDepthFormat(format_))
		{
			usage |= D3DUSAGE_DEPTHSTENCIL;
		}

		IDirect3DVolumeTexture9* d3dTexture3D;
		TIF(d3dDevice_->CreateVolumeTexture(widths_[0], heights_[0], depths_[0],
			numMipMaps_, usage, D3D9Mapping::MappingFormat(format_),
			pool, &d3dTexture3D, NULL));
		return MakeCOMPtr(d3dTexture3D);
	}

	void D3D9Texture3D::QueryBaseTexture()
	{
		IDirect3DBaseTexture9* d3dBaseTexture = NULL;
		d3dTexture3D_->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&d3dBaseTexture));
		d3dBaseTexture_ = MakeCOMPtr(d3dBaseTexture);
	}

	void D3D9Texture3D::UpdateParams()
	{
		D3DVOLUME_DESC desc;
		std::memset(&desc, 0, sizeof(desc));

		numMipMaps_ = d3dTexture3D_->GetLevelCount();
		BOOST_ASSERT(numMipMaps_ != 0);

		widths_.resize(numMipMaps_);
		heights_.resize(numMipMaps_);
		depths_.resize(numMipMaps_);
		for (uint32_t level = 0; level < numMipMaps_; ++ level)
		{
			TIF(d3dTexture3D_->GetLevelDesc(level, &desc));

			widths_[level] = desc.Width;
			heights_[level] = desc.Height;
			depths_[level] = desc.Depth;
		}

		bool srgb = IsSRGB(format_);
		format_ = D3D9Mapping::MappingFormat(desc.Format);
		if (srgb)
		{
			format_ = MakeSRGB(format_);
		}

		bpp_	= NumFormatBits(format_);
	}
}
