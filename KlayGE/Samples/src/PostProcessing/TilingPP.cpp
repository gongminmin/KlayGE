#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/MapVector.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
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
	class DownsamplerNxN : public PostProcess
	{
	public:
		explicit DownsamplerNxN(uint32_t n)
			: PostProcess(RenderTechniquePtr()),
				ds_2x2_(n), ds_tex_(n - 1), ds_fb_(n - 1)
		{
		}

		void Source(TexturePtr const & src_tex, bool flipping)
		{
			PostProcess::Source(src_tex, flipping);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			uint32_t w = src_tex->Width(0);
			uint32_t h = src_tex->Height(0);
			for (size_t i = 0; i < ds_tex_.size(); ++ i)
			{
				ds_tex_[i] = rf.MakeTexture2D(w, h, 1, src_tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

				ds_fb_[i] = rf.MakeFrameBuffer();
				ds_fb_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ds_tex_[i], 0));
				if (0 == i)
				{
					ds_2x2_[i].Source(src_tex, flipping);
				}
				else
				{
					ds_2x2_[i].Source(ds_tex_[i - 1], ds_fb_[i - 1]->RequiresFlipping());
				}
				ds_2x2_[i].Destinate(ds_fb_[i]);

				w = std::max(w / 2, static_cast<uint32_t>(1));
				h = std::max(h / 2, static_cast<uint32_t>(1));
			}
		}

		void Destinate(FrameBufferPtr const & fb)
		{
			ds_2x2_.back().Source(ds_tex_[ds_tex_.size() - 1], ds_fb_[ds_tex_.size() - 1]->RequiresFlipping());
			ds_2x2_.back().Destinate(fb);
		}

		void Apply()
		{
			for (size_t i = 0; i < ds_2x2_.size(); ++ i)
			{
				ds_2x2_[i].Apply();
			}
		}

	private:
		std::vector<Downsampler2x2PostProcess> ds_2x2_;
		std::vector<TexturePtr> ds_tex_;
		std::vector<FrameBufferPtr> ds_fb_;
	};
}

TilingPostProcess::TilingPostProcess()
	: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("TilingPP.fxml")->TechniqueByName("Tiling"))
{
	downsampler_ = MakeSharedPtr<DownsamplerNxN>(LOG_2_TILE_SIZE);

	tile_per_row_line_ep_ = technique_->Effect().ParameterByName("tile_per_row_line");
}

void TilingPostProcess::Source(TexturePtr const & tex, bool flipping)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	downsample_tex_ = rf.MakeTexture2D(tex->Width(0) / TILE_SIZE, tex->Height(0) / TILE_SIZE,
		1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

	downsample_fb_ = rf.MakeFrameBuffer();
	downsample_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*downsample_tex_, 0));
	downsampler_->Source(tex, flipping);
	downsampler_->Destinate(downsample_fb_);

	PostProcess::Source(downsample_tex_, downsample_fb_->RequiresFlipping());
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
