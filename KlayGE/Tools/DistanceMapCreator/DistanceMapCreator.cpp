#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <ctime>
using namespace std;

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
using namespace KlayGE;

//-----------------------------------------------------------------------------
// Name: ComputeDistanceMap()
// Desc: Compute the distance map from height map
//-----------------------------------------------------------------------------
void ComputeDistanceMap(std::vector<unsigned char>& distances, int width, int height, int depth,
						std::vector<unsigned char> const & height_map)
{
	std::vector<unsigned char> heights(height_map);
	for (int y = 0; y < height; ++ y)
	{
		for (int x = 0; x < width; ++ x)
		{
			heights[y * width + x]
				= static_cast<unsigned char>((heights[y * width + x] / (256.0f / depth)) + 0.5f);
		}
	}

	clock_t start = clock();

	distances.resize(depth * height * width);
	for (int z = 0; z < depth; ++ z)
	{
		cout << "z = " << z << endl;
		for (int y = 0; y < height; ++ y)
		{
			for (int x = 0; x < width; ++ x)
			{
				if (heights[y * width + x] < z)
				{
					int const max_depth = z - heights[y * width + x];
					int dis_sq = max_depth * max_depth;

					int const y_begin = std::max(0, y - max_depth);
                    int const y_end = std::min(height, y + max_depth);
					for (int y1 = y_begin; y1 < y_end; ++ y1)
					{
						int const dy = (y1 - y) * (y1 - y);
						if (dy < dis_sq)
						{
							int const x_begin = std::max(0, x - max_depth);
							int const x_end = std::min(width, x + max_depth);
							for (int x1 = x_begin; x1 < x_end; ++ x1)
							{
								int const dx = (x1 - x) * (x1 - x);
								if (dx + dy < dis_sq)
								{
									int const dz = (heights[y1 * width + x1] - z) * (heights[y1 * width + x1] - z);
									if (dx + dy + dz < dis_sq)
									{
										dis_sq = dx + dy + dz;
									}
								}
							}
						}
					}

					float const dist = MathLib::Clamp(std::sqrt(static_cast<float>(dis_sq)) / (depth / 256.0f),
						0.0f, 255.0f);
					distances[(z * height + y) * width + x] = static_cast<unsigned char>(dist + 0.5f);
				}
				else
				{
					distances[(z * height + y) * width + x] = 0;
				}
			}
		}
	}

	cout << clock() - start << endl;
}

class EmptyApp : public KlayGE::App3DFramework
{
public:
	void DoUpdate(uint32_t /*pass*/)
	{
	}
};

int main(int argc, char* argv[])
{
	int width = 256, height = 256, depth = 16;

	std::string height_name("height.dds");
	std::string distance_name("distance.dds");

	if (argc > 1)
	{
		height_name = argv[1];
	}
	if (argc > 2)
	{
		distance_name = argv[2];
	}
	if (argc > 3)
	{
		std::stringstream ss(argv[3]);
		ss >> width;
	}
	if (argc > 4)
	{
		std::stringstream ss(argv[4]);
		ss >> height;
	}
	if (argc > 5)
	{
		std::stringstream ss(argv[5]);
		ss >> depth;
	}

	EmptyApp app;

	RenderFactory& render_factory(D3D9RenderFactoryInstance());
	Context::Instance().RenderFactoryInstance(render_factory);

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("DistanceMapCreator", settings);

	TexturePtr height_map_texture = render_factory.MakeTexture2D(width, height, 1, PF_L8);
	{
		TexturePtr temp_texture = LoadTexture(height_name);
		temp_texture->CopyToTexture(*height_map_texture);
	}

	std::vector<unsigned char> height_map(height * width);
	height_map_texture->CopyToMemory2D(0, &height_map[0]);

	std::vector<unsigned char> distances(depth * height * width);
	ComputeDistanceMap(distances, 256, 256, 16, height_map);

	TexturePtr distance_map_texture = render_factory.MakeTexture3D(width, height, depth, 1, PF_L8);
	distance_map_texture->CopyMemoryToTexture3D(0, &distances[0], PF_L8,
		width, height, depth, 0, 0, 0, width, height, depth);
	SaveToFile(distance_map_texture, distance_name);

	cout << "Distance map is saved to " << distance_name << endl;
}
