#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
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

TilingPostProcess::TilingPostProcess()
	: PostProcess(L"Tiling", false,
		std::vector<std::string>(),
		std::vector<std::string>(1, "src_tex"),
		std::vector<std::string>(1, "output"),
		RenderEffectPtr(), nullptr)
{
	auto effect = SyncLoadRenderEffect("TilingPP.fxml");
	this->Technique(effect, effect->TechniqueByName("Tiling"));

	downsampler_ = SyncLoadPostProcess("Copy.ppml", "bilinear_copy");

	tile_per_row_line_ep_ = effect->ParameterByName("tile_per_row_line");
}

void TilingPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	downsample_tex_ = rf.MakeTexture2D(tex->Width(0) / 2, tex->Height(0) / 2,
		4, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);

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
	downsample_tex_->BuildMipSubLevels();

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
