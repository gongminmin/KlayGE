#ifndef _ASCII_LUMS_BUILDER_HPP
#define _ASCII_LUMS_BUILDER_HPP

#include <vector>
#include <KlayGE/MapVector.hpp>

namespace KlayGE
{
	typedef std::vector<uint8_t> ascii_tile_type;
	typedef std::vector<ascii_tile_type> ascii_tiles_type;

	class ascii_lums_builder
	{
	private:
		typedef MapVector<float, uint8_t> lum_to_char_type;
		typedef std::vector<std::pair<float, lum_to_char_type::const_iterator> > diff_lum_to_iter_type;

	public:
		ascii_lums_builder(size_t input_num_ascii, size_t output_num_ascii,
			size_t ascii_width, size_t ascii_height);

		ascii_tiles_type build(ascii_tiles_type const & ascii_data);

	private:
		std::vector<float> cal_lums(ascii_tiles_type const & ascii_data);
		lum_to_char_type cal_lum_to_char_map(ascii_tiles_type const & ascii_data);

		std::vector<uint8_t> get_final_asciis(lum_to_char_type const & lum_to_char);

	private:
		static bool cmp_diff_lum_to_iter(diff_lum_to_iter_type::value_type const & lhs,
								diff_lum_to_iter_type::value_type const & rhs);

	private:
		size_t input_num_ascii_;
		size_t output_num_ascii_;
		size_t ascii_width_, ascii_height_;
	};
}

#endif		// _ASCII_LUMS_BUILDER_HPP
