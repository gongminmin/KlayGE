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

float3 find_minimum_quantization_error(float3 normal)
{
	normal /= max(abs(normal.x()), max(abs(normal.y()), abs(normal.z())));
	float min_error = 1e10f;
	float3 ret = normal;
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
			ret = quantized_p;
		}
	}
	return ret;
}

void gen_normals_fitting(std::vector<uint8_t>& fitting_map, int dim)
{
	std::vector<uint8_t> length_map(dim * dim);
	float3 normal;
	normal.z() = 1;
	for (int y = 0; y < dim; ++ y)
	{
		normal.y() = -(y + 0.5f) / dim;
		for (int x = 0; x < dim; ++ x)
		{
			normal.x() = (x + 0.5f) / dim;

			float3 quantized_normal = find_minimum_quantization_error(MathLib::normalize(normal));
			length_map[y * dim + x] = static_cast<uint8_t>(MathLib::clamp(MathLib::length(quantized_normal), 0.0f, 1.0f) * 255 + 0.5f);
		}
	}

	fitting_map.resize(dim * dim);
	for (int y = 0; y < dim; ++ y)
	{
		for (int x = 0; x < dim; ++ x)
		{
			float fx = static_cast<float>(x) / dim;
			float fy = static_cast<float>(y) / dim;
			fy *= fx;

			int old_x = MathLib::clamp(static_cast<int>(fx * dim + 0.5f), 0, dim - 1);
			int old_y = MathLib::clamp(static_cast<int>(fy * dim + 0.5f), 0, dim - 1);
			fitting_map[y * dim + x] = length_map[old_y * dim + old_x];
		}
	}
}

int main()
{
	int const HALF_DIM = 1024;

	std::vector<std::vector<uint8_t> > fitting_map;
	std::vector<ElementInitData> init_data;

	int dim = HALF_DIM;
	int mipmap = 0;
	while (dim > 1)
	{
		fitting_map.push_back(std::vector<uint8_t>());
		init_data.push_back(ElementInitData());

		gen_normals_fitting(fitting_map.back(), dim);
	
		init_data.back().data = &fitting_map.back()[0];
		init_data.back().slice_pitch = init_data.back().row_pitch = fitting_map.back().size();

		dim /= 2;
		++ mipmap;
	}
	
	SaveTexture("normals_fitting.dds", Texture::TT_2D, HALF_DIM, HALF_DIM, 1, mipmap, 1, EF_R8, init_data);

	cout << "DONE" << endl;

	return 0;
}
