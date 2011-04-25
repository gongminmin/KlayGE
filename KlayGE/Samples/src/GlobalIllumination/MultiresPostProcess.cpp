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
			: PostProcess(name),
				num_levels_(1),
				base_level_(0)
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
			: PostProcess(name, param_names, input_pin_names, output_pin_names, tech),
				num_levels_(num_levels),
				base_level_(base_level)
	{
		this->CreateVB();
	}

	void MultiresPostProcess::CreateVB()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

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
				pos_tc.push_back(boost::make_tuple(base_point, float3(0, 0, static_cast<float>(i))));
				pos_tc.push_back(boost::make_tuple(base_point + float2(0, size.y()), float3(0, 1, static_cast<float>(i))));
				pos_tc.push_back(boost::make_tuple(base_point + float2(size.x(), 0), float3(1, 0, static_cast<float>(i))));
				pos_tc.push_back(boost::make_tuple(base_point + size, float3(1, 1, static_cast<float>(i))));
			}
			base_point.x() += size.x();
			size /= 2;
		}

		ElementInitData init_data;
		init_data.row_pitch = static_cast<uint32_t>(pos_tc.size() * sizeof(pos_tc_type));
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