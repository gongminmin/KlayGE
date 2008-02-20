#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/thread.hpp>
#include <KlayGE/CpuTopology.hpp>

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

#include <emmintrin.h>

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

int bitwhere(uint64_t v)
{
	float f = static_cast<float>(v);
	return (*reinterpret_cast<uint32_t*>(&f) >> 23) - 127;
}

class ttf_to_dist
{
public:
	ttf_to_dist(FT_Library ft_lib, FT_Face ft_face, kfont_header& header, int start_code, int end_code,
				boost::mutex& disp_mutex,
				std::vector<uint8_t>& char_width, std::vector<int32_t>& char_index, std::vector<std::vector<uint8_t> >& char_dist,
				Timer& timer, double& last_disp_time,
				int& cur_num_char, int total_chars)
		: ft_lib_(ft_lib), ft_face_(ft_face), header_(&header), start_code_(start_code), end_code_(end_code),
			disp_mutex_(&disp_mutex),
			char_width_(&char_width), char_index_(&char_index), char_dist_(&char_dist),
			timer_(&timer), last_disp_time_(&last_disp_time),
			cur_num_char_(&cur_num_char), total_chars_(total_chars)
	{
	}

	void operator()()
	{
		FT_GlyphSlot ft_slot = ft_face_->glyph;

		std::vector<uint8_t> char_bitmap(INTERNAL_CHAR_SIZE / 8 * INTERNAL_CHAR_SIZE);
		std::vector<int2> edge_points;
		for (int ch = start_code_; ch < end_code_; ++ ch)
		{
			std::fill(char_bitmap.begin(), char_bitmap.end(), 0);

			FT_Load_Char(ft_face_, ch, FT_LOAD_RENDER);

			(*char_width_)[ch] = static_cast<uint8_t>(std::min(header_->char_size, 
					static_cast<uint32_t>(ft_slot->advance.x / 64.0f * header_->char_size / INTERNAL_CHAR_SIZE)));

			int const buf_width = std::min(ft_slot->bitmap.width, INTERNAL_CHAR_SIZE);
			int const buf_height = std::min(ft_slot->bitmap.rows, INTERNAL_CHAR_SIZE);
			
			int const y_start = std::max<int>(INTERNAL_CHAR_SIZE * 3 / 4 - ft_slot->bitmap_top, 0);
			if ((buf_width > 0) && (buf_height > 0))
			{
				uint8_t* font_data = &char_bitmap[(y_start * INTERNAL_CHAR_SIZE + ft_slot->bitmap_left) / 8];
				uint8_t const * src_data = ft_slot->bitmap.buffer;
				for (int y = 0; y < buf_height; ++ y)
				{
#ifdef _SSE2_SUPPORT
					for (int x = 0, x_end = buf_width & ~0xF; x < x_end; x += 16)
					{
						__m128i mask = _mm_loadu_si128(reinterpret_cast<__m128i const *>(&src_data[x]));
						*reinterpret_cast<uint16_t*>(&font_data[x / 8]) = static_cast<uint16_t>(_mm_movemask_epi8(mask));
					}
					for (int x = buf_width & ~0xF; x < buf_width; ++ x)
					{
						if (src_data[x] >= 128)
						{
							font_data[x / 8] |= 1UL << (x & 0x7);
						}
					}
#elif defined _MMX_SUPPORT
					for (int x = 0, x_end = buf_width & ~0x7; x < x_end; x += 8)
					{
						__m64 mask = *reinterpret_cast<__m64 const *>(&src_data[x]);
						font_data[x / 8] = static_cast<uint8_t>(_mm_movemask_pi8(mask));
					}
					for (int x = buf_width & ~0x7; x < buf_width; ++ x)
					{
						if (src_data[x] >= 128)
						{
							font_data[x / 8] |= 1UL << (x & 0x7);
						}
					}
#else
					for (int x = 0; x < buf_width; ++ x)
					{
						if (src_data[x] >= 128)
						{
							font_data[x / 8] |= 1UL << (x & 0x7);
						}
					}
#endif
					font_data += INTERNAL_CHAR_SIZE / 8;
					src_data += ft_slot->bitmap.pitch;
				}
			}
#ifdef _MMX_SUPPORT
			_m_empty();
#endif

			edge_points.resize(0);
			for (int y = y_start, y_end = buf_height + y_start; y < y_end; ++ y)
			{
#ifdef _SSE2_SUPPORT
				for (int x = ft_slot->bitmap_left, x_end = ft_slot->bitmap_left + buf_width; x < x_end; x += sizeof(__m128i) * 8)
				{
					__m128i center = _mm_loadu_si128(reinterpret_cast<__m128i*>(&char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8]));
					if (_mm_movemask_epi8(_mm_cmpeq_epi32(center, _mm_set1_epi8(0))) != 0xFFFF)
					{
						__m128i up;
						if (y != 0)
						{
							up = _mm_loadu_si128(reinterpret_cast<__m128i*>(&char_bitmap[((y - 1) * INTERNAL_CHAR_SIZE + x) / 8]));
						}
						else
						{
							up = _mm_set1_epi8(0);
						}
						__m128i down;
						if (y != INTERNAL_CHAR_SIZE - 1)
						{
							down = _mm_loadu_si128(reinterpret_cast<__m128i*>(&char_bitmap[((y + 1) * INTERNAL_CHAR_SIZE + x) / 8]));
						}
						else
						{
							down = _mm_set1_epi8(0);
						}
						__m128i left = _mm_slli_epi64(center, 1);
						if (x != 0)
						{
							__m128i t = _mm_loadu_si128(reinterpret_cast<__m128i*>(&char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8 - 8]));
							left = _mm_or_si128(left, _mm_srli_epi64(t, 63));
						}
						else
						{
							__m128i t = _mm_srli_si128(center, 8);
							left = _mm_or_si128(left, _mm_srli_epi64(t, 63));
						}
						__m128i right = _mm_srli_epi64(center, 1);
						if (x != INTERNAL_CHAR_SIZE - 1)
						{
							__m128i t = _mm_loadu_si128(reinterpret_cast<__m128i*>(&char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8 + 8]));
							right = _mm_or_si128(right, _mm_slli_epi64(t, 63));
						}
						else
						{
							__m128i t = _mm_slli_si128(center, 8);
							right = _mm_or_si128(right, _mm_slli_epi64(t, 63));
						}
						__m128i mask = _mm_and_si128(center, up);
						mask = _mm_and_si128(mask, down);
						mask = _mm_and_si128(mask, left);
						mask = _mm_and_si128(mask, right);
						mask = _mm_xor_si128(center, mask);
						mask = _mm_and_si128(center, mask);
						uint64_t m64[2];
						_mm_storeu_si128(reinterpret_cast<__m128i*>(m64), mask);
						while (m64[0] != 0)
						{
							edge_points.push_back(int2(x + bitwhere(m64[0] & (~m64[0] + 1)), y));
							m64[0] &= m64[0] - 1;
						}
						while (m64[1] != 0)
						{
							edge_points.push_back(int2(x + bitwhere(m64[1] & (~m64[1] + 1)) + 64, y));
							m64[1] &= m64[1] - 1;
						}
					}
				}
#else
				for (int x = ft_slot->bitmap_left, x_end = ft_slot->bitmap_left + buf_width; x < x_end; x += sizeof(uint64_t) * 8)
				{
					uint64_t center = *reinterpret_cast<uint64_t*>(&char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8]);
					if (center != 0)
					{
						uint64_t up = 0;
						if (y != 0)
						{
							up = *reinterpret_cast<uint64_t*>(&char_bitmap[((y - 1) * INTERNAL_CHAR_SIZE + x) / 8]);
						}
						uint64_t down = 0;
						if (y != INTERNAL_CHAR_SIZE - 1)
						{
							down = *reinterpret_cast<uint64_t*>(&char_bitmap[((y + 1) * INTERNAL_CHAR_SIZE + x) / 8]);
						}
						uint64_t left = center << 1;
						if (x != 0)
						{
							left |= char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8 - 1] >> 7;
						}
						uint64_t right = center >> 1;
						if (x != INTERNAL_CHAR_SIZE - 1)
						{
							right |= static_cast<uint64_t>(char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8 + sizeof(uint64_t)] & 0x1) << (sizeof(uint64_t) * 8 - 1);
						}
						uint64_t mask = center & up & down & left & right;
						mask = center & (center ^ mask);
						while (mask != 0)
						{
							edge_points.push_back(int2(x + bitwhere(mask & (~mask + 1)), y));
							mask &= mask - 1;
						}
					}
				}
