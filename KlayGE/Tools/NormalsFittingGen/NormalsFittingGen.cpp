#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Texture.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace KlayGE;

float quantize255(float c)
{
	int r = static_cast<int>(MathLib::clamp(c * 0.5f + 0.5f, 0.0f, 1.0f) * 255 + 0.5f);
	float v = r / 255.0f * 2 - 1;
	return v;
}

float find_minimum_quantization_error(float3 normal)
{
	normal /= max(abs(normal.x()), max(abs(normal.y()), abs(normal.z())));
	float min_error = 1e10f;
	float ret = 1;
	for (int step = 1; step < 128; ++ step)
	{
		float t = (step + 0.5f) / 127.5f;

		// compute the probe
		float3 p = normal * t;

		// quantize the probe
		float3 quantized_p = float3(quantize255(p.x()), quantize255(p.y()), quantize255(p.z()));

		// error computation for the probe
		float3 diff = (quantized_p - p) / t;
		float error = max(abs(diff.x()), max(abs(diff.y()), abs(diff.z())));

		// find the minimum
		if (error < min_error)
		{
			min_error = error;
			ret = t;
		}
	}
	return ret;
}

void gen_normals_fitting(std::vector<uint8_t>& fitting_map, int dim)
{
	fitting_map.resize(dim * dim);
	float3 normal;
	normal.z() = 1;
	for (int y = 0; y < dim; ++ y)
	{
		normal.y() = -(y + 0.5f) / dim;
		for (int x = 0; x < dim; ++ x)
		{
			normal.x() = (x + 0.5f) / dim;

			float quantized_length = find_minimum_quantization_error(normal);
			fitting_map[y * dim + x] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(quantized_length * 255 + 0.5f), 0, 255));
		}
	}
}

int main()
{
	int const HALF_DIM = 1024;

	std::vector<std::vector<uint8_t> > fitting_map;
	fitting_map.push_back(std::vector<uint8_t>());
	gen_normals_fitting(fitting_map.back(), HALF_DIM);
	
	int dim = HALF_DIM / 2;
	int mipmap = 1;
	while (dim > 1)
	{
		fitting_map.push_back(std::vector<uint8_t>());

		std::vector<uint8_t> const & last_level = fitting_map[fitting_map.size() - 2];
		std::vector<uint8_t>& this_level = fitting_map.back();

		this_level.resize(dim * dim);
		for (int y = 0; y < dim; ++ y)
		{
			for (int x = 0; x < dim; ++ x)
			{
				this_level[y * dim + x] = static_cast<uint8_t>((last_level[(y * 2 + 0) * dim * 2 + (x * 2 + 0)]
					+ last_level[(y * 2 + 0) * dim * 2 + (x * 2 + 1)]
					+ last_level[(y * 2 + 1) * dim * 2 + (x * 2 + 0)]
					+ last_level[(y * 2 + 1) * dim * 2 + (x * 2 + 1)]) / 4.0f + 0.5f);
			}
		}

		dim /= 2;
		++ mipmap;
	}

	dim = HALF_DIM;
	std::vector<ElementInitData> init_data(mipmap);
	for (int level = 0; level < mipmap; ++ level)
	{
		std::vector<uint8_t>& this_level = fitting_map[level];
		for (int y = 0; y < dim; ++ y)
		{
			for (int x = 0; x < y; ++ x)
			{
				this_level[y * dim + x] = 0;
			}
		}

		init_data[level].data = &this_level[0];
		init_data[level].slice_pitch = init_data[level].row_pitch = this_level.size();

		dim /= 2;
	}
	
	SaveTexture("normals_fitting.dds", Texture::TT_2D, HALF_DIM, HALF_DIM, 1, mipmap, 1, EF_R8, init_data);

	cout << "DONE" << endl;

	return 0;
}
