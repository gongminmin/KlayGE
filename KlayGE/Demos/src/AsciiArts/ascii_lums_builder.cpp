#include <KlayGE/KlayGE.hpp>

#include <numeric>
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>
#include <boost/assert.hpp>

#include "ascii_lums_builder.hpp"

namespace KlayGE
{
	ascii_lums_builder::ascii_lums_builder(size_t input_num_ascii, size_t output_num_ascii, size_t lum_level,
			size_t ascii_width, size_t ascii_height)
				: input_num_ascii_(input_num_ascii),
					output_num_ascii_(output_num_ascii),
					ascii_width_(ascii_width), ascii_height_(ascii_height)
	{
	}

	ascii_tiles_type ascii_lums_builder::build(ascii_tiles_type const & ascii_data)
	{
		BOOST_ASSERT(ascii_data.size() == input_num_ascii_);

		lum_to_char_type lum_to_char = this->cal_lum_to_char_map(ascii_data);
		std::vector<uint8_t> final_chars = get_final_asciis(lum_to_char);

		ascii_tiles_type ret(output_num_ascii_);
		for (size_t i = 0; i < output_num_ascii_; ++ i)
		{
			BOOST_ASSERT(ascii_data[final_chars[i]].size() == ascii_width_ * ascii_height_);

			ret[i] = ascii_data[final_chars[i]];
		}

		return ret;
	}

	std::vector<float> ascii_lums_builder::cal_lums(ascii_tiles_type const & ascii_data)
	{
		BOOST_ASSERT(ascii_data.size() == input_num_ascii_);

		std::vector<float> ret(input_num_ascii_);
		for (size_t i = 0; i < ret.size(); ++ i)
		{
			BOOST_ASSERT(ascii_data[i].size() == ascii_width_ * ascii_height_);

			ret[i] = std::accumulate(ascii_data[i].begin(), ascii_data[i].end(), 0) / 256.0f;
		}

		return ret;
	}

	ascii_lums_builder::lum_to_char_type ascii_lums_builder::cal_lum_to_char_map(ascii_tiles_type const & ascii_data)
	{
		BOOST_ASSERT(ascii_data.size() == input_num_ascii_);
		BOOST_ASSERT(ascii_data.size() >= output_num_ascii_);

		lum_to_char_type ret;

		std::vector<float> lums = cal_lums(ascii_data);

		float max_lum = *std::max_element(lums.begin(), lums.end());
		for (std::vector<float>::const_iterator iter = lums.begin();
			iter != lums.end(); ++ iter)
		{
			float char_lum = *iter / max_lum * output_num_ascii_;
			if (ret.find(char_lum) == ret.end())
			{
				ret.insert(std::make_pair(char_lum, static_cast<uint8_t>(iter - lums.begin())));
			}
		}
		BOOST_ASSERT(ret.size() >= output_num_ascii_);

		return ret;
	}

	std::vector<uint8_t> ascii_lums_builder::get_final_asciis(lum_to_char_type const & lum_to_char)
	{
		BOOST_ASSERT(lum_to_char.size() >= output_num_ascii_);

		diff_lum_to_iter_type diff_lum_to_iter;

		for (lum_to_char_type::const_iterator iter = lum_to_char.begin(); iter != lum_to_char.end(); ++ iter)
		{
			float diff_lum;

			if (iter != lum_to_char.begin())
			{
				lum_to_char_type::const_iterator prev_iter = iter;
				-- prev_iter;
				diff_lum = iter->first - prev_iter->first;
			}
			else
			{
				diff_lum = iter->first;
			}

			diff_lum_to_iter.push_back(std::make_pair(diff_lum, iter));
		}
		BOOST_ASSERT(diff_lum_to_iter.size() >= output_num_ascii_);

		std::partial_sort(diff_lum_to_iter.begin(), diff_lum_to_iter.begin() + output_num_ascii_,
			diff_lum_to_iter.end(), cmp_diff_lum_to_iter);
		diff_lum_to_iter.resize(output_num_ascii_);

		lum_to_char_type final_lum_to_char;
		for (size_t i = 0; i < diff_lum_to_iter.size(); ++ i)
		{
			final_lum_to_char.insert(*diff_lum_to_iter[i].second);
		}

		std::vector<uint8_t> ret;
		for (lum_to_char_type::iterator iter = final_lum_to_char.begin();
			iter != final_lum_to_char.end(); ++ iter)
		{
			ret.push_back(iter->second);
		}

		return ret;
	}

	bool ascii_lums_builder::cmp_diff_lum_to_iter(diff_lum_to_iter_type::value_type const & lhs,
							diff_lum_to_iter_type::value_type const & rhs)
	{
		return lhs.first > rhs.first;
	}
}
