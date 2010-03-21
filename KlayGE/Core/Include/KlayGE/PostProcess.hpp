// PostProcess.hpp
// KlayGE 后期处理类 头文件
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

#ifndef _POSTPROCESS_HPP
#define _POSTPROCESS_HPP

#pragma once

#include <vector>

#include <boost/noncopyable.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API PostProcess : boost::noncopyable, public RenderableHelper
	{
	public:
		PostProcess();
		PostProcess(std::vector<std::string> const & input_pin_names, RenderTechniquePtr const & tech);
		virtual ~PostProcess()
		{
		}

		void Technique(RenderTechniquePtr const & tech);

		uint32_t NumInputPins() const;
		uint32_t InputPinByName(std::string const & name) const;
		std::string const & InputPinName(uint32_t index) const;
		virtual void InputPin(uint32_t index, TexturePtr const & tex, bool flipping);

		virtual void Destinate(FrameBufferPtr const & fb);
		FrameBufferPtr const & Destinate()
		{
			return frame_buffer_;
		}

		virtual void Apply();

		virtual void OnRenderBegin();

	private:
		void CreateVB();

	protected:
		std::vector<std::pair<std::string, TexturePtr> > input_pins_;
		bool flipping_;

		FrameBufferPtr frame_buffer_;

		GraphicsBufferPtr pos_vb_;

		RenderEffectParameterPtr texel_to_pixel_offset_ep_;
		RenderEffectParameterPtr flipping_ep_;
		std::vector<RenderEffectParameterPtr> input_pins_ep_;
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

		void InputPin(uint32_t index, TexturePtr const & tex, bool flipping)
		{
			T::InputPin(index, tex, flipping);
			this->CalSampleOffsets(tex->Width(0), 3);
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

		void InputPin(uint32_t index, TexturePtr const & tex, bool flipping)
		{
			T::InputPin(index, tex, flipping);
			this->CalSampleOffsets(tex->Height(0), 3);
		}
	};

	template <typename T>
	class BlurPostProcess : public PostProcess
	{
	public:
		BlurPostProcess(int kernel_radius, float multiplier)
			: blur_x_(kernel_radius, multiplier), blur_y_(kernel_radius, multiplier)
		{
		}

		void InputPin(uint32_t index, TexturePtr const & tex, bool flipping)
		{
			flipping_ = flipping;

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			blurx_tex_ = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, tex->Format(),
				1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

			FrameBufferPtr blur_x_fb = rf.MakeFrameBuffer();
			blur_x_fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blurx_tex_, 0, 0));
			blur_x_.InputPin(index, tex, flipping_);
			blur_x_.Destinate(blur_x_fb);
			blur_y_.InputPin(index, blurx_tex_, blur_x_fb->RequiresFlipping());
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
