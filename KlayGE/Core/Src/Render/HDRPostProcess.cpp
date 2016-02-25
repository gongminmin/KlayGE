// HDRPostProcess.cpp
// KlayGE HDR后期处理类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2006-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// 改进了Tone mapping (2010.7.7)
//
// 3.4.0
// 初次建立 (2006.8.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/FFT.hpp>
#include <KlayGE/PostProcess.hpp>

#include <boost/assert.hpp>

#include <KlayGE/HDRPostProcess.hpp>

namespace KlayGE
{
	SumLumPostProcess::SumLumPostProcess(RenderTechniquePtr const & tech)
		: PostProcess(L"SumLum",
			std::vector<std::string>(),
			std::vector<std::string>(1, "src_tex"),
			std::vector<std::string>(1, "out_tex"),
			tech)
	{
		tex_coord_offset_ep_ = technique_->Effect().ParameterByName("tex_coord_offset");
	}

	SumLumPostProcess::~SumLumPostProcess()
	{
	}

	void SumLumPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		PostProcess::InputPin(index, tex);
		this->GetSampleOffsets4x4(tex->Width(0), tex->Height(0));
	}

	TexturePtr const & SumLumPostProcess::InputPin(uint32_t index) const
	{
		return PostProcess::InputPin(index);
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

		*tex_coord_offset_ep_ = tex_coord_offset;
	}


	SumLumLogPostProcess::SumLumLogPostProcess()
			: SumLumPostProcess(SyncLoadRenderEffect("SumLum.fxml")->TechniqueByName("SumLumLog"))
	{
	}


	SumLumLogPostProcessCS::SumLumLogPostProcessCS()
			: SumLumPostProcess(SyncLoadRenderEffect("SumLum.fxml")->TechniqueByName("SumLumLogCS"))
	{
	}

	void SumLumLogPostProcessCS::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.DefaultFrameBuffer());
		re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color);

		*(technique_->Effect().ParameterByName("dst_tex_dim")) = int2(64, 64);

		this->OnRenderBegin();
		re.Dispatch(*technique_, 2, 2, 1);
		this->OnRenderEnd();
	}


	SumLumIterativePostProcess::SumLumIterativePostProcess()
			: SumLumPostProcess(SyncLoadRenderEffect("SumLum.fxml")->TechniqueByName("SumLumIterative"))
	{
	}


	AdaptedLumPostProcess::AdaptedLumPostProcess()
			: PostProcess(L"AdaptedLum",
					std::vector<std::string>(),
					std::vector<std::string>(1, "src_tex"),
					std::vector<std::string>(1, "output"),
					RenderTechniquePtr()),
				last_index_(false)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		std::vector<int> data_v(4, 0);
		ElementInitData init_data;
		init_data.row_pitch = sizeof(int);
		init_data.slice_pitch = 0;
		init_data.data = &data_v[0];
		ElementFormat fmt;
		if (caps.pack_to_rgba_required)
		{
			if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));
				fmt = EF_ARGB8;
			}
		}
		else
		{
			fmt = EF_R32F;
		}

		this->Technique(SyncLoadRenderEffect("SumLum.fxml")->TechniqueByName("AdaptedLum"));

		adapted_textures_[0] = rf.MakeTexture2D(1, 1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &init_data);
		adapted_textures_[1] = rf.MakeTexture2D(1, 1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &init_data);
		this->OutputPin(0, adapted_textures_[last_index_]);

		last_lum_tex_ep_ = technique_->Effect().ParameterByName("last_lum_tex");
		frame_delta_ep_ = technique_->Effect().ParameterByName("frame_delta");
	}

	void AdaptedLumPostProcess::Apply()
	{
		this->OutputPin(0, adapted_textures_[!last_index_]);

		PostProcess::Apply();
	}

	void AdaptedLumPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*last_lum_tex_ep_ = adapted_textures_[last_index_];
		*frame_delta_ep_ = Context::Instance().AppInstance().FrameTime();

		last_index_ = !last_index_;
	}


	AdaptedLumPostProcessCS::AdaptedLumPostProcessCS()
			: PostProcess(L"AdaptedLumCS", 
					std::vector<std::string>(),
					std::vector<std::string>(1, "src_tex"),
					std::vector<std::string>(1, "out_tex"),
					SyncLoadRenderEffect("SumLum.fxml")->TechniqueByName("AdaptedLumCS"))
	{
		frame_delta_ep_ = technique_->Effect().ParameterByName("frame_delta");
	}

	void AdaptedLumPostProcessCS::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.DefaultFrameBuffer());
		re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color);

		this->OnRenderBegin();
		re.Dispatch(*technique_, 1, 1, 1);
		this->OnRenderEnd();
	}

	void AdaptedLumPostProcessCS::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*frame_delta_ep_ = Context::Instance().AppInstance().FrameTime();
	}


	ToneMappingPostProcess::ToneMappingPostProcess()
		: PostProcess(L"ToneMapping")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		input_pins_.emplace_back("src_tex", TexturePtr());
		input_pins_.emplace_back("lum_tex", TexturePtr());
		input_pins_.emplace_back("bloom_tex", TexturePtr());

		output_pins_.emplace_back("out_tex", TexturePtr());
		
		RenderEffectPtr effect = SyncLoadRenderEffect("ToneMapping.fxml");
		RenderTechniquePtr tech;
		if (caps.max_shader_model >= ShaderModel(3, 0))
		{
			tech = effect->TechniqueByName("ToneMapping30");
		}
		else
		{
			tech = effect->TechniqueByName("ToneMapping20");
		}

		this->Technique(tech);
	}


	ImageStatPostProcess::ImageStatPostProcess()
		: PostProcess(L"ImageStat")
	{
		sum_lums_1st_ = MakeSharedPtr<SumLumLogPostProcess>();
		sum_lums_.resize(3);
		for (size_t i = 0; i < sum_lums_.size(); ++ i)
		{
			sum_lums_[i] = MakeSharedPtr<SumLumIterativePostProcess>();
		}
		adapted_lum_ = MakeSharedPtr<AdaptedLumPostProcess>();
	}

	void ImageStatPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		std::vector<TexturePtr> lum_texs(sum_lums_.size() + 1);
		ElementFormat fmt;
		if (caps.pack_to_rgba_required)
		{
			if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));
				fmt = EF_ARGB8;
			}
		}
		else
		{
			fmt = EF_R16F;
		}

		int len = 1;
		for (size_t i = 0; i < sum_lums_.size() + 1; ++ i)
		{
			lum_texs[sum_lums_.size() - i] = rf.MakeTexture2D(len, len, 1, 1, fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write, nullptr);
			len *= 4;
		}

		{
			sum_lums_1st_->InputPin(index, tex);
			sum_lums_1st_->OutputPin(index, lum_texs[0]);
		}
		for (size_t i = 0; i < sum_lums_.size(); ++ i)
		{
			sum_lums_[i]->InputPin(index, lum_texs[i]);
			sum_lums_[i]->OutputPin(index, lum_texs[i + 1]);
		}

		{
			adapted_lum_->InputPin(index, lum_texs[sum_lums_.size()]);
		}
	}

	TexturePtr const & ImageStatPostProcess::InputPin(uint32_t index) const
	{
		return sum_lums_1st_->InputPin(index);
	}

	void ImageStatPostProcess::OutputPin(uint32_t index, TexturePtr const & tex, int level, int array_index, int face)
	{
		adapted_lum_->OutputPin(index, tex, level, array_index, face);
	}

	TexturePtr const & ImageStatPostProcess::OutputPin(uint32_t index) const
	{
		return adapted_lum_->OutputPin(index);
	}

	void ImageStatPostProcess::Apply()
	{
		// 降采样4x4 log
		sum_lums_1st_->Apply();
		for (size_t i = 0; i < sum_lums_.size(); ++ i)
		{
			// 降采样4x4
			sum_lums_[i]->Apply();
		}

		adapted_lum_->Apply();
	}


	ImageStatPostProcessCS::ImageStatPostProcessCS()
		: PostProcess(L"ImageStatCS")
	{
		sum_lums_1st_ = MakeSharedPtr<SumLumLogPostProcessCS>();
		adapted_lum_ = MakeSharedPtr<AdaptedLumPostProcessCS>();
	}

	void ImageStatPostProcessCS::InputPin(uint32_t index, TexturePtr const & tex)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		TexturePtr lum_tex = rf.MakeTexture2D(2, 2, 1, 1, EF_R16F, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered, nullptr);

		float init_lum = 0;
		ElementInitData init_data;
		init_data.row_pitch = sizeof(float);
		init_data.slice_pitch = 0;
		init_data.data = &init_lum;
		TexturePtr adapted_lum_tex = rf.MakeTexture2D(1, 1, 1, 1, EF_R32F, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered, &init_data);

		sum_lums_1st_->InputPin(index, tex);
		sum_lums_1st_->OutputPin(index, lum_tex);
		adapted_lum_->InputPin(index, lum_tex);
		adapted_lum_->OutputPin(index, adapted_lum_tex);
	}

	TexturePtr const & ImageStatPostProcessCS::InputPin(uint32_t index) const
	{
		return sum_lums_1st_->InputPin(index);
	}

	void ImageStatPostProcessCS::OutputPin(uint32_t index, TexturePtr const & tex, int level, int array_index, int face)
	{
		adapted_lum_->OutputPin(index, tex, level, array_index, face);
	}

	TexturePtr const & ImageStatPostProcessCS::OutputPin(uint32_t index) const
	{
		return adapted_lum_->OutputPin(index);
	}

	void ImageStatPostProcessCS::Apply()
	{
		// 降采样4x4 log
		sum_lums_1st_->Apply();
		adapted_lum_->Apply();
	}


	LensEffectsPostProcess::LensEffectsPostProcess()
		: PostProcess(L"LensEffects")
	{
		bright_pass_downsampler_ = SyncLoadPostProcess("LensEffects.ppml", "sqr_bright");
		downsamplers_[0] = SyncLoadPostProcess("Copy.ppml", "bilinear_copy");
		downsamplers_[1] = SyncLoadPostProcess("Copy.ppml", "bilinear_copy");
		blurs_[0] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess>>(8, 1.0f);
		blurs_[1] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess>>(8, 1.0f);
		blurs_[2] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess>>(8, 1.0f);

		glow_merger_ = SyncLoadPostProcess("LensEffects.ppml", "glow_merger");
	}

	void LensEffectsPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		uint32_t const width = tex->Width(0);
		uint32_t const height = tex->Height(0);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		std::array<TexturePtr, 3> downsample_texs;
		std::array<TexturePtr, 3> glow_texs;

		ElementFormat fmt = tex->Format();
		for (size_t i = 0; i < downsample_texs.size(); ++ i)
		{
			downsample_texs[i] = rf.MakeTexture2D(width / (2 << i), height / (2 << i), 1, 1, fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write, nullptr);
		
			glow_texs[i] = rf.MakeTexture2D(width / (2 << i), height / (2 << i), 1, 1, fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write, nullptr);
		}
		
		{
			bright_pass_downsampler_->InputPin(index, tex);
			bright_pass_downsampler_->OutputPin(index, downsample_texs[0]);
		}
		for (size_t i = 0; i < downsamplers_.size(); ++ i)
		{
			downsamplers_[i]->InputPin(0, downsample_texs[i]);
			downsamplers_[i]->OutputPin(0, downsample_texs[i + 1]);
		}
		for (size_t i = 0; i < blurs_.size(); ++ i)
		{
			blurs_[i]->InputPin(0, downsample_texs[i]);
			blurs_[i]->OutputPin(0, glow_texs[i]);
		}

		glow_merger_->InputPin(0, glow_texs[0]);
		glow_merger_->InputPin(1, glow_texs[1]);
		glow_merger_->InputPin(2, glow_texs[2]);

		TexturePtr lens_effects_tex = rf.MakeTexture2D(tex->Width(0) / 2, tex->Height(0) / 2, 1, 1, fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write, nullptr);
		glow_merger_->OutputPin(0, lens_effects_tex);
	}

	TexturePtr const & LensEffectsPostProcess::InputPin(uint32_t index) const
	{
		return bright_pass_downsampler_->InputPin(index);
	}

	void LensEffectsPostProcess::OutputPin(uint32_t index, TexturePtr const & tex, int level, int array_index, int face)
	{
		glow_merger_->OutputPin(index, tex, level, array_index, face);
	}

	TexturePtr const & LensEffectsPostProcess::OutputPin(uint32_t index) const
	{
		return glow_merger_->OutputPin(index);
	}

	void LensEffectsPostProcess::Apply()
	{
		bright_pass_downsampler_->Apply();
		for (size_t i = 0; i < downsamplers_.size(); ++ i)
		{
			downsamplers_[i]->Apply();
		}
		for (size_t i = 0; i < blurs_.size(); ++ i)
		{
			blurs_[i]->Apply();
		}

		glow_merger_->Apply();
	}


	uint32_t const WIDTH = 512;
	uint32_t const HEIGHT = 512;

	FFTLensEffectsPostProcess::FFTLensEffectsPostProcess()
		: PostProcess(L"FFTLensEffects")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		pattern_real_tex_ = SyncLoadTexture("lens_effects_real.dds", EAH_GPU_Read | EAH_Immutable);
		pattern_imag_tex_ = SyncLoadTexture("lens_effects_imag.dds", EAH_GPU_Read | EAH_Immutable);

		ElementFormat fmt;
		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));
			fmt = EF_ABGR16F;
		}
		resized_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		{
			std::vector<uint8_t> zero_data(WIDTH * HEIGHT, 0);
			ElementInitData resized_data;
			resized_data.data = &zero_data[0];
			resized_data.row_pitch = WIDTH * sizeof(uint8_t);
			resized_data.slice_pitch = WIDTH * HEIGHT * sizeof(uint8_t);
			empty_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_R8, 1, 0, EAH_GPU_Read | EAH_Immutable, &resized_data);
		}

		uint32_t tex_creation_flags = EAH_GPU_Read | EAH_GPU_Write;
		if (caps.cs_support)
		{
			if (caps.max_shader_model >= ShaderModel(5, 0))
			{
				fft_ = MakeSharedPtr<GpuFftCS5>(WIDTH, HEIGHT, true);
				ifft_ = MakeSharedPtr<GpuFftCS5>(WIDTH, HEIGHT, false);
				tex_creation_flags |= EAH_GPU_Unordered;
			}
			else
			{
				fft_ = MakeSharedPtr<GpuFftCS4>(WIDTH, HEIGHT, true);
				ifft_ = MakeSharedPtr<GpuFftCS4>(WIDTH, HEIGHT, false);
			}
		}
		else
		{
			fft_ = MakeSharedPtr<GpuFftPS>(WIDTH, HEIGHT, true);
			ifft_ = MakeSharedPtr<GpuFftPS>(WIDTH, HEIGHT, false);
		}

		freq_real_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, tex_creation_flags, nullptr);
		freq_imag_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, tex_creation_flags, nullptr);

		mul_real_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, tex_creation_flags, nullptr);
		mul_imag_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, tex_creation_flags, nullptr);

		bilinear_copy_pp_ = SyncLoadPostProcess("Copy.ppml", "bilinear_copy");

		bright_pass_pp_ = SyncLoadPostProcess("LensEffects.ppml", "scaled_bright_pass");
		bright_pass_pp_->OutputPin(0, resized_tex_);

		complex_mul_pp_ = SyncLoadPostProcess("LensEffects.ppml", "complex_mul");
		complex_mul_pp_->InputPin(0, freq_real_tex_);
		complex_mul_pp_->InputPin(1, freq_imag_tex_);
		complex_mul_pp_->InputPin(2, pattern_real_tex_);
		complex_mul_pp_->InputPin(3, pattern_imag_tex_);
		complex_mul_pp_->OutputPin(0, mul_real_tex_);
		complex_mul_pp_->OutputPin(1, mul_imag_tex_);

		scaled_copy_pp_ = SyncLoadPostProcess("LensEffects.ppml", "scaled_copy");
		scaled_copy_pp_->InputPin(0, mul_real_tex_);
	}

	void FFTLensEffectsPostProcess::InputPin(uint32_t /*index*/, TexturePtr const & tex)
	{
		input_tex_ = tex;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		size_t n = 1;
		{
			uint32_t ori_w = tex->Width(0);
			uint32_t ori_h = tex->Height(0);
			uint32_t ori_s = std::max(ori_w, ori_h);

			while (ori_s > static_cast<uint32_t>(WIDTH))
			{
				ori_w >>= 1;
				ori_h >>= 1;
				ori_s >>= 1;

				++ n;
			}
		}
		n = std::max<size_t>(n, 2);

		restore_chain_.resize(n - 1);
		for (size_t i = 1; i < n; ++ i)
		{
			restore_chain_[i - 1] = rf.MakeTexture2D(std::max(1U, tex->Width(0) >> i), std::max(1U, tex->Height(0) >> i),
				1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		}

		uint32_t const final_width = restore_chain_.back()->Width(0);
		uint32_t const final_height = restore_chain_.back()->Height(0);

		bright_pass_pp_->SetParam(0, float4(1, 1, static_cast<float>(final_width) / WIDTH, static_cast<float>(final_height) / HEIGHT));
		bright_pass_pp_->SetParam(1, static_cast<float>(1UL << (2 * restore_chain_.size())));
		bright_pass_pp_->InputPin(0, input_tex_);

		scaled_copy_pp_->SetParam(0, float4(static_cast<float>(final_width) / WIDTH, static_cast<float>(final_height) / HEIGHT, 1, 1));
		scaled_copy_pp_->OutputPin(0, restore_chain_.back());
	}

	TexturePtr const & FFTLensEffectsPostProcess::InputPin(uint32_t /*index*/) const
	{
		return input_tex_;
	}

	void FFTLensEffectsPostProcess::OutputPin(uint32_t /*index*/, TexturePtr const & /*tex*/, int /*level*/, int /*array_index*/, int /*face*/)
	{
	}

	TexturePtr const & FFTLensEffectsPostProcess::OutputPin(uint32_t /*index*/) const
	{
		return restore_chain_[0];
	}
		
	void FFTLensEffectsPostProcess::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		FrameBufferPtr fb = bright_pass_pp_->OutputFrameBuffer();
		re.BindFrameBuffer(fb);
		fb->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
		bright_pass_pp_->Render();

		fft_->Execute(freq_real_tex_, freq_imag_tex_, resized_tex_, empty_tex_);

		complex_mul_pp_->Apply();

		ifft_->Execute(mul_real_tex_, mul_imag_tex_, mul_real_tex_, mul_imag_tex_);
		scaled_copy_pp_->Apply();

		for (size_t i = restore_chain_.size() - 1; i > 0; -- i)
		{
			bilinear_copy_pp_->InputPin(0, restore_chain_[i]);
			bilinear_copy_pp_->OutputPin(0, restore_chain_[i - 1]);
			bilinear_copy_pp_->Apply();
		}
	}


	HDRPostProcess::HDRPostProcess(bool fft_lens_effects)
		: PostProcess(L"HDR")
	{
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		cs_support_ = caps.cs_support && (caps.max_shader_model >= ShaderModel(5, 0));

		fp_texture_support_ = caps.fp_color_support;

		if (fp_texture_support_)
		{
			if (cs_support_)
			{
				image_stat_ = MakeSharedPtr<ImageStatPostProcessCS>();
			}
			else
			{
				image_stat_ = MakeSharedPtr<ImageStatPostProcess>();
			}
		}

		if (fft_lens_effects && fp_texture_support_)
		{
			if (caps.rendertarget_format_support(EF_ABGR32F, 1, 0))
			{
				lens_effects_ = MakeSharedPtr<FFTLensEffectsPostProcess>();
			}
			else
			{
				lens_effects_ = MakeSharedPtr<LensEffectsPostProcess>();
			}
		}
		else
		{
			lens_effects_ = MakeSharedPtr<LensEffectsPostProcess>();
		}

		tone_mapping_ = MakeSharedPtr<ToneMappingPostProcess>();
	}

	void HDRPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		if (fp_texture_support_)
		{
			image_stat_->InputPin(index, tex);
		}

		lens_effects_->InputPin(0, tex);

		tone_mapping_->InputPin(0, tex);
		if (fp_texture_support_ && cs_support_)
		{
			tone_mapping_->InputPin(1, image_stat_->OutputPin(0));
		}
		tone_mapping_->InputPin(2, lens_effects_->OutputPin(0));
	}

	TexturePtr const & HDRPostProcess::InputPin(uint32_t index) const
	{
		return lens_effects_->InputPin(index);
	}

	void HDRPostProcess::OutputPin(uint32_t index, TexturePtr const & tex, int level, int array_index, int face)
	{
		tone_mapping_->OutputPin(index, tex, level, array_index, face);
	}

	TexturePtr const & HDRPostProcess::OutputPin(uint32_t index) const
	{
		return tone_mapping_->OutputPin(index);
	}

	void HDRPostProcess::Apply()
	{
		if (fp_texture_support_)
		{
			image_stat_->Apply();

			if (!cs_support_)
			{
				tone_mapping_->InputPin(1, image_stat_->OutputPin(0));
			}
		}

		lens_effects_->Apply();
		tone_mapping_->Apply();
	}
}
