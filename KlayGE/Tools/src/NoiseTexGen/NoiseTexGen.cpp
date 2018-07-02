#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Noise.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace KlayGE;

#define OUTPUT_PATH "../../media/Textures/2D/"

static uint8_t const permutation[] =
{
	151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static uint8_t const grad3[] = 
{
	255, 255, 128, 128,
	0,   255, 128, 128,
	255, 0,   128, 128,
	0,   0,   128, 128,

	255, 128, 255, 128,
	0,   128, 255, 128,
	255, 128, 0,   128,
	0,   128, 0,   128,

	128, 255, 255, 128,
	128, 0,   255, 128,
	128, 255, 0,   128,
	128, 0,   0,   128
};

static uint8_t const grad4[] =
{
	128, 255, 255, 255,
	128, 255, 255, 0,
	128, 255, 0,   255,
	128, 255, 0,   0,

	128, 0,   255, 255,
	128, 0,   255, 0,
	128, 0,   0,   255,
	128, 0,   0,   0,
	
	255, 128, 255, 255,
	255, 128, 255, 0,
	255, 128, 0,   255,
	255, 128, 0,   0,
	
	0,   128, 255, 255,
	0,   128, 255, 0,
	0,   128, 0,   255,
	0,   128, 0,   0,
	
	255, 255, 128, 255,
	255, 255, 128, 0,
	255, 0,   128, 255,
	255, 0,   128, 0,
	
	0,   255, 128, 255,
	0,   255, 128, 0,
	0,   0,   128, 255,
	0,   0,   128, 0,
	
	255, 255, 255, 128,
	255, 255, 0,   128,
	255, 0,   255, 128,
	255, 0,   0,   128,
	
	0,   255, 255, 128,
	0,   255, 0,   128,
	0,   0,   255, 128,
	0,   0,   0,   128
};

static uint8_t simplex[][4] =
{
	{ 0, 1, 2, 3 },
	{ 0, 1, 3, 2 },
	{ 0, 0, 0, 0 },
	{ 0, 2, 3, 1 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 1, 2, 3, 0 },

	{ 0, 2, 1, 3 },
	{ 0, 0, 0, 0 },
	{ 0, 3, 1, 2 },
	{ 0, 3, 2, 1 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 1, 3, 2, 0 },

	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },

	{ 1, 2, 0, 3 },
	{ 0, 0, 0, 0 },
	{ 1, 3, 0, 2 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 2, 3, 0, 1 },
	{ 2, 3, 1, 0 },

	{ 1, 0, 2, 3 },
	{ 1, 0, 3, 2 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 2, 0, 3, 1 },
	{ 0, 0, 0, 0 },
	{ 2, 1, 3, 0 },

	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },

	{ 2, 0, 1, 3 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 3, 0, 1, 2 },
	{ 3, 0, 2, 1 },
	{ 0, 0, 0, 0 },
	{ 3, 1, 2, 0 },

	{ 2, 1, 0, 3 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 3, 1, 0, 2 },
	{ 0, 0, 0, 0 },
	{ 3, 2, 0, 1 },
	{ 3, 2, 1, 0 }
};

void GenSimplexPerm()
{
	std::vector<uint8_t> perm_2d(256 * 256);
	for (int y = 0; y < 256; ++ y)
	{
		for (int x = 0; x < 256; ++ x)
		{
			perm_2d[y * 256 + x] = permutation[(x + permutation[y & 255]) & 255];
		}
	}

	std::vector<ElementInitData> init_data(1);

	init_data[0].data = simplex;
	init_data[0].slice_pitch = init_data[0].row_pitch = sizeof(simplex);
	SaveTexture(OUTPUT_PATH "noise_simplex.dds", Texture::TT_2D,
		64, 1, 1, 1, 1, EF_ABGR8, init_data);

	std::vector<uint8_t> grad_perm(256 * 256 * 4);
	for (int y = 0; y < 256; ++ y)
	{
		for (int x = 0; x < 256; ++ x)
		{
			int index = perm_2d[y * 256 + x] % 12;
			grad_perm[(y * 256 + x) * 4 + 0] = grad3[index * 4 + 0];
			grad_perm[(y * 256 + x) * 4 + 1] = grad3[index * 4 + 1];
			grad_perm[(y * 256 + x) * 4 + 2] = grad3[index * 4 + 2];
			grad_perm[(y * 256 + x) * 4 + 3] = perm_2d[y * 256 + x];
		}
	}
	init_data[0].data = &grad_perm[0];
	init_data[0].row_pitch = 256 * 4;
	init_data[0].slice_pitch = static_cast<uint32_t>(grad_perm.size() * sizeof(grad_perm[0]));
	SaveTexture(OUTPUT_PATH "noise_grad3_perm.dds", Texture::TT_2D,
		256, 256, 1, 1, 1, EF_ABGR8, init_data);

	for (int y = 0; y < 256; ++ y)
	{
		for (int x = 0; x < 256; ++ x)
		{
			int index = perm_2d[y * 256 + x] % 32;
			grad_perm[(y * 256 + x) * 4 + 0] = grad4[index * 4 + 0];
			grad_perm[(y * 256 + x) * 4 + 1] = grad4[index * 4 + 1];
			grad_perm[(y * 256 + x) * 4 + 2] = grad4[index * 4 + 2];
			grad_perm[(y * 256 + x) * 4 + 3] = grad4[index * 4 + 3];
		}
	}
	init_data[0].data = &grad_perm[0];
	init_data[0].row_pitch = 256 * 4;
	init_data[0].slice_pitch = static_cast<uint32_t>(grad_perm.size() * sizeof(grad_perm[0]));
	SaveTexture(OUTPUT_PATH "noise_grad4_perm.dds", Texture::TT_2D,
		256, 256, 1, 1, 1, EF_ABGR8, init_data);
}

