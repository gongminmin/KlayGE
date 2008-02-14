#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/Math.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 4251 4275 4273 4512 4701)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "freetype235_D.lib")
#else
	#pragma comment(lib, "freetype235.lib")
#endif
#endif

#include "kdtree.hpp"

using namespace std;
using namespace KlayGE;

float const SCALE = 60;
int const INTERNAL_CHAR_SIZE = 4096;
int const RESTORE_SIZE = 12;
int const NUM_CHARS = 65536;

#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(push, 1)
#endif
struct kfont_header
{
	uint32_t fourcc;
	uint32_t version;
	uint32_t start_ptr;
	uint32_t non_empty_chars;
	uint32_t char_size;
};
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(pop)
#endif

int main(int argc, char* argv[])
{
	kfont_header header;
	header.fourcc = MakeFourCC<'K', 'F', 'N', 'T'>::value;
	header.version = 1;
	header.start_ptr = sizeof(header);
	header.non_empty_chars = 0;
	header.char_size = 32;

	std::string ttf_name;
	std::string kfont_name;
	int start_code;
	int end_code;
	uint32_t char_size;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help", "Produce help message")
		("input-name,I", boost::program_options::value<std::string>(), "Input font name.")
		("output-name,O", boost::program_options::value<std::string>(), "Output font name. Optional.")
		("start-code,S", boost::program_options::value<int>(&start_code)->default_value(0), "Start code.")
		("end-code,E", boost::program_options::value<int>(&end_code)->default_value(65536), "End code.")
		("char-size,C", boost::program_options::value<uint32_t>(&char_size)->default_value(32), "Character size.");
	header.char_size = char_size;

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if (vm.count("help"))
	{
		cout << desc << endl;
		return 1;
	}
	if (vm.count("input-name"))
	{
		ttf_name = vm["input-name"].as<std::string>();
	}
	else
	{
		cout << "Input font name was not set." << endl;
		return 1;
	}
	if (vm.count("output-name"))
	{
		kfont_name = vm["output-name"].as<std::string>();
	}
	else
	{
		kfont_name = ttf_name.substr(0, ttf_name.find_last_of('.')) + ".kfont";
	}

	cout << "\tInput font name: " << ttf_name << endl;
	cout << "\tOutput font name: " << kfont_name << endl;
	cout << "\tStart code: " << start_code << endl;
	cout << "\tEnd code: " << end_code << endl;
	cout << "\tCharacter size: " << header.char_size << endl;
	cout << endl;

	std::vector<uint8_t> char_width(NUM_CHARS, 0);
	std::vector<int32_t> char_index(NUM_CHARS, -1);
	std::vector<uint8_t> char_dist;
	{
		ifstream kfont_input(kfont_name.c_str(), ios_base::binary);
		if (kfont_input)
		{
			kfont_input.read(reinterpret_cast<char*>(&header), sizeof(header));

			if (header.fourcc != MakeFourCC<'K', 'F', 'N', 'T'>::value)
			{
				cout << "Wrong font file." << endl;
				return 1;
			}
			if (header.version != 1)
			{
				cout << "Wrong version." << endl;
				return 1;
			}

			kfont_input.seekg(header.start_ptr, ios_base::beg);

			kfont_input.read(reinterpret_cast<char*>(&char_width[0]),
				static_cast<std::streamsize>(char_width.size() * sizeof(char_width[0])));
			kfont_input.read(reinterpret_cast<char*>(&char_index[0]),
				static_cast<std::streamsize>(char_index.size() * sizeof(char_index[0])));

			char_dist.resize(header.non_empty_chars * header.char_size * header.char_size);
			kfont_input.read(reinterpret_cast<char*>(&char_dist[0]),
				static_cast<std::streamsize>(char_dist.size() * sizeof(char_dist[0])));
		}
	}

	std::vector<uint8_t> ttf;
	{
		ifstream ttf_input(ttf_name.c_str(), ios_base::binary);
		ttf_input.seekg(0, ios_base::end);
		ttf.resize(ttf_input.tellg());
		ttf_input.seekg(0, ios_base::beg);
		ttf_input.read(reinterpret_cast<char*>(&ttf[0]),
			static_cast<std::streamsize>(ttf.size() * sizeof(ttf[0])));
	}

	::FT_Library	ft_lib;
	::FT_Face		ft_face;
	::FT_GlyphSlot	ft_slot;

	::FT_Init_FreeType(&ft_lib);
	::FT_New_Memory_Face(ft_lib, &ttf[0], static_cast<FT_Long>(ttf.size()), 0, &ft_face);
	::FT_Set_Pixel_Sizes(ft_face, 0, INTERNAL_CHAR_SIZE - 2);
	::FT_Select_Charmap(ft_face, FT_ENCODING_UNICODE);
	ft_slot = ft_face->glyph;

	Timer timer;
	double start_time = timer.current_time();

	int const dxdy[] = { 0, -1,  -1, 0,  +1, 0,  0, +1 };

	std::vector<uint8_t> char_bitmap(INTERNAL_CHAR_SIZE * INTERNAL_CHAR_SIZE);
	std::vector<int2> edge_points;
	for (int ch = start_code; ch < end_code; ++ ch)
	{
		::FT_Load_Char(ft_face, ch, FT_LOAD_RENDER);

		int const buf_width = std::min(ft_slot->bitmap.width, INTERNAL_CHAR_SIZE - 2);
		int const buf_height = std::min(ft_slot->bitmap.rows, INTERNAL_CHAR_SIZE - 2);
		
		char_width[ch] = static_cast<uint8_t>(std::min(char_size, 
			static_cast<uint32_t>(((0 != buf_width) ? buf_width : INTERNAL_CHAR_SIZE / 2) * header.char_size / static_cast<float>(INTERNAL_CHAR_SIZE) + 2.5f)));

		std::fill(char_bitmap.begin(), char_bitmap.end(), 0);

		int const y_start = std::max<int>(INTERNAL_CHAR_SIZE * 3 / 4 - ft_slot->metrics.horiBearingY / 64, 0) + 1;
		if ((buf_width > 0) && (buf_height > 0))
		{
			uint8_t* font_data = &char_bitmap[y_start * INTERNAL_CHAR_SIZE + 1];
			for (int y = 0; y < buf_height; ++ y)
			{
				for (int x = 0; x < buf_width; ++ x)
				{
					if (ft_slot->bitmap.buffer[y * buf_width + x] >= 128)
					{
						font_data[x] = 255;
					}
				}
				font_data += INTERNAL_CHAR_SIZE;
			}
		}

		edge_points.resize(0);
		for (int y = y_start, y_end = buf_height + y_start; y < y_end; ++ y)
		{
			for (int x = 1; x < buf_width + 1; ++ x)
			{
				if (char_bitmap[y * INTERNAL_CHAR_SIZE + x] != 0)
				{
					for (int i = 0; i < 4; ++ i)
					{
						int const dx = x + dxdy[i * 2 + 0];
						int const dy = y + dxdy[i * 2 + 1];
						if (0 == char_bitmap[dy * INTERNAL_CHAR_SIZE + dx])
						{
							edge_points.push_back(int2(x, y));
							break;
						}
					}
				}
			}
		}

		if (!edge_points.empty())
		{
			if (-1 == char_index[ch])
			{
				char_index[ch] = header.non_empty_chars;
				++ header.non_empty_chars;
				char_dist.resize(char_dist.size() + header.char_size * header.char_size);
			}

			kdtree<int2> kd(&edge_points[0], edge_points.size());

			uint32_t start_pos = char_index[ch] * header.char_size * header.char_size;
			int max_dist_sq = 2 * INTERNAL_CHAR_SIZE * INTERNAL_CHAR_SIZE;
			for (uint32_t y = 0; y < header.char_size; ++ y)
			{
				for (uint32_t x = 0; x < header.char_size; ++ x)
				{
					int const map_x = x * INTERNAL_CHAR_SIZE / header.char_size + INTERNAL_CHAR_SIZE / header.char_size / 2;
					int const map_y = y * INTERNAL_CHAR_SIZE / header.char_size + INTERNAL_CHAR_SIZE / header.char_size / 2;
					kd.query_position(int2(map_x, map_y), 1);
					float v = MathLib::sqrt(static_cast<float>(kd.squared_distance(0)) / max_dist_sq);
					float value = MathLib::clamp(v * SCALE, 0.0f, 1.0f);
					if (0 == char_bitmap[map_y * INTERNAL_CHAR_SIZE + map_x])
					{
						value = -value;
					}

					char_dist[start_pos + y * header.char_size + x] = static_cast<uint8_t>((value / 2 + 0.5f) * 255);
				}
			}
		}

		static double last_disp_time = 0;
		double this_disp_time = timer.current_time();
		if ((ch == end_code - 1) || (this_disp_time - last_disp_time > 1))
		{
			cout << '\r';
			cout.width(5);
			cout << ch - start_code + 1 << " / ";
			cout.width(5);
			cout << end_code - start_code;
			cout.precision(2);
			cout << "  Time remaining (estimated): "
				<< fixed << (timer.current_time() - start_time) / (ch - start_code + 1) * (end_code - ch - 1) << " s     ";

			last_disp_time = this_disp_time;
		}
	}
	cout << endl;

	cout << "Time elapsed: " << timer.current_time() - start_time << " s" << endl;

	{
		ofstream kfont_output(kfont_name.c_str(), ios_base::binary);
		if (kfont_output)
		{
			kfont_output.write(reinterpret_cast<char*>(&header), sizeof(header));

			kfont_output.write(reinterpret_cast<char*>(&char_width[0]),
				static_cast<std::streamsize>(char_width.size() * sizeof(char_width[0])));
			kfont_output.write(reinterpret_cast<char*>(&char_index[0]),
				static_cast<std::streamsize>(char_index.size() * sizeof(char_index[0])));
			kfont_output.write(reinterpret_cast<char*>(&char_dist[0]),
				static_cast<std::streamsize>(char_dist.size() * sizeof(char_dist[0])));
		}
	}

	::FT_Done_Face(ft_face);
	::FT_Done_FreeType(ft_lib);
}
