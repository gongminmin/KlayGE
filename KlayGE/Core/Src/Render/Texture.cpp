// Texture.cpp
// KlayGE 纹理类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2005-2009
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 支持BC4/BC5纹理压缩的读写和转换 (2008.12.8)
// 多线程纹理载入 (2009.1.22)
//
// 3.5.0
// 支持有符号格式 (2007.2.12)
//
// 3.3.0
// 支持GR16和ABGR16 (2006.6.7)
//
// 2.4.0
// 初次建立 (2005.3.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/BlockCompression.hpp>

#include <cstring>
#include <fstream>

#include <boost/bind.hpp>

#include <KlayGE/Texture.hpp>

namespace
{
	using namespace KlayGE;

#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(push, 1)
#endif

	enum
	{
		// The surface has alpha channel information in the pixel format.
		DDSPF_ALPHAPIXELS = 0x00000001,

		// The FourCC code is valid.
		DDSPF_FOURCC = 0x00000004,

		// The RGB data in the pixel format structure is valid.
		DDSPF_RGB = 0x00000040,

		// Luminance data in the pixel format is valid.
		// Use this flag for luminance-only or luminance+alpha surfaces,
		// the bit depth is then ddpf.dwLuminanceBitCount.
		DDSPF_LUMINANCE = 0x00020000,

		// Bump map dUdV data in the pixel format is valid.
		DDSPF_BUMPDUDV = 0x00080000
	};

	struct DDSPIXELFORMAT
	{
		uint32_t	size;				// size of structure
		uint32_t	flags;				// pixel format flags
		uint32_t	four_cc;			// (FOURCC code)
		uint32_t	rgb_bit_count;		// how many bits per pixel
		uint32_t	r_bit_mask;			// mask for red bit
		uint32_t	g_bit_mask;			// mask for green bits
		uint32_t	b_bit_mask;			// mask for blue bits
		uint32_t	rgb_alpha_bit_mask;	// mask for alpha channels
	};

	enum
	{
		// Indicates a complex surface structure is being described.  A
		// complex surface structure results in the creation of more than
		// one surface.  The additional surfaces are attached to the root
		// surface.  The complex structure can only be destroyed by
		// destroying the root.
		DDSCAPS_COMPLEX		= 0x00000008,

		// Indicates that this surface can be used as a 3D texture.  It does not
		// indicate whether or not the surface is being used for that purpose.
		DDSCAPS_TEXTURE		= 0x00001000,

		// Indicates surface is one level of a mip-map. This surface will
		// be attached to other DDSCAPS_MIPMAP surfaces to form the mip-map.
		// This can be done explicitly, by creating a number of surfaces and
		// attaching them with AddAttachedSurface or by implicitly by CreateSurface.
		// If this bit is set then DDSCAPS_TEXTURE must also be set.
		DDSCAPS_MIPMAP		= 0x00400000,
	};

	enum
	{
		// This flag is used at CreateSurface time to indicate that this set of
		// surfaces is a cubic environment map
		DDSCAPS2_CUBEMAP	= 0x00000200,

		// These flags preform two functions:
		// - At CreateSurface time, they define which of the six cube faces are
		//   required by the application.
		// - After creation, each face in the cubemap will have exactly one of these
		//   bits set.
		DDSCAPS2_CUBEMAP_POSITIVEX	= 0x00000400,
		DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x00000800,
		DDSCAPS2_CUBEMAP_POSITIVEY	= 0x00001000,
		DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x00002000,
		DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x00004000,
		DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x00008000,

		// Indicates that the surface is a volume.
		// Can be combined with DDSCAPS_MIPMAP to indicate a multi-level volume
		DDSCAPS2_VOLUME		= 0x00200000,
	};

	struct DDSCAPS2
	{
		uint32_t	caps1;			// capabilities of surface wanted
		uint32_t	caps2;
		uint32_t	reserved[2];
	};

	enum
	{
		DDSD_CAPS			= 0x00000001,	// default, dds_caps field is valid.
		DDSD_HEIGHT			= 0x00000002,	// height field is valid.
		DDSD_WIDTH			= 0x00000004,	// width field is valid.
		DDSD_PITCH			= 0x00000008,	// pitch is valid.
		DDSD_PIXELFORMAT	= 0x00001000,	// pixel_format is valid.
		DDSD_MIPMAPCOUNT	= 0x00020000,	// mip_map_count is valid.
		DDSD_LINEARSIZE		= 0x00080000,	// linear_size is valid
		DDSD_DEPTH			= 0x00800000,	// depth is valid
	};

	struct DDSSURFACEDESC2
	{
		uint32_t	size;					// size of the DDSURFACEDESC structure
		uint32_t	flags;					// determines what fields are valid
		uint32_t	height;					// height of surface to be created
		uint32_t	width;					// width of input surface
		union
		{
			int32_t		pitch;				// distance to start of next line (return value only)
			uint32_t	linear_size;		// Formless late-allocated optimized surface size
		};
		uint32_t		depth;				// the depth if this is a volume texture
		uint32_t		mip_map_count;		// number of mip-map levels requestde
		uint32_t		reserved1[11];		// reserved
		DDSPIXELFORMAT	pixel_format;		// pixel format description of the surface
		DDSCAPS2		dds_caps;			// direct draw surface capabilities
		uint32_t		reserved2;
	};
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(pop)
#endif
}

