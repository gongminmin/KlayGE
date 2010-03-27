// PostProcess.hpp
// KlayGE 后期处理类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2006-2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 使用InputPin和OutputPin来指定输入输出 (2010.3.23)
// 增加了PostProcessChain和ppml格式 (2010.3.26)
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
		explicit PostProcess(std::wstring const & name);
		PostProcess(std::wstring const & name,
			std::vector<std::string> const & input_pin_names,
			std::vector<std::string> const & output_pin_names,
			RenderTechniquePtr const & tech);
		virtual ~PostProcess()
		{
		}

		void Technique(RenderTechniquePtr const & tech);

		uint32_t NumInputPins() const;
		uint32_t InputPinByName(std::string const & name) const;
		std::string const & InputPinName(uint32_t index) const;
		virtual void InputPin(uint32_t index, TexturePtr const & tex);
		virtual TexturePtr const & InputPin(uint32_t index) const;

		uint32_t NumOutputPins() const;
		uint32_t OutputPinByName(std::string const & name) const;
		std::string const & OutputPinName(uint32_t index) const;
		virtual void OutputPin(uint32_t index, TexturePtr const & tex);
		virtual TexturePtr const & OutputPin(uint32_t index) const;

		FrameBufferPtr const & OutputFrameBuffer() const
		{
			return frame_buffer_;
		}

		virtual void Apply();

		virtual void OnRenderBegin();

	private:
		void CreateVB();

	protected:
		std::vector<std::pair<std::string, TexturePtr> > input_pins_;
		std::vector<std::pair<std::string, TexturePtr> > output_pins_;
		uint32_t num_bind_output_;

		FrameBufferPtr frame_buffer_;

		GraphicsBufferPtr pos_vb_;

		RenderEffectParameterPtr texel_to_pixel_offset_ep_;
		RenderEffectParameterPtr flipping_ep_;
		std::vector<RenderEffectParameterPtr> input_pins_ep_;
		std::vector<RenderEffectParameterPtr> output_pins_ep_;
	};

	KLAYGE_CORE_API PostProcessPtr LoadPostProcess(ResIdentifierPtr const & ppml, std::string const & pp_name);


	class KLAYGE_CORE_API PostProcessChain : public PostProcess
	{
	public:
		explicit PostProcessChain(std::wstring const & name);
		PostProcessChain(std::wstring const & name,
			std::vector<std::string> const & input_pin_names,
			std::vector<std::string> const & output_pin_names,
			RenderTechniquePtr const & tech);
		virtual ~PostProcessChain()
		{
		}

		void Append(PostProcessPtr const & pp);

		uint32_t NumInputPins() const;
		uint32_t InputPinByName(std::string const & name) const;
		std::string const & InputPinName(uint32_t index) const;
		virtual void InputPin(uint32_t index, TexturePtr const & tex);
		virtual TexturePtr const & InputPin(uint32_t index) const;

		uint32_t NumOutputPins() const;
		uint32_t OutputPinByName(std::string const & name) const;
		std::string const & OutputPinName(uint32_t index) const;
		virtual void OutputPin(uint32_t index, TexturePtr const & tex);
		virtual TexturePtr const & OutputPin(uint32_t index) const;

		virtual void Apply();

	protected:
		std::vector<PostProcessPtr> pp_chain_;
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

	template <typename T>
	class BlurXPostProcess : public T
	{
	public:
		BlurXPostProcess(int kernel_radius, float multiplier)
			: T("BlurX", kernel_radius, multiplier)
		{
		}

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			T::InputPin(index, tex);
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

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			T::InputPin(index, tex);
			this->CalSampleOffsets(tex->Height(0), 3);
		}
	};

	template <typename T>
	class BlurPostProcess : public PostProcessChain
	{
	public:
		BlurPostProcess(int kernel_radius, float multiplier)
			: PostProcessChain(L"Blur")
		{
			this->Append(MakeSharedPtr<BlurXPostProcess<T> >(kernel_radius, multiplier));
			this->Append(MakeSharedPtr<BlurYPostProcess<T> >(kernel_radius, multiplier));
		}

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			pp_chain_[0]->InputPin(index, tex);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			TexturePtr blur_x = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, tex->Format(),
					1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			pp_chain_[0]->OutputPin(0, blur_x);
			pp_chain_[1]->InputPin(0, blur_x);
		}
	};
}

#endif		// _POSTPROCESS_HPP
