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

#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class Downsampler2x2PostProcess : public PostProcess
	{
	public:
		Downsampler2x2PostProcess();
	};

	class BrightPassDownsampler2x2PostProcess : public PostProcess
	{
	public:
		BrightPassDownsampler2x2PostProcess();
	};

	class BlurPostProcess : public PostProcess
	{
	public:
		BlurPostProcess(std::string const & tech, int kernel_radius, float multiplier);
		virtual ~BlurPostProcess();

		void OnRenderBegin();

	protected:
		float GaussianDistribution(float x, float y, float rho);
		void CalSampleOffsets(uint32_t tex_size, float deviation);

	protected:
		std::vector<float> color_weight_;
		std::vector<float> tex_coord_offset_;

		int kernel_radius_;
		float multiplier_;
	};

	class BlurXPostProcess : public BlurPostProcess
	{
	public:
		BlurXPostProcess(int length, float multiplier);

		void Source(TexturePtr const & src_tex, bool flipping);
	};

	class BlurYPostProcess : public BlurPostProcess
	{
	public:
		BlurYPostProcess(int length, float multiplier);

		void Source(TexturePtr const & src_tex, bool flipping);
	};

	class SumLumPostProcess : public PostProcess
	{
	public:
		explicit SumLumPostProcess(std::string const & tech);
		virtual ~SumLumPostProcess();

		void Source(TexturePtr const & src_tex, bool flipping);

		void OnRenderBegin();

	private:
		void GetSampleOffsets4x4(uint32_t width, uint32_t height);

	private:
		std::vector<float4> tex_coord_offset_;
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
	};

	class ToneMappingPostProcess : public PostProcess
	{
	public:
		ToneMappingPostProcess();

		void SetTexture(TexturePtr const & lum_tex, TexturePtr const & bloom_tex);

		void OnRenderBegin();

	private:
		TexturePtr lum_texture_;
		TexturePtr bloom_texture_;
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
		void Apply();

	private:
		TexturePtr downsample_tex_;
		TexturePtr blurx_tex_;
		TexturePtr blury_tex_;
		std::vector<TexturePtr> lum_texs_;

		PostProcessPtr tone_mapping_;
		PostProcessPtr downsampler_;
		PostProcessPtr blur_x_;
		PostProcessPtr blur_y_;
		std::vector<PostProcessPtr> sum_lums_;
		PostProcessPtr adapted_lum_;
	};
}

#endif		// _HDRPOSTPROCESS_HPP
