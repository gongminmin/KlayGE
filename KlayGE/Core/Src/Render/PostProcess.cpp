// PostProcess.cpp
// KlayGE 后期处理类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2006-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 增加了BlurPostProcess (2007.3.24)
//
// 3.5.0
// 增加了GammaCorrectionProcess (2007.1.22)
//
// 3.3.0
// 初次建立 (2006.6.23)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	PostProcess::PostProcess(KlayGE::RenderTechniquePtr tech)
			: RenderableHelper(L"PostProcess")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleFan);

		pos_vb_ = rf.MakeVertexBuffer(BU_Static);
		pos_vb_->Resize(sizeof(float2) * 4);
		{
			GraphicsBuffer::Mapper mapper(*pos_vb_, BA_Write_Only);
			float2* addr = mapper.Pointer<float2>();
			addr[0] = float2(-1, +1);
			addr[1] = float2(+1, +1);
			addr[2] = float2(+1, -1);
			addr[3] = float2(-1, -1);
			box_ = Box(float3(-1, -1, -1), float3(1, 1, 1));
		}
		rl_->BindVertexStream(pos_vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

		technique_ = tech;
	}

	void PostProcess::Source(TexturePtr const & tex, bool flipping)
	{
		src_texture_ = tex;
		flipping_ = flipping;
	}

	void PostProcess::Destinate(RenderTargetPtr const & rt)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		
		if (rt)
		{
			render_target_ = rt;
		}
		else
		{
			render_target_ = re.DefaultRenderTarget();
		}
	}

	void PostProcess::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		RenderTargetPtr backup_rt = re.CurRenderTarget();
		re.BindRenderTarget(render_target_);

		this->Render();

		re.BindRenderTarget(backup_rt);
	}

	void PostProcess::OnRenderBegin()
	{
		*(technique_->Effect().ParameterByName("src_sampler")) = src_texture_;

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		float4 texel_to_pixel = re.TexelToPixelOffset();
		texel_to_pixel.x() /= render_target_->Width() / 2.0f;
		texel_to_pixel.y() /= render_target_->Height() / 2.0f;
		*(technique_->Effect().ParameterByName("texel_to_pixel_offset")) = texel_to_pixel;

		*(technique_->Effect().ParameterByName("flipping")) = flipping_ ? -1 : +1;
	}


	GammaCorrectionProcess::GammaCorrectionProcess()
		: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("GammaCorrection.kfx")->TechniqueByName("GammaCorrection"))
	{
	}

	void GammaCorrectionProcess::Gamma(float gamma)
	{
		inv_gamma_ = gamma;
	}

	void GammaCorrectionProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*(technique_->Effect().ParameterByName("inv_gamma")) = inv_gamma_;
	}


	Downsampler2x2PostProcess::Downsampler2x2PostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("Downsample.kfx")->TechniqueByName("Downsample"))
	{
	}


	BrightPassDownsampler2x2PostProcess::BrightPassDownsampler2x2PostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("Downsample.kfx")->TechniqueByName("BrightPassDownsample"))
	{
	}


	SeparableBlurPostProcess::SeparableBlurPostProcess(std::string const & tech, int kernel_radius, float multiplier)
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("Blur.kfx")->TechniqueByName(tech)),
				color_weight_(8, 0), tex_coord_offset_(8, 0),
				kernel_radius_(kernel_radius), multiplier_(multiplier)
	{
		BOOST_ASSERT((kernel_radius > 0) && (kernel_radius <= 8));
	}

	SeparableBlurPostProcess::~SeparableBlurPostProcess()
	{
	}

	void SeparableBlurPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*(technique_->Effect().ParameterByName("color_weight")) = color_weight_;
		*(technique_->Effect().ParameterByName("tex_coord_offset")) = tex_coord_offset_;
	}

	float SeparableBlurPostProcess::GaussianDistribution(float x, float y, float rho)
	{
		float g = 1.0f / sqrt(2.0f * PI * rho * rho);
		g *= exp(-(x * x + y * y) / (2 * rho * rho));
		return g;
	}

	void SeparableBlurPostProcess::CalSampleOffsets(uint32_t tex_size, float deviation)
	{
		std::vector<float> tmp_weights(kernel_radius_ * 2, 0);
		std::vector<float> tmp_offset(kernel_radius_ * 2, 0);

		float const tu = 1.0f / tex_size;

		float sum_weight = 0;
		for (int i = 0; i < 2 * kernel_radius_; ++ i)
		{
			float weight = this->GaussianDistribution(i - kernel_radius_ + 0.5f, 0, kernel_radius_ / deviation);
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
			tmp_offset[i + kernel_radius_] = static_cast<float>(i);
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
			: SeparableBlurPostProcess("BlurX", length, multiplier)
	{
	}

	void BlurXPostProcess::Source(TexturePtr const & src_tex, bool flipping)
	{
		SeparableBlurPostProcess::Source(src_tex, flipping);

		this->CalSampleOffsets(src_texture_->Width(0), 3);
	}

	BlurYPostProcess::BlurYPostProcess(int length, float multiplier)
			: SeparableBlurPostProcess("BlurY", length, multiplier)
	{
	}

	void BlurYPostProcess::Source(TexturePtr const & src_tex, bool flipping)
	{
		SeparableBlurPostProcess::Source(src_tex, flipping);

		this->CalSampleOffsets(src_texture_->Height(0), 3);
	}


	BlurPostProcess::BlurPostProcess(int kernel_radius, float multiplier)
		: PostProcess(RenderTechniquePtr()),
			blur_x_(kernel_radius, multiplier), blur_y_(kernel_radius, multiplier)
	{
	}

	void BlurPostProcess::Destinate(RenderTargetPtr const & rt)
	{
		PostProcess::Destinate(rt);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		blurx_tex_ = rf.MakeTexture2D(render_target_->Width(), render_target_->Height(), 1, render_target_->Format());

		FrameBufferPtr fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blurx_tex_, 0));
		blur_x_.Source(src_texture_, flipping_);
		blur_x_.Destinate(fb);
		blur_y_.Source(blurx_tex_, fb->RequiresFlipping());
		blur_y_.Destinate(render_target_);
	}

	void BlurPostProcess::Apply()
	{
		blur_x_.Apply();
		blur_y_.Apply();
	}
}