#endif
			}

			if (!edge_points.empty())
			{
				(*char_dist_)[ch].resize(header_->char_size * header_->char_size);

				kdtree<int2> kd(&edge_points[0], edge_points.size());

				int const max_dist_sq = 2 * INTERNAL_CHAR_SIZE * INTERNAL_CHAR_SIZE;
				for (uint32_t y = 0; y < header_->char_size; ++ y)
				{
					for (uint32_t x = 0; x < header_->char_size; ++ x)
					{
						int const map_x = x * INTERNAL_CHAR_SIZE / header_->char_size + INTERNAL_CHAR_SIZE / header_->char_size / 2;
						int const map_y = y * INTERNAL_CHAR_SIZE / header_->char_size + INTERNAL_CHAR_SIZE / header_->char_size / 2;
						float value;
						if (kd.query_position(int2(map_x, map_y), 1) > 0)
						{
							float v = MathLib::sqrt(static_cast<float>(kd.squared_distance(0)) / max_dist_sq);
							value = MathLib::clamp(v * SCALE, 0.0f, 1.0f);
						}
						else
						{
							value = 1.0f;
						}
						if (0 == ((char_bitmap[(map_y * INTERNAL_CHAR_SIZE + map_x) / 8] >> (map_x & 0x7)) & 0x1))
						{
							value = -value;
						}

						(*char_dist_)[ch][y * header_->char_size + x] = static_cast<uint8_t>((value / 2 + 0.5f) * 255);
					}
				}
			}

			{
				boost::mutex::scoped_lock lock(*disp_mutex_);

				++ *cur_num_char_;

				double this_disp_time = timer_->elapsed();
				if ((*cur_num_char_ == total_chars_) || (this_disp_time - *last_disp_time_ > 1))
				{
					cout << '\r';
					cout.width(5);
					cout << *cur_num_char_ << " / ";
					cout.width(5);
					cout << total_chars_;
					cout.precision(2);
					cout << "  Time remaining (estimated): "
						<< fixed << this_disp_time / *cur_num_char_ * (total_chars_ - *cur_num_char_) << " s     ";

					*last_disp_time_ = this_disp_time;
				}
			}
		}
	}

