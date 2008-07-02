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

#include <KlayGE/PreDeclare.hpp>

#include <vector>

#include <KlayGE/Timer.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class SumLumPostProcess : public PostProcess
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

	class SumLumLogPostProcess : public SumLumPostProcess
	{
	public:
		SumLumLogPostProcess();
	};

	class SumLumIterativePostProcess : public SumLumPostProcess
	{
	public:
		SumLumIterativePostProcess();
	};

	class AdaptedLumPostProcess : public PostProcess
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

		RenderEffectParameterPtr last_lum_sampler_ep_;
		RenderEffectParameterPtr frame_delta_ep_;
	};

	class ToneMappingPostProcess : public PostProcess
	{
	public:
		ToneMappingPostProcess();

		void SetTexture(TexturePtr const & lum_tex, TexturePtr const & bloom_tex);

	private:
		RenderEffectParameterPtr lum_sampler_ep_;
		RenderEffectParameterPtr bloom_sampler_ep_;
	};


	class HDRPostProcess : public PostProcess
	{
		enum
		{
			NUM_TONEMAP_TEXTURES = 3
		};

	public:
		HDRPostProcess();

		void Source(TexturePtr const & tex, bool flipping);
		void Destinate(FrameBufferPtr const & fb);
		void Apply();

	private:
		TexturePtr downsample_tex_;
		TexturePtr blur_tex_;
		std::vector<TexturePtr> lum_texs_;

		BrightPassDownsampler2x2PostProcess downsampler_;
		BlurPostProcess blur_;
		SumLumLogPostProcess sum_lums_1st_;
		std::vector<SumLumIterativePostProcess> sum_lums_;
		AdaptedLumPostProcess adapted_lum_;
		ToneMappingPostProcess tone_mapping_;
	};
}

#endif		// _HDRPOSTPROCESS_HPP
