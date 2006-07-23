// Heightmap.hpp
// KlayGE Heightmap地形生成类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://klayge.sourceforge.net
//
// 2.0.0
// 初次建立 (2003.10.5)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _HEIGHTMAP_HPP
#define _HEIGHTMAP_HPP

#include <vector>
#include <KlayGE/Vector.hpp>

namespace KlayGE
{
	// 高度图地形生成
	/////////////////////////////////////////////////////////////////////////////////
	class Heightmap
	{
	public:
		template <typename HeightFunc>
		void Terrain(float start_x, float start_y, float end_x, float end_y, float span_x, float span_y,
			std::vector<float3>& vertices, std::vector<uint16_t>& indices,
			HeightFunc& Height)
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

			int const num_x = static_cast<int>((end_x - start_x) / span_x);
			int const num_y = static_cast<int>((end_y - start_y) / span_y);

			float pos_x = start_x;
			float pos_y = start_y;
			for (int y = 0; y < num_y; ++ y)
			{
				pos_x = start_x;
				for (int x = 0; x < num_x; ++ x)
				{
					pos_x += span_x;

					float3 vec(pos_x, Height(pos_x, pos_y), pos_y);
					vertices.push_back(vec);
				}
				pos_y += span_y;
			}

			for (int y = 0; y < num_y - 1; ++ y)
			{
				for (int x = 0; x < num_x - 1; ++ x)
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
	};
}

#endif		// _HEIGHTMAP_HPP

