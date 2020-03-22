// HDRPostProcess.hpp
// KlayGE HDR后期处理类 头文件
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

#ifndef _HDRPOSTPROCESS_HPP
#define _HDRPOSTPROCESS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <vector>
#include <array>

#include <KFL/Timer.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SumLumPostProcess : public PostProcess
	{
	public:
		SumLumPostProcess();

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		using PostProcess::InputPin;

	private:
		void GetSampleOffsets4x4(uint32_t width, uint32_t height);

	protected:
		std::vector<float4> tex_coord_offset_;
	};

	class KLAYGE_CORE_API SumLumLogPostProcess final : public SumLumPostProcess
	{
	public:
		SumLumLogPostProcess();
	};

	class KLAYGE_CORE_API SumLumLogPostProcessCS final : public SumLumPostProcess
	{
	public:
		SumLumLogPostProcessCS();
		void Apply();
	};

	class KLAYGE_CORE_API SumLumIterativePostProcess : public SumLumPostProcess
	{
	public:
		SumLumIterativePostProcess();
	};

	class KLAYGE_CORE_API AdaptedLumPostProcess final : public PostProcess
	{
	public:
		AdaptedLumPostProcess();

		void Apply();
		void OnRenderBegin();

	private:
		TexturePtr adapted_textures_[2];
		RenderTargetViewPtr adapted_rtvs_[2];
		bool last_index_;

		RenderEffectParameter* last_lum_tex_ep_;
		RenderEffectParameter* frame_delta_ep_;
	};

	class KLAYGE_CORE_API AdaptedLumPostProcessCS final : public PostProcess
	{
	public:
		AdaptedLumPostProcessCS();

		void Apply();
		void OnRenderBegin();

	private:
		RenderEffectParameter* frame_delta_ep_;
	};

	class KLAYGE_CORE_API ImageStatPostProcess final : public PostProcess
	{
	public:
		ImageStatPostProcess();

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		ShaderResourceViewPtr const& InputPin(uint32_t index) const override;
		void OutputPin(uint32_t index, RenderTargetViewPtr const& rtv) override;
		RenderTargetViewPtr const& RtvOutputPin(uint32_t index) const override;
		void Apply() override;

	private:
		PostProcessPtr sum_lums_1st_;
		std::vector<PostProcessPtr> sum_lums_;
		PostProcessPtr adapted_lum_;
	};

	class KLAYGE_CORE_API ImageStatPostProcessCS final : public PostProcess
	{
	public:
		ImageStatPostProcessCS();

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		ShaderResourceViewPtr const& InputPin(uint32_t index) const override;
		void OutputPin(uint32_t index, UnorderedAccessViewPtr const& uav) override;
		UnorderedAccessViewPtr const& UavOutputPin(uint32_t index) const override;
		void Apply() override;

	private:
		PostProcessPtr sum_lums_1st_;
		PostProcessPtr adapted_lum_;
	};

	class KLAYGE_CORE_API LensEffectsPostProcess final : public PostProcess
	{
	public:
		LensEffectsPostProcess();

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		ShaderResourceViewPtr const& InputPin(uint32_t index) const override;
		void OutputPin(uint32_t index, RenderTargetViewPtr const& rtv) override;
		RenderTargetViewPtr const& RtvOutputPin(uint32_t index) const override;
		void Apply() override;

	private:
		PostProcessPtr bright_pass_downsampler_;
		std::array<PostProcessPtr, 2> downsamplers_;
		std::array<PostProcessPtr, 3> blurs_;
		PostProcessPtr glow_merger_;
	};

	class KLAYGE_CORE_API FFTLensEffectsPostProcess final : public PostProcess
	{
	public:
		FFTLensEffectsPostProcess();

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		ShaderResourceViewPtr const& InputPin(uint32_t index) const override;
		void OutputPin(uint32_t index, RenderTargetViewPtr const& rtv) override;
		RenderTargetViewPtr const& RtvOutputPin(uint32_t index) const override;
		void Apply() override;

	private:
		PostProcessPtr bilinear_copy_pp_;
		PostProcessPtr bright_pass_pp_;
		PostProcessPtr complex_mul_pp_;
		PostProcessPtr scaled_copy_pp_;

		std::vector<TexturePtr> restore_chain_texs_;
		std::vector<ShaderResourceViewPtr> restore_chain_srvs_;
		std::vector<RenderTargetViewPtr> restore_chain_rtvs_;

		ShaderResourceViewPtr input_srv_;

		ShaderResourceViewPtr resized_srv_;
		ShaderResourceViewPtr empty_srv_;

		TexturePtr freq_real_tex_;
		TexturePtr freq_imag_tex_;
		TexturePtr pattern_real_tex_;
		TexturePtr pattern_imag_tex_;
		TexturePtr mul_real_tex_;
		ShaderResourceViewPtr mul_real_srv_;
		TexturePtr mul_imag_tex_;
		ShaderResourceViewPtr mul_imag_srv_;

		GpuFftPtr fft_;
		GpuFftPtr ifft_;
	};


	class KLAYGE_CORE_API HDRPostProcess final : public PostProcess
	{
	public:
		explicit HDRPostProcess(bool fft_lens_effects);

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		ShaderResourceViewPtr const& InputPin(uint32_t index) const override;
		void OutputPin(uint32_t index, RenderTargetViewPtr const& rtv) override;
		RenderTargetViewPtr const& RtvOutputPin(uint32_t index) const override;
		void Apply() override;

	private:
		PostProcessPtr image_stat_;
		PostProcessPtr lens_effects_;
		PostProcessPtr tone_mapping_;

		bool cs_support_;
		bool fp_texture_support_;
	};
}

#endif		// _HDRPOSTPROCESS_HPP
