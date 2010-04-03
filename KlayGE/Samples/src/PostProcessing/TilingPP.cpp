#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <numeric>
#include <boost/assert.hpp>

#include "TilingPP.hpp"

using namespace KlayGE;

int const LOG_2_TILE_SIZE = 4;
int const TILE_SIZE = 1 << LOG_2_TILE_SIZE;

namespace
{
	class DownsamplerNxN : public PostProcessChain
	{
	public:
		explicit DownsamplerNxN(uint32_t n)
			: PostProcessChain(L"DownsamplerNxN")
		{
			for (uint32_t i = 0; i < n; ++ i)
			{
				this->Append(LoadPostProcess(ResLoader::Instance().Load("Downsampler2x2.ppml"), "downsampler2x2"));
			}
		}

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			uint32_t w = std::max(tex->Width(0) / 2, static_cast<uint32_t>(1));
			uint32_t h = std::max(tex->Height(0) / 2, static_cast<uint32_t>(1));
			for (size_t i = 0; i < pp_chain_.size(); ++ i)
			{
				if (0 == i)
				{
					pp_chain_[i]->InputPin(index, tex);
				}
				else
				{
					pp_chain_[i]->InputPin(index, pp_chain_[i - 1]->OutputPin(index));
				}
				if (i != pp_chain_.size() - 1)
				{
					TexturePtr ds_tex = rf.MakeTexture2D(w, h, 1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
					pp_chain_[i]->OutputPin(index, ds_tex);
				}

				w = std::max(w / 2, static_cast<uint32_t>(1));
				h = std::max(h / 2, static_cast<uint32_t>(1));
			}
		}
	};
}

TilingPostProcess::TilingPostProcess()
	: PostProcess(L"Tiling",
		std::vector<std::string>(1, "src_tex"),
		std::vector<std::string>(1, "output"),
		Context::Instance().RenderFactoryInstance().LoadEffect("TilingPP.fxml")->TechniqueByName("Tiling"))
{
	downsampler_ = MakeSharedPtr<DownsamplerNxN>(LOG_2_TILE_SIZE);

	tile_per_row_line_ep_ = technique_->Effect().ParameterByName("tile_per_row_line");
}

void TilingPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	downsample_tex_ = rf.MakeTexture2D(tex->Width(0) / TILE_SIZE, tex->Height(0) / TILE_SIZE,
		1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

	downsampler_->InputPin(index, tex);
	downsampler_->OutputPin(index, downsample_tex_);

	PostProcess::InputPin(index, downsample_tex_);
}

TexturePtr const & TilingPostProcess::InputPin(uint32_t index) const
{
	return downsampler_->InputPin(index);
}

void TilingPostProcess::Apply()
{
	downsampler_->Apply();
	PostProcess::Apply();
}

void TilingPostProcess::OnRenderBegin()
{
	PostProcess::OnRenderBegin();

	RenderEngine const & re(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	FrameBuffer const & fb(*re.CurFrameBuffer());

	*tile_per_row_line_ep_ = float4(static_cast<float>(TILE_SIZE) / fb.Width(), static_cast<float>(TILE_SIZE) / fb.Height(),
		static_cast<float>(TILE_SIZE), 1.0f / TILE_SIZE);
}
