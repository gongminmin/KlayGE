#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/thread.hpp>
#include <KlayGE/CpuInfo.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/LZMACodec.hpp>
#include <KlayGE/aligned_allocator.hpp>
#include <KlayGE/atomic.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4251 4275 4273 4512 4701 4702)
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
#include FT_STROKER_H

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
#pragma comment(lib, "freetype2312_D.lib")
#else
#pragma comment(lib, "freetype2312.lib")
#endif
#endif

using namespace std;
using namespace KlayGE;

uint32_t const INTERNAL_CHAR_SIZE = 4096;
uint32_t const NUM_CHARS = 65536;

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

struct font_info
{
	uint16_t advance_x;
	uint16_t advance_y;

	int16_t top;
	int16_t left;
	uint16_t width;
	uint16_t height;

	uint32_t dist_index;
};

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


// A type to hold the distance map while it's being constructed
class DistanceMap
{
public:
	DistanceMap(uint32_t width, uint32_t height)
		: data_(width * height, float2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max())),
			width_(width), height_(height)
	{
	}

	float2& operator()(int x, int y)
	{
		return data_[y * width_ + x];
	}

	float2 const & operator()(int x, int y) const
	{
		return data_[y * width_ + x];
	}

	bool inside(int x, int y) const
	{
		return (static_cast<uint32_t>(x) < width_)
			&& (static_cast<uint32_t>(y) < height_);
	}

	// Do a single pass over the data.
	// Start at (x,y,z) and walk in the direction (cx,cy,cz)
	// Combine each pixel (x,y,z) with the value at (x+dx,y+dy,z+dz)
	void combine(int dx, int dy,
		int cx, int cy,
		int x, int y)
	{
		while (inside(x, y) && inside(x + dx, y + dy))
		{
			float2 v1 = operator()(x, y);
			float2 v2 = operator()(x + dx, y + dy) + float2(static_cast<float>(abs(dx)), static_cast<float>(abs(dy)));
			if (MathLib::dot(v1, v1) > MathLib::dot(v2, v2))
			{
				operator()(x, y) = v2;
			}

			x += cx;
			y += cy;
		}
	}

private:
	std::vector<float2> data_;
	uint32_t width_, height_;
};

void ComputeDistanceField(std::vector<float>& distances, uint32_t width, uint32_t height,
						DistanceMap& dmap)
{
	// Compute the rest of dmap by sequential sweeps over the data
	// using a 3d variant of Danielsson's algorithm

	{
		for (uint32_t y = 1; y < height; ++ y)
		{
			dmap.combine(0, -1,
				1, 0,
				0, y);
			dmap.combine(-1, 0,
				1, 0,
				1, y);
			dmap.combine(+1, 0,
				-1, 0,
				width - 2, y);
		}
		for (int y = height - 1; y >= 0; -- y)
		{
			dmap.combine(0, +1,
				1, 0,
				0, y);
			dmap.combine(-1, 0,
				1, 0,
				1, y);
			dmap.combine(+1, 0,
				-1, 0,
				width - 1, y);
		}
	}

	for (uint32_t y = 0; y < height; ++ y)
	{
		for (uint32_t x = 0; x < width; ++ x)
		{
			distances[y * width + x] = MathLib::length_sq(dmap(x, y));
		}
	}
}

struct raster_user_struct
{
	FT_BBox bbox;
	int buf_width;
	int buf_height;
	uint8_t* char_bitmap;
};
		
void RasterCallback(int y, int count, FT_Span const * const spans, void* const user) 
{
	static const uint8_t EDGE_MASK[8] = 
	{
		0x00,
		0x01,
		0x03,
		0x07,
		0x0F,
		0x1F,
		0x3F,
		0x7F
	};

	raster_user_struct* sptr = static_cast<raster_user_struct*>(user);
	for (int i = 0; i < count; ++ i) 
	{
		if (spans[i].coverage > 127)
		{
			int const x0 = spans[i].x - sptr->bbox.xMin;
			int const y0 = sptr->buf_height - 1 - (y - sptr->bbox.yMin);
			int const x_end = x0 + spans[i].len;
			int x = x0;
			for (; ((x & 0x7) != 0) && (x < x_end); ++ x)
			{
				sptr->char_bitmap[(y0 * INTERNAL_CHAR_SIZE + x) / 8] |= 1UL << (x & 0x7);
			}
			for (; x < (x_end & ~0x7); x += 8)
			{
				sptr->char_bitmap[(y0 * INTERNAL_CHAR_SIZE + x) / 8] = 0xFF;
			}
			if (x < x_end)
			{
				sptr->char_bitmap[(y0 * INTERNAL_CHAR_SIZE + x) / 8] = EDGE_MASK[x_end - x];
			}
		}
	}
}

