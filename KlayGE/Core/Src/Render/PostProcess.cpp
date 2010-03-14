// PostProcess.cpp
// KlayGE 后期处理类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2006-2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 输入源可以有多个 (2010.3.14)
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
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>

#include <cstring>

#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	PostProcess::PostProcess()
			: RenderableHelper(L"PostProcess")
	{
		this->CreateVB();
	}

	PostProcess::PostProcess(std::vector<std::string> const & input_pin_names, KlayGE::RenderTechniquePtr const & tech)
			: RenderableHelper(L"PostProcess"),
				input_pins_(input_pin_names.size()),
				input_pins_ep_(input_pin_names.size())
	{
		this->CreateVB();

		for (size_t i = 0; i < input_pin_names.size(); ++ i)
		{
			input_pins_[i].first = input_pin_names[i];
		}
		this->Technique(tech);
	}

	void PostProcess::Technique(RenderTechniquePtr const & tech)
	{
		technique_ = tech;

		if (technique_)
		{
			texel_to_pixel_offset_ep_ = technique_->Effect().ParameterByName("texel_to_pixel_offset");
			flipping_ep_ = technique_->Effect().ParameterByName("flipping");

			input_pins_ep_.resize(input_pins_.size());
			for (size_t i = 0; i < input_pins_.size(); ++ i)
			{
				input_pins_ep_[i] = technique_->Effect().ParameterByName(input_pins_[i].first);
			}
		}
	}

	uint32_t PostProcess::NumInputPins() const
	{
		return static_cast<uint32_t>(input_pins_.size());
	}

	uint32_t PostProcess::InputPinByName(std::string const & name) const
	{
		for (size_t i = 0; i < input_pins_.size(); ++ i)
		{
			if (input_pins_[i].first == name)
			{
				return static_cast<uint32_t>(i);
			}
		}
		return 0xFFFFFFFF;
	}

	std::string const & PostProcess::InputPinName(uint32_t index) const
	{
		return input_pins_[index].first;
	}

	void PostProcess::InputPin(uint32_t index, TexturePtr const & tex, bool flipping)
	{
		input_pins_[index].second = tex;
		*(input_pins_ep_[index]) = tex;

		if (0 == index)
		{
			flipping_ = flipping;
			*flipping_ep_ = static_cast<int32_t>(flipping ? -1 : +1);
		}
	}

	void PostProcess::Destinate(FrameBufferPtr const & fb)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (fb)
		{
			frame_buffer_ = fb;
		}
		else
		{
			frame_buffer_ = re.DefaultFrameBuffer();
		}
	}

	void PostProcess::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(frame_buffer_);
		this->Render();
	}

	void PostProcess::OnRenderBegin()
	{
		if (frame_buffer_)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 texel_to_pixel = re.TexelToPixelOffset();
			texel_to_pixel.x() /= frame_buffer_->Width() / 2.0f;
			texel_to_pixel.y() /= frame_buffer_->Height() / 2.0f;
			*texel_to_pixel_offset_ep_ = texel_to_pixel;
		}
	}

	void PostProcess::CreateVB()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleStrip);

		float2 pos[] =
		{
			float2(-1, +1),
			float2(+1, +1),
			float2(-1, -1),
			float2(+1, -1)
		};
		box_ = Box(float3(-1, -1, -1), float3(1, 1, 1));
		ElementInitData init_data;
		init_data.row_pitch = sizeof(pos);
		init_data.data = &pos[0];
		pos_vb_ = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindVertexStream(pos_vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));
	}


	GammaCorrectionProcess::GammaCorrectionProcess()
		: PostProcess(std::vector<std::string>(1, "src_tex"), Context::Instance().RenderFactoryInstance().LoadEffect("GammaCorrection.fxml")->TechniqueByName("GammaCorrection"))
	{
		inv_gamma_ep_ = technique_->Effect().ParameterByName("inv_gamma");
	}

	void GammaCorrectionProcess::Gamma(float gamma)
	{
		*inv_gamma_ep_ = 1 / gamma;
	}


	Downsampler2x2PostProcess::Downsampler2x2PostProcess()
		: PostProcess(std::vector<std::string>(1, "src_tex"), Context::Instance().RenderFactoryInstance().LoadEffect("Downsample.fxml")->TechniqueByName("Downsample"))
	{
	}


	BrightPassDownsampler2x2PostProcess::BrightPassDownsampler2x2PostProcess()
		: PostProcess(std::vector<std::string>(1, "src_tex"), Context::Instance().RenderFactoryInstance().LoadEffect("Downsample.fxml")->TechniqueByName("BrightPassDownsample"))
	{
	}


	SeparableBoxFilterPostProcess::SeparableBoxFilterPostProcess(std::string const & tech, int kernel_radius, float multiplier)
		: PostProcess(std::vector<std::string>(1, "src_tex"), Context::Instance().RenderFactoryInstance().LoadEffect("Blur.fxml")->TechniqueByName(tech)),
			kernel_radius_(kernel_radius), multiplier_(multiplier)
	{
		BOOST_ASSERT((kernel_radius > 0) && (kernel_radius <= 8));

		color_weight_ep_ = technique_->Effect().ParameterByName("color_weight");
		tex_coord_offset_ep_ = technique_->Effect().ParameterByName("tex_coord_offset");
	}

	SeparableBoxFilterPostProcess::~SeparableBoxFilterPostProcess()
	{
	}

	void SeparableBoxFilterPostProcess::CalSampleOffsets(uint32_t tex_size, float /*deviation*/)
	{
		std::vector<float> color_weight(kernel_radius_ + 1, multiplier_ / (2 * kernel_radius_ + 1));
		std::vector<float> tex_coord_offset(kernel_radius_ + 1, 0);

		float const tu = 1.0f / tex_size;

		for (int i = 0; i < kernel_radius_; ++ i)
		{
			color_weight[i] *= 2;
			tex_coord_offset[i] = (i * 2 - kernel_radius_ + 0.5f) * tu;
		}
		tex_coord_offset[kernel_radius_] = kernel_radius_ * tu;

		*color_weight_ep_ = color_weight;
		*tex_coord_offset_ep_ = tex_coord_offset;
	}


	SeparableGaussianFilterPostProcess::SeparableGaussianFilterPostProcess(std::string const & tech, int kernel_radius, float multiplier)
			: PostProcess(std::vector<std::string>(1, "src_tex"), Context::Instance().RenderFactoryInstance().LoadEffect("Blur.fxml")->TechniqueByName(tech)),
				kernel_radius_(kernel_radius), multiplier_(multiplier)
	{
		BOOST_ASSERT((kernel_radius > 0) && (kernel_radius <= 8));

		color_weight_ep_ = technique_->Effect().ParameterByName("color_weight");
		tex_coord_offset_ep_ = technique_->Effect().ParameterByName("tex_coord_offset");
	}

	SeparableGaussianFilterPostProcess::~SeparableGaussianFilterPostProcess()
	{
	}

	float SeparableGaussianFilterPostProcess::GaussianDistribution(float x, float y, float rho)
	{
		float g = 1.0f / sqrt(2.0f * PI * rho * rho);
		g *= exp(-(x * x + y * y) / (2 * rho * rho));
		return g;
	}

	void SeparableGaussianFilterPostProcess::CalSampleOffsets(uint32_t tex_size, float deviation)
	{
		std::vector<float> color_weight(kernel_radius_, 0);
		std::vector<float> tex_coord_offset(kernel_radius_, 0);

		std::vector<float> tmp_weights(kernel_radius_ * 2, 0);
		std::vector<float> tmp_offset(kernel_radius_ * 2, 0);

		float const tu = 1.0f / tex_size;

		float sum_weight = 0;
		for (int i = 0; i < 2 * kernel_radius_; ++ i)
		{
			float weight = this->GaussianDistribution(static_cast<float>(i - kernel_radius_), 0, kernel_radius_ / deviation);
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

		// Bilinear filtering taps
		// Ordering is left to right.
		for (int i = 0; i < kernel_radius_; ++ i)
		{
			float const scale = tmp_weights[i * 2] + tmp_weights[i * 2 + 1];
			float const frac = tmp_weights[i * 2] / scale;

			tex_coord_offset[i] = (tmp_offset[i * 2] + (1 - frac)) * tu;
			color_weight[i] = multiplier_ * scale;
		}

		*color_weight_ep_ = color_weight;
		*tex_coord_offset_ep_ = tex_coord_offset;
	}
}
