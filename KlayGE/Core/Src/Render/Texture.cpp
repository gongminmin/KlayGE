// Texture.cpp
// KlayGE 纹理类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2005-2007
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Util.hpp>

#include <fstream>

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
	// 载入DDS格式文件
	TexturePtr LoadTexture(std::string const & tex_name)
	{
		boost::shared_ptr<std::istream> file(ResLoader::Instance().Load(tex_name));

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

		ElementFormat format = EF_ARGB8;
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
			}
		}
		else
		{
			if ((desc.pixel_format.flags & DDSPF_RGB) != 0)
			{
				switch (desc.pixel_format.rgb_bit_count)
				{
				case 16:
					if ((0xF800 == desc.pixel_format.r_bit_mask)
						&& (0x7E0 == desc.pixel_format.g_bit_mask)
						&& (0x1F == desc.pixel_format.b_bit_mask))
					{
						format = EF_R5G6B5;
					}
					else
					{
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
							format = EF_A2RGB10;
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
						if ((desc.pixel_format.flags & DDSPF_ALPHAPIXELS) != 0)
						{
							format = EF_AL4;
						}
						else
						{
							format = EF_L8;
						}
						break;

					case 16:
						if ((desc.pixel_format.flags & DDSPF_ALPHAPIXELS) != 0)
						{
							format = EF_AL8;
						}
						else
						{
							format = EF_L16;
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
									format = EF_SIGNED_A2RGB10;
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

		KlayGE::TexturePtr texture;
		{
			RenderFactory& renderFactory = Context::Instance().RenderFactoryInstance();
			if ((desc.dds_caps.caps2 & DDSCAPS2_CUBEMAP) != 0)
			{
				texture = renderFactory.MakeTextureCube(desc.width,
						static_cast<uint16_t>(desc.mip_map_count), format);
			}
			else
			{
				if ((desc.dds_caps.caps2 & DDSCAPS2_VOLUME) != 0)
				{
					texture = renderFactory.MakeTexture3D(desc.width,
						desc.height, desc.depth, static_cast<uint16_t>(desc.mip_map_count), format);
				}
				else
				{
					texture = renderFactory.MakeTexture2D(desc.width,
						desc.height, static_cast<uint16_t>(desc.mip_map_count), format);
				}
			}
		}

		std::vector<uint8_t> data;
		switch (texture->Type())
		{
		case Texture::TT_1D:
			{
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

						image_size = ((texture->Width(level) + 3) / 4) * block_size;
					}
					else
					{
						image_size = main_image_size / (1UL << (level * 2));
					}

					data.resize(image_size);

					file->read(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(data.size()));
					BOOST_ASSERT(file->gcount() == static_cast<int>(data.size()));

					texture->CopyMemoryToTexture2D(level, &data[0], format,
						texture->Width(level), texture->Height(level), 0, 0,
						texture->Width(level), texture->Height(level), 0, 0,
						texture->Width(level) * NumFormatBytes(texture->Format()));
				}
			}
			break;

		case Texture::TT_2D:
			{
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

						image_size = ((texture->Width(level) + 3) / 4) * ((texture->Height(level) + 3) / 4) * block_size;
					}
					else
					{
						image_size = main_image_size / (1UL << (level * 2));
					}

					data.resize(image_size);

					file->read(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(data.size()));
					BOOST_ASSERT(file->gcount() == static_cast<int>(data.size()));

					texture->CopyMemoryToTexture2D(level, &data[0], format,
						texture->Width(level), texture->Height(level), 0, 0,
						texture->Width(level), texture->Height(level), 0, 0,
						texture->Width(level) * NumFormatBytes(texture->Format()));
				}
			}
			break;

		case Texture::TT_3D:
			{
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

						image_size = ((texture->Width(level) + 3) / 4) * ((texture->Height(level) + 3) / 4) * texture->Height(level) * block_size;
					}
					else
					{
						image_size = main_image_size / (1UL << (level * 2)) * texture->Depth(level);
					}

					data.resize(image_size);

					file->read(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(data.size()));
					BOOST_ASSERT(file->gcount() == static_cast<int>(data.size()));

					texture->CopyMemoryToTexture3D(level, &data[0], format,
						texture->Width(level), texture->Height(level), texture->Depth(level), 0, 0, 0,
						texture->Width(level), texture->Height(level), texture->Depth(level), 0, 0, 0,
						texture->Width(level) * NumFormatBytes(texture->Format()),
						texture->Width(level) * texture->Height(level) * NumFormatBytes(texture->Format()));
				}
			}
			break;

		case Texture::TT_Cube:
			{
				for (uint32_t face = Texture::CF_Positive_X; face <= Texture::CF_Negative_Z; ++ face)
				{
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

							image_size = ((texture->Width(level) + 3) / 4) * ((texture->Height(level) + 3) / 4) * block_size;
						}
						else
						{
							image_size = main_image_size / (1UL << (level * 2));
						}

						data.resize(image_size);

						file->read(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(data.size()));
						BOOST_ASSERT(file->gcount() == static_cast<int>(data.size()));

						texture->CopyMemoryToTextureCube(static_cast<Texture::CubeFaces>(face),
							level, &data[0], format,
							texture->Width(level), texture->Height(level), 0, 0,
							texture->Width(level), texture->Height(level), 0, 0,
							texture->Width(level) * NumFormatBytes(texture->Format()));
					}
				}
			}
			break;
		}

		return texture;
	}

	// 把纹理保存入DDS文件
	void SaveTexture(TexturePtr texture, std::string const & tex_name)
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

		desc.width = texture->Width(0);
		desc.height = texture->Height(0);

		if (texture->NumMipMaps() != 0)
		{
			desc.flags |= DDSD_MIPMAPCOUNT;
			desc.mip_map_count = texture->NumMipMaps();
		}

		desc.pixel_format.size = sizeof(desc.pixel_format);

		if (IsSRGB(texture->Format()))
		{
			desc.reserved1[0] = 1;
		}

		if ((EF_ABGR16 == texture->Format())
			|| IsFloatFormat(texture->Format()) || IsCompressedFormat(texture->Format()))
		{
			desc.pixel_format.flags |= DDSPF_FOURCC;

			switch (texture->Format())
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

			default:
				BOOST_ASSERT(false);
				break;
			}
		}
		else
		{
			switch (texture->Format())
			{
			case EF_R5G6B5:
				desc.pixel_format.flags |= DDSPF_RGB;
				desc.pixel_format.rgb_bit_count = 16;

				desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
				desc.pixel_format.r_bit_mask = 0x0000F800;
				desc.pixel_format.g_bit_mask = 0x000007E0;
				desc.pixel_format.b_bit_mask = 0x0000001F;
				break;

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

			case EF_SIGNED_ABGR8:
				desc.pixel_format.flags |= DDSPF_BUMPDUDV;
				desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0xFF00000;
				desc.pixel_format.r_bit_mask = 0x000000FF;
				desc.pixel_format.g_bit_mask = 0x0000FF00;
				desc.pixel_format.b_bit_mask = 0x00FF0000;
				break;

			case EF_A2RGB10:
				desc.pixel_format.flags |= DDSPF_RGB;
				desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0xC0000000;
				desc.pixel_format.r_bit_mask = 0x3FF00000;
				desc.pixel_format.g_bit_mask = 0x000FFC00;
				desc.pixel_format.b_bit_mask = 0x000003FF;
				break;

			case EF_SIGNED_A2RGB10:
				desc.pixel_format.flags |= DDSPF_BUMPDUDV;
				desc.pixel_format.rgb_bit_count = 32;

				desc.pixel_format.rgb_alpha_bit_mask = 0xC0000000;
				desc.pixel_format.r_bit_mask = 0x3FF00000;
				desc.pixel_format.g_bit_mask = 0x000FFC00;
				desc.pixel_format.b_bit_mask = 0x000003FF;
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

			case EF_AL4:
				desc.pixel_format.flags |= DDSPF_LUMINANCE;
				desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
				desc.pixel_format.rgb_bit_count = 8;

				desc.pixel_format.rgb_alpha_bit_mask = 0x000000F0;
				desc.pixel_format.r_bit_mask = 0x0000000F;
				desc.pixel_format.g_bit_mask = 0x00000000;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			case EF_L8:
				desc.pixel_format.flags |= DDSPF_LUMINANCE;
				desc.pixel_format.rgb_bit_count = 8;

				desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
				desc.pixel_format.r_bit_mask = 0x000000FF;
				desc.pixel_format.g_bit_mask = 0x00000000;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			case EF_AL8:
				desc.pixel_format.flags |= DDSPF_LUMINANCE;
				desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
				desc.pixel_format.rgb_bit_count = 16;

				desc.pixel_format.rgb_alpha_bit_mask = 0x0000FF00;
				desc.pixel_format.r_bit_mask = 0x000000FF;
				desc.pixel_format.g_bit_mask = 0x00000000;
				desc.pixel_format.b_bit_mask = 0x00000000;
				break;

			case EF_L16:
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
		if (texture->NumMipMaps() != 1)
		{
			desc.dds_caps.caps1 |= DDSCAPS_MIPMAP;
			desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
		}
		if (Texture::TT_3D == texture->Type())
		{
			desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
			desc.dds_caps.caps2 |= DDSCAPS2_VOLUME;
			desc.flags |= DDSD_DEPTH;
			desc.depth = texture->Depth(0);
		}
		if (Texture::TT_Cube == texture->Type())
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

		uint32_t main_image_size = texture->Width(0) * texture->Height(0) * NumFormatBytes(texture->Format());
		if (IsCompressedFormat(texture->Format()))
		{
			if (EF_BC1 == texture->Format())
			{
				main_image_size = texture->Width(0) * texture->Height(0) / 2;
			}
			else
			{
				main_image_size = texture->Width(0) * texture->Height(0);
			}

			desc.flags |= DDSD_LINEARSIZE;
			desc.linear_size = main_image_size;
		}

		file.write(reinterpret_cast<char*>(&desc), sizeof(desc));

		std::vector<uint8_t> data;
		switch (texture->Type())
		{
		case Texture::TT_1D:
			{
				for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
				{
					uint32_t image_size;
					if (IsCompressedFormat(texture->Format()))
					{
						int block_size;
						if (EF_BC1 == texture->Format())
						{
							block_size = 8;
						}
						else
						{
							block_size = 16;
						}

						image_size = ((texture->Width(level) + 3) / 4) * block_size;
					}
					else
					{
						image_size = main_image_size / (1UL << (level * 2));
					}

					data.resize(image_size);
					texture->CopyToMemory1D(level, &data[0]);

					file.write(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(data.size()));
				}
			}
			break;

		case Texture::TT_2D:
			{
				for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
				{
					uint32_t image_size;
					if (IsCompressedFormat(texture->Format()))
					{
						int block_size;
						if (EF_BC1 == texture->Format())
						{
							block_size = 8;
						}
						else
						{
							block_size = 16;
						}

						image_size = ((texture->Width(level) + 3) / 4) * ((texture->Height(level) + 3) / 4) * block_size;
					}
					else
					{
						image_size = main_image_size / (1UL << (level * 2));
					}

					data.resize(image_size);
					texture->CopyToMemory2D(level, &data[0]);

					file.write(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(data.size()));
				}
			}
			break;

		case Texture::TT_3D:
			{
				for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
				{
					uint32_t image_size;
					if (IsCompressedFormat(texture->Format()))
					{
						int block_size;
						if (EF_BC1 == texture->Format())
						{
							block_size = 8;
						}
						else
						{
							block_size = 16;
						}

						image_size = ((texture->Width(level) + 3) / 4) * ((texture->Height(level) + 3) / 4) * texture->Depth(level) * block_size;
					}
					else
					{
						image_size = main_image_size / (1UL << (level * 2)) * texture->Depth(level);
					}

					data.resize(image_size);
					texture->CopyToMemory3D(level, &data[0]);

					file.write(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(data.size()));
				}
			}
			break;

		case Texture::TT_Cube:
			{
				for (uint32_t face = Texture::CF_Positive_X; face <= Texture::CF_Negative_Z; ++ face)
				{
					for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
					{
						uint32_t image_size;
						if (IsCompressedFormat(texture->Format()))
						{
							int block_size;
							if (EF_BC1 == texture->Format())
							{
								block_size = 8;
							}
							else
							{
								block_size = 16;
							}

							image_size = ((texture->Width(level) + 3) / 4) * ((texture->Height(level) + 3) / 4) * block_size;
						}
						else
						{
							image_size = main_image_size / (1UL << (level * 2));
						}

						data.resize(image_size);
						texture->CopyToMemoryCube(static_cast<Texture::CubeFaces>(face), level, &data[0]);

						file.write(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(data.size()));
					}
				}
			}
			break;
		}
	}


	class NullTexture : public Texture
	{
	public:
		NullTexture(TextureType type)
			: Texture(type)
		{
		}

		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Texture");
			return name;
		}

		void Usage(TextureUsage /*usage*/)
		{
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

		void CopyToMemory1D(int /*level*/, void* /*data*/)
		{
		}
		void CopyToMemory2D(int /*level*/, void* /*data*/)
		{
		}
		void CopyToMemory3D(int /*level*/, void* /*data*/)
		{
		}
		void CopyToMemoryCube(CubeFaces /*face*/, int /*level*/, void* /*data*/)
		{
		}

		void CopyMemoryToTexture1D(int /*level*/, void const * /*data*/, ElementFormat /*pf*/,
			uint32_t /*dst_width*/, uint32_t /*dst_xOffset*/, uint32_t /*src_width*/, uint32_t /*src_xOffset*/)
		{
		}

		void CopyMemoryToTexture2D(int /*level*/, void const * /*data*/, ElementFormat /*pf*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/,
			uint32_t /*src_row_pitch*/)
		{
		}

		void CopyMemoryToTexture3D(int /*level*/, void const * /*data*/, ElementFormat /*pf*/,
				uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
				uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/, uint32_t /*dst_zOffset*/,
				uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/,
				uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/, uint32_t /*src_zOffset*/,
				uint32_t /*src_row_pitch*/, uint32_t /*src_slice_pitch*/)
		{
		}

		void CopyMemoryToTextureCube(CubeFaces /*face*/, int /*level*/, void const * /*data*/, ElementFormat /*pf*/,
				uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
				uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/,
				uint32_t /*src_row_pitch*/)
		{
		}

		void BuildMipSubLevels()
		{
		}
	};


	Texture::Texture(Texture::TextureType type)
			: usage_(TU_Default), type_(type)
	{
	}

	Texture::~Texture()
	{
	}

	TexturePtr Texture::NullObject()
	{
		static TexturePtr obj(new NullTexture(TT_2D));
		return obj;
	}

	uint16_t Texture::NumMipMaps() const
	{
		return numMipMaps_;
	}

	Texture::TextureUsage Texture::Usage() const
	{
		return usage_;
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
}
