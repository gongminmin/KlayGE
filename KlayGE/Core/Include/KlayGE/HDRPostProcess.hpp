// HDRPostProcess.hpp
// KlayGE HDR后期处理类 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
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
		explicit SumLumPostProcess(std::string const & tech);
		virtual ~SumLumPostProcess();

		void Source(TexturePtr const & src_tex, bool flipping);

	private:
		void GetSampleOffsets4x4(uint32_t width, uint32_t height);

	private:
		std::vector<float4> tex_coord_offset_;

		RenderEffectParameterPtr tex_coord_offset_ep_;
	};

	class KLAYGE_CORE_API SumLumLogPostProcess : public SumLumPostProcess
	{
	public:
		SumLumLogPostProcess();
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

		TexturePtr AdaptedLum() const;

	private:
		FrameBufferPtr fb_[2];
		TexturePtr adapted_textures_[2];
		bool last_index_;

		Timer timer_;

		RenderEffectParameterPtr last_lum_tex_ep_;
		RenderEffectParameterPtr frame_delta_ep_;
	};

	class KLAYGE_CORE_API ToneMappingPostProcess : public PostProcess
	{
	public:
		explicit ToneMappingPostProcess(bool blue_shift);

		void SetTexture(TexturePtr const & lum_tex, TexturePtr const & bloom_tex);

	private:
		RenderEffectParameterPtr lum_tex_ep_;
		RenderEffectParameterPtr bloom_tex_ep_;
	};


	class KLAYGE_CORE_API HDRPostProcess : public PostProcess
	{
		enum
		{
			NUM_TONEMAP_TEXTURES = 3
		};

	public:
		HDRPostProcess(bool bright_pass, bool blur_shift);

		void Source(TexturePtr const & tex, bool flipping);
		void Destinate(FrameBufferPtr const & fb);
		void Apply();

	private:
		TexturePtr downsample_tex_;
		TexturePtr blur_tex_;
		std::vector<TexturePtr> lum_texs_;

		PostProcessPtr downsampler_;
		PostProcessPtr blur_;
		PostProcessPtr sum_lums_1st_;
		std::vector<PostProcessPtr> sum_lums_;
		PostProcessPtr adapted_lum_;
		PostProcessPtr tone_mapping_;
	};
}

#endif		// _HDRPOSTPROCESS_HPP
