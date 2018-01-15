#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ResLoader.hpp>
#include <kfont/kfont.hpp>

#include <numeric>
#include <boost/assert.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing" // Ignore aliasing in flat_tree.hpp
#elif defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter in boost
#endif
#include <boost/container/flat_map.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include "AsciiArtsPP.hpp"

using namespace KlayGE;

int const CELL_WIDTH = 16;
int const CELL_HEIGHT = 16;
int const INPUT_NUM_ASCII = 128;
size_t const ASCII_WIDTH = 16;
size_t const ASCII_HEIGHT = 16;

size_t const OUTPUT_NUM_ASCII = 64;

namespace
{
	typedef std::vector<uint8_t> ascii_tile_type;
	typedef std::vector<ascii_tile_type> ascii_tiles_type;

	std::vector<ascii_tile_type> LoadFromKFont(std::string const & font_name)
	{
		ResIdentifierPtr kfont_input = ResLoader::Instance().Open(font_name);
		std::shared_ptr<KFont> kfont_loader = MakeSharedPtr<KFont>();
		kfont_loader->Load(kfont_input);

		float const dist_base = kfont_loader->DistBase() / 32768.0f * 32 + 1;
		float const dist_scale = (kfont_loader->DistScale() / 32768.0f + 1.0f) * 32;
		uint32_t const char_size = kfont_loader->CharSize();

		std::vector<ascii_tile_type> ret(INPUT_NUM_ASCII);

		std::vector<uint8_t> char_data(char_size * char_size);
		for (int ch = 0; ch < INPUT_NUM_ASCII; ++ ch)
		{
			ret[ch].resize(ASCII_WIDTH * ASCII_HEIGHT);

			int32_t index = kfont_loader->CharIndex(static_cast<wchar_t>(ch));
			if (index >= 0)
			{
				kfont_loader->GetDistanceData(&char_data[0], char_size, index);
				for (size_t dy = 0; dy < char_size; ++ dy)
				{
					for (size_t dx = 0; dx < char_size; ++ dx)
					{
						float texel = char_data[dy * char_size + dx] / 255.0f;
						texel = texel * dist_scale + dist_base;
						char_data[dy * char_size + dx]
							= static_cast<uint8_t>(MathLib::clamp<int>(static_cast<int>(texel * 255.0f + 0.5f), 0, 255));
					}
				}

				ResizeTexture(&ret[ch][0], ASCII_WIDTH, ASCII_WIDTH * ASCII_HEIGHT, EF_R8, ASCII_WIDTH, ASCII_HEIGHT, 1,
					&char_data[char_size / 6], char_size, char_size * char_size, EF_R8, char_size * 2 / 3, char_size, 1, true);
			}
			else
			{
				memset(&ret[ch][0], 0, ret[ch].size());
			}
		}

		return ret;
	}

	TexturePtr FillTexture(ascii_tiles_type const & ascii_lums)
	{
		BOOST_ASSERT(OUTPUT_NUM_ASCII == ascii_lums.size());

		std::vector<uint8_t> data_v(OUTPUT_NUM_ASCII * ASCII_WIDTH * ASCII_HEIGHT);
		for (size_t i = 0; i < OUTPUT_NUM_ASCII; ++ i)
		{
			for (size_t y = 0; y < ASCII_HEIGHT; ++ y)
			{
				for (size_t x = 0; x < ASCII_WIDTH; ++ x)
				{
					data_v[y * OUTPUT_NUM_ASCII * ASCII_WIDTH + i * ASCII_WIDTH + x]
						= ascii_lums[i][y * ASCII_WIDTH + x];
				}
			}
		}

		ElementInitData init_data;
		init_data.data = &data_v[0];
		init_data.row_pitch = OUTPUT_NUM_ASCII * ASCII_WIDTH;
		init_data.slice_pitch = 0;

		return Context::Instance().RenderFactoryInstance().MakeTexture2D(OUTPUT_NUM_ASCII * ASCII_WIDTH,
			ASCII_HEIGHT, 1, 1, EF_R8, 1, 0, EAH_GPU_Read | EAH_Immutable, init_data);
	}

	class ascii_lums_builder
	{
	private:
		typedef boost::container::flat_map<float, uint8_t> lum_to_char_type;
		typedef std::vector<std::pair<float, lum_to_char_type::const_iterator>> diff_lum_to_iter_type;

	public:
		ascii_lums_builder(size_t input_num_ascii, size_t output_num_ascii,
				size_t ascii_width, size_t ascii_height)
			: input_num_ascii_(input_num_ascii),
						output_num_ascii_(output_num_ascii),
						ascii_width_(ascii_width), ascii_height_(ascii_height)
		{
			KFL_UNUSED(ascii_width_);
			KFL_UNUSED(ascii_height_);
		}

