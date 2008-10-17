// PostProcess.hpp
// KlayGE 后期处理类 头文件
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

#ifndef _POSTPROCESS_HPP
#define _POSTPROCESS_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API PostProcess : public RenderableHelper
	{
	public:
		explicit PostProcess(RenderTechniquePtr tech);
		virtual ~PostProcess()
		{
		}

		virtual void Source(TexturePtr const & tex, bool flipping);
		virtual void Destinate(FrameBufferPtr const & fb);

		virtual void Apply();

		virtual void OnRenderBegin();

	protected:
		TexturePtr src_texture_;
		bool flipping_;

		FrameBufferPtr frame_buffer_;

		GraphicsBufferPtr pos_vb_;

		RenderEffectParameterPtr texel_to_pixel_offset_ep_;
		RenderEffectParameterPtr src_sampler_ep_;
		RenderEffectParameterPtr flipping_ep_;
	};

	class KLAYGE_CORE_API GammaCorrectionProcess : public PostProcess
	{
	public:
		explicit GammaCorrectionProcess();

		void Gamma(float gamma);

	private:
		RenderEffectParameterPtr inv_gamma_ep_;
	};
	
	class KLAYGE_CORE_API SeparableBlurPostProcess : public PostProcess
	{
	public:
		SeparableBlurPostProcess(std::string const & tech, int kernel_radius, float multiplier);
		virtual ~SeparableBlurPostProcess();

	protected:
		float GaussianDistribution(float x, float y, float rho);
		void CalSampleOffsets(uint32_t tex_size, float deviation);

	protected:
		int kernel_radius_;
		float multiplier_;

		RenderEffectParameterPtr color_weight_ep_;
		RenderEffectParameterPtr tex_coord_offset_ep_;
	};

	class KLAYGE_CORE_API Downsampler2x2PostProcess : public PostProcess
	{
	public:
		Downsampler2x2PostProcess();
	};

	class KLAYGE_CORE_API BrightPassDownsampler2x2PostProcess : public PostProcess
	{
	public:
		BrightPassDownsampler2x2PostProcess();
	};

	class KLAYGE_CORE_API BlurXPostProcess : public SeparableBlurPostProcess
	{
	public:
		BlurXPostProcess(int kernel_radius, float multiplier);

		void Source(TexturePtr const & src_tex, bool flipping);
	};

	class KLAYGE_CORE_API BlurYPostProcess : public SeparableBlurPostProcess
	{
	public:
		BlurYPostProcess(int kernel_radius, float multiplier);

		void Source(TexturePtr const & src_tex, bool flipping);
	};

	class KLAYGE_CORE_API BlurPostProcess : public PostProcess
	{
	public:
		BlurPostProcess(int kernel_radius, float multiplier);

		void Destinate(FrameBufferPtr const & fb);

		void Apply();

	private:
		BlurXPostProcess blur_x_;
		BlurYPostProcess blur_y_;

		TexturePtr blurx_tex_;
	};
}

#endif		// _POSTPROCESS_HPP
