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
#include <KlayGE/RenderView.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/FFT.hpp>
#include <KlayGE/PostProcess.hpp>

#include <boost/assert.hpp>

#include <KlayGE/HDRPostProcess.hpp>

namespace KlayGE
{
	SumLumPostProcess::SumLumPostProcess()
		: PostProcess(L"SumLum", false,
			MakeSpan<std::string>(),
			MakeSpan<std::string>({"src_tex"}),
			MakeSpan<std::string>({"out_tex"}),
			RenderEffectPtr(), nullptr)
	{
	}

	SumLumPostProcess::~SumLumPostProcess()
	{
	}

	void SumLumPostProcess::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		PostProcess::InputPin(index, srv);

		auto const* tex = srv->TextureResource().get();
		this->GetSampleOffsets4x4(tex->Width(0), tex->Height(0));
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

		*(effect_->ParameterByName("tex_coord_offset")) = tex_coord_offset;
	}


	SumLumLogPostProcess::SumLumLogPostProcess()
	{
		auto effect = SyncLoadRenderEffect("SumLum.fxml");
		this->Technique(effect, effect->TechniqueByName("SumLumLog"));
	}


	SumLumLogPostProcessCS::SumLumLogPostProcessCS()
	{
		auto effect = SyncLoadRenderEffect("SumLum.fxml");
		this->Technique(effect, effect->TechniqueByName("SumLumLogCS"));
	}

	void SumLumLogPostProcessCS::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.DefaultFrameBuffer());
		re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color);

		*(effect_->ParameterByName("dst_tex_dim")) = int2(64, 64);

		this->OnRenderBegin();
		re.Dispatch(*effect_, *technique_, 2, 2, 1);
		this->OnRenderEnd();
	}


	SumLumIterativePostProcess::SumLumIterativePostProcess()
	{
		auto effect = SyncLoadRenderEffect("SumLum.fxml");
		this->Technique(effect, effect->TechniqueByName("SumLumIterative"));
	}


	AdaptedLumPostProcess::AdaptedLumPostProcess()
			: PostProcess(L"AdaptedLum", false,
					MakeSpan<std::string>(),
					MakeSpan<std::string>({"src_tex"}),
					MakeSpan<std::string>({"output"}),
					RenderEffectPtr(), nullptr),
				last_index_(false)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		std::array<int, 4> data_v = { 0, 0, 0, 0 };
		ElementInitData init_data;
		init_data.row_pitch = sizeof(int);
		init_data.slice_pitch = 0;
		init_data.data = &data_v[0];
		ElementFormat fmt;
		if (caps.pack_to_rgba_required)
		{
			fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
			BOOST_ASSERT(fmt != EF_Unknown);
		}
		else
		{
			fmt = EF_R32F;
		}

		auto effect = SyncLoadRenderEffect("SumLum.fxml");
		this->Technique(effect, effect->TechniqueByName("AdaptedLum"));

		for (size_t i = 0; i < 2; ++i)
		{
			adapted_textures_[i] = rf.MakeTexture2D(1, 1, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, MakeSpan<1>(init_data));
			adapted_rtvs_[i] = rf.Make2DRtv(adapted_textures_[i], 0, 1, 0);
		}
		this->OutputPin(0, adapted_rtvs_[last_index_]);

		last_lum_tex_ep_ = effect_->ParameterByName("last_lum_tex");
		frame_delta_ep_ = effect_->ParameterByName("frame_delta");
	}

	void AdaptedLumPostProcess::Apply()
	{
		this->OutputPin(0, adapted_rtvs_[!last_index_]);

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
			: PostProcess(L"AdaptedLumCS", false,
					MakeSpan<std::string>(),
					MakeSpan<std::string>({"src_tex"}),
					MakeSpan<std::string>({"out_tex"}),
					RenderEffectPtr(), nullptr)
	{
		auto effect = SyncLoadRenderEffect("SumLum.fxml");
		this->Technique(effect, effect->TechniqueByName("AdaptedLumCS"));

		frame_delta_ep_ = effect_->ParameterByName("frame_delta");
	}

	void AdaptedLumPostProcessCS::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.DefaultFrameBuffer());
		re.DefaultFrameBuffer()->Discard(FrameBuffer::CBM_Color);

		this->OnRenderBegin();
		re.Dispatch(*effect_, *technique_, 1, 1, 1);
		this->OnRenderEnd();
	}

	void AdaptedLumPostProcessCS::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*frame_delta_ep_ = Context::Instance().AppInstance().FrameTime();
	}


	ImageStatPostProcess::ImageStatPostProcess()
		: PostProcess(L"ImageStat", false)
	{
		sum_lums_1st_ = MakeSharedPtr<SumLumLogPostProcess>();
		sum_lums_.resize(3);
		for (size_t i = 0; i < sum_lums_.size(); ++ i)
		{
			sum_lums_[i] = MakeSharedPtr<SumLumIterativePostProcess>();
		}
		adapted_lum_ = MakeSharedPtr<AdaptedLumPostProcess>();
	}

	void ImageStatPostProcess::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		std::vector<TexturePtr> lum_texs(sum_lums_.size() + 1);
		std::vector<ShaderResourceViewPtr> lum_srvs(lum_texs.size());
		std::vector<RenderTargetViewPtr> lum_rtvs(lum_texs.size());
		ElementFormat fmt;
		if (caps.pack_to_rgba_required)
		{
			fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
			BOOST_ASSERT(fmt != EF_Unknown);
		}
		else
		{
			fmt = EF_R16F;
		}

		int len = 1;
		for (size_t i = 0; i < sum_lums_.size() + 1; ++ i)
		{
			lum_texs[sum_lums_.size() - i] = rf.MakeTexture2D(len, len, 1, 1, fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write);
			lum_srvs[sum_lums_.size() - i] = rf.MakeTextureSrv(lum_texs[sum_lums_.size() - i]);
			lum_rtvs[sum_lums_.size() - i] = rf.Make2DRtv(lum_texs[sum_lums_.size() - i], 0, 1, 0);
			len *= 4;
		}

		{
			sum_lums_1st_->InputPin(index, srv);
			sum_lums_1st_->OutputPin(index, lum_rtvs[0]);
		}
		for (size_t i = 0; i < sum_lums_.size(); ++ i)
		{
			sum_lums_[i]->InputPin(index, lum_srvs[i]);
			sum_lums_[i]->OutputPin(index, lum_rtvs[i + 1]);
		}

		{
			adapted_lum_->InputPin(index, lum_srvs[sum_lums_.size()]);
		}
	}

	ShaderResourceViewPtr const& ImageStatPostProcess::InputPin(uint32_t index) const
	{
		return sum_lums_1st_->InputPin(index);
	}

	void ImageStatPostProcess::OutputPin(uint32_t index, RenderTargetViewPtr const& rtv)
	{
		adapted_lum_->OutputPin(index, rtv);
	}

	RenderTargetViewPtr const& ImageStatPostProcess::RtvOutputPin(uint32_t index) const
	{
		return adapted_lum_->RtvOutputPin(index);
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
		: PostProcess(L"ImageStatCS", false)
	{
		sum_lums_1st_ = MakeSharedPtr<SumLumLogPostProcessCS>();
		adapted_lum_ = MakeSharedPtr<AdaptedLumPostProcessCS>();
	}

	void ImageStatPostProcessCS::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		TexturePtr lum_tex = rf.MakeTexture2D(2, 2, 1, 1, EF_R16F, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered);

		float init_lum = 0;
		ElementInitData init_data;
		init_data.row_pitch = sizeof(float);
		init_data.slice_pitch = 0;
		init_data.data = &init_lum;
		TexturePtr adapted_lum_tex =
			rf.MakeTexture2D(1, 1, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered, MakeSpan<1>(init_data));

		sum_lums_1st_->InputPin(index, srv);
		sum_lums_1st_->OutputPin(index, rf.Make2DUav(lum_tex, 0, 1, 0));
		adapted_lum_->InputPin(index, rf.MakeTextureSrv(lum_tex));
		adapted_lum_->OutputPin(index, rf.Make2DUav(adapted_lum_tex, 0, 1, 0));
	}

	ShaderResourceViewPtr const& ImageStatPostProcessCS::InputPin(uint32_t index) const
	{
		return sum_lums_1st_->InputPin(index);
	}

	void ImageStatPostProcessCS::OutputPin(uint32_t index, UnorderedAccessViewPtr const& uav)
	{
		adapted_lum_->OutputPin(index, uav);
	}

	UnorderedAccessViewPtr const& ImageStatPostProcessCS::UavOutputPin(uint32_t index) const
	{
		return adapted_lum_->UavOutputPin(index);
	}

	void ImageStatPostProcessCS::Apply()
	{
		// 降采样4x4 log
		sum_lums_1st_->Apply();
		adapted_lum_->Apply();
	}


	LensEffectsPostProcess::LensEffectsPostProcess()
		: PostProcess(L"LensEffects", false)
	{
		bright_pass_downsampler_ = SyncLoadPostProcess("LensEffects.ppml", "sqr_bright");
		downsamplers_[0] = SyncLoadPostProcess("Copy.ppml", "BilinearCopy");
		downsamplers_[1] = SyncLoadPostProcess("Copy.ppml", "BilinearCopy");
		blurs_[0] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess>>(8, 1.0f);
		blurs_[1] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess>>(8, 1.0f);
		blurs_[2] = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess>>(8, 1.0f);

		glow_merger_ = SyncLoadPostProcess("LensEffects.ppml", "glow_merger");
	}

	void LensEffectsPostProcess::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		auto const* tex = srv->TextureResource().get();
		uint32_t const width = tex->Width(0);
		uint32_t const height = tex->Height(0);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		std::array<TexturePtr, 3> downsample_texs;
		std::array<ShaderResourceViewPtr, 3> downsample_srvs;
		std::array<RenderTargetViewPtr, 3> downsample_rtvs;
		std::array<TexturePtr, 3> glow_texs;
		std::array<ShaderResourceViewPtr, 3> glow_srvs;
		std::array<RenderTargetViewPtr, 3> glow_rtvs;

		ElementFormat fmt = tex->Format();
		for (size_t i = 0; i < downsample_texs.size(); ++ i)
		{
			downsample_texs[i] = rf.MakeTexture2D(width / (2 << i), height / (2 << i), 1, 1, fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write);
			downsample_srvs[i] = rf.MakeTextureSrv(downsample_texs[i]);
			downsample_rtvs[i] = rf.Make2DRtv(downsample_texs[i], 0, 1, 0);
		
			glow_texs[i] = rf.MakeTexture2D(width / (2 << i), height / (2 << i), 1, 1, fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write);
			glow_srvs[i] = rf.MakeTextureSrv(glow_texs[i]);
			glow_rtvs[i] = rf.Make2DRtv(glow_texs[i], 0, 1, 0);
		}
		
		{
			bright_pass_downsampler_->InputPin(index, srv);
			bright_pass_downsampler_->OutputPin(index, downsample_rtvs[0]);
		}
		for (size_t i = 0; i < downsamplers_.size(); ++ i)
		{
			downsamplers_[i]->InputPin(0, downsample_srvs[i]);
			downsamplers_[i]->OutputPin(0, downsample_rtvs[i + 1]);
		}
		for (size_t i = 0; i < blurs_.size(); ++ i)
		{
			blurs_[i]->InputPin(0, downsample_srvs[i]);
			blurs_[i]->OutputPin(0, glow_rtvs[i]);
		}

		glow_merger_->InputPin(0, glow_srvs[0]);
		glow_merger_->InputPin(1, glow_srvs[1]);
		glow_merger_->InputPin(2, glow_srvs[2]);

		TexturePtr lens_effects_tex = rf.MakeTexture2D(tex->Width(0) / 2, tex->Height(0) / 2, 1, 1, fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write);
		glow_merger_->OutputPin(0, rf.Make2DRtv(lens_effects_tex, 0, 1, 0));
	}

	ShaderResourceViewPtr const& LensEffectsPostProcess::InputPin(uint32_t index) const
	{
		return bright_pass_downsampler_->InputPin(index);
	}

	void LensEffectsPostProcess::OutputPin(uint32_t index, RenderTargetViewPtr const& rtv)
	{
		glow_merger_->OutputPin(index, rtv);
	}

	RenderTargetViewPtr const & LensEffectsPostProcess::RtvOutputPin(uint32_t index) const
	{
		return glow_merger_->RtvOutputPin(index);
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
		: PostProcess(L"FFTLensEffects", false)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		pattern_real_tex_ = SyncLoadTexture("lens_effects_real.dds", EAH_GPU_Read | EAH_Immutable);
		pattern_imag_tex_ = SyncLoadTexture("lens_effects_imag.dds", EAH_GPU_Read | EAH_Immutable);

		auto const fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_B10G11R11F, EF_ABGR16F}), 1, 0);
		BOOST_ASSERT(fmt != EF_Unknown);
		auto resized_tex = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		resized_srv_ = rf.MakeTextureSrv(resized_tex);
		{
			std::vector<uint8_t> zero_data(WIDTH * HEIGHT, 0);
			ElementInitData resized_data;
			resized_data.data = &zero_data[0];
			resized_data.row_pitch = WIDTH * sizeof(uint8_t);
			resized_data.slice_pitch = WIDTH * HEIGHT * sizeof(uint8_t);
			auto empty_tex = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_R8, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(resized_data));
			empty_srv_ = rf.MakeTextureSrv(empty_tex);
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

		freq_real_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, tex_creation_flags);
		freq_imag_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, tex_creation_flags);

		mul_real_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, tex_creation_flags);
		mul_real_srv_ = rf.MakeTextureSrv(mul_real_tex_);
		mul_imag_tex_ = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, tex_creation_flags);
		mul_imag_srv_ = rf.MakeTextureSrv(mul_imag_tex_);

		bilinear_copy_pp_ = SyncLoadPostProcess("Copy.ppml", "BilinearCopy");

		bright_pass_pp_ = SyncLoadPostProcess("LensEffects.ppml", "scaled_bright_pass");
		bright_pass_pp_->OutputPin(0, rf.Make2DRtv(resized_tex, 0, 1, 0));

		complex_mul_pp_ = SyncLoadPostProcess("LensEffects.ppml", "complex_mul");
		complex_mul_pp_->InputPin(0, rf.MakeTextureSrv(freq_real_tex_));
		complex_mul_pp_->InputPin(1, rf.MakeTextureSrv(freq_imag_tex_));
		complex_mul_pp_->InputPin(2, rf.MakeTextureSrv(pattern_real_tex_));
		complex_mul_pp_->InputPin(3, rf.MakeTextureSrv(pattern_imag_tex_));
		complex_mul_pp_->OutputPin(0, rf.Make2DRtv(mul_real_tex_, 0, 1, 0));
		complex_mul_pp_->OutputPin(1, rf.Make2DRtv(mul_imag_tex_, 0, 1, 0));

		scaled_copy_pp_ = SyncLoadPostProcess("LensEffects.ppml", "scaled_copy");
		scaled_copy_pp_->InputPin(0, mul_real_srv_);
	}

	void FFTLensEffectsPostProcess::InputPin(uint32_t /*index*/, ShaderResourceViewPtr const & srv)
	{
		input_srv_ = srv;
		auto const* tex = srv->TextureResource().get();

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

		restore_chain_texs_.resize(n - 1);
		restore_chain_srvs_.resize(n - 1);
		restore_chain_rtvs_.resize(n - 1);
		for (size_t i = 1; i < n; ++ i)
		{
			restore_chain_texs_[i - 1] = rf.MakeTexture2D(std::max(1U, tex->Width(0) >> i), std::max(1U, tex->Height(0) >> i),
				1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			restore_chain_srvs_[i - 1] = rf.MakeTextureSrv(restore_chain_texs_[i - 1]);
			restore_chain_rtvs_[i - 1] = rf.Make2DRtv(restore_chain_texs_[i - 1], 0, 1, 0);
		}

		uint32_t const final_width = restore_chain_texs_.back()->Width(0);
		uint32_t const final_height = restore_chain_texs_.back()->Height(0);

		bright_pass_pp_->SetParam(0, float4(1, 1, static_cast<float>(final_width) / WIDTH, static_cast<float>(final_height) / HEIGHT));
		bright_pass_pp_->SetParam(1, static_cast<float>(1UL << (2 * restore_chain_texs_.size())));
		bright_pass_pp_->InputPin(0, input_srv_);

		scaled_copy_pp_->SetParam(0, float4(static_cast<float>(final_width) / WIDTH, static_cast<float>(final_height) / HEIGHT, 1, 1));
		scaled_copy_pp_->OutputPin(0, restore_chain_rtvs_.back());
	}

	ShaderResourceViewPtr const& FFTLensEffectsPostProcess::InputPin(uint32_t /*index*/) const
	{
		return input_srv_;
	}

	void FFTLensEffectsPostProcess::OutputPin(uint32_t index, RenderTargetViewPtr const& rtv)
	{
		KFL_UNUSED(index);
		KFL_UNUSED(rtv);
	}

	RenderTargetViewPtr const & FFTLensEffectsPostProcess::RtvOutputPin(uint32_t /*index*/) const
	{
		return restore_chain_rtvs_[0];
	}
		
	void FFTLensEffectsPostProcess::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		FrameBufferPtr fb = bright_pass_pp_->OutputFrameBuffer();
		re.BindFrameBuffer(fb);
		fb->AttachedRtv(FrameBuffer::Attachment::Color0)->ClearColor(Color(0, 0, 0, 0));
		bright_pass_pp_->Render();

		fft_->Execute(freq_real_tex_, freq_imag_tex_, resized_srv_, empty_srv_);

		complex_mul_pp_->Apply();

		ifft_->Execute(mul_real_tex_, mul_imag_tex_, mul_real_srv_, mul_imag_srv_);
		scaled_copy_pp_->Apply();

		for (size_t i = restore_chain_texs_.size() - 1; i > 0; -- i)
		{
			bilinear_copy_pp_->InputPin(0, restore_chain_srvs_[i]);
			bilinear_copy_pp_->OutputPin(0, restore_chain_rtvs_[i - 1]);
			bilinear_copy_pp_->Apply();
		}
	}


	HDRPostProcess::HDRPostProcess(bool fft_lens_effects)
		: PostProcess(L"HDR", false)
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
			if (caps.TextureRenderTargetFormatSupport(EF_ABGR32F, 1, 0))
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

		tone_mapping_ = SyncLoadPostProcess("ToneMapping.ppml", "tone_mapping");
	}

	void HDRPostProcess::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
	{
		if (fp_texture_support_)
		{
			image_stat_->InputPin(index, srv);
		}

		lens_effects_->InputPin(0, srv);

		tone_mapping_->InputPin(0, srv);
		auto& rf = Context::Instance().RenderFactoryInstance();
		if (fp_texture_support_ && cs_support_)
		{
			tone_mapping_->InputPin(1, rf.MakeTextureSrv(image_stat_->UavOutputPin(0)->TextureResource()));
		}
		tone_mapping_->InputPin(2, rf.MakeTextureSrv(lens_effects_->RtvOutputPin(0)->TextureResource()));
	}

	ShaderResourceViewPtr const& HDRPostProcess::InputPin(uint32_t index) const
	{
		return lens_effects_->InputPin(index);
	}

	void HDRPostProcess::OutputPin(uint32_t index, RenderTargetViewPtr const& rtv)
	{
		tone_mapping_->OutputPin(index, rtv);
	}

	RenderTargetViewPtr const & HDRPostProcess::RtvOutputPin(uint32_t index) const
	{
		return tone_mapping_->RtvOutputPin(index);
	}

	void HDRPostProcess::Apply()
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		if (fp_texture_support_)
		{
			image_stat_->Apply();

			if (!cs_support_)
			{
				tone_mapping_->InputPin(1, rf.MakeTextureSrv(image_stat_->RtvOutputPin(0)->TextureResource()));
			}
		}

		lens_effects_->Apply();

		auto const & graphics_cfg = Context::Instance().Config().graphics_cfg;

		tone_mapping_->SetParam(0, graphics_cfg.bloom);
		tone_mapping_->SetParam(1, static_cast<int32_t>(graphics_cfg.blue_shift ? 1 : 0));

		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		tone_mapping_->SetParam(2, re.HDRRescale());

		tone_mapping_->Apply();
	}
}