class ttf_to_dist
{
public:
	ttf_to_dist(FT_Library ft_lib, FT_Face ft_face, uint32_t char_size, uint32_t const * validate_chars,
		font_info* char_info, float* char_dist_data,
		int32_t& cur_num_char, atomic<int32_t>& cur_package,
		uint32_t num_chars, uint32_t thread_id, uint32_t num_threads, uint32_t num_chars_per_package)
		: ft_lib_(ft_lib), ft_face_(ft_face), char_size_(char_size), validate_chars_(validate_chars),
			char_info_(char_info), char_dist_data_(char_dist_data),
			cur_num_char_(&cur_num_char),
			cur_package_(&cur_package),
			num_chars_(num_chars), thread_id_(thread_id), num_threads_(num_threads), num_chars_per_package_(num_chars_per_package)
	{
		CPUInfo cpu;
		if (cpu.IsFeatureSupport(CPUInfo::CF_SSE2))
		{
			edge_extract = boost::bind(&ttf_to_dist::edge_extract_sse2, this, _1, _2, _3, _4);
		}
		else
		{
			edge_extract = boost::bind(&ttf_to_dist::edge_extract_cpp, this, _1, _2, _3, _4);
		}
	}

	void operator()()
	{
		FT_GlyphSlot ft_slot = ft_face_->glyph;

		int const max_dist_sq = 2 * INTERNAL_CHAR_SIZE * INTERNAL_CHAR_SIZE;
		float const scale = static_cast<float>(INTERNAL_CHAR_SIZE * INTERNAL_CHAR_SIZE) / (char_size_ * char_size_) / max_dist_sq;

		std::vector<uint8_t, aligned_allocator<uint8_t, 16> > char_bitmap(INTERNAL_CHAR_SIZE / 8 * INTERNAL_CHAR_SIZE);
		std::vector<int2> edge_points;

		raster_user_struct raster_user;
		raster_user.char_bitmap = &char_bitmap[0];

		FT_Raster_Params params;
		memset(&params, 0, sizeof(params));
		params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
		params.gray_spans = RasterCallback;
		params.user = &raster_user;

		int32_t num_packages = (num_chars_ + num_chars_per_package_ - 1) / num_chars_per_package_;
		int32_t working_package = (*cur_package_) ++;
		while (working_package < num_packages)
		{
			uint32_t const start_code = working_package * num_chars_per_package_;
			uint32_t const end_code = std::min(num_chars_, start_code + num_chars_per_package_);
			for (uint32_t c = start_code; c < end_code; ++ c)
			{
				uint32_t const ch = validate_chars_[c];
				font_info& ci = char_info_[ch];

				std::fill(char_bitmap.begin(), char_bitmap.end(), 0);

				FT_UInt gindex = FT_Get_Char_Index(ft_face_, ch);
				FT_Load_Glyph(ft_face_, gindex, FT_LOAD_NO_BITMAP);
				ci.advance_x = static_cast<uint16_t>(ft_slot->advance.x / 64.0f / INTERNAL_CHAR_SIZE * char_size_);
				ci.advance_y = static_cast<uint16_t>(ft_slot->advance.y / 64.0f / INTERNAL_CHAR_SIZE * char_size_);

				FT_Outline_Get_CBox(&ft_slot->outline, &raster_user.bbox);
				raster_user.bbox.xMin /= 64;
				raster_user.bbox.xMax /= 64;
				raster_user.bbox.yMin /= 64;
				raster_user.bbox.yMax /= 64;

				int const buf_width = std::min(static_cast<int>(raster_user.bbox.xMax - raster_user.bbox.xMin), static_cast<int>(INTERNAL_CHAR_SIZE));
				int const buf_height = std::min(static_cast<int>(raster_user.bbox.yMax - raster_user.bbox.yMin), static_cast<int>(INTERNAL_CHAR_SIZE));
				raster_user.buf_width = buf_width;
				raster_user.buf_height = buf_height;

				FT_Outline_Render(ft_lib_, &ft_slot->outline, &params);

				edge_points.resize(0);
				for (int y = 0; y < buf_height; ++ y)
				{
					edge_extract(edge_points, buf_width, &char_bitmap[0], y);
				}

				if (!edge_points.empty())
				{
					float const x_offset = (INTERNAL_CHAR_SIZE - buf_width) / 2.0f;
					float const y_offset = (INTERNAL_CHAR_SIZE - buf_height) / 2.0f;

					DistanceMap dmap(char_size_, char_size_);
					std::vector<float> tmp_dist(char_size_ * char_size_, static_cast<float>(max_dist_sq));
					for (size_t pt = 0; pt < edge_points.size(); ++ pt)
					{
						int2 const & ep = edge_points[pt];
						float2 map_xy = float2(static_cast<float>(ep.x() + x_offset), static_cast<float>(ep.y() + y_offset))
							/ static_cast<float>(INTERNAL_CHAR_SIZE) * static_cast<float>(char_size_ - 2) - 0.5f;
						int2 p0(static_cast<int32_t>(map_xy.x()), static_cast<int32_t>(map_xy.y()));
						int2 p1(p0.x() + 1, p0.y() + 1);
						float2 s(map_xy.x() - p0.x(), map_xy.y() - p0.y());
						if ((static_cast<uint32_t>(p0.x()) < char_size_) && (static_cast<uint32_t>(p0.y()) < char_size_))
						{
							float2 new_pos = s;
							float new_dist = MathLib::length_sq(new_pos);
							float& old_dist = tmp_dist[p0.y() * char_size_ + p0.x()];
							if (old_dist > new_dist)
							{
								dmap(p0.x(), p0.y()) = new_pos;
								old_dist = new_dist;
							}
						}
						if ((static_cast<uint32_t>(p1.x()) < char_size_) && (static_cast<uint32_t>(p0.y()) < char_size_))
						{
							float2 new_pos = float2(1 - s.x(), s.y());
							float new_dist = MathLib::length_sq(new_pos);
							float& old_dist = tmp_dist[p0.y() * char_size_ + p1.x()];
							if (old_dist > new_dist)
							{
								dmap(p1.x(), p0.y()) = new_pos;
								old_dist = new_dist;
							}
						}
						if ((static_cast<uint32_t>(p0.x()) < char_size_) && (static_cast<uint32_t>(p1.y()) < char_size_))
						{
							float2 new_pos = float2(s.x(), 1 - s.y());
							float new_dist = MathLib::length_sq(new_pos);
							float& old_dist = tmp_dist[p1.y() * char_size_ + p0.x()];
							if (old_dist > new_dist)
							{
								dmap(p0.x(), p1.y()) = new_pos;
								old_dist = new_dist;
							}
						}
						if ((static_cast<uint32_t>(p1.x()) < char_size_) && (static_cast<uint32_t>(p1.y()) < char_size_))
						{
							float2 new_pos = float2(1 - s.x(), 1 - s.y());
							float new_dist = MathLib::length_sq(new_pos);
							float& old_dist = tmp_dist[p1.y() * char_size_ + p1.x()];
							if (old_dist > new_dist)
							{
								dmap(p1.x(), p1.y()) = new_pos;
								old_dist = new_dist;
							}
						}
					}

					std::vector<float> distances(char_size_ * char_size_);
					ComputeDistanceField(distances, char_size_, char_size_, dmap);

					ci.left = static_cast<int16_t>((raster_user.bbox.xMin - x_offset) / INTERNAL_CHAR_SIZE * (char_size_ - 2) + 1);
					ci.top = static_cast<int16_t>((3 / 4.0f - (raster_user.bbox.yMax + y_offset) / INTERNAL_CHAR_SIZE) * (char_size_ - 2) + 1);
					ci.width = static_cast<uint16_t>(std::min<float>(1.0f, (buf_width + x_offset) / INTERNAL_CHAR_SIZE) * char_size_);
					ci.height = static_cast<uint16_t>(std::min<float>(1.0f, (buf_height + y_offset) / INTERNAL_CHAR_SIZE) * char_size_);

					for (uint32_t y = 0; y < char_size_; ++ y)
					{
						for (uint32_t x = 0; x < char_size_; ++ x)
						{
							int2 const map_xy = float2(x + 0.5f, y + 0.5f) * static_cast<float>(INTERNAL_CHAR_SIZE) / static_cast<float>(char_size_ - 2)
								- float2(static_cast<float>(x_offset), static_cast<float>(y_offset));
							float value = MathLib::sqrt(distances[y * char_size_ + x] * scale);
							if ((static_cast<uint32_t>(map_xy.x()) < INTERNAL_CHAR_SIZE) && (static_cast<uint32_t>(map_xy.y()) < INTERNAL_CHAR_SIZE))
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

							char_dist_data_[ci.dist_index + y * char_size_ + x] = value;
						}
					}
				}
				else
				{
					ci.dist_index = static_cast<uint32_t>(-1);
				}

				++ *cur_num_char_;
			}

			working_package = (*cur_package_) ++;
		}
	}

private:
	void edge_extract_sse2(std::vector<int2>& edge_points, int width, uint8_t const * char_bitmap, int y)
	{
		__m128i zero = _mm_set1_epi8(0);
		for (int x = 0; x < width; x += sizeof(__m128i) * 8)
		{
			__m128i center = _mm_load_si128(reinterpret_cast<__m128i const *>(&char_bitmap[(y * INTERNAL_CHAR_SIZE + x) / 8]));
			if (_mm_movemask_epi8(_mm_cmpeq_epi32(center, zero)) != 0xFFFF)
			{
				__m128i up;
				if (y != 0)
				{
					up = _mm_load_si128(reinterpret_cast<__m128i const *>(&char_bitmap[((y - 1) * INTERNAL_CHAR_SIZE + x) / 8]));
				}
				else
				{
					up = zero;
				}
				__m128i down;
				if (y != INTERNAL_CHAR_SIZE - 1)
				{
					down = _mm_load_si128(reinterpret_cast<__m128i const *>(&char_bitmap[((y + 1) * INTERNAL_CHAR_SIZE + x) / 8]));
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

	void edge_extract_cpp(std::vector<int2>& edge_points, int width, uint8_t const * char_bitmap, int y)
	{
		for (int x = 0; x < width; x += sizeof(uint64_t) * 8)
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
	uint32_t const * validate_chars_;
	font_info* char_info_;
	float* char_dist_data_;
	int32_t* cur_num_char_;
	atomic<int32_t>* cur_package_;
	uint32_t num_chars_;
	uint32_t thread_id_;
	uint32_t num_threads_;
	uint32_t num_chars_per_package_;

	boost::function<void(std::vector<int2>&, int, uint8_t const *, int)> edge_extract;
};

void compute_distance(std::vector<font_info>& char_info, std::vector<float>& char_dist_data,
					  int num_threads, std::vector<uint8_t> const & ttf, int start_code, int end_code, uint32_t char_size)
{
	thread_pool tp(1, num_threads);

	std::vector<int32_t> cur_num_char(num_threads, 0);

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
			if (static_cast<uint32_t>(-1) == char_info[i].dist_index)
			{
				char_info[i].dist_index = static_cast<uint32_t>(char_dist_data.size());
				char_dist_data.resize(char_dist_data.size() + char_size * char_size);
			}
		}
	}

	atomic<int32_t> cur_package(0);
	for (int i = 0; i < num_threads; ++ i)
	{
		joiners[i] = tp(ttf_to_dist(ft_libs[i], ft_faces[i], char_size, &validate_chars[0],
			&char_info[0], &char_dist_data[0], cur_num_char[i],
			cur_package, static_cast<uint32_t>(validate_chars.size()), i, num_threads, 64));
	}
	
	Timer timer;
	int32_t total_chars = static_cast<int32_t>(validate_chars.size());
	double last_disp_time = 0;
	for (;;)
	{
		int32_t dist_cur_num_char = 0;
		for (int i = 0; i < num_threads; ++ i)
		{
			dist_cur_num_char += cur_num_char[i];
		}

		double this_disp_time = timer.elapsed();
		if ((dist_cur_num_char == total_chars) || (this_disp_time - last_disp_time > 1))
		{
			cout << '\r';
			cout.width(5);
			cout << dist_cur_num_char << " / ";
			cout.width(5);
			cout << total_chars;
			cout.precision(2);
			cout << "  Time remaining (estimated): "
				<< fixed << this_disp_time / dist_cur_num_char * (total_chars - dist_cur_num_char) << " s     ";

			last_disp_time = this_disp_time;

			if (dist_cur_num_char == total_chars)
			{
				break;
			}
		}

		KlayGE::Sleep(1000);
	}

	for (int i = 0; i < num_threads; ++ i)
	{
		joiners[i]();

		FT_Done_Face(ft_faces[i]);
		FT_Done_FreeType(ft_libs[i]);
	}
}

void quantizer(std::vector<uint8_t>& lzma_dist, uint32_t non_empty_chars,
				std::pair<int32_t, int32_t> const * char_index,
				font_info const * char_info, float const * char_dist_data,
				uint32_t char_size_sq, int16_t& base, int16_t& scale)
{
	float max_value = -1;
	float min_value = 1;
	for (size_t i = 0; i < non_empty_chars; ++ i)
	{
		int const ch = char_index[i].first;
		float const * dist = &char_dist_data[char_info[ch].dist_index];
		for (size_t j = 0; j < char_size_sq; ++ j)
		{
			float value = dist[j];

			min_value = std::min(min_value, value);
			max_value = std::max(max_value, value);
		}
	}

	if (abs(max_value - min_value) < 2.0f / 65536)
	{
		max_value = min_value + 2.0f / 65536;
	}

	float fscale = max_value - min_value;
	base = static_cast<int16_t>(min_value * 32768 + 0.5f);
	scale = static_cast<int16_t>((fscale - 1) * 32768 + 0.5f);
	float inv_scale = 255 / fscale;

	float mse = 0;
	float const frscale = (scale / 32768.0f + 1) / 255.0f;
	float const fbase = base / 32768.0f;

	LZMACodec lzma_enc;
	lzma_enc.EncodeProps(5, char_size_sq);
	std::vector<uint8_t> uint8_dist(char_size_sq);
	std::vector<uint8_t> char_lzma_dist;
	for (size_t i = 0; i < non_empty_chars; ++ i)
	{
		int const ch = char_index[i].first;
		float const * dist = &char_dist_data[char_info[ch].dist_index];
		for (size_t j = 0; j < char_size_sq; ++ j)
		{
			uint8_dist[j] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((dist[j] - min_value) * inv_scale + 0.5f), 0, 255));

			float const d = dist[j] - (uint8_dist[j] * frscale + fbase);
			mse += d * d;
		}

		lzma_enc.Encode(char_lzma_dist, &uint8_dist[0], char_size_sq);
		uint64_t len = static_cast<uint64_t>(char_lzma_dist.size());

		lzma_dist.insert(lzma_dist.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len + 1));
		lzma_dist.insert(lzma_dist.end(), char_lzma_dist.begin(), char_lzma_dist.end());
	}

