// MultiresPostProcess.cpp
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/XMLDom.hpp>

#include <cstring>
#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>

#include "MultiresPostProcess.hpp"

namespace KlayGE
{
	MultiresPostProcess::MultiresPostProcess(std::wstring const & name)
			: RenderableHelper(name),
				num_levels_(1),
				base_level_(0),
				num_bind_output_(0)
	{
		this->CreateVB();
	}

	MultiresPostProcess::MultiresPostProcess(std::wstring const & name,
		uint32_t num_levels,
		uint32_t base_level,
		std::vector<std::string> const & param_names,
		std::vector<std::string> const & input_pin_names,
		std::vector<std::string> const & output_pin_names,
		RenderTechniquePtr const & tech)
			: RenderableHelper(name),
				num_levels_(num_levels),
				base_level_(base_level),
				input_pins_(input_pin_names.size()),
				output_pins_(output_pin_names.size()),
				num_bind_output_(0),
				params_(param_names.size()),
				input_pins_ep_(input_pin_names.size())
	{
		this->CreateVB();

		for (size_t i = 0; i < input_pin_names.size(); ++ i)
		{
			input_pins_[i].first = input_pin_names[i];
		}
		for (size_t i = 0; i < output_pin_names.size(); ++ i)
		{
			output_pins_[i].first = output_pin_names[i];
		}
		for (size_t i = 0; i < param_names.size(); ++ i)
		{
			params_[i].first = param_names[i];
		}
		this->Technique(tech);
	}

	void MultiresPostProcess::Technique(RenderTechniquePtr const & tech)
	{
		technique_ = tech;
		this->UpdateBinds();
	}

	void MultiresPostProcess::UpdateBinds()
	{
		if (technique_)
		{
			flipping_ep_ = technique_->Effect().ParameterByName("flipping");
			*flipping_ep_ = static_cast<int32_t>(frame_buffer_->RequiresFlipping() ? -1 : +1);

			input_pins_ep_.resize(input_pins_.size());
			for (size_t i = 0; i < input_pins_.size(); ++ i)
			{
				input_pins_ep_[i] = technique_->Effect().ParameterByName(input_pins_[i].first);
			}

			output_pins_ep_.resize(output_pins_.size());
			for (size_t i = 0; i < output_pins_.size(); ++ i)
			{
				output_pins_ep_[i] = technique_->Effect().ParameterByName(output_pins_[i].first);
			}

			for (size_t i = 0; i < params_.size(); ++ i)
			{
				params_[i].second = technique_->Effect().ParameterByName(params_[i].first);
			}
		}
	}

	uint32_t MultiresPostProcess::NumParams() const
	{
		return static_cast<uint32_t>(params_.size());
	}

	uint32_t MultiresPostProcess::ParamByName(std::string const & name) const
	{
		for (size_t i = 0; i < params_.size(); ++ i)
		{
			if (params_[i].first == name)
			{
				return static_cast<uint32_t>(i);
			}
		}
		return 0xFFFFFFFF;
	}

	std::string const & MultiresPostProcess::ParamName(uint32_t index) const
	{
		return params_[index].first;
	}

	void MultiresPostProcess::SetParam(uint32_t index, bool const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, uint32_t const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, int32_t const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, float const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, uint2 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, uint3 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, uint4 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, int2 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, int3 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, int4 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, float2 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, float3 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, float4 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, float4x4 const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<bool> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<uint32_t> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<int32_t> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<float> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<uint2> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<uint3> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<uint4> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<int2> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<int3> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<int4> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<float2> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<float3> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<float4> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::SetParam(uint32_t index, std::vector<float4x4> const & value)
	{
		*params_[index].second = value;
	}

	void MultiresPostProcess::GetParam(uint32_t index, bool& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, uint32_t& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, int32_t& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, float& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, uint2& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, uint3& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, uint4& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, int2& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, int3& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, int4& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, float2& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, float3& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, float4& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, float4x4& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<bool>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<uint32_t>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<int32_t>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<float>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<uint2>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<uint3>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<uint4>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<int2>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<int3>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<int4>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<float2>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<float3>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<float4>& value)
	{
		params_[index].second->Value(value);
	}

	void MultiresPostProcess::GetParam(uint32_t index, std::vector<float4x4>& value)
	{
		params_[index].second->Value(value);
	}

	uint32_t MultiresPostProcess::NumInputPins() const
	{
		return static_cast<uint32_t>(input_pins_.size());
	}

	uint32_t MultiresPostProcess::InputPinByName(std::string const & name) const
	{
		for (size_t i = 0; i < input_pins_.size(); ++ i)
		{
			if (input_pins_[i].first == name)
			{
				return static_cast<uint32_t>(i);
			}
		}
		return 0xFFFFFFFF;
	}

	std::string const & MultiresPostProcess::InputPinName(uint32_t index) const
	{
		return input_pins_[index].first;
	}

	void MultiresPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		input_pins_[index].second = tex;
		*(input_pins_ep_[index]) = tex;
	}

	TexturePtr const & MultiresPostProcess::InputPin(uint32_t index) const
	{
		BOOST_ASSERT(index < input_pins_.size());
		return input_pins_[index].second;
	}

	uint32_t MultiresPostProcess::NumOutputPins() const
	{
		return static_cast<uint32_t>(output_pins_.size());
	}

	uint32_t MultiresPostProcess::OutputPinByName(std::string const & name) const
	{
		for (size_t i = 0; i < output_pins_.size(); ++ i)
		{
			if (output_pins_[i].first == name)
			{
				return static_cast<uint32_t>(i);
			}
		}
		return 0xFFFFFFFF;
	}

	std::string const & MultiresPostProcess::OutputPinName(uint32_t index) const
	{
		return output_pins_[index].first;
	}