namespace KlayGE
{
	TextureLoader::TextureLoader(std::string const & tex_name, uint32_t access_hint)
	{
		tl_thread_ = GlobalThreadPool()(boost::bind(&TextureLoader::LoadDDS, this, tex_name, access_hint));
	}

	boost::shared_ptr<TextureLoader::TexDesc> TextureLoader::LoadDDS(std::string const & tex_name, uint32_t access_hint)
	{
		boost::shared_ptr<TexDesc> tex_desc(new TexDesc);
		tex_desc->access_hint = access_hint;

		LoadTexture(tex_name, tex_desc->type, tex_desc->width, tex_desc->height, tex_desc->depth,
			tex_desc->numMipMaps, tex_desc->format, tex_desc->tex_data, tex_desc->data_block);

		RenderFactory& renderFactory = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = renderFactory.RenderEngineInstance().DeviceCaps();

		if ((EF_BC5 == tex_desc->format) && !caps.bc5_support)
		{
			BC1_layout tmp;
			for (size_t i = 0; i < tex_desc->tex_data.size(); ++ i)
			{
				for (size_t j = 0; j < tex_desc->tex_data[i].slice_pitch; j += sizeof(BC4_layout) * 2)
				{
					char* p = static_cast<char*>(const_cast<void*>(tex_desc->tex_data[i].data)) + j;

					BC4ToBC1G(tmp, *reinterpret_cast<BC4_layout const *>(p + sizeof(BC4_layout)));
					memcpy(p + sizeof(BC4_layout), &tmp, sizeof(BC1_layout));
				}
			}

			tex_desc->format = EF_BC3;
		}

		if ((EF_ARGB8 == tex_desc->format) && !caps.argb8_support)
		{
			for (size_t i = 0; i < tex_desc->tex_data.size(); ++ i)
			{
				uint8_t* p = static_cast<uint8_t*>(const_cast<void*>(tex_desc->tex_data[i].data));
				for (size_t y = 0; y < tex_desc->height; ++ y)
				{
					for (size_t x = 0; x < tex_desc->width; ++ x)
					{
						std::swap(p[x * 4 + 0], p[x * 4 + 2]);
					}
					p += tex_desc->tex_data[i].row_pitch;
				}
			}

			tex_desc->format = EF_ABGR8;
		}

		return tex_desc;
	}

