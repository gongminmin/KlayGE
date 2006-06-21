// Distance map生成器
// 算法主体来自GPU Gems 2第8章
//

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

// A type to hold the distance map while it's being constructed
template <typename DistanceType>
class DistanceMap
{
public:
	DistanceMap(int width, int height, int depth)
		: data_(width * height * depth * 3),
			width_(width), height_(height), depth_(depth)
	{
	}

	DistanceType& operator()(int x, int y, int z, int i)
	{
		return data_[((z * height_ + y) * width_ + x) * 3 + i];
	}

	DistanceType const & operator()(int x, int y, int z, int i) const
	{
		return data_[((z * height_ + y) * width_ + x) * 3 + i];
	}

	bool inside(int x, int y, int z) const
	{
		return (0 <= x) && (x < width_)
			&& (0 <= y) && (y < height_)
			&& (0 <= z) && (z < depth_); 
	}

	// Do a single pass over the data.
	// Start at (x,y,z) and walk in the direction (cx,cy,cz)
	// Combine each pixel (x,y,z) with the value at (x+dx,y+dy,z+dz)
	void combine(int dx, int dy, int dz,
		int cx, int cy, int cz,
		int x, int y, int z)
	{
		while (inside(x, y, z) && inside(x + dx, y + dy, z + dz))
		{
			int d[3] =
			{
				abs(dx),
				abs(dy),
				abs(dz)
			};

			uint32_t v1[3], v2[3];
			for (int i = 0; i < 3; ++ i)
			{
				v1[i] = operator()(x, y, z, i);
				v2[i] = operator()(x + dx, y + dy, z + dz, i) + d[i];
			}

			if (v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2] > v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2])
			{
				for (int i = 0; i < 3; ++ i)
				{
					operator()(x, y, z, i) = static_cast<DistanceType>(v2[i]);
				}
			}

			x += cx;
			y += cy;
			z += cz;
		}
	}

private:
	std::vector<DistanceType> data_;
	int width_, height_, depth_;
};

void ComputeDistanceField(std::vector<uint8_t>& distances, int width, int height, int depth,
						std::vector<uint8_t> const & volume)
{
	// and +infinity above
	DistanceMap<uint16_t> dmap(width, height, depth);
	for (int z = 0; z < depth; ++ z)
	{
		for (int y = 0; y < height; ++ y)
		{
			for (int x = 0; x < width; ++ x)
			{
				for (int i = 0; i < 3; ++ i)
				{
					if (volume[(z * height + y) * width + x] != 0)
					{
						dmap(x, y, z, i) = 0;
					}
					else
					{
						dmap(x, y, z, i) = std::numeric_limits<uint16_t>::max();
					}
				}
			}
		}
	}


	// Compute the rest of dmap by sequential sweeps over the data
	// using a 3d variant of Danielsson's algorithm

	for (int z = 1; z < depth; ++ z)
	{
		// combine with everything with dz = -1
		for (int y = 0; y < height; ++ y)
		{
			dmap.combine(0, 0, -1,
				1, 0, 0,
				0, y, z);
		}

		for (int y = 1; y < height; ++ y)
		{
			dmap.combine(0, -1, 0,
				1, 0, 0,
				0, y, z);
			dmap.combine(-1, 0, 0,
				1, 0, 0,
				1, y, z);
			dmap.combine(+1, 0, 0,
				-1, 0, 0,
				width - 2, y, z);
		}

		for (int y = height - 1; y >= 0; -- y)
		{
			dmap.combine(0, +1, 0,
				1, 0, 0,
				0, y, z);
			dmap.combine(-1, 0, 0,
				1, 0, 0,
				1, y, z);
			dmap.combine(+1, 0, 0,
				-1, 0, 0,
				width - 1, y, z);
		}
	}

	for (int z = depth - 1; z >= 0; -- z)
	{
		cout << ".";

		// combine with everything with dz = +1
		for (int y = 0; y < height; ++ y)
		{
			dmap.combine(0, 0, +1,
				1, 0, 0,
				0, y, z);
		}

		for (int y = 1; y < height; ++ y)
		{
			dmap.combine(0, -1, 0,
				1, 0, 0,
				0, y, z);
			dmap.combine(-1, 0, 0,
				1, 0, 0,
				1, y, z);
			dmap.combine(+1, 0, 0,
				-1, 0, 0,
				width - 2, y, z);
		}
		for (int y = height - 1; y >= 0; -- y)
		{
			dmap.combine(0, +1, 0,
				1, 0, 0,
				0, y, z);
			dmap.combine(-1, 0, 0,
				1, 0, 0,
				1, y, z);
			dmap.combine(+1, 0, 0,
				-1, 0, 0,
				width - 1, y, z);
		}
	}

	for (int z = 0; z < depth; ++ z)
	{
		for (int y = 0; y < height; ++ y)
		{
			for (int x = 0; x < width; ++ x)
			{
				float value = 0;
				for (int i = 0; i < 3; ++ i)
				{
					value += dmap(x, y, z, i) * dmap(x, y, z, i);
				}
				distances[(z * height + y) * width + x]
					= static_cast<uint8_t>(MathLib::clamp(sqrt(value) / depth, 0.0f, 1.0f) * 255);
			}
		}
	}
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

	std::string src_name("height.dds");
	std::string distance_name("distance.dds");

	if (argc > 1)
	{
		src_name = argv[1];
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

	std::vector<uint8_t> volume(width * height * depth);
	TexturePtr src_texture = LoadTexture(src_name);
	if (Texture::TT_2D == src_texture->Type())
	{
		TexturePtr height_map_texture = render_factory.MakeTexture2D(width, height, 1, EF_L8);
		src_texture->CopyToTexture(*height_map_texture);
		
		std::vector<uint8_t> height_map(height * width);
		height_map_texture->CopyToMemory2D(0, &height_map[0]);

		for (int z = 0; z < depth; ++ z)
		{
			for (int y = 0; y < height; ++ y)
			{
				for (int x = 0; x < width; ++ x)
				{
					if (height_map[y * width + x] >= z * (256 / depth))
					{
						volume[z * width * height + y * width + x] = 255;
					}
					else
					{
						volume[z * width * height + y * width + x] = 0;
					}
				}
			}
		}
	}
	else
	{
		BOOST_ASSERT(Texture::TT_3D == temp_texture->Type());

		TexturePtr vol_map_texture = render_factory.MakeTexture3D(width, height, depth, 1, EF_L8);
		src_texture->CopyToTexture(*vol_map_texture);

		vol_map_texture->CopyToMemory3D(0, &volume[0]);
	}

	clock_t start = clock();

	std::vector<uint8_t> distances(width * height * depth);
	ComputeDistanceField(distances, width, height, depth, volume);

	cout << endl << "Computing time: " << clock() - start << " ms" << endl;

	TexturePtr distance_map_texture = render_factory.MakeTexture3D(width, height, depth, 1, EF_L8);
	distance_map_texture->CopyMemoryToTexture3D(0, &distances[0], EF_L8,
		width, height, depth, 0, 0, 0, width, height, depth);
	SaveTexture(distance_map_texture, distance_name);

	cout << "Distance map is saved to " << distance_name << endl;
}
