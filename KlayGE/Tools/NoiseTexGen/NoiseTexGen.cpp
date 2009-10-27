#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Texture.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace KlayGE;

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
	128, 255, 255, 128,
	128, 255, 0,   128,
	128, 0,   255, 128,
	128, 0,   0,   128,
	255, 128, 255, 128,
	255, 128, 0,   128,
	0,   128, 255, 128,
	0,   128, 0,   128,
	255, 255, 128, 128,
	255, 0,   128, 128,
	0,   255, 128, 128,
	0,   0,   128, 128,
	128, 255, 255, 128,
	255, 0,   128, 128,
	128, 255, 0,   128,
	0,   0,   128, 128
};

static uint8_t const grad4[] =
{
	128, 0, 0, 0,
	128, 0, 0, 255,
	128, 0, 255, 0,
	128, 0, 255, 255,
	128, 255, 0, 0,
	128, 255, 0, 255,
	128, 255, 255, 0,
	128, 255, 255, 255,
	0, 0, 128, 0,
	0, 255, 128, 0,
	255, 0, 128, 0,
	255, 255, 128, 0,
	0, 0, 128, 255,
	0, 255, 128, 255,
	255, 0, 128, 255,
	255, 255, 128, 255,
	
	0, 128, 0, 0,
	255, 128, 0, 0,
	0, 128, 0, 255,
	255, 128, 0, 255,
	0, 128, 255, 0,
	255, 128, 255, 0,
	0, 128, 255, 255,
	255, 128, 255, 255,
	128, 0, 0, 128,
	128, 0, 0, 128,
	128, 0, 255, 128,
	128, 0, 255, 128,
	128, 255, 0, 128,
	128, 255, 0, 128,
	128, 255, 255, 128,
	128, 255, 255, 128,
};

uint8_t perm_2d[256][256 * 4];

int main()
{
	for (int y = 0; y < 256; ++ y)
	{
		for (int x = 0; x < 256; ++ x)
		{
			int A = permutation[x & 255] + y;
			int B = permutation[(x + 1) & 255] + y;
			perm_2d[y][x * 4 + 2] = permutation[A & 255];
			perm_2d[y][x * 4 + 1] = permutation[(A + 1) & 255];
			perm_2d[y][x * 4 + 0] = permutation[B & 255];
			perm_2d[y][x * 4 + 3] = permutation[(B + 1) & 255];
		}
	}

	uint8_t grad3_perm[256 * 4];
	for (int x = 0; x < 256; ++ x)
	{
		for (int i = 0; i < 4; ++ i)
		{
			grad3_perm[x * 4 + i] = grad3[(permutation[x] & 15) * 4 + i];
		}
	}

	uint8_t grad4_perm[256 * 4];
	for (int x = 0; x < 256; ++ x)
	{
		for (int i = 0; i < 4; ++ i)
		{
			grad4_perm[x * 4 + i] = grad4[(permutation[x] & 31) * 4 + i];
		}
	}
	
	std::vector<ElementInitData> init_data(1);

	init_data[0].data = permutation;
	init_data[0].slice_pitch = init_data[0].row_pitch = sizeof(permutation);
	SaveTexture("noise_perm.dds", Texture::TT_2D,
		256, 1, 1, 1, 1, EF_R8, init_data);

	init_data[0].data = perm_2d;
	init_data[0].row_pitch = 256 * 4;
	init_data[0].slice_pitch = sizeof(perm_2d);
	SaveTexture("noise_perm_2d.dds", Texture::TT_2D,
		256, 256, 1, 1, 1, EF_ARGB8, init_data);

	init_data[0].data = grad3_perm;
	init_data[0].slice_pitch = init_data[0].row_pitch = sizeof(grad3_perm);
	SaveTexture("noise_grad3_perm.dds", Texture::TT_2D,
		256, 1, 1, 1, 1, EF_ARGB8, init_data);

	init_data[0].data = grad4_perm;
	init_data[0].slice_pitch = init_data[0].row_pitch = sizeof(grad4_perm);
	SaveTexture("noise_grad4_perm.dds", Texture::TT_2D,
		256, 1, 1, 1, 1, EF_ARGB8, init_data);

	cout << "DONE" << endl;

	return 0;
}