void GenfBmTexs()
{
	uint32_t const TEX_SIZE = 512;
	float const STRIDE = 8;

	MathLib::SimplexNoise<float> noiser = MathLib::SimplexNoise<float>::Instance();

	std::vector<float> fdata(TEX_SIZE * TEX_SIZE);
	float min_v = +1e10f;
	float max_v = -1e10f;
	for (uint32_t y = 0; y < TEX_SIZE; ++ y)
	{
		for (uint32_t x = 0; x < TEX_SIZE; ++ x)
		{
			float v = noiser.tileable_fBm((x + 0.5f) / TEX_SIZE * STRIDE, (y + 0.5f) / TEX_SIZE * STRIDE, STRIDE, STRIDE, 5, 2, 0.5f);
			fdata[y * TEX_SIZE + x] = v;
			min_v = std::min(min_v, v);
			max_v = std::max(max_v, v);
		}
	}
	float inv_range = 1 / (max_v - min_v);
	std::vector<uint8_t> data(TEX_SIZE * TEX_SIZE);
	for (uint32_t i = 0; i < data.size(); ++ i)
	{
		float v = (fdata[i] - min_v) * inv_range;
		data[i] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(v * 255 + 0.5f), 0, 255));
	}
	std::vector<ElementInitData> init_data(1);
	init_data[0].data = &data[0];
	init_data[0].row_pitch = TEX_SIZE;
	init_data[0].slice_pitch = TEX_SIZE * TEX_SIZE;
	SaveTexture(OUTPUT_PATH "fBm5_tex.dds", Texture::TT_2D,
		TEX_SIZE, TEX_SIZE, 1, 1, 1, EF_R8, init_data);

	int err = system("Mipmapper " OUTPUT_PATH "fBm5_tex.dds");
	KFL_UNUSED(err);
	err = system("TexCompressor BC4 " OUTPUT_PATH "fBm5_tex.dds");
	KFL_UNUSED(err);

	std::vector<float3> fdata3(TEX_SIZE * TEX_SIZE);
	for (uint32_t y = 0; y < TEX_SIZE; ++ y)
	{
		for (uint32_t x = 0; x < TEX_SIZE; ++ x)
		{
			float f0 = fdata[y * TEX_SIZE + x];
			float d = 2;
			float fx = noiser.tileable_fBm((x + d + 0.5f) / TEX_SIZE * STRIDE, (y + 0.5f) / TEX_SIZE * STRIDE, STRIDE, STRIDE, 5, 2, 0.5f);
			float fy = noiser.tileable_fBm((x + 0.5f) / TEX_SIZE * STRIDE, (y + d + 0.5f) / TEX_SIZE * STRIDE, STRIDE, STRIDE, 5, 2, 0.5f);
			fdata3[y * TEX_SIZE + x] = MathLib::normalize(float3(fx - f0, fy - f0, STRIDE * 16 / TEX_SIZE)) * 0.5f + 0.5f;
		}
	}
	std::vector<uint32_t> data3(TEX_SIZE * TEX_SIZE);
	for (uint32_t i = 0; i < fdata3.size(); ++ i)
	{
		data3[i] = (static_cast<uint8_t>(MathLib::clamp(static_cast<int>(fdata3[i].x() * 255 + 0.5f), 0, 255)) << 0)
			| (static_cast<uint8_t>(MathLib::clamp(static_cast<int>(fdata3[i].y() * 255 + 0.5f), 0, 255)) << 8);
	}
	init_data[0].data = &data3[0];
	init_data[0].row_pitch = TEX_SIZE * sizeof(uint32_t);
	init_data[0].slice_pitch = TEX_SIZE * sizeof(uint32_t)* TEX_SIZE;
	SaveTexture(OUTPUT_PATH "fBm5_grad_tex.dds", Texture::TT_2D,
		TEX_SIZE, TEX_SIZE, 1, 1, 1, EF_ABGR8, init_data);

	err = system("Mipmapper " OUTPUT_PATH "fBm5_grad_tex.dds");
	KFL_UNUSED(err);
	err = system("TexCompressor BC5 " OUTPUT_PATH "fBm5_grad_tex.dds");
	KFL_UNUSED(err);
}

int main()
{
	cout << "Generating textures for simplex..." << endl;
	GenSimplexPerm();

	cout << "Generating fBm textures..." << endl;
	GenfBmTexs();

	cout << "DONE" << endl;

	return 0;
}