private:
	FT_Library ft_lib_;
	FT_Face ft_face_;
	kfont_header* header_;
	int start_code_;
	int end_code_;
	boost::mutex* disp_mutex_;
	std::vector<uint8_t>* char_width_;
	std::vector<int32_t>* char_index_;
	std::vector<std::vector<uint8_t> >* char_dist_;
	Timer* timer_;
	double* last_disp_time_;
	int* cur_num_char_;
	int total_chars_;
};

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

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-name,I", boost::program_options::value<std::string>(), "Input font name.")
		("output-name,O", boost::program_options::value<std::string>(), "Output font name. Optional.")
		("start-code,S", boost::program_options::value<int>(&start_code)->default_value(0), "Start code.")
		("end-code,E", boost::program_options::value<int>(&end_code)->default_value(65536), "End code.")
		("char-size,C", boost::program_options::value<uint32_t>(&header.char_size)->default_value(32), "Character size.");

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

	std::vector<uint8_t> char_width(NUM_CHARS, 0);
	std::vector<int32_t> char_index(NUM_CHARS, -1);
	std::vector<std::vector<uint8_t> > char_dist(NUM_CHARS);
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

			std::vector<uint8_t> tmp_char_dist(header.non_empty_chars * header.char_size * header.char_size);
			kfont_input.read(reinterpret_cast<char*>(&tmp_char_dist[0]),
				static_cast<std::streamsize>(tmp_char_dist.size() * sizeof(tmp_char_dist[0])));

			for (int ch = 0; ch < NUM_CHARS; ++ ch)
			{
				if (char_index[ch] != -1)
				{
					char_dist[ch].assign(tmp_char_dist.begin() + char_index[ch] * header.char_size * header.char_size,
						tmp_char_dist.begin() + (char_index[ch] + 1) * header.char_size * header.char_size);
				}
			}
		}
	}

	CpuTopology cpu;
	int num_threads = cpu.NumHWThreads();
	thread_pool tp(1, num_threads);

	cout << "\tInput font name: " << ttf_name << endl;
	cout << "\tOutput font name: " << kfont_name << endl;
	cout << "\tStart code: " << start_code << endl;
	cout << "\tEnd code: " << end_code << endl;
	cout << "\tCharacter size: " << header.char_size << endl;
	cout << "\tNumber of threads: " << num_threads << endl;
	cout << endl;

	int const total_chars = end_code - start_code;
	int const num_chars_per_thread = (total_chars + num_threads - 1) / num_threads;

	std::vector<uint8_t> ttf;
	{
		ifstream ttf_input(ttf_name.c_str(), ios_base::binary);
		ttf_input.seekg(0, ios_base::end);
		ttf.resize(ttf_input.tellg());
		ttf_input.seekg(0, ios_base::beg);
		ttf_input.read(reinterpret_cast<char*>(&ttf[0]),
			static_cast<std::streamsize>(ttf.size() * sizeof(ttf[0])));
	}

	boost::mutex disp_mutex;
	double last_disp_time = 0;
	int cur_num_char = 0;

	Timer timer;

	std::vector<FT_Library> ft_libs(num_threads);
	std::vector<FT_Face> ft_faces(num_threads);
	std::vector<joiner<void> > joiners(num_threads);
	for (int i = 0; i < num_threads; ++ i)
	{
		FT_Init_FreeType(&ft_libs[i]);
		FT_New_Memory_Face(ft_libs[i], &ttf[0], static_cast<FT_Long>(ttf.size()), 0, &ft_faces[i]);
		FT_Set_Pixel_Sizes(ft_faces[i], 0, INTERNAL_CHAR_SIZE);
		FT_Select_Charmap(ft_faces[i], FT_ENCODING_UNICODE);

		int start_code_thread = start_code + i * num_chars_per_thread;
		int end_code_thread = std::min(start_code + (i + 1) * num_chars_per_thread, end_code);

		joiners[i] = tp(ttf_to_dist(ft_libs[i], ft_faces[i], header, start_code_thread, end_code_thread,
					disp_mutex,
					char_width, char_index, char_dist,
					timer, last_disp_time,
					cur_num_char, total_chars));
	}
	for (int i = 0; i < num_threads; ++ i)
	{
		joiners[i]();

		FT_Done_Face(ft_faces[i]);
		FT_Done_FreeType(ft_libs[i]);
	}

	std::fill(char_index.begin(), char_index.end(), -1);
	header.non_empty_chars = 0;
	for (size_t i = 0; i < char_dist.size(); ++ i)
	{
		if (!char_dist[i].empty())
		{
			char_index[i] = header.non_empty_chars;
			++ header.non_empty_chars;
		}
	}
	cout << endl;

	cout << "Time elapsed: " << timer.elapsed() << " s" << endl;

	{
		ofstream kfont_output(kfont_name.c_str(), ios_base::binary);
		if (kfont_output)
		{
			kfont_output.write(reinterpret_cast<char*>(&header), sizeof(header));

			kfont_output.write(reinterpret_cast<char*>(&char_width[0]),
				static_cast<std::streamsize>(char_width.size() * sizeof(char_width[0])));
			kfont_output.write(reinterpret_cast<char*>(&char_index[0]),
				static_cast<std::streamsize>(char_index.size() * sizeof(char_index[0])));

			std::vector<uint8_t> tmp_char_dist(header.non_empty_chars * header.char_size * header.char_size);
			for (size_t i = 0; i < char_index.size(); ++ i)
			{
				if (char_index[i] != -1)
				{
					std::copy(char_dist[i].begin(), char_dist[i].end(),
						tmp_char_dist.begin() + char_index[i] * header.char_size * header.char_size);
				}
			}
			kfont_output.write(reinterpret_cast<char*>(&tmp_char_dist[0]),
				static_cast<std::streamsize>(tmp_char_dist.size() * sizeof(tmp_char_dist[0])));
		}
	}
}
