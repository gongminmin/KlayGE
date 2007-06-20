// HDRPostProcess.cpp
// KlayGE HDR后期处理类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.8.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>

namespace KlayGE
{
	SumLumPostProcess::SumLumPostProcess(std::string const & tech)
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.kfx")->TechniqueByName(tech))
	{
	}

	SumLumPostProcess::~SumLumPostProcess()
	{
	}

	void SumLumPostProcess::Source(TexturePtr const & src_tex, bool flipping)
	{
		PostProcess::Source(src_tex, flipping);

		this->GetSampleOffsets4x4(src_tex->Width(0), src_tex->Height(0));
	}

	void SumLumPostProcess::GetSampleOffsets4x4(uint32_t width, uint32_t height)
	{
		std::vector<float4> tex_coord_offset(2);

		float const tu = 1.0f / width;
		float const tv = 1.0f / height;

		// Sample from the 16 surrounding points.
		int index = 0;
		for (int y = -1; y <= 2; y += 2)
		{
			for (int x = -1; x <= 2; x += 4)
			{
				tex_coord_offset[index].x() = (x + 0) * tu;
				tex_coord_offset[index].y() = y * tv;
				tex_coord_offset[index].z() = (x + 2) * tu;
				tex_coord_offset[index].w() = y * tv;

				++ index;
			}
		}

		*(technique_->Effect().ParameterByName("tex_coord_offset")) = tex_coord_offset;
	}


	SumLumLogPostProcess::SumLumLogPostProcess()
			: SumLumPostProcess("SumLumLog")
	{
	}

	SumLumIterativePostProcess::SumLumIterativePostProcess()
			: SumLumPostProcess("SumLumIterative")
	{
	}


	AdaptedLumPostProcess::AdaptedLumPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.kfx")->TechniqueByName("AdaptedLum")),
				last_index_(false)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		for (int i = 0; i < 2; ++ i)
		{
			TexturePtr tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(1, 1, 1, EF_R32F);
			float data = 0;
			tex->CopyMemoryToTexture2D(0, &data, EF_R32F, 1, 1, 0, 0, 1, 1);
			adapted_textures_[i] = tex;

			fb_[i] = rf.MakeFrameBuffer();
			fb_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*adapted_textures_[i], 0));
		}

		this->Destinate(fb_[last_index_]);
	}

	void AdaptedLumPostProcess::Apply()
	{
		this->Destinate(fb_[!last_index_]);

		PostProcess::Apply();
	}

	void AdaptedLumPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*(technique_->Effect().ParameterByName("last_lum_sampler")) = adapted_textures_[last_index_];
		*(technique_->Effect().ParameterByName("frame_delta")) = float(timer_.elapsed());
		timer_.restart();

		last_index_ = !last_index_;
	}

	TexturePtr AdaptedLumPostProcess::AdaptedLum() const
	{
		return adapted_textures_[last_index_];
	}


	ToneMappingPostProcess::ToneMappingPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("ToneMapping.kfx")->TechniqueByName("ToneMapping20"))
	{
		RenderEffect& effect = technique_->Effect();
		if (effect.TechniqueByName("ToneMapping30")->Validate())
		{
			technique_ = effect.TechniqueByName("ToneMapping30");
		}
	}

	void ToneMappingPostProcess::SetTexture(TexturePtr const & lum_tex, TexturePtr const & bloom_tex)
	{
		*(technique_->Effect().ParameterByName("lum_sampler")) = lum_tex;
		*(technique_->Effect().ParameterByName("bloom_sampler")) = bloom_tex;
	}


	HDRPostProcess::HDRPostProcess()
		: PostProcess(RenderTechniquePtr()),
			blur_(8, 2)
	{
		sum_lums_.resize(NUM_TONEMAP_TEXTURES);
	}

	void HDRPostProcess::Source(TexturePtr const & tex, bool flipping)
	{
		PostProcess::Source(tex, flipping);

		uint32_t const width = tex->Width(0);
		uint32_t const height = tex->Height(0);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		downsample_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, EF_ABGR16F);
		blur_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, EF_ABGR16F);

		lum_texs_.clear();
		int len = 1;
		for (size_t i = 0; i < sum_lums_.size() + 1; ++ i)
		{
			lum_texs_.push_back(rf.MakeTexture2D(len, len, 1, EF_GR16F));
			len *= 4;
		}
		std::reverse(lum_texs_.begin(), lum_texs_.end());

		bool tmp_flipping;
		{
			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*downsample_tex_, 0));
			downsampler_.Source(src_texture_, flipping);
			downsampler_.Destinate(fb);
			tmp_flipping = fb->RequiresFlipping();
		}

		{
			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blur_tex_, 0));
			blur_.Source(downsample_tex_, tmp_flipping);
			blur_.Destinate(fb);
			tmp_flipping = fb->RequiresFlipping();
		}

		{
			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*lum_texs_[0], 0));
			sum_lums_1st_.Source(src_texture_, tmp_flipping);
			sum_lums_1st_.Destinate(fb);
			tmp_flipping = fb->RequiresFlipping();
		}
		for (size_t i = 0; i < sum_lums_.size(); ++ i)
		{
			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*lum_texs_[i + 1], 0));
			sum_lums_[i].Source(lum_texs_[i], tmp_flipping);
			sum_lums_[i].Destinate(fb);
			tmp_flipping = fb->RequiresFlipping();
		}

		{
			adapted_lum_.Source(lum_texs_[sum_lums_.size()], tmp_flipping);
		}

		{
			tone_mapping_.Source(src_texture_, flipping);
		}
	}

	void HDRPostProcess::Destinate(FrameBufferPtr const & fb)
	{
		tone_mapping_.Destinate(fb);
	}

	void HDRPostProcess::Apply()
	{
		// 降采样
		downsampler_.Apply();
		// Blur
		blur_.Apply();

		// 降采样4x4 log
		sum_lums_1st_.Apply();
		for (size_t i = 0; i < sum_lums_.size(); ++ i)
		{
			// 降采样4x4
			sum_lums_[i].Apply();
		}

		adapted_lum_.Apply();

		{
			// Tone mapping
			tone_mapping_.SetTexture(adapted_lum_.AdaptedLum(), blur_tex_);
			tone_mapping_.Apply();
		}
	}
}
