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

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API PostProcess : public RenderableHelper
	{
	public:
		explicit PostProcess(RenderTechniquePtr const & tech);
		virtual ~PostProcess()
		{
		}

		void Technique(RenderTechniquePtr const & tech);

		virtual void Source(TexturePtr const & tex, bool flipping);
		virtual void Destinate(FrameBufferPtr const & fb);
		FrameBufferPtr const & Destinate()
		{
			return frame_buffer_;
		}

		virtual void Apply();

		virtual void OnRenderBegin();

	protected:
		TexturePtr src_texture_;
		bool flipping_;

		FrameBufferPtr frame_buffer_;

		GraphicsBufferPtr pos_vb_;

		RenderEffectParameterPtr texel_to_pixel_offset_ep_;
		RenderEffectParameterPtr src_tex_ep_;
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

	class KLAYGE_CORE_API SeparableBoxFilterPostProcess : public PostProcess
	{
	public:
		SeparableBoxFilterPostProcess(std::string const & tech, int kernel_radius, float multiplier);
		virtual ~SeparableBoxFilterPostProcess();

	protected:
		void CalSampleOffsets(uint32_t tex_size, float deviation);

	protected:
		int kernel_radius_;
		float multiplier_;

		RenderEffectParameterPtr color_weight_ep_;
		RenderEffectParameterPtr tex_coord_offset_ep_;
	};

	class KLAYGE_CORE_API SeparableGaussianFilterPostProcess : public PostProcess
	{
	public:
		SeparableGaussianFilterPostProcess(std::string const & tech, int kernel_radius, float multiplier);
		virtual ~SeparableGaussianFilterPostProcess();

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

	template <typename T>
	class BlurXPostProcess : public T
	{
	public:
		BlurXPostProcess(int kernel_radius, float multiplier)
			: T("BlurX", kernel_radius, multiplier)
		{
		}

		void Source(TexturePtr const & src_tex, bool flipping)
		{
			T::Source(src_tex, flipping);
			this->CalSampleOffsets(src_texture_->Width(0), 3);
		}
	};

	template <typename T>
	class BlurYPostProcess : public T
	{
	public:
		BlurYPostProcess(int kernel_radius, float multiplier)
			: T("BlurY", kernel_radius, multiplier)
		{
		}

		void Source(TexturePtr const & src_tex, bool flipping)
		{
			T::Source(src_tex, flipping);
			this->CalSampleOffsets(src_texture_->Height(0), 3);
		}
	};

	template <typename T>
	class BlurPostProcess : public PostProcess
	{
	public:
		BlurPostProcess(int kernel_radius, float multiplier)
			: PostProcess(RenderTechniquePtr()),
				blur_x_(kernel_radius, multiplier), blur_y_(kernel_radius, multiplier)
		{
		}

		void Source(TexturePtr const & src_tex, bool flipping)
		{
			PostProcess::Source(src_tex, flipping);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			blurx_tex_ = rf.MakeTexture2D(src_texture_->Width(0), src_texture_->Height(0), 1, 1, src_texture_->Format(),
				1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

			FrameBufferPtr blur_x_fb = rf.MakeFrameBuffer();
			blur_x_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blurx_tex_, 0, 0));
			blur_x_.Source(src_texture_, flipping_);
			blur_x_.Destinate(blur_x_fb);
			blur_y_.Source(blurx_tex_, blur_x_fb->RequiresFlipping());
		}

		void Destinate(FrameBufferPtr const & fb)
		{
			PostProcess::Destinate(fb);
			blur_y_.Destinate(fb);
		}

		void Apply()
		{
			blur_x_.Apply();
			blur_y_.Apply();
		}

	private:
		BlurXPostProcess<T> blur_x_;
		BlurYPostProcess<T> blur_y_;

		TexturePtr blurx_tex_;
	};
}

#endif		// _POSTPROCESS_HPP