		ascii_tiles_type build(ascii_tiles_type const & ascii_data)
		{
			BOOST_ASSERT(ascii_data.size() == input_num_ascii_);

			lum_to_char_type lum_to_char = this->cal_lum_to_char_map(ascii_data);
			std::vector<uint8_t> final_chars = this->get_final_asciis(lum_to_char);

			ascii_tiles_type ret(output_num_ascii_);
			for (size_t i = 0; i < output_num_ascii_; ++ i)
			{
				BOOST_ASSERT(ascii_data[final_chars[i]].size() == ascii_width_ * ascii_height_);

				ret[i] = ascii_data[final_chars[i]];
			}

			return ret;
		}

	private:
		std::vector<float> cal_lums(ascii_tiles_type const & ascii_data)
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
		lum_to_char_type cal_lum_to_char_map(ascii_tiles_type const & ascii_data)
		{
			BOOST_ASSERT(ascii_data.size() == input_num_ascii_);
			BOOST_ASSERT(ascii_data.size() >= output_num_ascii_);

			lum_to_char_type ret;

			std::vector<float> lums = this->cal_lums(ascii_data);

			float max_lum = *std::max_element(lums.begin(), lums.end());
			for (auto iter = lums.begin(); iter != lums.end(); ++ iter)
			{
				float char_lum = *iter / max_lum * output_num_ascii_;
				if (ret.find(char_lum) == ret.end())
				{
					ret.emplace(char_lum, static_cast<uint8_t>(iter - lums.begin()));
				}
			}
			BOOST_ASSERT(ret.size() >= output_num_ascii_);

			return ret;
		}

		std::vector<uint8_t> get_final_asciis(lum_to_char_type const & lum_to_char)
		{
			BOOST_ASSERT(lum_to_char.size() >= output_num_ascii_);

			diff_lum_to_iter_type diff_lum_to_iter;

			for (auto iter = lum_to_char.begin(); iter != lum_to_char.end(); ++ iter)
			{
				float diff_lum;

				if (iter != lum_to_char.begin())
				{
					auto prev_iter = iter;
					-- prev_iter;
					diff_lum = iter->first - prev_iter->first;
				}
				else
				{
					diff_lum = iter->first;
				}

				diff_lum_to_iter.emplace_back(diff_lum, iter);
			}
			BOOST_ASSERT(diff_lum_to_iter.size() >= output_num_ascii_);

			std::partial_sort(diff_lum_to_iter.begin(), diff_lum_to_iter.begin() + output_num_ascii_,
				diff_lum_to_iter.end(), cmp_diff_lum_to_iter);
			diff_lum_to_iter.resize(output_num_ascii_);

			lum_to_char_type final_lum_to_char;
			for (auto const & lum_to_iter : diff_lum_to_iter)
			{
				final_lum_to_char.insert(*lum_to_iter.second);
			}

			std::vector<uint8_t> ret;
			for (auto const & l2c : final_lum_to_char)
			{
				ret.push_back(l2c.second);
			}

			return ret;
		}

	private:
		static bool cmp_diff_lum_to_iter(diff_lum_to_iter_type::value_type const & lhs,
								diff_lum_to_iter_type::value_type const & rhs)
		{
			return lhs.first > rhs.first;
		}

	private:
		size_t input_num_ascii_;
		size_t output_num_ascii_;
		size_t ascii_width_, ascii_height_;
	};
}

AsciiArtsPostProcess::AsciiArtsPostProcess()
	: PostProcess(L"AsciiArts", false,
			{},
			{ "src_tex" },
			{ "output" },
			RenderEffectPtr(), nullptr)
{
	auto effect = SyncLoadRenderEffect("AsciiArtsPP.fxml");
	this->Technique(effect, effect->TechniqueByName("AsciiArts"));

	ascii_lums_builder builder(INPUT_NUM_ASCII, OUTPUT_NUM_ASCII, ASCII_WIDTH, ASCII_HEIGHT);

	downsampler_ = SyncLoadPostProcess("Copy.ppml", "bilinear_copy");

	cell_per_row_line_ep_ = effect->ParameterByName("cell_per_row_line");
	*(effect->ParameterByName("lums_tex")) = FillTexture(builder.build(LoadFromKFont("gkai00mp.kfont")));
}

void AsciiArtsPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	downsample_tex_ = rf.MakeTexture2D(tex->Width(0) / 2, tex->Height(0) / 2,
		4, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);

	downsampler_->InputPin(index, tex);
	downsampler_->OutputPin(index, downsample_tex_);

	PostProcess::InputPin(index, downsample_tex_);

	*cell_per_row_line_ep_ = float2(static_cast<float>(CELL_WIDTH) / tex->Width(0), static_cast<float>(CELL_HEIGHT) / tex->Height(0));
}

TexturePtr const & AsciiArtsPostProcess::InputPin(uint32_t index) const
{
	return downsampler_->InputPin(index);
}

void AsciiArtsPostProcess::Apply()
{
	downsampler_->Apply();
	downsample_tex_->BuildMipSubLevels();

	PostProcess::Apply();
}
