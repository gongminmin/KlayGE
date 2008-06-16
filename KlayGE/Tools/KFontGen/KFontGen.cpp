#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/atomic.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/thread.hpp>
#include <KlayGE/CpuInfo.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4251 4275 4273 4512 4701)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <mmintrin.h>
#include <emmintrin.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
#pragma comment(lib, "freetype236_D.lib")
#else
#pragma comment(lib, "freetype236.lib")
#endif
#endif

#include "kdtree.hpp"

using namespace std;
using namespace KlayGE;

int const INTERNAL_CHAR_SIZE = 4096;
int const NUM_CHARS = 65536;

#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(push, 1)
#endif
struct kfont_header
{
	uint32_t fourcc;
	uint32_t version;
	uint32_t start_ptr;
	uint32_t validate_chars;
	uint32_t non_empty_chars;
	uint32_t char_size;

	int16_t base;
	int16_t scale;
};
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(pop)
#endif

int bsf(uint64_t v)
{
	v &= ~v + 1;
	union
	{
		float f;
		uint32_t u;
	} fnu;
	fnu.f = static_cast<float>(v);
	return (fnu.u >> 23) - 127;
}

class disp_thread
{
public:
	disp_thread(Timer& timer, atomic<int32_t>& cur_num_char, int32_t total_chars)
		: timer_(&timer), cur_num_char_(&cur_num_char), total_chars_(total_chars)
	{
	}

	void operator()()
	{
		double last_disp_time = 0;
		for (;;)
		{
			int32_t dist_cur_num_char = cur_num_char_->value();

			double this_disp_time = timer_->elapsed();
			if ((dist_cur_num_char == total_chars_) || (this_disp_time - last_disp_time > 1))
			{
				cout << '\r';
				cout.width(5);
				cout << dist_cur_num_char << " / ";
				cout.width(5);
				cout << total_chars_;
				cout.precision(2);
				cout << "  Time remaining (estimated): "
					<< fixed << this_disp_time / dist_cur_num_char * (total_chars_ - dist_cur_num_char) << " s     ";

				last_disp_time = this_disp_time;

				if (dist_cur_num_char == total_chars_)
				{
					break;
				}
			}

			KlayGE::Sleep(500);
		}
	}

private:
	Timer* timer_;
	atomic<int32_t>* cur_num_char_;
	int32_t total_chars_;
};

struct font_info
{
	uint16_t advance_x;
	uint16_t advance_y;

	uint16_t top;
	uint16_t left;
	uint16_t width;
	uint16_t height;

	std::vector<float> dist;
};

class ttf_to_dist
{
public:
	ttf_to_dist(FT_Library ft_lib, FT_Face ft_face, uint32_t char_size, std::vector<uint32_t>& validate_chars, uint32_t start_code, uint32_t end_code,
		std::vector<font_info>& char_info,
		atomic<int32_t>& cur_num_char)
		: ft_lib_(ft_lib), ft_face_(ft_face), char_size_(char_size), validate_chars_(&validate_chars), start_code_(start_code), end_code_(end_code),
			char_info_(&char_info),
			cur_num_char_(&cur_num_char)
	{
#ifndef KLAYGE_CPU_X64
		CPUInfo cpu;
		if (cpu.IsFeatureSupport(CPUInfo::CF_SSE2))
		{
			binary_font_extract = boost::bind(&ttf_to_dist::binary_font_extract_sse2, this, _1, _2, _3);
			edge_extract = boost::bind(&ttf_to_dist::edge_extract_sse2, this, _1, _2, _3, _4, _5);
		}
		else
		{
			edge_extract = boost::bind(&ttf_to_dist::edge_extract_cpp, this, _1, _2, _3, _4, _5);

			if (cpu.IsFeatureSupport(CPUInfo::CF_MMX))
			{
				binary_font_extract = boost::bind(&ttf_to_dist::binary_font_extract_mmx, this, _1, _2, _3);

			}
			else
			{
				binary_font_extract = boost::bind(&ttf_to_dist::binary_font_extract_cpp, this, _1, _2, _3);
			}
		}
#else
		binary_font_extract = boost::bind(&ttf_to_dist::binary_font_extract_sse2, this, _1, _2, _3);
		edge_extract = boost::bind(&ttf_to_dist::edge_extract_sse2, this, _1, _2, _3, _4, _5);
#endif
	}

