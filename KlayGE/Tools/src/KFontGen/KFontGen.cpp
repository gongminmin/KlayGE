#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Timer.hpp>
#include <KFL/Math.hpp>
#include <KFL/Thread.hpp>
#include <KFL/CpuInfo.hpp>
#include <KlayGE/LZMACodec.hpp>
#include <KlayGE/DistanceField.hpp>

#include <kfont/kfont.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <atomic>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/program_options.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#if defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
#ifdef DEBUG
extern "C"
{
	int z_verbose = 0;
	void z_error (char *m)
	{
		fprintf(stderr, "%s\n", m);
		exit(1);
	}
}
#endif
#endif


using namespace std;
using namespace KlayGE;

uint32_t const NUM_CHARS = 65536;

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

struct raster_user_struct
{
	FT_BBox bbox;
	int buf_width;
	int buf_height;
	uint32_t internal_char_size;
	bool non_empty;
	uint8_t* char_bitmap;
};

void RasterCallback(int y, int count, FT_Span const * const spans, void* const user) 
{
	raster_user_struct* sptr = static_cast<raster_user_struct*>(user);

	int const y0 = sptr->buf_height - 1 - (y - sptr->bbox.yMin);
	if (y0 >= 0)
	{
		for (int i = 0; i < count; ++ i) 
		{
			if (spans[i].coverage > 127)
			{
				sptr->non_empty = true;
				int const x0 = spans[i].x - sptr->bbox.xMin;
				int const x_end = x0 + spans[i].len;
				int const x_align_8 = ((x0 + 7) & ~0x7);
				int x = max(0, x0);
				sptr->char_bitmap[(y0 * sptr->internal_char_size + x) / 8] |= static_cast<uint8_t>(~(0xFF >> (x_align_8 - x)));
				if (x_align_8 < x_end)
				{
					x = x_align_8;
				}
				else
				{
					sptr->char_bitmap[(y0 * sptr->internal_char_size + x) / 8] &= static_cast<uint8_t>((1UL << (x_end - (x & ~0x7))) - 1);
					x = x_end;
				}
				for (; x < (x_end & ~0x7); x += 8)
				{
					sptr->char_bitmap[(y0 * sptr->internal_char_size + x) / 8] = 0xFF;
				}
				if (x < x_end)
				{
					sptr->char_bitmap[(y0 * sptr->internal_char_size + x) / 8] = static_cast<uint8_t>((1UL << (x_end - x)) - 1);
				}
			}
		}
	}
}

class ttf_to_dist
{
public:
	ttf_to_dist(FT_Library ft_lib, FT_Face ft_face, uint32_t internal_char_size, uint32_t char_size,
		uint32_t const * validate_chars, font_info* char_info, float* char_dist_data,
		int32_t& cur_num_char, std::atomic<int32_t>& cur_package, uint32_t num_chars,
		float& min_value, float& max_value, uint32_t thread_id, uint32_t num_threads, uint32_t num_chars_per_package)
		: ft_lib_(ft_lib), ft_face_(ft_face), internal_char_size_(internal_char_size), char_size_(char_size),
			validate_chars_(validate_chars), char_info_(char_info), char_dist_data_(char_dist_data),
			cur_num_char_(&cur_num_char), cur_package_(&cur_package), num_chars_(num_chars),
			thread_id_(thread_id), num_threads_(num_threads), num_chars_per_package_(num_chars_per_package),
			min_value_(&min_value), max_value_(&max_value)
	{
	}

