// HeightMap.cpp
// KlayGE HeightMap地形生成类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2006-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// 合并入Core (2010.8.21)
//
// 3.4.0
// 初次建立 (2006.7.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Vector.hpp>

#include <KlayGE/HeightMap.hpp>

namespace KlayGE
{
	void HeightMap::BuildTerrain(float start_x, float start_y, float end_x, float end_y, float span_x, float span_y,
		std::vector<float3>& vertices, std::vector<uint16_t>& indices,
		std::function<float(float, float)> HeightFunc)
	{
		vertices.resize(0);
		indices.resize(0);

		if ((end_x - start_x) * span_x < 0)
		{
			span_x = -span_x;
		}
		if ((end_y - start_y) * span_y < 0)
		{
			span_y = -span_y;
		}

		uint16_t const num_x = static_cast<uint16_t>((end_x - start_x) / span_x);
		uint16_t const num_y = static_cast<uint16_t>((end_y - start_y) / span_y);

		float pos_x = start_x;
		float pos_y = start_y;
		for (uint16_t y = 0; y < num_y; ++ y)
		{
			pos_x = start_x;
			for (uint16_t x = 0; x < num_x; ++ x)
			{
				pos_x += span_x;

				float3 vec(pos_x, HeightFunc(pos_x, pos_y), pos_y);
				vertices.push_back(vec);
			}
			pos_y += span_y;
		}

		for (uint16_t y = 0; y < num_y - 1; ++ y)
		{
			for (uint16_t x = 0; x < num_x - 1; ++ x)
			{
				indices.push_back((y + 0) * num_x + (x + 0));
				indices.push_back((y + 1) * num_x + (x + 0));
				indices.push_back((y + 1) * num_x + (x + 1));

				indices.push_back((y + 1) * num_x + (x + 1));
				indices.push_back((y + 0) * num_x + (x + 1));
				indices.push_back((y + 0) * num_x + (x + 0));
			}
		}
	}
}