	TexturePtr TextureLoader::operator()()
	{
		if (!texture_)
		{
			boost::shared_ptr<TexDesc> tex_desc = tl_thread_();

			RenderFactory& renderFactory = Context::Instance().RenderFactoryInstance();
			switch (tex_desc->type)
			{
			case Texture::TT_1D:
				texture_ = renderFactory.MakeTexture1D(tex_desc->width, tex_desc->numMipMaps,
					tex_desc->format, 1, 0, tex_desc->access_hint, &tex_desc->tex_data[0]);
				break;

			case Texture::TT_2D:
				texture_ = renderFactory.MakeTexture2D(tex_desc->width, tex_desc->height, tex_desc->numMipMaps,
					tex_desc->format, 1, 0, tex_desc->access_hint, &tex_desc->tex_data[0]);
				break;

			case Texture::TT_3D:
				texture_ = renderFactory.MakeTexture3D(tex_desc->width, tex_desc->height, tex_desc->depth, tex_desc->numMipMaps,
					tex_desc->format, 1, 0, tex_desc->access_hint, &tex_desc->tex_data[0]);
				break;

			case Texture::TT_Cube:
				texture_ = renderFactory.MakeTextureCube(tex_desc->width, tex_desc->numMipMaps,
					tex_desc->format, 1, 0, tex_desc->access_hint, &tex_desc->tex_data[0]);
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		return texture_;
	}

	// 载入DDS格式文件
	void LoadTexture(std::string const & tex_name, Texture::TextureType& type,
		uint32_t& width, uint32_t& height, uint32_t& depth, uint16_t& numMipMaps,
		ElementFormat& format, std::vector<ElementInitData>& init_data, std::vector<uint8_t>& data_block)
	{
		ResIdentifierPtr file = ResLoader::Instance().Load(tex_name);

		uint32_t magic;
		file->read(reinterpret_cast<char*>(&magic), sizeof(magic));
		BOOST_ASSERT((MakeFourCC<'D', 'D', 'S', ' '>::value) == magic);

		DDSSURFACEDESC2 desc;
		file->read(reinterpret_cast<char*>(&desc), sizeof(desc));

		BOOST_ASSERT((desc.flags & DDSD_CAPS) != 0);
		BOOST_ASSERT((desc.flags & DDSD_PIXELFORMAT) != 0);
		BOOST_ASSERT((desc.flags & DDSD_WIDTH) != 0);
		BOOST_ASSERT((desc.flags & DDSD_HEIGHT) != 0);

		if (0 == (desc.flags & DDSD_MIPMAPCOUNT))
		{
			desc.mip_map_count = 1;
		}

		format = EF_ARGB8;
		if ((desc.pixel_format.flags & DDSPF_FOURCC) != 0)
		{
			switch (desc.pixel_format.four_cc)
			{
			case 36:
				format = EF_ABGR16;
				break;

			case 110:
				format = EF_SIGNED_ABGR16;
				break;

			case 111:
				format = EF_R16F;
				break;

			case 112:
				format = EF_GR16F;
				break;

			case 113:
				format = EF_ABGR16F;
				break;

			case 114:
				format = EF_R32F;
				break;

			case 115:
				format = EF_GR32F;
				break;

			case 116:
				format = EF_ABGR32F;
				break;

			case MakeFourCC<'D', 'X', 'T', '1'>::value:
				format = EF_BC1;
				break;

			case MakeFourCC<'D', 'X', 'T', '3'>::value:
				format = EF_BC2;
				break;

			case MakeFourCC<'D', 'X', 'T', '5'>::value:
				format = EF_BC3;
				break;

			case MakeFourCC<'A', 'T', 'I', '1'>::value:
				format = EF_BC4;
				break;

			case MakeFourCC<'A', 'T', 'I', '2'>::value:
				format = EF_BC5;
				break;
			}
		}
		else
		{
			if ((desc.pixel_format.flags & DDSPF_RGB) != 0)
			{
				switch (desc.pixel_format.rgb_bit_count)
				{
				case 16:
					if ((0xF000 == desc.pixel_format.rgb_alpha_bit_mask)
						&& (0x0F00 == desc.pixel_format.r_bit_mask)
						&& (0x00F0 == desc.pixel_format.g_bit_mask)
						&& (0x000F == desc.pixel_format.b_bit_mask))
					{
						format = EF_ARGB4;
					}
					else
					{
						BOOST_ASSERT(false);
					}
					break;

				case 32:
					if ((0x00FF0000 == desc.pixel_format.r_bit_mask)
						&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
						&& (0x000000FF == desc.pixel_format.b_bit_mask))
					{
						if ((desc.pixel_format.flags & DDSPF_ALPHAPIXELS) != 0)
						{
							format = EF_ARGB8;
						}
						else
						{
							BOOST_ASSERT(false);
						}
					}
					else
					{
						if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
							&& (0x3FF00000 == desc.pixel_format.r_bit_mask)
							&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
							&& (0x000003FF == desc.pixel_format.b_bit_mask))
						{
							format = EF_A2BGR10;
						}
						else
						{
							if ((0xFF000000 == desc.pixel_format.rgb_alpha_bit_mask)
								&& (0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
								&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								format = EF_ABGR8;
							}
							else
							{
								if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
									&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
									&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
									&& (0x00000000 == desc.pixel_format.b_bit_mask))
								{
									format = EF_GR16;
								}
								else
								{
									BOOST_ASSERT(false);
								}
							}
						}
					}
					break;
				}
			}
			else
			{
				if ((desc.pixel_format.flags & DDSPF_LUMINANCE) != 0)
				{
					switch (desc.pixel_format.rgb_bit_count)
					{
					case 8:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							format = EF_R8;
						}
						else
						{
							BOOST_ASSERT(false);
						}
						break;

					case 16:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							format = EF_R16;
						}
						else
						{
							BOOST_ASSERT(false);
						}
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
				}
				else
				{
					if ((desc.pixel_format.flags & DDSPF_BUMPDUDV) != 0)
					{
						switch (desc.pixel_format.rgb_bit_count)
						{
						case 16:
							if ((0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask))
							{
								format = EF_SIGNED_GR8;
							}
							else
							{
								if (0x0000FFFF == desc.pixel_format.r_bit_mask)
								{
									format = EF_SIGNED_R16;
								}
								else
								{
									BOOST_ASSERT(false);
								}
							}
							break;

						case 32:
							if ((0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
								&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								format = EF_SIGNED_ABGR8;
							}
							else
							{
								if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
									&& (0x3FF00000 == desc.pixel_format.r_bit_mask)
									&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
									&& (0x000003FF == desc.pixel_format.b_bit_mask))
								{
									format = EF_SIGNED_A2BGR10;
								}
								else
								{
									if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
										&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
										&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
										&& (0x00000000 == desc.pixel_format.b_bit_mask))
									{
										format = EF_SIGNED_GR16;
									}
									else
									{
										BOOST_ASSERT(false);
									}
								}
							}
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
					}
					else
					{
						if ((desc.pixel_format.flags & DDSPF_ALPHAPIXELS) != 0)
						{
							format = EF_A8;
						}
						else
						{
							BOOST_ASSERT(false);
						}
					}
				}
			}
		}

		uint32_t main_image_size;
		if ((desc.flags & DDSD_LINEARSIZE) != 0)
		{
			main_image_size = desc.linear_size;
		}
		else
		{
			if ((desc.flags & DDSD_PITCH) != 0)
			{
				main_image_size = desc.pitch * desc.height;
			}
			else
			{
				if ((desc.flags & desc.pixel_format.flags & 0x00000040) != 0)
				{
					main_image_size = desc.width * desc.height * desc.pixel_format.rgb_bit_count / 8;
				}
				else
				{
					main_image_size = desc.width * desc.height * NumFormatBytes(format);
				}
			}
		}

		if (desc.reserved1[0] != 0)
		{
			format = MakeSRGB(format);
		}

		width = desc.width;
		numMipMaps = static_cast<uint16_t>(desc.mip_map_count);

		{
			if ((desc.dds_caps.caps2 & DDSCAPS2_CUBEMAP) != 0)
			{
				type = Texture::TT_Cube;
				height = desc.width;
				depth = 1;
			}
			else
			{
				if ((desc.dds_caps.caps2 & DDSCAPS2_VOLUME) != 0)
				{
					type = Texture::TT_3D;
					height = desc.width;
					depth = desc.depth;
				}
				else
				{
					type = Texture::TT_2D;
					height = desc.height;
					depth = 1;
				}
			}
		}

		std::vector<size_t> base;
		uint32_t format_size = NumFormatBytes(format);
		switch (type)
		{
		case Texture::TT_1D:
			{
				uint32_t the_width = width;
				init_data.resize(numMipMaps);
				base.resize(numMipMaps);
				for (uint32_t level = 0; level < numMipMaps; ++ level)
				{
					uint32_t image_size;
					if (IsCompressedFormat(format))
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

						image_size = ((the_width + 3) / 4) * block_size;
					}
					else
					{
						image_size = main_image_size / (1UL << (level * 2));
					}

					base[level] = data_block.size();
					data_block.resize(base[level] + image_size);
					init_data[level].row_pitch = image_size;
					init_data[level].slice_pitch = image_size;

					file->read(reinterpret_cast<char*>(&data_block[base[level]]), static_cast<std::streamsize>(image_size));
					BOOST_ASSERT(file->gcount() == static_cast<int>(image_size));

					the_width /= 2;
				}
			}
			break;

		case Texture::TT_2D:
			{
				uint32_t the_width = width;
				uint32_t the_height = height;
				init_data.resize(numMipMaps);
				base.resize(numMipMaps);
				for (uint32_t level = 0; level < numMipMaps; ++ level)
				{
					if (IsCompressedFormat(format))
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

						uint32_t image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

						base[level] = data_block.size();
						data_block.resize(base[level] + image_size);
						init_data[level].row_pitch = (the_width + 3) / 4 * block_size;
						init_data[level].slice_pitch = image_size;

						file->read(reinterpret_cast<char*>(&data_block[base[level]]), static_cast<std::streamsize>(image_size));
						BOOST_ASSERT(file->gcount() == static_cast<int>(image_size));
					}
					else
					{
						if (desc.flags & DDSD_PITCH)
						{
							init_data[level].row_pitch = static_cast<uint32_t>(desc.pitch);
						}
						else
						{
							init_data[level].row_pitch = the_width * format_size;
						}
						init_data[level].slice_pitch = init_data[level].row_pitch * the_height;
						base[level] = data_block.size();
						data_block.resize(base[level] + init_data[level].slice_pitch);

						file->read(reinterpret_cast<char*>(&data_block[base[level]]), static_cast<std::streamsize>(init_data[level].slice_pitch));
						BOOST_ASSERT(file->gcount() == static_cast<int>(init_data[level].slice_pitch));
					}

					the_width /= 2;
					the_height /= 2;
				}
			}
			break;