	void operator()()
	{
		FT_GlyphSlot ft_slot = ft_face_->glyph;

		int const max_dist_sq = 2 * INTERNAL_CHAR_SIZE * INTERNAL_CHAR_SIZE;

		std::vector<uint8_t> char_bitmap(INTERNAL_CHAR_SIZE / 8 * INTERNAL_CHAR_SIZE);
		std::vector<int2> edge_points;
		for (uint32_t c = start_code_; c < end_code_; ++ c)
		{
			uint32_t const ch = (*validate_chars_)[c];
			font_info& ci = (*char_info_)[ch];

			std::fill(char_bitmap.begin(), char_bitmap.end(), 0);

			FT_Load_Char(ft_face_, ch, FT_LOAD_RENDER);

			int const buf_width = std::min(ft_slot->bitmap.width, INTERNAL_CHAR_SIZE);
			int const buf_height = std::min(ft_slot->bitmap.rows, INTERNAL_CHAR_SIZE);

			if ((buf_width > 0) && (buf_height > 0))
			{
				uint8_t* font_data = &char_bitmap[0];
				uint8_t const * src_data = ft_slot->bitmap.buffer;
				for (int y = 0; y < buf_height; ++ y)
				{
					binary_font_extract(font_data, src_data, buf_width);

					font_data += INTERNAL_CHAR_SIZE / 8;
					src_data += ft_slot->bitmap.pitch;
				}
			}

			ci.advance_x = static_cast<uint16_t>(ft_slot->advance.x / 64.0f / INTERNAL_CHAR_SIZE * char_size_);
			ci.advance_y = static_cast<uint16_t>(ft_slot->advance.y / 64.0f / INTERNAL_CHAR_SIZE * char_size_);

			edge_points.resize(0);
			for (int y = 0; y < buf_height; ++ y)
			{
				edge_extract(edge_points, 0, buf_width, char_bitmap, y);
			}

			if (!edge_points.empty())
			{
				kdtree<int2> kd(&edge_points[0], edge_points.size());

				ci.left = static_cast<uint16_t>(static_cast<float>(ft_slot->bitmap_left) / INTERNAL_CHAR_SIZE * char_size_);
				ci.top = static_cast<uint16_t>((3 / 4.0f - static_cast<float>(ft_slot->bitmap_top) / INTERNAL_CHAR_SIZE) * char_size_);
				ci.width = static_cast<uint16_t>(std::min<float>(1.0f, static_cast<float>(buf_width) / INTERNAL_CHAR_SIZE + 0.1f) * char_size_);
				ci.height = static_cast<uint16_t>(std::min<float>(1.0f, static_cast<float>(buf_height) / INTERNAL_CHAR_SIZE + 0.1f) * char_size_);

				ci.dist.resize(char_size_ * char_size_);
				for (uint32_t y = 0; y < char_size_; ++ y)
				{
					for (uint32_t x = 0; x < char_size_; ++ x)
					{
						float value;
						int2 const map_xy = float2(x - 1.0f, y - 1.0f) * (INTERNAL_CHAR_SIZE / static_cast<float>(char_size_ - 2)) + INTERNAL_CHAR_SIZE / 2.0f / (char_size_ - 2);
						if (kd.query_position(map_xy) > 0)
						{
							value = MathLib::sqrt(static_cast<float>(kd.squared_distance(0)) / max_dist_sq);
						}
						else
						{
							value = 1.0f;
						}
						if ((map_xy.x() > 0) && (map_xy.y() > 0) && (map_xy.x() < INTERNAL_CHAR_SIZE) && (map_xy.y() < INTERNAL_CHAR_SIZE))
						{
							if (0 == ((char_bitmap[(map_xy.y() * INTERNAL_CHAR_SIZE + map_xy.x()) / 8] >> (map_xy.x() & 0x7)) & 0x1))
							{
								value = -value;
							}
						}
						else
						{
							value = -value;
						}

						ci.dist[y * char_size_ + x] = value;
					}
				}
			}

			++ *cur_num_char_;
		}
	}

private:
	void binary_font_extract_cpp(uint8_t* dst_data, uint8_t const * src_data, int size)
	{
		for (int x = 0; x < size; ++ x)
		{
			if (src_data[x] >= 128)
			{
				dst_data[x / 8] |= 1UL << (x & 0x7);
			}
		}
	}

#ifndef KLAYGE_CPU_X64
	void binary_font_extract_mmx(uint8_t* dst_data, uint8_t const * src_data, int size)
	{
		for (int x = 0, x_end = size & ~0x7; x < x_end; x += 8)
		{
			__m64 mask = *reinterpret_cast<__m64 const *>(&src_data[x]);
			dst_data[x / 8] = static_cast<uint8_t>(_mm_movemask_pi8(mask));
		}
		for (int x = size & ~0x7; x < size; ++ x)
		{
			if (src_data[x] >= 128)
			{
				dst_data[x / 8] |= 1UL << (x & 0x7);
			}
		}
		_m_empty();
	}
#endif