	void MultiresPostProcess::OutputPin(uint32_t index, TexturePtr const & tex, int level, int array_index, int face)
	{
		if (!output_pins_[index].second && tex)
		{
			++ num_bind_output_;
		}
		if (output_pins_[index].second && !tex)
		{
			-- num_bind_output_;
		}

		output_pins_[index].second = tex;
		if (tex)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderViewPtr view;
			if (Texture::TT_2D == tex->Type())
			{
				view = rf.Make2DRenderView(*tex, array_index, level);
			}
			else
			{
				BOOST_ASSERT(Texture::TT_Cube == tex->Type());
				view = rf.Make2DRenderView(*tex, array_index, static_cast<Texture::CubeFaces>(face), level);
			}
			frame_buffer_->Attach(FrameBuffer::ATT_Color0 + index, view);

			if (output_pins_ep_[index])
			{
				*(output_pins_ep_[index]) = tex;
			}
		}
	}

	TexturePtr const & MultiresPostProcess::OutputPin(uint32_t index) const
	{
		BOOST_ASSERT(index < output_pins_.size());
		return output_pins_[index].second;
	}

	void MultiresPostProcess::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		FrameBufferPtr const & fb = (0 == num_bind_output_) ? re.DefaultFrameBuffer() : frame_buffer_;
		re.BindFrameBuffer(fb);
		this->Render();
	}

	void MultiresPostProcess::OnRenderBegin()
	{
	}

	void MultiresPostProcess::CreateVB()
	{
		RenderFactory &rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleStrip);

		CreateMultiresQuads();

		frame_buffer_ = rf.MakeFrameBuffer();
		frame_buffer_->GetViewport().camera = rf.RenderEngineInstance().CurFrameBuffer()->GetViewport().camera;
	}

	void MultiresPostProcess::CreateMultiresQuads()
	{
		RenderFactory &rf = Context::Instance().RenderFactoryInstance();

		typedef boost::tuple<float2, float3> pos_tc_type;
		std::vector<pos_tc_type> pos_tc;
		pos_tc.reserve(4 * num_levels_);

		float2 base_point(-1, -1), size(1, 2);
		for (uint32_t i = 0; i != base_level_ + num_levels_; ++i)
		{
			if (i >= base_level_)
			{
				pos_tc.push_back(boost::make_tuple(base_point, float3(0, 0, (float)i)));
				pos_tc.push_back(boost::make_tuple(base_point + float2(0, size.y()), float3(0, 1, (float)i)));
				pos_tc.push_back(boost::make_tuple(base_point + float2(size.x(), 0), float3(1, 0, (float)i)));
				pos_tc.push_back(boost::make_tuple(base_point + size, float3(1, 1, (float)i)));
			}
			base_point.x() += size.x();
			size /= 2;
		}

		ElementInitData init_data;
		init_data.row_pitch = pos_tc.size() * sizeof(pos_tc_type);
		init_data.data = &(pos_tc[0]);
		pos_tc_vb_ = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		rl_->BindVertexStream(pos_tc_vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F), vertex_element(VEU_TextureCoord, 0, EF_BGR32F)));
	}

	void MultiresPostProcess::ActivateSubLevels(uint32_t num_levels, uint32_t base_level /*= 0*/)
	{
		num_levels_ = num_levels;
		base_level_ = base_level;

		CreateMultiresQuads();
	}

	MultiresPostProcessPtr LoadMultiresPostProcess(ResIdentifierPtr const & ppml, std::string const & pp_name, uint32_t num_levels, uint32_t base_level /*= 0*/)
	{
		XMLDocument doc;
		XMLNodePtr root = doc.Parse(ppml);

		std::wstring wname;
		std::vector<std::string> param_names;
		std::vector<std::string> input_pin_names;
		std::vector<std::string> output_pin_names;
		RenderTechniquePtr tech;

		for (XMLNodePtr pp_node = root->FirstNode("multires_post_processor"); pp_node; pp_node = pp_node->NextSibling("multires_post_processor"))
		{
			std::string name = pp_node->Attrib("name")->ValueString();
			if (pp_name == name)
			{
				Convert(wname, name);

				XMLNodePtr params_chunk = pp_node->FirstNode("params");
				if (params_chunk)
				{
					for (XMLNodePtr p_node = params_chunk->FirstNode("param"); p_node; p_node = p_node->NextSibling("param"))
					{
						param_names.push_back(p_node->Attrib("name")->ValueString());
					}
				}
				XMLNodePtr input_chunk = pp_node->FirstNode("input");
				if (input_chunk)
				{
					for (XMLNodePtr pin_node = input_chunk->FirstNode("pin"); pin_node; pin_node = pin_node->NextSibling("pin"))
					{
						input_pin_names.push_back(pin_node->Attrib("name")->ValueString());
					}
				}
				XMLNodePtr output_chunk = pp_node->FirstNode("output");
				if (output_chunk)
				{
					for (XMLNodePtr pin_node = output_chunk->FirstNode("pin"); pin_node; pin_node = pin_node->NextSibling("pin"))
					{
						output_pin_names.push_back(pin_node->Attrib("name")->ValueString());
					}
				}
				XMLNodePtr shader_chunk = pp_node->FirstNode("shader");
				if (shader_chunk)
				{
					std::string effect_name = shader_chunk->Attrib("effect")->ValueString();
					std::string tech_name = shader_chunk->Attrib("tech")->ValueString();
					tech = Context::Instance().RenderFactoryInstance().LoadEffect(effect_name)->TechniqueByName(tech_name);
				}
			}
		}

		return MakeSharedPtr<MultiresPostProcess>(wname, num_levels, base_level, param_names, input_pin_names, output_pin_names, tech);
	}

}