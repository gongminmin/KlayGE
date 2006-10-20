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
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/HDRPostProcess.hpp>

namespace KlayGE
{
	Downsampler2x2PostProcess::Downsampler2x2PostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("Downsample.kfx")->TechniqueByName("Downsample"))
	{
	}


	BlurPostProcess::BlurPostProcess(std::string const & tech, int kernel_radius, float multiplier)
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("Blur.kfx")->TechniqueByName(tech)),
				color_weight_(8, 0), tex_coord_offset_(8, 0),
				kernel_radius_(kernel_radius), multiplier_(multiplier)
	{
		BOOST_ASSERT((kernel_radius > 0) && (kernel_radius <= 8));
	}

	BlurPostProcess::~BlurPostProcess()
	{
	}

	void BlurPostProcess::Source(TexturePtr const & src_tex, Sampler::TexFilterOp filter, Sampler::TexAddressingMode am)
	{
		PostProcess::Source(src_tex, filter, am);

		this->CalSampleOffsets(src_sampler_->GetTexture()->Width(0), 3);
	}

	void BlurPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*(technique_->Effect().ParameterByName("color_weight")) = color_weight_;
		*(technique_->Effect().ParameterByName("tex_coord_offset")) = tex_coord_offset_;
	}

	float BlurPostProcess::GaussianDistribution(float x, float y, float rho)
	{
		float g = 1.0f / sqrt(2.0f * PI * rho * rho);
		g *= exp(-(x * x + y * y) / (2 * rho * rho));
		return g;
	}

	void BlurPostProcess::CalSampleOffsets(uint32_t tex_size, float deviation)
	{
		std::vector<float> tmp_weights(kernel_radius_ * 2, 0);
		std::vector<float> tmp_offset(kernel_radius_ * 2, 0);

		float const tu = 1.0f / tex_size;

		float sum_weight = 0;
		for (int i = 0; i < 2 * kernel_radius_; ++ i)
		{
			float weight = this->GaussianDistribution(i - kernel_radius_ + 0.5f, 0, deviation);
			tmp_weights[i] = weight;
			sum_weight += weight;
		}
		for (int i = 0; i < 2 * kernel_radius_; ++ i)
		{
			tmp_weights[i] /= sum_weight;
		}

		// Fill the offsets
		for (int i = 0; i < kernel_radius_; ++ i)
		{
			tmp_offset[i]                  = static_cast<float>(i - kernel_radius_);
			tmp_offset[i + kernel_radius_] = static_cast<float>(i + 1);
		}

		color_weight_.resize(kernel_radius_);
		tex_coord_offset_.resize(kernel_radius_);

		// Bilinear filtering taps 
		// Ordering is left to right.
		for (int i = 0; i < kernel_radius_; ++ i)
		{
			float const scale = tmp_weights[i * 2] + tmp_weights[i * 2 + 1];
			float const frac = tmp_weights[i * 2] / scale;

			tex_coord_offset_[i] = (tmp_offset[i * 2] + (1 - frac)) * tu;
			color_weight_[i] = multiplier_ * scale;
		}
	}


	BlurXPostProcess::BlurXPostProcess(int length, float multiplier)
			: BlurPostProcess("BlurX", length, multiplier)
	{
	}

	BlurYPostProcess::BlurYPostProcess(int length, float multiplier)
			: BlurPostProcess("BlurY", length, multiplier)
	{
	}


	SumLumPostProcess::SumLumPostProcess(std::string const & tech)
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.kfx")->TechniqueByName(tech))
	{
	}

	SumLumPostProcess::~SumLumPostProcess()
	{
	}

	void SumLumPostProcess::Source(TexturePtr const & src_tex, Sampler::TexFilterOp filter, Sampler::TexAddressingMode am)
	{
		PostProcess::Source(src_tex, filter, am);

		this->GetSampleOffsets4x4(src_tex->Width(0), src_tex->Height(0));
	}

	void SumLumPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*(technique_->Effect().ParameterByName("tex_coord_offset")) = tex_coord_offset_;
	}

	void SumLumPostProcess::GetSampleOffsets4x4(uint32_t width, uint32_t height)
	{
		tex_coord_offset_.resize(2);

		float const tu = 1.0f / width;
		float const tv = 1.0f / height;

		// Sample from the 16 surrounding points.
		int index = 0;
		for (int y = -1; y <= 2; y += 2)
		{
			for (int x = -1; x <= 2; x += 4)
			{
				tex_coord_offset_[index].x() = (x + 0) * tu;
				tex_coord_offset_[index].y() = y * tv;
				tex_coord_offset_[index].z() = (x + 2) * tu;
				tex_coord_offset_[index].w() = y * tv;

				++ index;
			}
		}
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
			adapted_samplers_[i].reset(new Sampler);
			adapted_samplers_[i]->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			adapted_samplers_[i]->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			adapted_samplers_[i]->Filtering(Sampler::TFO_Point);

			TexturePtr tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(1, 1, 1, EF_R32F);
			float data = 0;
			tex->CopyMemoryToTexture2D(0, &data, EF_R32F, 1, 1, 0, 0, 1, 1);
			adapted_samplers_[i]->SetTexture(tex);

			fb_[i] = rf.MakeFrameBuffer();
			fb_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*adapted_samplers_[i]->GetTexture(), 0));
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

		*(technique_->Effect().ParameterByName("last_lum_sampler")) = adapted_samplers_[last_index_];
		*(technique_->Effect().ParameterByName("frame_delta")) = float(timer_.elapsed());
		timer_.restart();

		last_index_ = !last_index_;
	}

	TexturePtr AdaptedLumPostProcess::AdaptedLum() const
	{
		return adapted_samplers_[last_index_]->GetTexture();
	}


	ToneMappingPostProcess::ToneMappingPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("ToneMapping.kfx")->TechniqueByName("ToneMapping20")),
				lum_sampler_(new Sampler), bloom_sampler_(new Sampler)
	{
		lum_sampler_->Filtering(Sampler::TFO_Point);
		lum_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
		lum_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);

		bloom_sampler_->Filtering(Sampler::TFO_Bilinear);
		bloom_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
		bloom_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);

		RenderEffect& effect = technique_->Effect();
		if (effect.TechniqueByName("ToneMapping30")->Validate())
		{
			technique_ = effect.TechniqueByName("ToneMapping30");
		}
	}

	void ToneMappingPostProcess::SetTexture(TexturePtr const & lum_tex, TexturePtr const & bloom_tex)
	{
		lum_sampler_->SetTexture(lum_tex);
		bloom_sampler_->SetTexture(bloom_tex);
	}

	void ToneMappingPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*(technique_->Effect().ParameterByName("lum_sampler")) = lum_sampler_;
		*(technique_->Effect().ParameterByName("bloom_sampler")) = bloom_sampler_;
	}


	HDRPostProcess::HDRPostProcess()
		: PostProcess(RenderTechniquePtr())
	{
		tone_mapping_.reset(new ToneMappingPostProcess);
		downsampler_.reset(new Downsampler2x2PostProcess);
		blur_x_.reset(new BlurXPostProcess(8, 2));
		blur_y_.reset(new BlurYPostProcess(8, 2));
		sum_lums_.resize(NUM_TONEMAP_TEXTURES + 1);
		sum_lums_[0].reset(new SumLumLogPostProcess);
		for (int i = 1; i < NUM_TONEMAP_TEXTURES + 1; ++ i)
		{
			sum_lums_[i].reset(new SumLumIterativePostProcess);
		}
		adapted_lum_.reset(new AdaptedLumPostProcess);
	}

	void HDRPostProcess::Source(TexturePtr const & tex, Sampler::TexFilterOp filter, Sampler::TexAddressingMode am)
	{
		PostProcess::Source(tex, filter, am);

		uint32_t const width = tex->Width(0);
		uint32_t const height = tex->Height(0);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		downsample_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, EF_ABGR16F);
		blurx_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, EF_ABGR16F);
		blury_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, EF_ABGR16F);

		lum_texs_.clear();
		int len = 1;
		for (int i = 0; i < NUM_TONEMAP_TEXTURES + 1; ++ i)
		{
			lum_texs_.push_back(rf.MakeTexture2D(len, len, 1, EF_GR16F));
			len *= 4;
		}
		std::reverse(lum_texs_.begin(), lum_texs_.end());

		{
			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*downsample_tex_, 0));
			downsampler_->Source(src_sampler_->GetTexture(), Sampler::TFO_Bilinear, am);
			downsampler_->Destinate(fb);
		}

		{
			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blurx_tex_, 0));
			blur_x_->Source(downsample_tex_, Sampler::TFO_Bilinear, Sampler::TAM_Clamp);
			blur_x_->Destinate(fb);
		}
		{
			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blury_tex_, 0));
			blur_y_->Source(blurx_tex_, Sampler::TFO_Bilinear, Sampler::TAM_Clamp);
			blur_y_->Destinate(fb);
		}

		{
			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*lum_texs_[0], 0));
			sum_lums_[0]->Source(src_sampler_->GetTexture(), Sampler::TFO_Bilinear, am);
			sum_lums_[0]->Destinate(fb);
		}
		for (int i = 1; i < NUM_TONEMAP_TEXTURES + 1; ++ i)
		{
			FrameBufferPtr fb = rf.MakeFrameBuffer();
			fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*lum_texs_[i], 0));
			sum_lums_[i]->Source(lum_texs_[i - 1], Sampler::TFO_Bilinear, Sampler::TAM_Clamp);
			sum_lums_[i]->Destinate(fb);
		}

		{
			adapted_lum_->Source(lum_texs_[NUM_TONEMAP_TEXTURES], Sampler::TFO_Point, Sampler::TAM_Clamp);
		}

		{
			tone_mapping_->Source(src_sampler_->GetTexture(), Sampler::TFO_Bilinear, am);
			tone_mapping_->Destinate(render_target_);
		}
	}

	void HDRPostProcess::Apply()
	{
		// 降采样
		downsampler_->Apply();
		// Blur X
		blur_x_->Apply();
		// Blur Y
		blur_y_->Apply();

		// 降采样4x4 log
		sum_lums_[0]->Apply();
		for (size_t i = 1; i < sum_lums_.size() - 1; ++ i)
		{
			// 降采样4x4
			sum_lums_[i]->Apply();
		}
		// 降采样4x4 exp
		sum_lums_[sum_lums_.size() - 1]->Apply();

		adapted_lum_->Apply();

		{
			// Tone mapping
			ToneMappingPostProcessPtr ppor = checked_pointer_cast<ToneMappingPostProcess>(tone_mapping_);
			ppor->SetTexture(checked_pointer_cast<AdaptedLumPostProcess>(adapted_lum_)->AdaptedLum(), blury_tex_);
			ppor->Apply();
		}
	}
}