	void binary_font_extract_sse2(uint8_t* dst_data, uint8_t const * src_data, int size)
	{
		for (int x = 0, x_end = size & ~0xF; x < x_end; x += 16)
		{
			__m128i mask = _mm_loadu_si128(reinterpret_cast<__m128i const *>(&src_data[x]));
			*reinterpret_cast<uint16_t*>(&dst_data[x / 8]) = static_cast<uint16_t>(_mm_movemask_epi8(mask));
		}
		for (int x = size & ~0xF; x < size; ++ x)
		{
			if (src_data[x] >= 128)
			{
				dst_data[x / 8] |= 1UL << (x & 0x7);
			}
		}
	}

	void edge_extract_sse2(std::vector<int2>& edge_points, int start, int end, std::vector<uint8_t> const & char_bitmap, int y)
	{
		__m128i zero = _mm_set1_epi8(0);
		for (int x = start; x < end; x += sizeof(__m128i) * 8)
		{
			__m128i center = _mm_loadu_si128(reinterpret_cast<__m128i const *>(&char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8]));
			if (_mm_movemask_epi8(_mm_cmpeq_epi32(center, zero)) != 0xFFFF)
			{
				__m128i up;
				if (y != 0)
				{
					up = _mm_loadu_si128(reinterpret_cast<__m128i const *>(&char_bitmap[((y - 1) * INTERNAL_CHAR_SIZE + x) / 8]));
				}
				else
				{
					up = zero;
				}
				__m128i down;
				if (y != INTERNAL_CHAR_SIZE - 1)
				{
					down = _mm_loadu_si128(reinterpret_cast<__m128i const *>(&char_bitmap[((y + 1) * INTERNAL_CHAR_SIZE + x) / 8]));
				}
				else
				{
					down = zero;
				}
				__m128i left = _mm_slli_epi64(center, 1);
				if (x != 0)
				{
					__m128i t = _mm_loadu_si128(reinterpret_cast<__m128i const *>(&char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8 - 8]));
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
					__m128i t = _mm_loadu_si128(reinterpret_cast<__m128i const *>(&char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8 + 8]));
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
					edge_points.push_back(int2(x + bsf(m64[0]), y));
					m64[0] &= m64[0] - 1;
				}
				while (m64[1] != 0)
				{
					edge_points.push_back(int2(x + bsf(m64[1]) + 64, y));
					m64[1] &= m64[1] - 1;
				}
			}
		}
	}

	void edge_extract_cpp(std::vector<int2>& edge_points, int start, int end, std::vector<uint8_t> const & char_bitmap, int y)
	{
		for (int x = start; x < end; x += sizeof(uint64_t) * 8)
		{
			uint64_t center = *reinterpret_cast<uint64_t const *>(&char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8]);
			if (center != 0)
			{
				uint64_t up = 0;
				if (y != 0)
				{
					up = *reinterpret_cast<uint64_t const *>(&char_bitmap[((y - 1) * INTERNAL_CHAR_SIZE + x) / 8]);
				}
				uint64_t down = 0;
				if (y != INTERNAL_CHAR_SIZE - 1)
				{
					down = *reinterpret_cast<uint64_t const *>(&char_bitmap[((y + 1) * INTERNAL_CHAR_SIZE + x) / 8]);
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
					edge_points.push_back(int2(x + bsf(mask), y));
					mask &= mask - 1;
				}
			}
		}
	}

