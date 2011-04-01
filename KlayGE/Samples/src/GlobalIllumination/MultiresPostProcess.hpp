// MultiresPostProcess.hpp
//////////////////////////////////////////////////////////////////////////////////

#ifndef _MULTIRESPOSTPROCESS_HPP
#define _MULTIRESPOSTPROCESS_HPP

#pragma once

#include <vector>

#include <boost/noncopyable.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	class MultiresPostProcess;
	typedef boost::shared_ptr<MultiresPostProcess> MultiresPostProcessPtr;

	class MultiresPostProcess : boost::noncopyable, public RenderableHelper
	{
	public:
		explicit MultiresPostProcess(std::wstring const & name);
		MultiresPostProcess(std::wstring const & name,
			uint32_t num_levels,
			uint32_t base_level,
			std::vector<std::string> const & param_names,
			std::vector<std::string> const & input_pin_names,
			std::vector<std::string> const & output_pin_names,
			RenderTechniquePtr const & tech);
		virtual ~MultiresPostProcess()
		{
		}

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

		FrameBufferPtr const & OutputFrameBuffer() const
		{
			return frame_buffer_;
		}

		void ActivateSubLevels(uint32_t num_levels, uint32_t base_level = 0);

		virtual void Apply();

		virtual void OnRenderBegin();

	private:
		void CreateVB();
		void CreateMultiresQuads();

	protected:
		void UpdateBinds();

	protected:
		uint32_t num_levels_;
		uint32_t base_level_;

		std::vector<std::pair<std::string, TexturePtr> > input_pins_;
		std::vector<std::pair<std::string, TexturePtr> > output_pins_;
		uint32_t num_bind_output_;
		std::vector<std::pair<std::string, RenderEffectParameterPtr> > params_;

		FrameBufferPtr frame_buffer_;

		GraphicsBufferPtr pos_tc_vb_;

		RenderEffectParameterPtr flipping_ep_;
		std::vector<RenderEffectParameterPtr> input_pins_ep_;
		std::vector<RenderEffectParameterPtr> output_pins_ep_;
	};

	MultiresPostProcessPtr LoadMultiresPostProcess(ResIdentifierPtr const & ppml, std::string const & pp_name, uint32_t num_levels, uint32_t base_level = 0);

}

#endif		// _MULTIRESPOSTPROCESS_HPP
