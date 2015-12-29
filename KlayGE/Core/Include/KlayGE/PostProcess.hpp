// PostProcess.hpp
// KlayGE 后期处理类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2006-2010
// Homepage: http://www.klayge.org
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
			std::vector<std::string> const & param_names,
			std::vector<std::string> const & input_pin_names,
			std::vector<std::string> const & output_pin_names,
			RenderTechniquePtr const & tech);
		virtual ~PostProcess()
		{
		}

		virtual PostProcessPtr Clone();

		void Technique(RenderTechniquePtr const & tech);

		virtual uint32_t NumParams() const;
		virtual uint32_t ParamByName(std::string const & name) const;
		virtual std::string const & ParamName(uint32_t index) const;
		virtual void SetParam(uint32_t index, bool const & value);
		virtual void SetParam(uint32_t index, uint32_t const & value);
		virtual void SetParam(uint32_t index, int32_t const & value);
		virtual void SetParam(uint32_t index, float const & value);
		virtual void SetParam(uint32_t index, uint2 const & value);
		virtual void SetParam(uint32_t index, uint3 const & value);
		virtual void SetParam(uint32_t index, uint4 const & value);
		virtual void SetParam(uint32_t index, int2 const & value);
		virtual void SetParam(uint32_t index, int3 const & value);
		virtual void SetParam(uint32_t index, int4 const & value);
		virtual void SetParam(uint32_t index, float2 const & value);
		virtual void SetParam(uint32_t index, float3 const & value);
		virtual void SetParam(uint32_t index, float4 const & value);
		virtual void SetParam(uint32_t index, float4x4 const & value);
		virtual void SetParam(uint32_t index, std::vector<bool> const & value);
		virtual void SetParam(uint32_t index, std::vector<uint32_t> const & value);
		virtual void SetParam(uint32_t index, std::vector<int32_t> const & value);
		virtual void SetParam(uint32_t index, std::vector<float> const & value);
		virtual void SetParam(uint32_t index, std::vector<uint2> const & value);
		virtual void SetParam(uint32_t index, std::vector<uint3> const & value);
		virtual void SetParam(uint32_t index, std::vector<uint4> const & value);
		virtual void SetParam(uint32_t index, std::vector<int2> const & value);
		virtual void SetParam(uint32_t index, std::vector<int3> const & value);
		virtual void SetParam(uint32_t index, std::vector<int4> const & value);
		virtual void SetParam(uint32_t index, std::vector<float2> const & value);
		virtual void SetParam(uint32_t index, std::vector<float3> const & value);
		virtual void SetParam(uint32_t index, std::vector<float4> const & value);
		virtual void SetParam(uint32_t index, std::vector<float4x4> const & value);
		virtual void GetParam(uint32_t index, bool& value);
		virtual void GetParam(uint32_t index, uint32_t& value);
		virtual void GetParam(uint32_t index, int32_t& value);
		virtual void GetParam(uint32_t index, float& value);
		virtual void GetParam(uint32_t index, uint2& value);
		virtual void GetParam(uint32_t index, uint3& value);
		virtual void GetParam(uint32_t index, uint4& value);
		virtual void GetParam(uint32_t index, int2& value);
		virtual void GetParam(uint32_t index, int3& value);
		virtual void GetParam(uint32_t index, int4& value);
		virtual void GetParam(uint32_t index, float2& value);
		virtual void GetParam(uint32_t index, float3& value);
		virtual void GetParam(uint32_t index, float4& value);
		virtual void GetParam(uint32_t index, float4x4& value);
		virtual void GetParam(uint32_t index, std::vector<bool>& value);
		virtual void GetParam(uint32_t index, std::vector<uint32_t>& value);
		virtual void GetParam(uint32_t index, std::vector<int32_t>& value);
		virtual void GetParam(uint32_t index, std::vector<float>& value);
		virtual void GetParam(uint32_t index, std::vector<uint2>& value);
		virtual void GetParam(uint32_t index, std::vector<uint3>& value);
		virtual void GetParam(uint32_t index, std::vector<uint4>& value);
		virtual void GetParam(uint32_t index, std::vector<int2>& value);
		virtual void GetParam(uint32_t index, std::vector<int3>& value);
		virtual void GetParam(uint32_t index, std::vector<int4>& value);
		virtual void GetParam(uint32_t index, std::vector<float2>& value);
		virtual void GetParam(uint32_t index, std::vector<float3>& value);
		virtual void GetParam(uint32_t index, std::vector<float4>& value);
		virtual void GetParam(uint32_t index, std::vector<float4x4>& value);

		virtual uint32_t NumInputPins() const;
		virtual uint32_t InputPinByName(std::string const & name) const;
		virtual std::string const & InputPinName(uint32_t index) const;
		virtual void InputPin(uint32_t index, TexturePtr const & tex);
		virtual TexturePtr const & InputPin(uint32_t index) const;

		virtual uint32_t NumOutputPins() const;
		virtual uint32_t OutputPinByName(std::string const & name) const;
		virtual std::string const & OutputPinName(uint32_t index) const;
		virtual void OutputPin(uint32_t index, TexturePtr const & tex, int level = 0, int array_index = 0, int face = 0);
		virtual TexturePtr const & OutputPin(uint32_t index) const;

		void CSPixelPerThreadX(uint32_t x)
		{
			cs_pixel_per_thread_x_ = x;
		}
		void CSPixelPerThreadY(uint32_t y)
		{
			cs_pixel_per_thread_y_ = y;
		}
		void CSPixelPerThreadZ(uint32_t z)
		{
			cs_pixel_per_thread_z_ = z;
		}
		uint32_t CSPixelPerThreadX() const
		{
			return cs_pixel_per_thread_x_;
		}
		uint32_t CSPixelPerThreadY() const
		{
			return cs_pixel_per_thread_y_;
		}
		uint32_t CSPixelPerThreadZ() const
		{
			return cs_pixel_per_thread_z_;
		}

		FrameBufferPtr const & OutputFrameBuffer() const
		{
			return frame_buffer_;
		}

		virtual void Apply();

		virtual void OnRenderBegin();

	private:
		void CreateVB();
		void UpdateBinds();

	protected:
		bool cs_based_;
		uint32_t cs_pixel_per_thread_x_;
		uint32_t cs_pixel_per_thread_y_;
		uint32_t cs_pixel_per_thread_z_;

		std::vector<std::pair<std::string, TexturePtr>> input_pins_;
		std::vector<std::pair<std::string, TexturePtr>> output_pins_;
		uint32_t num_bind_output_;
		std::vector<std::pair<std::string, RenderEffectParameterPtr>> params_;

		FrameBufferPtr frame_buffer_;

		std::vector<RenderEffectParameterPtr> input_pins_ep_;
		std::vector<RenderEffectParameterPtr> output_pins_ep_;

		RenderEffectParameterPtr width_height_ep_;
		RenderEffectParameterPtr inv_width_height_ep_;
	};

	KLAYGE_CORE_API PostProcessPtr SyncLoadPostProcess(std::string const & ppml_name, std::string const & pp_name);
	KLAYGE_CORE_API PostProcessPtr ASyncLoadPostProcess(std::string const & ppml_name, std::string const & pp_name);


	class KLAYGE_CORE_API PostProcessChain : public PostProcess
	{
	public:
		explicit PostProcessChain(std::wstring const & name);
		PostProcessChain(std::wstring const & name,
			std::vector<std::string> const & param_names,
			std::vector<std::string> const & input_pin_names,
			std::vector<std::string> const & output_pin_names,
			RenderTechniquePtr const & tech);
		virtual ~PostProcessChain()
		{
		}

		void Append(PostProcessPtr const & pp);
		uint32_t NumPostProcesses() const;
		PostProcessPtr const & GetPostProcess(uint32_t index) const;

		virtual uint32_t NumParams() const;
		virtual uint32_t ParamByName(std::string const & name) const;
		virtual std::string const & ParamName(uint32_t index) const;
		virtual void SetParam(uint32_t index, bool const & value);
		virtual void SetParam(uint32_t index, uint32_t const & value);
		virtual void SetParam(uint32_t index, int32_t const & value);
		virtual void SetParam(uint32_t index, float const & value);
		virtual void SetParam(uint32_t index, uint2 const & value);
		virtual void SetParam(uint32_t index, uint3 const & value);
		virtual void SetParam(uint32_t index, uint4 const & value);
		virtual void SetParam(uint32_t index, int2 const & value);
		virtual void SetParam(uint32_t index, int3 const & value);
		virtual void SetParam(uint32_t index, int4 const & value);
		virtual void SetParam(uint32_t index, float2 const & value);
		virtual void SetParam(uint32_t index, float3 const & value);
		virtual void SetParam(uint32_t index, float4 const & value);
		virtual void SetParam(uint32_t index, float4x4 const & value);
		virtual void SetParam(uint32_t index, std::vector<bool> const & value);
		virtual void SetParam(uint32_t index, std::vector<uint32_t> const & value);
		virtual void SetParam(uint32_t index, std::vector<int32_t> const & value);
		virtual void SetParam(uint32_t index, std::vector<float> const & value);
		virtual void SetParam(uint32_t index, std::vector<uint2> const & value);
		virtual void SetParam(uint32_t index, std::vector<uint3> const & value);
		virtual void SetParam(uint32_t index, std::vector<uint4> const & value);
		virtual void SetParam(uint32_t index, std::vector<int2> const & value);
		virtual void SetParam(uint32_t index, std::vector<int3> const & value);
		virtual void SetParam(uint32_t index, std::vector<int4> const & value);
		virtual void SetParam(uint32_t index, std::vector<float2> const & value);
		virtual void SetParam(uint32_t index, std::vector<float3> const & value);
		virtual void SetParam(uint32_t index, std::vector<float4> const & value);
		virtual void SetParam(uint32_t index, std::vector<float4x4> const & value);
		virtual void GetParam(uint32_t index, bool& value);
		virtual void GetParam(uint32_t index, uint32_t& value);
		virtual void GetParam(uint32_t index, int32_t& value);
		virtual void GetParam(uint32_t index, float& value);
		virtual void GetParam(uint32_t index, uint2& value);
		virtual void GetParam(uint32_t index, uint3& value);
		virtual void GetParam(uint32_t index, uint4& value);
		virtual void GetParam(uint32_t index, int2& value);
		virtual void GetParam(uint32_t index, int3& value);
		virtual void GetParam(uint32_t index, int4& value);
		virtual void GetParam(uint32_t index, float2& value);
		virtual void GetParam(uint32_t index, float3& value);
		virtual void GetParam(uint32_t index, float4& value);
		virtual void GetParam(uint32_t index, float4x4& value);
		virtual void GetParam(uint32_t index, std::vector<bool>& value);
		virtual void GetParam(uint32_t index, std::vector<uint32_t>& value);
		virtual void GetParam(uint32_t index, std::vector<int32_t>& value);
		virtual void GetParam(uint32_t index, std::vector<float>& value);
		virtual void GetParam(uint32_t index, std::vector<uint2>& value);
		virtual void GetParam(uint32_t index, std::vector<uint3>& value);
		virtual void GetParam(uint32_t index, std::vector<uint4>& value);
		virtual void GetParam(uint32_t index, std::vector<int2>& value);
		virtual void GetParam(uint32_t index, std::vector<int3>& value);
		virtual void GetParam(uint32_t index, std::vector<int4>& value);
		virtual void GetParam(uint32_t index, std::vector<float2>& value);
		virtual void GetParam(uint32_t index, std::vector<float3>& value);
		virtual void GetParam(uint32_t index, std::vector<float4>& value);
		virtual void GetParam(uint32_t index, std::vector<float4x4>& value);

		virtual uint32_t NumInputPins() const;
		virtual uint32_t InputPinByName(std::string const & name) const;
		virtual std::string const & InputPinName(uint32_t index) const;
		virtual void InputPin(uint32_t index, TexturePtr const & tex);
		virtual TexturePtr const & InputPin(uint32_t index) const;

		virtual uint32_t NumOutputPins() const;
		virtual uint32_t OutputPinByName(std::string const & name) const;
		virtual std::string const & OutputPinName(uint32_t index) const;
		virtual void OutputPin(uint32_t index, TexturePtr const & tex, int level = 0, int array_index = 0, int face = 0);
		virtual TexturePtr const & OutputPin(uint32_t index) const;

		virtual void Apply();

	protected:
		std::vector<PostProcessPtr> pp_chain_;
	};
	

	class KLAYGE_CORE_API SeparableBoxFilterPostProcess : public PostProcess
	{
	public:
		SeparableBoxFilterPostProcess(RenderTechniquePtr const & tech, int kernel_radius, float multiplier, bool x_dir);
		virtual ~SeparableBoxFilterPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		using PostProcess::InputPin;

		void KernelRadius(int radius);
		void Multiplier(float multiplier);

	protected:
		void CalSampleOffsets(uint32_t tex_size);

	protected:
		int kernel_radius_;
		float multiplier_;
		bool x_dir_;

		RenderEffectParameterPtr src_tex_size_ep_;
		RenderEffectParameterPtr color_weight_ep_;
		RenderEffectParameterPtr tex_coord_offset_ep_;
	};

	class KLAYGE_CORE_API SeparableGaussianFilterPostProcess : public PostProcess
	{
	public:
		SeparableGaussianFilterPostProcess(RenderTechniquePtr const & tech, int kernel_radius, float multiplier, bool x_dir);
		virtual ~SeparableGaussianFilterPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		using PostProcess::InputPin;

		void KernelRadius(int radius);
		void Multiplier(float multiplier);

	protected:
		float GaussianDistribution(float x, float y, float rho);
		void CalSampleOffsets(uint32_t tex_size, float deviation);

	protected:
		int kernel_radius_;
		float multiplier_;
		bool x_dir_;

		RenderEffectParameterPtr src_tex_size_ep_;
		RenderEffectParameterPtr color_weight_ep_;
		RenderEffectParameterPtr tex_coord_offset_ep_;
	};

	class KLAYGE_CORE_API SeparableBilateralFilterPostProcess : public PostProcess
	{
	public:
		SeparableBilateralFilterPostProcess(RenderTechniquePtr const & tech, int kernel_radius, float multiplier, bool x_dir);
		virtual ~SeparableBilateralFilterPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		using PostProcess::InputPin;

		void KernelRadius(int radius);
		void Multiplier(float multiplier);

	protected:
		void CalSampleOffsets(uint32_t tex_size);

	protected:
		int kernel_radius_;
		float multiplier_;
		bool x_dir_;

		RenderEffectParameterPtr kernel_radius_ep_;
		RenderEffectParameterPtr src_tex_size_ep_;
		RenderEffectParameterPtr init_g_ep_;
		RenderEffectParameterPtr blur_factor_ep_;
		RenderEffectParameterPtr sharpness_factor_ep_;
	};

	class KLAYGE_CORE_API SeparableLogGaussianFilterPostProcess : public PostProcess
	{
	public:
		SeparableLogGaussianFilterPostProcess(int kernel_radius, bool linear_depth, bool x_dir);
		virtual ~SeparableLogGaussianFilterPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		using PostProcess::InputPin;

		void KernelRadius(int radius);

	protected:
		float GaussianDistribution(float x, float y, float rho);
		void CalSampleOffsets(uint32_t tex_size, float deviation);

	protected:
		int kernel_radius_;
		bool x_dir_;

		RenderEffectParameterPtr color_weight_ep_;
		RenderEffectParameterPtr tex_coord_offset_ep_;
	};

	template <typename T>
	class BlurPostProcess : public PostProcessChain
	{
	public:
		BlurPostProcess(int kernel_radius, float multiplier)
			: PostProcessChain(L"Blur")
		{
			this->Append(MakeSharedPtr<T>(RenderTechniquePtr(), kernel_radius, multiplier, true));
			this->Append(MakeSharedPtr<T>(RenderTechniquePtr(), kernel_radius, multiplier, false));
		}
		BlurPostProcess(int kernel_radius, float multiplier, RenderTechniquePtr const & tech_x, RenderTechniquePtr const & tech_y)
			: PostProcessChain(L"Blur")
		{
			this->Append(MakeSharedPtr<T>(tech_x, kernel_radius, multiplier, true));
			this->Append(MakeSharedPtr<T>(tech_y, kernel_radius, multiplier, false));
		}

		void InputPin(uint32_t index, TexturePtr const & tex)
		{
			pp_chain_[0]->InputPin(index, tex);

			if (0 == index)
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				TexturePtr blur_x = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, tex->Format(),
						1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				pp_chain_[0]->OutputPin(0, blur_x);
				pp_chain_[1]->InputPin(0, blur_x);
			}
			else
			{
				pp_chain_[1]->InputPin(index, tex);
			}
		}

		using PostProcessChain::InputPin;
	};


	class KLAYGE_CORE_API BicubicFilteringPostProcess : public PostProcessChain
	{
	public:
		BicubicFilteringPostProcess();

		virtual void InputPin(uint32_t index, TexturePtr const & tex) override;
		using PostProcess::InputPin;

		virtual void SetParam(uint32_t index, float2 const & value) override;
		using PostProcess::SetParam;
	};

	class KLAYGE_CORE_API LogGaussianBlurPostProcess : public PostProcessChain
	{
	public:
		LogGaussianBlurPostProcess(int kernel_radius, bool linear_depth);

		void ESMScaleFactor(float factor, Camera const & camera);

		virtual void InputPin(uint32_t index, TexturePtr const & tex) override;
		using PostProcess::InputPin;
	};
}

#endif		// _POSTPROCESS_HPP