private:
	FT_Library ft_lib_;
	FT_Face ft_face_;
	uint32_t char_size_;
	std::vector<uint32_t>* validate_chars_;
	uint32_t start_code_;
	uint32_t end_code_;
	std::vector<font_info>* char_info_;
	atomic<int32_t>* cur_num_char_;

	boost::function<void(uint8_t*, uint8_t const *, int)> binary_font_extract;
	boost::function<void(std::vector<int2>&, int, int, std::vector<uint8_t> const &, int)> edge_extract;
};

void quantizer(std::vector<uint8_t>& uint8_dist, std::vector<std::pair<int32_t, int32_t> > const & char_index, std::vector<font_info> const & char_info, int16_t& base, int16_t& scale)
{
	std::vector<float> dist_block;
	for (size_t i = 0; i < char_index.size(); ++ i)
	{
		int const ch = char_index[i].first;
		dist_block.insert(dist_block.end(), char_info[ch].dist.begin(), char_info[ch].dist.end());
	}

	float max_value = -1;
	float min_value = 1;
	for (size_t i = 0; i < dist_block.size(); ++ i)
	{
		float value = dist_block[i];

		min_value = std::min(min_value, value);
		max_value = std::max(max_value, value);
	}

	uint32_t const L = 256;

	// Newton's Method
	float const num_steps = L - 1;
	for (size_t i = 0; i < 8; ++ i)
	{
		float const inv_scale = num_steps / (max_value - min_value);

		float steps[L];
		for (size_t j = 0; j < L; ++ j)
		{
			steps[j] = MathLib::lerp(min_value, max_value, j / num_steps);
		}

		float f_min = 0;
		float f_max = 0;
		float d_f_min = 0;
		float d_f_max = 0;
		for (size_t j = 0; j < dist_block.size(); ++ j)
		{
			float const quantized = (dist_block[j] - min_value) * inv_scale;
			uint32_t const s = MathLib::clamp<uint32_t>(static_cast<uint32_t>(quantized + 0.5f), 0, L - 1);
			if (s < L)
			{
				float const diff = dist_block[j] - steps[s];
				float const fs = s / num_steps;

				f_min += (1 - fs) * diff;
				d_f_min += (1 - fs) * (1 - fs);

				f_max += fs * diff;
				d_f_max += fs * fs;
			}
		}

		if (d_f_min > 0.0f)
		{
			min_value -= f_min / d_f_min;
		}
		if (d_f_max > 0.0f)
		{
			max_value -= f_max / d_f_max;
		}

		if (min_value > max_value)
		{
			std::swap(min_value, max_value);
		}
	}

	float fscale = max_value - min_value;
	base = static_cast<int16_t>(min_value * 32768);
	scale = static_cast<int16_t>((fscale - 1) * 32768);
	float inv_scale = 255 / fscale;

	uint8_dist.resize(dist_block.size());
	for (size_t i = 0; i < dist_block.size(); ++ i)
	{
		uint8_dist[i] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((dist_block[i] - min_value) * inv_scale), 0, 255));
	}
}

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
	int num_threads;

	CPUInfo cpu;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-name,I", boost::program_options::value<std::string>(), "Input font name.")
		("output-name,O", boost::program_options::value<std::string>(), "Output font name. Optional.")
		("start-code,S", boost::program_options::value<int>(&start_code)->default_value(0), "Start code.")
		("end-code,E", boost::program_options::value<int>(&end_code)->default_value(65536), "End code.")
		("char-size,C", boost::program_options::value<uint32_t>(&header.char_size)->default_value(32), "Character size.")
		("threads,T", boost::program_options::value<int>(&num_threads)->default_value(cpu.NumHWThreads()), "Number of Threads.")
		("version,v", "Version.");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if (vm.count("help"))
	{
		cout << desc << endl;
		return 1;
	}
	if (vm.count("version"))
	{
		cout << "KlayGE Font Generator, Version 1.0.0" << endl;
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

	std::vector<std::pair<int32_t, int32_t> > char_index;
	std::vector<font_info> char_info(NUM_CHARS);
	{
		memset(&char_info[0], 0, sizeof(char_info[0]) * char_info.size());

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

			char_index.resize(header.non_empty_chars);
			kfont_input.read(reinterpret_cast<char*>(&char_index[0]),
				static_cast<std::streamsize>(char_index.size() * sizeof(char_index[0])));

			std::vector<std::pair<int32_t, std::pair<uint16_t, uint16_t> > > advance(header.validate_chars);
			kfont_input.read(reinterpret_cast<char*>(&advance[0]),
				static_cast<std::streamsize>(advance.size() * sizeof(advance[0])));
			for (size_t i = 0; i < advance.size(); ++ i)
			{
				int const ch = advance[i].first;

				char_info[ch].advance_x = advance[i].second.first;
				char_info[ch].advance_y = advance[i].second.second;
			}

			for (size_t i = 0; i < char_index.size(); ++ i)
			{
				int const ch = char_index[i].first;

				kfont_input.read(reinterpret_cast<char*>(&char_info[ch].top), sizeof(char_info[ch].top));
				kfont_input.read(reinterpret_cast<char*>(&char_info[ch].left), sizeof(char_info[ch].left));
				kfont_input.read(reinterpret_cast<char*>(&char_info[ch].width), sizeof(char_info[ch].width));
				kfont_input.read(reinterpret_cast<char*>(&char_info[ch].height), sizeof(char_info[ch].height));
			}

			for (size_t i = 0; i < char_index.size(); ++ i)
			{
				int const ch = char_index[i].first;

				std::vector<uint8_t> uint8_dist(header.char_size * header.char_size);
				kfont_input.read(reinterpret_cast<char*>(&uint8_dist[0]),
					static_cast<std::streamsize>(uint8_dist.size() * sizeof(uint8_dist[0])));

				char_info[ch].dist.resize(uint8_dist.size());
				for (size_t i = 0; i < char_info[ch].dist.size(); ++ i)
				{
					char_info[ch].dist[i] = uint8_dist[0] / 256.0f * (header.scale / 32768.0f - 1) + header.base / 32768.0f;
				}
			}
		}
	}

	thread_pool tp(1, num_threads + 1);

	cout << "\tInput font name: " << ttf_name << endl;
	cout << "\tOutput font name: " << kfont_name << endl;
	cout << "\tStart code: " << start_code << endl;
	cout << "\tEnd code: " << end_code << endl;
	cout << "\tCharacter size: " << header.char_size << endl;
	cout << "\tNumber of threads: " << num_threads << endl;
	cout << endl;
	if (cpu.IsFeatureSupport(CPUInfo::CF_SSE2))
	{
		cout << "SSE2 is used." << endl;
	}
	else
	{
		if (cpu.IsFeatureSupport(CPUInfo::CF_MMX))
		{
			cout << "MMX is used." << endl;
		}
	}
	cout << endl;

	std::vector<uint8_t> ttf;
	{
		ifstream ttf_input(ttf_name.c_str(), ios_base::binary);
		ttf_input.seekg(0, ios_base::end);
		ttf.resize(ttf_input.tellg());
		ttf_input.seekg(0, ios_base::beg);
		ttf_input.read(reinterpret_cast<char*>(&ttf[0]),
			static_cast<std::streamsize>(ttf.size() * sizeof(ttf[0])));
	}

	atomic<int32_t> cur_num_char(0);

	cout << "Compute distance field..." << endl;

	std::vector<FT_Library> ft_libs(num_threads);
	std::vector<FT_Face> ft_faces(num_threads);
	std::vector<joiner<void> > joiners(num_threads);

	for (int i = 0; i < num_threads; ++ i)
	{
		FT_Init_FreeType(&ft_libs[i]);
		FT_New_Memory_Face(ft_libs[i], &ttf[0], static_cast<FT_Long>(ttf.size()), 0, &ft_faces[i]);
		FT_Set_Pixel_Sizes(ft_faces[i], 0, INTERNAL_CHAR_SIZE);
		FT_Select_Charmap(ft_faces[i], FT_ENCODING_UNICODE);
	}

	std::vector<uint32_t> validate_chars;
	for (int i = start_code; i <= end_code; ++ i)
	{
		uint32_t mapping = FT_Get_Char_Index(ft_faces[0], i);
		if (mapping > 0)
		{
			validate_chars.push_back(i);
		}
	}

	uint32_t const num_chars_per_package = static_cast<uint32_t>((validate_chars.size() + num_threads - 1) / num_threads);

	Timer timer;
	joiner<void> disp_joiner = tp(disp_thread(timer, cur_num_char, static_cast<int32_t>(validate_chars.size())));
	for (int i = 0; i < num_threads; ++ i)
	{
		uint32_t const sc = i * num_chars_per_package;
		uint32_t const ec = std::min(sc + num_chars_per_package, static_cast<uint32_t>(validate_chars.size()));

		joiners[i] = tp(ttf_to_dist(ft_libs[i], ft_faces[i], header.char_size, validate_chars, sc, ec, char_info, cur_num_char));
	}
	for (int i = 0; i < num_threads; ++ i)
	{
		joiners[i]();

		FT_Done_Face(ft_faces[i]);
		FT_Done_FreeType(ft_libs[i]);
	}
	disp_joiner();
	cout << "\rTime elapsed: " << timer.elapsed() << " s                                        " << endl;

	char_index.clear();
	header.non_empty_chars = 0;
	for (size_t i = 0; i < char_info.size(); ++ i)
	{
		if (!char_info[i].dist.empty())
		{
			char_index.push_back(std::make_pair(static_cast<int32_t>(i), header.non_empty_chars));
			++ header.non_empty_chars;
		}
	}

	std::vector<std::pair<int32_t, std::pair<uint16_t, uint16_t> > > advance;
	header.validate_chars = 0;
	for (size_t i = 0; i < char_info.size(); ++ i)
	{
		if ((char_info[i].advance_x != 0) || (char_info[i].advance_y != 0))
		{
			advance.push_back(std::make_pair(static_cast<int32_t>(i), std::make_pair(char_info[i].advance_x, char_info[i].advance_y)));
			++ header.validate_chars;
		}
	}

	cout << "Quantize..." << endl;
	timer.restart();
	std::vector<uint8_t> uint8_dist(header.non_empty_chars * header.char_size * header.char_size);
	quantizer(uint8_dist, char_index, char_info, header.base, header.scale);
	cout << "Time elapsed: " << timer.elapsed() << " s" << endl;

	{
		ofstream kfont_output(kfont_name.c_str(), ios_base::binary);
		if (kfont_output)
		{
			kfont_output.write(reinterpret_cast<char*>(&header), sizeof(header));

			kfont_output.write(reinterpret_cast<char*>(&char_index[0]),
				static_cast<std::streamsize>(char_index.size() * sizeof(char_index[0])));

			kfont_output.write(reinterpret_cast<char*>(&advance[0]),
				static_cast<std::streamsize>(advance.size() * sizeof(advance[0])));

			for (size_t i = 0; i < char_index.size(); ++ i)
			{
				int const ch = char_index[i].first;

				kfont_output.write(reinterpret_cast<char*>(&char_info[ch].top), sizeof(char_info[ch].top));
				kfont_output.write(reinterpret_cast<char*>(&char_info[ch].left), sizeof(char_info[ch].left));
				kfont_output.write(reinterpret_cast<char*>(&char_info[ch].width), sizeof(char_info[ch].width));
				kfont_output.write(reinterpret_cast<char*>(&char_info[ch].height), sizeof(char_info[ch].height));
			}

			kfont_output.write(reinterpret_cast<char*>(&uint8_dist[0]),
				static_cast<std::streamsize>(uint8_dist.size() * sizeof(uint8_dist[0])));
		}
	}
}