		case Texture::TT_3D:
			{
				uint32_t the_width = width;
				uint32_t the_height = height;
				uint32_t the_depth = depth;
				init_data.resize(numMipMaps);
				base.resize(numMipMaps);
				for (uint32_t level = 0; level < numMipMaps; ++ level)
				{
					if (IsCompressedFormat(format))
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

						uint32_t image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * the_depth * block_size;

						base[level] = data_block.size();
						data_block.resize(base[level] + image_size);
						init_data[level].row_pitch = (the_width + 3) / 4 * block_size;
						init_data[level].slice_pitch = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

						file->read(reinterpret_cast<char*>(&data_block[base[level]]), static_cast<std::streamsize>(image_size));
						BOOST_ASSERT(file->gcount() == static_cast<int>(image_size));
					}
					else
					{
						if (desc.flags & DDSD_PITCH)
						{
							init_data[level].row_pitch = static_cast<uint32_t>(desc.pitch);
							init_data[level].slice_pitch = init_data[level].row_pitch * (the_height + 3) / 4 * 4;
						}
						else
						{
							init_data[level].row_pitch = the_width * format_size;
							init_data[level].slice_pitch = init_data[level].row_pitch * the_height;
						}
						base[level] = data_block.size();
						data_block.resize(base[level] + init_data[level].slice_pitch * the_depth);

						file->read(reinterpret_cast<char*>(&data_block[base[level]]), static_cast<std::streamsize>(init_data[level].slice_pitch * the_depth));
						BOOST_ASSERT(file->gcount() == static_cast<int>(init_data[level].slice_pitch * the_depth));
					}

					the_width /= 2;
					the_height /= 2;
					the_depth /= 2;
				}
			}
			break;

		case Texture::TT_Cube:
			{
				init_data.resize(6 * numMipMaps);
				base.resize(6 * numMipMaps);
				for (uint32_t face = Texture::CF_Positive_X; face <= Texture::CF_Negative_Z; ++ face)
				{
					uint32_t the_width = width;
					uint32_t the_height = height;
					for (uint32_t level = 0; level < numMipMaps; ++ level)
					{
						size_t const index = (face - Texture::CF_Positive_X) * numMipMaps + level;
						if (IsCompressedFormat(format))
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

							uint32_t image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

							base[index] = data_block.size();
							data_block.resize(base[index] + image_size);
							init_data[index].row_pitch = (the_width + 3) / 4 * block_size;
							init_data[index].slice_pitch = image_size;

							file->read(reinterpret_cast<char*>(&data_block[base[index]]), static_cast<std::streamsize>(image_size));
							BOOST_ASSERT(file->gcount() == static_cast<int>(image_size));
						}
						else
						{
							if (desc.flags & DDSD_PITCH)
							{
								init_data[index].row_pitch = static_cast<uint32_t>(desc.pitch);
							}
							else
							{
								init_data[index].row_pitch = the_width * format_size;
							}
							init_data[index].slice_pitch = init_data[index].row_pitch * the_width;
							base[index] = data_block.size();
							data_block.resize(base[index] + init_data[index].slice_pitch);

							file->read(reinterpret_cast<char*>(&data_block[base[index]]), static_cast<std::streamsize>(init_data[index].slice_pitch));
							BOOST_ASSERT(file->gcount() == static_cast<int>(init_data[index].slice_pitch));
						}
					}
				}
			}
			break;
		}

		for (size_t i = 0; i < base.size(); ++ i)
		{
			init_data[i].data = &data_block[base[i]];
		}
	}

	TextureLoader LoadTexture(std::string const & tex_name, uint32_t access_hint)
	{
		return TextureLoader(tex_name, access_hint);
	}

	KLAYGE_CORE_API void SaveTexture(std::string const & tex_name, Texture::TextureType type,
		uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps,
		ElementFormat format, std::vector<ElementInitData> const & init_data)
	{
		std::ofstream file(tex_name.c_str(), std::ios_base::binary);

		uint32_t magic = MakeFourCC<'D', 'D', 'S', ' '>::value;
		file.write(reinterpret_cast<char*>(&magic), sizeof(magic));

		DDSSURFACEDESC2 desc;
		std::memset(&desc, 0, sizeof(desc));

		desc.size = sizeof(desc);

		desc.flags |= DDSD_CAPS;
		desc.flags |= DDSD_PIXELFORMAT;
		desc.flags |= DDSD_WIDTH;
		desc.flags |= DDSD_HEIGHT;

		desc.width = width;
		desc.height = height;

		if (numMipMaps != 0)
		{
			desc.flags |= DDSD_MIPMAPCOUNT;
			desc.mip_map_count = numMipMaps;
		}

		desc.pixel_format.size = sizeof(desc.pixel_format);

		if (IsSRGB(format))
		{
			desc.reserved1[0] = 1;
		}

		if ((EF_ABGR16 == format)
			|| IsFloatFormat(format) || IsCompressedFormat(format))
		{
			desc.pixel_format.flags |= DDSPF_FOURCC;

			switch (format)
			{
			case EF_ABGR16:
				desc.pixel_format.four_cc = 36;
				break;

			case EF_SIGNED_ABGR16:
				desc.pixel_format.four_cc = 110;
				break;

			case EF_R16F:
				desc.pixel_format.four_cc = 111;
				break;

			case EF_GR16F:
				desc.pixel_format.four_cc = 112;
				break;

			case EF_ABGR16F:
				desc.pixel_format.four_cc = 113;
				break;

			case EF_R32F:
				desc.pixel_format.four_cc = 114;
				break;

			case EF_GR32F:
				desc.pixel_format.four_cc = 115;
				break;

			case EF_ABGR32F:
				desc.pixel_format.four_cc = 116;
				break;

			case EF_BC1:
			case EF_BC1_SRGB:
				desc.pixel_format.four_cc = MakeFourCC<'D', 'X', 'T', '1'>::value;
				break;

			case EF_BC2:
			case EF_BC2_SRGB:
				desc.pixel_format.four_cc = MakeFourCC<'D', 'X', 'T', '3'>::value;
				break;

			case EF_BC3:
			case EF_BC3_SRGB:
				desc.pixel_format.four_cc = MakeFourCC<'D', 'X', 'T', '5'>::value;
				break;

			case EF_BC4:
			case EF_BC4_SRGB:
				desc.pixel_format.four_cc = MakeFourCC<'A', 'T', 'I', '1'>::value;
				break;

			case EF_BC5:
			case EF_BC5_SRGB:
				desc.pixel_format.four_cc = MakeFourCC<'A', 'T', 'I', '2'>::value;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}
		else
		{
			switch (format)
			{
			case EF_ARGB4:
				desc.pixel_format.flags |= DDSPF_RGB;
				desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
				desc.pixel_format.rgb_bit_count = 16;

				desc.pixel_format.rgb_alpha_bit_mask = 0x0000F000;
				desc.pixel_format.r_bit_mask = 0x00000F00;
				desc.pixel_format.g_bit_mask = 0x000000F0;
				desc.pixel_format.b_bit_mask = 0x0000000F;
				break;

			case EF_SIGNED_GR8:
				desc.pixel_format.flags |= DDSPF_BUMPDUDV;
				desc.pixel_format.rgb_bit_count = 16;

				desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
				desc.pixel_format.r_bit_mask = 0x000000FF;
				desc.pixel_format.g_bit_mask = 0x0000FF00;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			case EF_SIGNED_R16:
				desc.pixel_format.flags |= DDSPF_BUMPDUDV;
				desc.pixel_format.rgb_bit_count = 16;

				desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
				desc.pixel_format.r_bit_mask = 0x0000FFFF;
				desc.pixel_format.g_bit_mask = 0x00000000;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			case EF_ARGB8:
			case EF_ARGB8_SRGB:
				desc.pixel_format.flags |= DDSPF_RGB;
				desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0xFF000000;
				desc.pixel_format.r_bit_mask = 0x00FF0000;
				desc.pixel_format.g_bit_mask = 0x0000FF00;
				desc.pixel_format.b_bit_mask = 0x000000FF;
				break;

			case EF_ABGR8:
			case EF_ABGR8_SRGB:
				desc.pixel_format.flags |= DDSPF_RGB;
				desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0xFF000000;
				desc.pixel_format.r_bit_mask = 0x000000FF;
				desc.pixel_format.g_bit_mask = 0x0000FF00;
				desc.pixel_format.b_bit_mask = 0x00FF0000;
				break;

			case EF_SIGNED_ABGR8:
				desc.pixel_format.flags |= DDSPF_BUMPDUDV;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0xFF000000;
				desc.pixel_format.r_bit_mask = 0x000000FF;
				desc.pixel_format.g_bit_mask = 0x0000FF00;
				desc.pixel_format.b_bit_mask = 0x00FF0000;
				break;

			case EF_A2BGR10:
				desc.pixel_format.flags |= DDSPF_RGB;
				desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0xC0000000;
				desc.pixel_format.r_bit_mask = 0x000003FF;
				desc.pixel_format.g_bit_mask = 0x000FFC00;
				desc.pixel_format.b_bit_mask = 0x3FF00000;
				break;

			case EF_SIGNED_A2BGR10:
				desc.pixel_format.flags |= DDSPF_BUMPDUDV;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0xC0000000;
				desc.pixel_format.r_bit_mask = 0x000003FF;
				desc.pixel_format.g_bit_mask = 0x000FFC00;
				desc.pixel_format.b_bit_mask = 0x3FF00000;
				break;

			case EF_GR16:
				desc.pixel_format.flags |= DDSPF_RGB;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
				desc.pixel_format.r_bit_mask = 0x0000FFFF;
				desc.pixel_format.g_bit_mask = 0xFFFF0000;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			case EF_SIGNED_GR16:
				desc.pixel_format.flags |= DDSPF_BUMPDUDV;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
				desc.pixel_format.r_bit_mask = 0x0000FFFF;
				desc.pixel_format.g_bit_mask = 0xFFFF0000;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			case EF_R8:
				desc.pixel_format.flags |= DDSPF_LUMINANCE;
				desc.pixel_format.rgb_bit_count = 8;

				desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
				desc.pixel_format.r_bit_mask = 0x000000FF;
				desc.pixel_format.g_bit_mask = 0x00000000;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			case EF_R16:
				desc.pixel_format.flags |= DDSPF_LUMINANCE;
				desc.pixel_format.rgb_bit_count = 16;

				desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
				desc.pixel_format.r_bit_mask = 0x0000FFFF;
				desc.pixel_format.g_bit_mask = 0x00000000;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			case EF_A8:
				desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
				desc.pixel_format.rgb_bit_count = 8;

				desc.pixel_format.rgb_alpha_bit_mask = 0x000000FF;
				desc.pixel_format.r_bit_mask = 0x00000000;
				desc.pixel_format.g_bit_mask = 0x00000000;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		desc.dds_caps.caps1 = DDSCAPS_TEXTURE;
		if (numMipMaps != 1)
		{
			desc.dds_caps.caps1 |= DDSCAPS_MIPMAP;
			desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
		}
		if (Texture::TT_3D == type)
		{
			desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
			desc.dds_caps.caps2 |= DDSCAPS2_VOLUME;
			desc.flags |= DDSD_DEPTH;
			desc.depth = depth;
		}
		if (Texture::TT_Cube == type)
		{
			desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
			desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP;
			desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_POSITIVEX;
			desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEX;
			desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_POSITIVEY;
			desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEY;
			desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_POSITIVEZ;
			desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEZ;
		}

		uint32_t format_size = NumFormatBytes(format);
		uint32_t main_image_size = width * height * format_size;
		if (IsCompressedFormat(format))
		{
			if (EF_BC1 == format)
			{
				main_image_size = width * height / 2;
			}
			else
			{
				main_image_size = width * height;
			}

			desc.flags |= DDSD_LINEARSIZE;
			desc.linear_size = main_image_size;
		}

		file.write(reinterpret_cast<char*>(&desc), sizeof(desc));

		switch (type)
		{
		case Texture::TT_1D:
			{
				int the_width = width;
				for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
				{
					uint32_t image_size;
					if (IsCompressedFormat(format))
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

						image_size = ((the_width + 3) / 4) * block_size;
					}
					else
					{
						image_size = main_image_size / (1UL << (level * 2));
					}

					file.write(reinterpret_cast<char const *>(init_data[level].data), static_cast<std::streamsize>(image_size));

					the_width /= 2;
				}
			}
			break;

		case Texture::TT_2D:
			{
				int the_width = width;
				int the_height = height;
				for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
				{
					if (IsCompressedFormat(format))
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

						uint32_t image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

						file.write(reinterpret_cast<char const *>(init_data[level].data), static_cast<std::streamsize>(image_size));
					}
					else
					{
						file.write(reinterpret_cast<char const *>(init_data[level].data), static_cast<std::streamsize>(the_width * the_height * format_size));
					}

					the_width /= 2;
					the_height /= 2;
				}
			}
			break;

		case Texture::TT_3D:
			{
				int the_width = width;
				int the_height = height;
				int the_depth = depth;
				for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
				{
					if (IsCompressedFormat(format))
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

						uint32_t image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * the_depth * block_size;

						file.write(reinterpret_cast<char const *>(init_data[level].data), static_cast<std::streamsize>(image_size));
					}
					else
					{
						file.write(reinterpret_cast<char const *>(init_data[level].data), static_cast<std::streamsize>(the_width * the_height * the_depth * format_size));
					}

					the_width /= 2;
					the_height /= 2;
					the_depth /= 2;
				}
			}
			break;

		case Texture::TT_Cube:
			{
				for (uint32_t face = Texture::CF_Positive_X; face <= Texture::CF_Negative_Z; ++ face)
				{
					int the_width = width;
					for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
					{
						size_t const index = (face - Texture::CF_Positive_X) * numMipMaps + level;
						if (IsCompressedFormat(format))
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

							uint32_t image_size = ((the_width + 3) / 4) * ((the_width + 3) / 4) * block_size;

							file.write(reinterpret_cast<char const *>(init_data[index].data), static_cast<std::streamsize>(image_size));
						}
						else
						{
							file.write(reinterpret_cast<char const *>(init_data[index].data), static_cast<std::streamsize>(the_width * the_width * format_size));
						}

						the_width /= 2;
					}
				}
			}
			break;
		}
	}

	// 把纹理保存入DDS文件
	void SaveTexture(TexturePtr texture, std::string const & tex_name)
	{
		RenderFactory& renderFactory = Context::Instance().RenderFactoryInstance();

		ElementFormat format = texture->Format();
		uint16_t numMipMaps = texture->NumMipMaps();

		TexturePtr texture_sys_mem;
		switch (texture->Type())
		{
		case Texture::TT_1D:
			texture_sys_mem = renderFactory.MakeTexture1D(texture->Width(0),
				numMipMaps, 1, 0, format, EAH_CPU_Read, NULL);
			break;

		case Texture::TT_2D:
			texture_sys_mem = renderFactory.MakeTexture2D(texture->Width(0), texture->Height(0),
				numMipMaps, format, 1, 0, EAH_CPU_Read, NULL);
			break;

		case Texture::TT_3D:
			texture_sys_mem = renderFactory.MakeTexture3D(texture->Width(0), texture->Height(0),
				texture->Depth(0), numMipMaps, format, 1, 0, EAH_CPU_Read, NULL);
			break;

		case Texture::TT_Cube:
			texture_sys_mem = renderFactory.MakeTextureCube(texture->Width(0),
				numMipMaps, format, 1, 0, EAH_CPU_Read, NULL);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
		texture->CopyToTexture(*texture_sys_mem);

		uint32_t format_size = NumFormatBytes(format);
		uint32_t main_image_size = texture_sys_mem->Width(0) * texture_sys_mem->Height(0) * format_size;
		if (IsCompressedFormat(format))
		{
			if (EF_BC1 == format)
			{
				main_image_size = texture_sys_mem->Width(0) * texture_sys_mem->Height(0) / 2;
			}
			else
			{
				main_image_size = texture_sys_mem->Width(0) * texture_sys_mem->Height(0);
			}
		}

		std::vector<ElementInitData> init_data;
		std::vector<size_t> base;
		std::vector<uint8_t> data_block;
		switch (texture_sys_mem->Type())
		{
		case Texture::TT_1D:
			{
				init_data.resize(numMipMaps);
				base.resize(numMipMaps);
				for (uint32_t level = 0; level < numMipMaps; ++ level)
				{
					uint32_t image_size;
					if (IsCompressedFormat(format))
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

						image_size = ((texture_sys_mem->Width(level) + 3) / 4) * block_size;
					}
					else
					{
						image_size = main_image_size / (1UL << (level * 2));
					}

					{
						Texture::Mapper mapper(*texture_sys_mem, level, TMA_Read_Only, 0, texture_sys_mem->Width(level));
						base[level] = data_block.size();
						data_block.resize(data_block.size() + image_size);
						memcpy(&data_block[base[level]], mapper.Pointer<char>(), image_size);
					}
				}
			}
			break;

		case Texture::TT_2D:
			{
				init_data.resize(numMipMaps);
				base.resize(numMipMaps);
				for (uint32_t level = 0; level < numMipMaps; ++ level)
				{
					uint32_t width = texture_sys_mem->Width(level);
					uint32_t height = texture_sys_mem->Height(level);

					if (IsCompressedFormat(format))
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

						uint32_t image_size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;

						{
							Texture::Mapper mapper(*texture_sys_mem, level, TMA_Read_Only, 0, 0, width, height);
							base[level] = data_block.size();
							data_block.resize(data_block.size() + image_size);
							memcpy(&data_block[base[level]], mapper.Pointer<char>(), image_size);
						}
					}
					else
					{
						Texture::Mapper mapper(*texture_sys_mem, level, TMA_Read_Only, 0, 0, width, height);
						char* data = mapper.Pointer<char>();

						base[level] = data_block.size();
						data_block.resize(data_block.size() + width * height * format_size);
						for (uint32_t y = 0; y < height; ++ y)
						{
							memcpy(&data_block[base[level] + y * width * format_size], data, width * format_size);
							data += mapper.RowPitch();
						}
					}
				}
			}
			break;

		case Texture::TT_3D:
			{
				init_data.resize(numMipMaps);
				base.resize(numMipMaps);
				for (uint32_t level = 0; level < numMipMaps; ++ level)
				{
					uint32_t width = texture_sys_mem->Width(level);
					uint32_t height = texture_sys_mem->Height(level);
					uint32_t depth = texture_sys_mem->Depth(level);

					if (IsCompressedFormat(format))
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

						uint32_t image_size = ((width + 3) / 4) * ((height + 3) / 4) * depth * block_size;

						{
							Texture::Mapper mapper(*texture_sys_mem, level, TMA_Read_Only, 0, 0, 0, width, height, depth);
							base[level] = data_block.size();
							data_block.resize(data_block.size() + image_size);
							memcpy(&data_block[base[level]], mapper.Pointer<char>(), image_size);
						}
					}
					else
					{
						Texture::Mapper mapper(*texture_sys_mem, level, TMA_Read_Only, 0, 0, 0, width, height, depth);
						char* data = mapper.Pointer<char>();

						base[level] = data_block.size();
						data_block.resize(data_block.size() + width * height * depth * format_size);
						for (uint32_t z = 0; z < depth; ++ z)
						{
							for (uint32_t y = 0; y < height; ++ y)
							{
								memcpy(&data_block[base[level] + (z * height + y) * width * format_size], data, width * format_size);
								data += mapper.RowPitch();
							}

							data += mapper.SlicePitch() - mapper.RowPitch() * height;
						}
					}
				}
			}
			break;

		case Texture::TT_Cube:
			{
				init_data.resize(6 * numMipMaps);
				base.resize(6 * numMipMaps);
				for (uint32_t face = Texture::CF_Positive_X; face <= Texture::CF_Negative_Z; ++ face)
				{
					for (uint32_t level = 0; level < numMipMaps; ++ level)
					{
						size_t const index = (face - Texture::CF_Positive_X) * numMipMaps + level;

						uint32_t width = texture_sys_mem->Width(level);
						uint32_t height = texture_sys_mem->Height(level);
						if (IsCompressedFormat(format))
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

							uint32_t image_size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;

							{
								Texture::Mapper mapper(*texture_sys_mem, static_cast<Texture::CubeFaces>(face), level, TMA_Read_Only, 0, 0, width, height);
								base[index] = data_block.size();
								data_block.resize(data_block.size() + image_size);
								memcpy(&data_block[base[index]], mapper.Pointer<char>(), image_size);
							}
						}
						else
						{
							Texture::Mapper mapper(*texture_sys_mem, static_cast<Texture::CubeFaces>(face), level, TMA_Read_Only, 0, 0, width, height);
							char* data = mapper.Pointer<char>();

							base[index] = data_block.size();
							data_block.resize(data_block.size() + width * height * format_size);
							for (uint32_t y = 0; y < height; ++ y)
							{
								memcpy(&data_block[base[index] + y * width * format_size], data, width * format_size);
								data += mapper.RowPitch();
							}
						}
					}
				}
			}
			break;
		}

		for (size_t i = 0; i < base.size(); ++ i)
		{
			init_data[i].data = &data_block[base[i]];
		}

		SaveTexture(tex_name, texture_sys_mem->Type(),
			texture_sys_mem->Width(0), texture_sys_mem->Height(0), texture_sys_mem->Depth(0), numMipMaps,
			format, init_data);
	}


	class NullTexture : public Texture
	{
	public:
		NullTexture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
			: Texture(type, sample_count, sample_quality, access_hint)
		{
		}

		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Texture");
			return name;
		}

        uint32_t Width(int /*level*/) const
		{
			return 0;
		}
		uint32_t Height(int /*level*/) const
		{
			return 0;
		}
		uint32_t Depth(int /*level*/) const
		{
			return 0;
		}

		void CopyToTexture(Texture& /*target*/)
		{
		}

		void CopyToTexture1D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_xOffset*/, uint32_t /*src_width*/, uint32_t /*src_xOffset*/)
		{
		}

		void CopyToTexture2D(Texture& /*target*/, int /*level*/,
				uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
				uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
		{
		}

		void CopyToTexture3D(Texture& /*target*/, int /*level*/,
				uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
				uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/, uint32_t /*dst_zOffset*/,
				uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/,
				uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/, uint32_t /*src_zOffset*/)
		{
		}

		void CopyToTextureCube(Texture& /*target*/, CubeFaces /*face*/, int /*level*/,
				uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
				uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
		{
		}

		void Map1D(int /*level*/, TextureMapAccess /*level*/,
			uint32_t /*x_offset*/, uint32_t /*width*/,
			void*& /*data*/)
		{
		}
		void Map2D(int /*level*/, TextureMapAccess /*level*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
		{
		}
		void Map3D(int /*level*/, TextureMapAccess /*level*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*z_offset*/,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& /*data*/, uint32_t& /*row_pitch*/, uint32_t& /*slice_pitch*/)
		{
		}
		void MapCube(CubeFaces /*level*/, int /*level*/, TextureMapAccess /*level*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
		{
		}

		void Unmap1D(int /*level*/)
		{
		}
		void Unmap2D(int /*level*/)
		{
		}
		void Unmap3D(int /*level*/)
		{
		}
		void UnmapCube(CubeFaces /*face*/, int /*level*/)
		{
		}

		void BuildMipSubLevels()
		{
		}
	};


	Texture::Texture(Texture::TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
			: type_(type), sample_count_(sample_count), sample_quality_(sample_quality), access_hint_(access_hint)
	{
	}

	Texture::~Texture()
	{
	}

	TexturePtr Texture::NullObject()
	{
		static TexturePtr obj = MakeSharedPtr<NullTexture>(TT_2D, 1, 0, 0);
		return obj;
	}

	uint16_t Texture::NumMipMaps() const
	{
		return numMipMaps_;
	}

	uint32_t Texture::Bpp() const
	{
		return bpp_;
	}

	ElementFormat Texture::Format() const
	{
		return format_;
	}

	Texture::TextureType Texture::Type() const
	{
		return type_;
	}

	uint32_t Texture::SampleCount() const
	{
		return sample_count_;
	}
	
	uint32_t Texture::SampleQuality() const
	{
		return sample_quality_;
	}

	uint32_t Texture::AccessHint() const
	{
		return access_hint_;
	}
}
