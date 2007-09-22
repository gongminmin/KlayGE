#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#pragma warning(push)
#pragma warning(disable: 4245)
#include <boost/filesystem.hpp>
#pragma warning(pop)
#include <boost/assert.hpp>

using namespace std;
namespace
{
	using namespace KlayGE;

	float3 const lum_weight(0.299f, 0.587f, 0.114f);

	float CalcLum(float r, float g, float b)
	{
		float y = lum_weight.x() * r + lum_weight.y() * g + lum_weight.z() * b;
		if (abs(y) < 0.0001f)
		{
			if (y > 0)
			{
				y = 0.0001f;
			}
			else
			{
				y = -0.0001f;
			}
		}

		return y;
	}

	void CompressHDR(std::vector<uint16_t>& y_data, std::vector<uint8_t>& c_data,
		std::vector<float> const & hdr_data, uint32_t width, uint32_t height)
	{
		BOOST_ASSERT(hdr_data.size() >= width * height * 4);

		float const log2 = log(2.0f);

		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float R = hdr_data[(y * width + x) * 4 + 0];
				float G = hdr_data[(y * width + x) * 4 + 1];
				float B = hdr_data[(y * width + x) * 4 + 2];
				float Y = CalcLum(R, G, B);

				float log_y = log(Y) / log2 + 16;

				y_data[y * width + x] = static_cast<uint16_t>(MathLib::clamp<uint32_t>(static_cast<uint32_t>(log_y * 2048), 0, 65535));
			}
		}

		for (uint32_t y = 0; y < height / 2; ++ y)
		{
			uint32_t const y0 = y * 2 + 0;
			uint32_t const y1 = y * 2 + 1;

			for (uint32_t x = 0; x < width / 2; ++ x)
			{
				uint32_t const x0 = x * 2 + 0;
				uint32_t const x1 = x * 2 + 1;

				float R = hdr_data[(y0 * width + x0) * 4 + 0]
					+ hdr_data[(y0 * width + x1) * 4 + 0]
					+ hdr_data[(y1 * width + x0) * 4 + 0]
					+ hdr_data[(y1 * width + x1) * 4 + 0];
				float G = hdr_data[(y0 * width + x0) * 4 + 1]
					+ hdr_data[(y0 * width + x1) * 4 + 1]
					+ hdr_data[(y1 * width + x0) * 4 + 1]
					+ hdr_data[(y1 * width + x1) * 4 + 1];
				float B = hdr_data[(y0 * width + x0) * 4 + 2]
					+ hdr_data[(y0 * width + x1) * 4 + 2]
					+ hdr_data[(y1 * width + x0) * 4 + 2]
					+ hdr_data[(y1 * width + x1) * 4 + 2];
				float Y = CalcLum(R, G, B);
				
				float log_u = sqrt(lum_weight.z() * B / Y);
				float log_v = sqrt(lum_weight.x() * R / Y);

				c_data[(y * width / 2 + x) * 4 + 0] = 0;
				c_data[(y * width / 2 + x) * 4 + 1] = static_cast<uint8_t>(MathLib::clamp(log_u * 256 + 0.5f, 0.0f, 255.0f));
				c_data[(y * width / 2 + x) * 4 + 2] = 0;
				c_data[(y * width / 2 + x) * 4 + 3] = static_cast<uint8_t>(MathLib::clamp(log_v * 256 + 0.5f, 0.0f, 255.0f));
			}
		}
	}

	std::pair<TexturePtr, TexturePtr> CompressHDRCube(TexturePtr tex)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		uint32_t const size = tex->Width(0);
		TexturePtr y_cube_map = rf.MakeTextureCube(size, 1, EF_L16);
		TexturePtr c_cube_map = rf.MakeTextureCube(size / 2, 1, EF_BC3);
		std::vector<float> hdr_data(size * size * 4);
		std::vector<uint16_t> y_data(size * size);
		std::vector<uint8_t> c_data(size / 2 * size / 2 * 4);

		for (int i = 0; i < 6; ++ i)
		{
			{
				Texture::Mapper mapper(*tex, static_cast<Texture::CubeFaces>(i), 0, TMA_Read_Only, 0, 0, size, size);
				uint8_t* data = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < size; ++ y)
				{
					memcpy(&hdr_data[y * size * 4], data, size * tex->Bpp() / 8);
					data += mapper.RowPitch();
				}
			}

			CompressHDR(y_data, c_data, hdr_data, size, size);

			{
				Texture::Mapper mapper(*y_cube_map, static_cast<Texture::CubeFaces>(i), 0, TMA_Write_Only, 0, 0, size, size);
				uint8_t* data = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < size; ++ y)
				{
					memcpy(data, &y_data[y * size], size * y_cube_map->Bpp() / 8);
					data += mapper.RowPitch();
				}
			}
			{
				Texture::Mapper mapper(*c_cube_map, static_cast<Texture::CubeFaces>(i), 0, TMA_Write_Only, 0, 0, size / 2, size / 2);
				uint8_t* data = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < size; ++ y)
				{
					memcpy(data, &c_data[y * size / 2 * 4], size / 2 * c_cube_map->Bpp() / 8);
					data += mapper.RowPitch();
				}
			}
		}

		return std::make_pair(y_cube_map, c_cube_map);
	}

	std::pair<TexturePtr, TexturePtr> CompressHDR2D(TexturePtr tex)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		uint32_t const width = tex->Width(0);
		uint32_t const height = tex->Height(0);
		TexturePtr y_map = rf.MakeTexture2D(width, height, 1, EF_L16);
		TexturePtr c_map = rf.MakeTexture2D(width / 2, height / 2, 1, EF_BC3);
		std::vector<float> hdr_data(width * height * 4);
		std::vector<uint16_t> y_data(width * height);
		std::vector<uint8_t> c_data(width / 2 * height / 2 * 4);

		{
			Texture::Mapper mapper(*tex, 0, TMA_Read_Only, 0, 0, width, height);
			uint8_t* data = mapper.Pointer<uint8_t>();
			for (uint32_t y = 0; y < height; ++ y)
			{
				memcpy(&hdr_data[y * width * 4], data, width * tex->Bpp() / 8);
				data += mapper.RowPitch();
			}
		}

		{
			Texture::Mapper mapper(*y_map, 0, TMA_Write_Only, 0, 0, width, height);
			uint8_t* data = mapper.Pointer<uint8_t>();
			for (uint32_t y = 0; y < height; ++ y)
			{
				memcpy(data, &y_data[y * width], width * y_map->Bpp() / 8);
				data += mapper.RowPitch();
			}
		}
		{
			Texture::Mapper mapper(*c_map, 0, TMA_Write_Only, 0, 0, width / 2, height / 2);
			uint8_t* data = mapper.Pointer<uint8_t>();
			for (uint32_t y = 0; y < height; ++ y)
			{
				memcpy(data, &c_data[y * width / 2 * 4], width / 2 * c_map->Bpp() / 8);
				data += mapper.RowPitch();
			}
		}

		return std::make_pair(y_map, c_map);
	}

	std::pair<TexturePtr, TexturePtr> CompressHDR(TexturePtr tex)
	{
		std::pair<TexturePtr, TexturePtr> ret;

		switch (tex->Type())
		{
		case Texture::TT_2D:
			{
				TexturePtr temp = Context::Instance().RenderFactoryInstance().MakeTexture2D(
					tex->Width(0), tex->Height(0), 1, EF_ABGR32F);
				tex->CopyToTexture(*temp);
				ret = CompressHDR2D(temp);
			}
			break;

		case Texture::TT_Cube:
			{
				TexturePtr temp = Context::Instance().RenderFactoryInstance().MakeTextureCube(
					tex->Width(0), 1, EF_ABGR32F);
				tex->CopyToTexture(*temp);
				ret = CompressHDRCube(temp);
			}
			break;

		default:
			cout << "Unsupported texture type" << endl;
			break;
		}

		return ret;
	}
}

class EmptyApp : public KlayGE::App3DFramework
{
public:
	EmptyApp(std::string const & name, KlayGE::RenderSettings const & settings)
		: App3DFramework(name, settings)
	{
	}

	uint32_t DoUpdate(uint32_t /*pass*/)
	{
		return URV_Finished;
	}
};

int main(int argc, char* argv[])
{
	using namespace KlayGE;
	using namespace boost::filesystem;

	if (argc < 2)
	{
		cout << "使用方法: HDRCompressor xxx.dds" << endl;
		return 1;
	}

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;

	EmptyApp app("HDRCompressor", settings);
	app.Create();

	std::pair<TexturePtr, TexturePtr> new_texs = CompressHDR(LoadTexture(argv[1]));

	path output_path(argv[1]);
	std::string y_file = basename(output_path) + "_y" + extension(output_path);
	std::string c_file = basename(output_path) + "_c" + extension(output_path);
	SaveTexture(new_texs.first, y_file);
	SaveTexture(new_texs.second, c_file);

	cout << "HDR texture is compressed into " << y_file << " and " << c_file << endl;

	return 0;
}