	cout << "Quantize MSE: " << mse << endl;
}

int main(int argc, char* argv[])
{
	kfont_header header;
	header.fourcc = MakeFourCC<'K', 'F', 'N', 'T'>::value;
	header.version = 2;
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
		("end-code,E", boost::program_options::value<int>(&end_code)->default_value(65535), "End code.")
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
	std::vector<float> char_dist_data;
	{
		memset(&char_info[0], 0, sizeof(char_info[0]) * char_info.size());
		for (size_t i = 0; i < char_info.size(); ++ i)
		{
			char_info[i].dist_index = static_cast<uint32_t>(-1);
		}

		ResIdentifierPtr kfont_input = ResLoader::Instance().Load(kfont_name);
		if (kfont_input)
		{
			kfont_input->read(&header, sizeof(header));

			if (header.fourcc != MakeFourCC<'K', 'F', 'N', 'T'>::value)
			{
				cout << "Wrong font file." << endl;
				return 1;
			}
			if (header.version != 2)
			{
				cout << "Wrong version." << endl;
				return 1;
			}

			kfont_input->seekg(header.start_ptr, ios_base::beg);

			char_index.resize(header.non_empty_chars);
			kfont_input->read(reinterpret_cast<char*>(&char_index[0]),
				static_cast<std::streamsize>(char_index.size() * sizeof(char_index[0])));

			std::vector<std::pair<int32_t, std::pair<uint16_t, uint16_t> > > advance(header.validate_chars);
			kfont_input->read(reinterpret_cast<char*>(&advance[0]),
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

				kfont_input->read(reinterpret_cast<char*>(&char_info[ch].top), sizeof(char_info[ch].top));
				kfont_input->read(reinterpret_cast<char*>(&char_info[ch].left), sizeof(char_info[ch].left));
				kfont_input->read(reinterpret_cast<char*>(&char_info[ch].width), sizeof(char_info[ch].width));
				kfont_input->read(reinterpret_cast<char*>(&char_info[ch].height), sizeof(char_info[ch].height));
			}

			LZMACodec lzma_dec;
			char_dist_data.resize(char_index.size() * header.char_size * header.char_size);
			for (size_t i = 0; i < char_index.size(); ++ i)
			{
				int const ch = char_index[i].first;

				uint64_t len;
				kfont_input->read(&len, sizeof(len));

				std::vector<uint8_t> uint8_dist;
				lzma_dec.Decode(uint8_dist, kfont_input, len, header.char_size * header.char_size);
				BOOST_ASSERT(uint8_dist.size() == header.char_size * header.char_size);

				char_info[ch].dist_index = static_cast<uint32_t>(i * header.char_size * header.char_size);
				for (size_t j = 0; j < uint8_dist.size(); ++ j)
				{
					char_dist_data[char_info[ch].dist_index + j]
						= uint8_dist[j] / 255.0f * (header.scale / 32768.0f + 1) + header.base / 32768.0f;
				}
			}
		}
	}

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
		ttf.resize(static_cast<size_t>(ttf_input.tellg()));
		ttf_input.seekg(0, ios_base::beg);
		ttf_input.read(reinterpret_cast<char*>(&ttf[0]),
			static_cast<std::streamsize>(ttf.size() * sizeof(ttf[0])));
	}

	Timer timer_total;
	Timer timer_stage;

	timer_stage.restart();
	cout << "Compute distance field..." << endl;
	compute_distance(char_info, char_dist_data, num_threads, ttf, start_code, end_code, header.char_size);
	cout << "\rTime elapsed: " << timer_stage.elapsed() << " s                                        " << endl;

	timer_stage.restart();
	cout << "Locate non-empty characters..." << endl;
	char_index.clear();
	header.non_empty_chars = 0;
	for (size_t i = 0; i < char_info.size(); ++ i)
	{
		if (char_info[i].dist_index != static_cast<uint32_t>(-1))
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
	cout << "Time elapsed: " << timer_stage.elapsed() << " s" << endl;

	cout << "Quantize..." << endl;
	timer_stage.restart();
	std::vector<uint8_t> lzma_dist;
	quantizer(lzma_dist, header.non_empty_chars, &char_index[0], &char_info[0], &char_dist_data[0],
		header.char_size * header.char_size, header.base, header.scale);
	cout << "Time elapsed: " << timer_stage.elapsed() << " s" << endl;

	int processed_chars = 0;
	for (int i = start_code; i <= end_code; ++ i)
	{
		if (char_info[i].dist_index != static_cast<uint32_t>(-1))
		{
			++ processed_chars;
		}
	}

	cout.precision(2);
	cout << fixed << processed_chars / timer_total.elapsed() << " Characters/Second" << endl;

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

			kfont_output.write(reinterpret_cast<char*>(&lzma_dist[0]),
				static_cast<std::streamsize>(lzma_dist.size() * sizeof(lzma_dist[0])));
		}
	}
}