	void operator()()
	{
		*max_value_ = -1;
		*min_value_ = 1;

		FT_GlyphSlot ft_slot = ft_face_->glyph;

		float const scale = 1 / MathLib::sqrt(static_cast<float>(char_size_ * char_size_ + char_size_ * char_size_));

		std::vector<uint8_t> char_bitmap(internal_char_size_ / 8 * internal_char_size_);
		std::vector<float> aa_char_bitmap_2x(char_size_ * char_size_ * 4);
		std::vector<float> dist_data(char_size_ * char_size_);

		raster_user_struct raster_user;
		raster_user.internal_char_size = internal_char_size_;
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

				memset(&char_bitmap[0], 0, char_bitmap.size());

				FT_UInt gindex = FT_Get_Char_Index(ft_face_, ch);
				FT_Load_Glyph(ft_face_, gindex, FT_LOAD_NO_BITMAP);
				ci.advance_x = static_cast<uint16_t>(ft_slot->advance.x / 64.0f / internal_char_size_ * char_size_);
				ci.advance_y = static_cast<uint16_t>(ft_slot->advance.y / 64.0f / internal_char_size_ * char_size_);

				FT_BBox& bbox = raster_user.bbox;
				FT_Outline_Get_CBox(&ft_slot->outline, &bbox);
				bbox.xMin = MathLib::sgn(bbox.xMin) * (MathLib::abs(bbox.xMin) + 63) / 64;
				bbox.xMax = MathLib::sgn(bbox.xMax) * (MathLib::abs(bbox.xMax) + 63) / 64;
				bbox.yMin = MathLib::sgn(bbox.yMin) * (MathLib::abs(bbox.yMin) + 63) / 64;
				bbox.yMax = MathLib::sgn(bbox.yMax) * (MathLib::abs(bbox.yMax) + 63) / 64;

				int const buf_width = std::min(static_cast<int>(bbox.xMax - bbox.xMin), static_cast<int>(internal_char_size_));
				int const buf_height = std::min(static_cast<int>(bbox.yMax - bbox.yMin), static_cast<int>(internal_char_size_));
				raster_user.buf_width = buf_width;
				raster_user.buf_height = buf_height;
				raster_user.non_empty = false;

				FT_Outline_Render(ft_lib_, &ft_slot->outline, &params);

				if (raster_user.non_empty)
				{
					float const x_offset = (internal_char_size_ - buf_width) / 2.0f;
					float const y_offset = (internal_char_size_ - buf_height) / 2.0f;

					ci.left = static_cast<int16_t>((bbox.xMin - x_offset) / internal_char_size_ * char_size_ + 0.5f);
					ci.top = static_cast<int16_t>((3 / 4.0f - (bbox.yMax + y_offset) / internal_char_size_) * char_size_ + 0.5f);
					ci.width = static_cast<uint16_t>(std::min<float>(1.0f, (buf_width + x_offset) / internal_char_size_) * char_size_ + 0.5f);
					ci.height = static_cast<uint16_t>(std::min<float>(1.0f, (buf_height + y_offset) / internal_char_size_) * char_size_ + 0.5f);

					memset(&aa_char_bitmap_2x[0], 0, sizeof(aa_char_bitmap_2x[0]) * aa_char_bitmap_2x.size());
					memset(&dist_data[0], 0, sizeof(dist_data[0]) * dist_data.size());

					{
						float const fblock = static_cast<float>(internal_char_size_) / ((char_size_ - 2) * 2);
						uint32_t const block = static_cast<uint32_t>(fblock + 0.5f);
						uint32_t const block_sq = block * block;
						for (uint32_t y = 2; y < char_size_ * 2 - 2; ++ y)
						{
							for (uint32_t x = 2; x < char_size_ * 2 - 2; ++ x)
							{
								int2 const map_xy = float2(x + 0.5f, y + 0.5f) * fblock
									- float2(static_cast<float>(x_offset), static_cast<float>(y_offset));
								uint64_t aa64 = 0;
								for (uint32_t dy = 0; dy < block; ++ dy)
								{
									for (uint32_t dx = 0; dx < block; ++ dx)
									{
										if ((static_cast<uint32_t>(map_xy.x() + dx) < internal_char_size_)
											&& (static_cast<uint32_t>(map_xy.y() + dy) < internal_char_size_))
										{
											aa64 += ((char_bitmap[((map_xy.y() + dy) * internal_char_size_ + (map_xy.x() + dx)) / 8] >> ((map_xy.x() + dx) & 0x7)) & 0x1) != 0 ? 1 : 0;
										}
									}
								}

								aa_char_bitmap_2x[y * char_size_ * 2 + x] = static_cast<float>(aa64) / block_sq;
							}
						}
					}

					ComputeDistance(aa_char_bitmap_2x, char_size_ * 2, char_size_ * 2, dist_data);

					for (uint32_t i = 0; i < dist_data.size(); ++ i)
					{
						float value = dist_data[i] * scale;

						char_dist_data_[ci.dist_index + i] = value;
						*min_value_ = std::min(*min_value_, value);
						*max_value_ = std::max(*max_value_, value);
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
	FT_Library ft_lib_;
	FT_Face ft_face_;
	uint32_t internal_char_size_;
	uint32_t char_size_;
	uint32_t const * validate_chars_;
	font_info* char_info_;
	float* char_dist_data_;
	int32_t* cur_num_char_;
	std::atomic<int32_t>* cur_package_;
	uint32_t num_chars_;
	uint32_t thread_id_;
	uint32_t num_threads_;
	uint32_t num_chars_per_package_;

	float* min_value_;
	float* max_value_;
};

void compute_distance(std::vector<font_info>& char_info, std::vector<float>& char_dist_data,
						float& min_value, float& max_value,
						int num_threads, std::vector<uint8_t> const & ttf, int start_code, int end_code,
						uint32_t internal_char_size, uint32_t char_size)
{
	thread_pool tp(1, num_threads);

	std::vector<int32_t> cur_num_char(num_threads, 0);

	std::vector<FT_Library> ft_libs(num_threads);
	std::vector<FT_Face> ft_faces(num_threads);
	std::vector<joiner<void>> joiners(num_threads);

	for (int i = 0; i < num_threads; ++ i)
	{
		FT_Init_FreeType(&ft_libs[i]);
		FT_New_Memory_Face(ft_libs[i], &ttf[0], static_cast<FT_Long>(ttf.size()), 0, &ft_faces[i]);
		FT_Set_Pixel_Sizes(ft_faces[i], 0, internal_char_size);
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

	if (!validate_chars.empty())
	{
		std::vector<float> max_values(num_threads);
		std::vector<float> min_values(num_threads);
		std::atomic<int32_t> cur_package(0);
		for (int i = 0; i < num_threads; ++ i)
		{
			joiners[i] = tp(ttf_to_dist(ft_libs[i], ft_faces[i], internal_char_size, char_size,
				&validate_chars[0], &char_info[0], &char_dist_data[0], cur_num_char[i],
				cur_package, static_cast<uint32_t>(validate_chars.size()),
				std::ref(min_values[i]), std::ref(max_values[i]),
				i, num_threads, 64));
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

		max_value = -1;
		min_value = 1;
		for (int i = 0; i < num_threads; ++ i)
		{
			joiners[i]();

			min_value = std::min(min_value, min_values[i]);
			max_value = std::max(max_value, max_values[i]);
		}

		if (abs(max_value - min_value) < 2.0f / 65536)
		{
			max_value = min_value + 2.0f / 65536;
		}
	}

	for (int i = 0; i < num_threads; ++ i)
	{
		FT_Done_Face(ft_faces[i]);
		FT_Done_FreeType(ft_libs[i]);
	}
}

struct quantizer_chars_param
{
	float min_value;
	float inv_scale;
	float frscale;
	float fbase;
	std::pair<int32_t, int32_t> const * char_index;
	font_info const * char_info;
	float const * char_dist_data;
	uint32_t char_size_sq;
	uint32_t s;
	uint32_t e;
};

void quantizer_chars(std::vector<uint8_t>& lzma_dist, float& mse, quantizer_chars_param const & param)
{
	lzma_dist.clear();
	mse = 0;

	float const min_value = param.min_value;
	float const inv_scale = param.inv_scale;
	float const frscale = param.frscale;
	float const fbase = param.fbase;

	LZMACodec lzma_enc;
	std::vector<uint8_t> uint8_dist(param.char_size_sq);
	std::vector<uint8_t> char_lzma_dist;
	for (uint32_t i = param.s; i < param.e; ++ i)
	{
		int const ch = param.char_index[i].first;
		float const * dist = &param.char_dist_data[param.char_info[ch].dist_index];
		for (size_t j = 0; j < param.char_size_sq; ++ j)
		{
			uint8_dist[j] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((dist[j] - min_value) * inv_scale + 0.5f), 0, 255));

			float const d = dist[j] - (uint8_dist[j] * frscale + fbase);
			mse += d * d;
		}

		lzma_enc.Encode(char_lzma_dist, &uint8_dist[0], uint8_dist.size());
		uint64_t len = static_cast<uint64_t>(char_lzma_dist.size());

		lzma_dist.insert(lzma_dist.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len + 1));
		lzma_dist.insert(lzma_dist.end(), char_lzma_dist.begin(), char_lzma_dist.end());
	}
}

void quantizer(std::vector<uint8_t>& lzma_dist, uint32_t non_empty_chars,
				int num_threads, std::pair<int32_t, int32_t> const * char_index,
				font_info const * char_info, float const * char_dist_data,
				uint32_t char_size_sq, float min_value, float max_value,
				int16_t& base, int16_t& scale)
{
	thread_pool tp(1, num_threads);

	std::vector<joiner<void>> joiners(num_threads);

	float fscale = max_value - min_value;
	base = static_cast<int16_t>(min_value * 32768 + 0.5f);
	scale = static_cast<int16_t>((fscale - 1) * 32768 + 0.5f);
	float inv_scale = 255 / fscale;

	float const frscale = (scale / 32768.0f + 1) / 255.0f;
	float const fbase = base / 32768.0f;

	std::vector<std::vector<uint8_t>> lzma_dists(num_threads);
	std::vector<float> mses(num_threads);
	for (int i = 0; i < num_threads; ++ i)
	{
		uint32_t n = (non_empty_chars + num_threads - 1) / num_threads;
		uint32_t s = i * n;
		uint32_t e = std::min(s + n, non_empty_chars);

		quantizer_chars_param param;
		param.min_value = min_value;
		param.inv_scale = inv_scale;
		param.frscale = frscale;
		param.fbase = fbase;
		param.char_index = char_index;
		param.char_info = char_info;
		param.char_dist_data = char_dist_data;
		param.char_size_sq = char_size_sq;
		param.s = s;
		param.e = e;
		joiners[i] = tp([&lzma_dists, &mses, param, i] { quantizer_chars(lzma_dists[i], mses[i], param); });
	}

	float mse = 0;
	for (int i = 0; i < num_threads; ++ i)
	{
		joiners[i]();

		mse += mses[i];
		lzma_dist.insert(lzma_dist.end(), lzma_dists[i].begin(), lzma_dists[i].end());
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
		("output-name,O", boost::program_options::value<std::string>(), "Output font name. Default is input-name.kfont.")
		("start-code,S", boost::program_options::value<int>(&start_code)->default_value(0), "Start code. Default is 0.")
		("end-code,E", boost::program_options::value<int>(&end_code)->default_value(65535), "End code. Default is 65535.")
		("char-size,C", boost::program_options::value<uint32_t>(&header.char_size)->default_value(32), "Character size. Default is 32.")
		("threads,T", boost::program_options::value<int>(&num_threads)->default_value(cpu.NumHWThreads()), "Number of Threads. Default is the number of CPU threads.")
		("version,v", "Version.");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if ((argc <= 1) || (vm.count("help") > 0))
	{
		cout << desc << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE Font Generator, Version 2.0.0" << endl;
		return 1;
	}
	if (vm.count("input-name") > 0)
	{
		ttf_name = vm["input-name"].as<std::string>();
	}
	else
	{
		cout << "Input font name was not set." << endl;
		return 1;
	}
	if (vm.count("output-name") > 0)
	{
		kfont_name = vm["output-name"].as<std::string>();
	}
	else
	{
		kfont_name = ttf_name.substr(0, ttf_name.find_last_of('.')) + ".kfont";
	}

	std::vector<std::pair<int32_t, int32_t>> char_index;
	std::vector<font_info> char_info(NUM_CHARS);
	std::vector<float> char_dist_data;
	{
		memset(&char_info[0], 0, sizeof(char_info[0]) * char_info.size());
		for (size_t i = 0; i < char_info.size(); ++ i)
		{
			char_info[i].dist_index = static_cast<uint32_t>(-1);
		}

		KFont kfont_input;
		if (kfont_input.Load(kfont_name))
		{
			if (kfont_input.CharSize() == header.char_size)
			{
				for (size_t i = 0; i < char_info.size(); ++ i)
				{
					wchar_t ch = static_cast<wchar_t>(i);
					std::pair<int32_t, uint32_t> const & offset_adv = kfont_input.CharIndexAdvance(ch);
					if (offset_adv.first != -1)
					{
						BOOST_ASSERT(offset_adv.first == static_cast<int32_t>(char_index.size()));

						char_index.emplace_back(ch, offset_adv.first);

						KFont::font_info const & ci = kfont_input.CharInfo(offset_adv.first);
						char_info[ch].top = ci.top;
						char_info[ch].left = ci.left;
						char_info[ch].width = ci.width;
						char_info[ch].height = ci.height;

						char_info[ch].dist_index = static_cast<uint32_t>(offset_adv.first * header.char_size * header.char_size);
					}
					char_info[ch].advance_x = offset_adv.second & 0xFFFF;
					char_info[ch].advance_y = offset_adv.second >> 16;
				}

				char_dist_data.resize(char_index.size() * header.char_size * header.char_size);
				for (size_t i = 0; i < char_index.size(); ++ i)
				{
					int const ch = char_index[i].first;

					std::vector<uint8_t> uint8_dist(header.char_size * header.char_size);
					kfont_input.GetDistanceData(&uint8_dist[0], header.char_size, char_index[i].second);

					int16_t base = kfont_input.DistBase();
					int16_t scale = kfont_input.DistScale();
					for (size_t j = 0; j < uint8_dist.size(); ++ j)
					{
						char_dist_data[char_info[ch].dist_index + j]
							= uint8_dist[j] / 255.0f * (scale / 32768.0f + 1) + base / 32768.0f;
					}
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

	std::vector<uint8_t> ttf;
	{
		std::ifstream ttf_input(ttf_name.c_str(), ios_base::binary);
		if (ttf_input)
		{
			ttf_input.seekg(0, ios_base::end);
			ttf.resize(static_cast<size_t>(ttf_input.tellg()));
			ttf_input.seekg(0, ios_base::beg);
			ttf_input.read(reinterpret_cast<char*>(&ttf[0]),
				static_cast<std::streamsize>(ttf.size() * sizeof(ttf[0])));
		}
		else
		{
			cout << "Can't find " << ttf_name << endl;
			return 1;
		}
	}

	uint32_t internal_char_size = header.char_size * 4;

	float min_value;
	float max_value;

	Timer timer_total;
	Timer timer_stage;

	timer_stage.restart();
	cout << "Compute distance field..." << endl;
	compute_distance(char_info, char_dist_data, min_value, max_value, 
		num_threads, ttf, start_code, end_code, internal_char_size, header.char_size);
	cout << "\rTime elapsed: " << timer_stage.elapsed() << " s                                        " << endl;

	timer_stage.restart();
	cout << "Locate non-empty characters..." << endl;
	char_index.clear();
	header.non_empty_chars = 0;
	for (size_t i = 0; i < char_info.size(); ++ i)
	{
		if (char_info[i].dist_index != static_cast<uint32_t>(-1))
		{
			char_index.emplace_back(static_cast<int32_t>(i), header.non_empty_chars);
			++ header.non_empty_chars;
		}
	}

	std::vector<std::pair<int32_t, std::pair<uint16_t, uint16_t>>> advance;
	header.validate_chars = 0;
	for (size_t i = 0; i < char_info.size(); ++ i)
	{
		if ((char_info[i].advance_x != 0) || (char_info[i].advance_y != 0))
		{
			advance.emplace_back(static_cast<int32_t>(i), std::make_pair(char_info[i].advance_x, char_info[i].advance_y));
			++ header.validate_chars;
		}
	}
	cout << "Time elapsed: " << timer_stage.elapsed() << " s" << endl;

	cout << "Quantize..." << endl;
	timer_stage.restart();
	std::vector<uint8_t> lzma_dist;
	if (!char_index.empty())
	{
		quantizer(lzma_dist, header.non_empty_chars, num_threads, &char_index[0], &char_info[0], &char_dist_data[0],
			header.char_size * header.char_size, min_value, max_value, header.base, header.scale);
	}
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
		KFont kfont_output;
		kfont_output.CharSize(header.char_size);
		kfont_output.DistBase(header.base);
		kfont_output.DistScale(header.scale);

		if (!advance.empty())
		{
			unordered_map<int32_t, std::pair<int32_t, uint32_t>> char_index_advance;
			for (size_t i = 0; i < advance.size(); ++ i)
			{
				char_index_advance.emplace(advance[i].first, std::make_pair(-1, (advance[i].second.second << 16) + advance[i].second.first));
			}
			for (size_t i = 0; i < char_index.size(); ++ i)
			{
				auto iter = char_index_advance.find(char_index[i].first);
				BOOST_ASSERT(iter != char_index_advance.end());

				iter->second.first = char_index[i].second;
			}

			std::vector<size_t> distances_size(char_index.size());
			std::vector<size_t> distances_addr(char_index.size());
			if (!lzma_dist.empty())
			{
				uint8_t const * p0 = &lzma_dist[0];
				uint8_t const * p = p0;
				for (uint32_t i = 0; i < char_index.size(); ++ i)
				{
					uint64_t len;
					std::memcpy(&len, p, sizeof(len));
					p += sizeof(len);

					distances_size[i] = static_cast<size_t>(len);
					distances_addr[i] = p - p0;

					p += len;
				}
			}
		
			for (auto const & cia : char_index_advance)
			{
				int const ch = cia.first;
				int const index = cia.second.first;

				KFont::font_info fi;
				fi.left = char_info[ch].left;
				fi.top = char_info[ch].top;
				fi.width = char_info[ch].width;
				fi.height = char_info[ch].height;

				uint8_t const * p;
				uint32_t size;
				if (index != -1)
				{
					p = &lzma_dist[distances_addr[index]];
					size = static_cast<uint32_t>(distances_size[index]);
				}
				else
				{
					p = nullptr;
					size = 0;
				}
				kfont_output.SetLZMADistanceData(static_cast<wchar_t>(ch), p, size, cia.second.second, fi);
			}
		}

		kfont_output.Save(kfont_name);
	}
}
