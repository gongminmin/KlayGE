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

#include <KlayGE/Timer.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SumLumPostProcess : public PostProcess
	{
	public:
		explicit SumLumPostProcess(RenderTechniquePtr const & tech);
		virtual ~SumLumPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		TexturePtr const & InputPin(uint32_t index) const;

	private:
		void GetSampleOffsets4x4(uint32_t width, uint32_t height);

	protected:
		std::vector<float4> tex_coord_offset_;

		RenderEffectParameterPtr tex_coord_offset_ep_;
	};

	class KLAYGE_CORE_API SumLumLogPostProcess : public SumLumPostProcess
	{
	public:
		SumLumLogPostProcess();
	};

	class KLAYGE_CORE_API SumLumLogPostProcessCS : public SumLumPostProcess
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

	class KLAYGE_CORE_API AdaptedLumPostProcess : public PostProcess
	{
	public:
		AdaptedLumPostProcess();

		void Apply();
		void OnRenderBegin();

	private:
		TexturePtr adapted_textures_[2];
		bool last_index_;

		Timer timer_;

		RenderEffectParameterPtr last_lum_tex_ep_;
		RenderEffectParameterPtr frame_delta_ep_;
	};

	class KLAYGE_CORE_API AdaptedLumPostProcessCS : public PostProcess
	{
	public:
		AdaptedLumPostProcessCS();

		void Apply();
		void OnRenderBegin();

	private:
		Timer timer_;

		RenderEffectParameterPtr frame_delta_ep_;
	};

	class KLAYGE_CORE_API ToneMappingPostProcess : public PostProcess
	{
	public:
		explicit ToneMappingPostProcess();
	};


	class KLAYGE_CORE_API HDRPostProcess : public PostProcess
	{
		enum
		{
			NUM_TONEMAP_TEXTURES = 3
		};

	public:
		HDRPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		TexturePtr const & InputPin(uint32_t index) const;
		void OutputPin(uint32_t index, TexturePtr const & tex, int level = 0, int array_index = 0, int face = 0);
		TexturePtr const & OutputPin(uint32_t index) const;
		void Apply();

	private:
		PostProcessPtr sum_lums_1st_;
		std::vector<PostProcessPtr> sum_lums_;
		PostProcessPtr adapted_lum_;
		PostProcessPtr bright_pass_downsampler_;
		PostProcessPtr downsamplers_[2];
		PostProcessPtr blurs_[3];
		PostProcessPtr glow_merger_;
		PostProcessPtr tone_mapping_;

		bool cs_support_;
	};
}

#endif		// _HDRPOSTPROCESS_HPP
