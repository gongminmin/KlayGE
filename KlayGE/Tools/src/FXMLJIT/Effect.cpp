/**
 * @file Effect.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KFL/XMLDom.hpp>
#include <KFL/Thread.hpp>

#include <sstream>
#include <fstream>
#include <boost/assert.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011 6334)
#endif
#include <boost/functional/hash.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "Effect.hpp"

namespace
{
	using namespace KlayGE;

	uint32_t const KFX_VERSION = 0x0104;

	mutex singleton_mutex;

	class type_define
	{
	public:
		static type_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<type_define>();
				}
			}
			return *instance_;
		}

		uint32_t type_code(std::string const & name) const
		{
			for (uint32_t i = 0; i < types_.size(); ++ i)
			{
				if (types_[i] == name)
				{
					return i;
				}
			}
			BOOST_ASSERT(false);
			return 0xFFFFFFFF;
		}

		std::string const & type_name(uint32_t code) const
		{
			if (code < types_.size())
			{
				return types_[code];
			}
			BOOST_ASSERT(false);

			static std::string empty_str("");
			return empty_str;
		}

		type_define()
		{
			types_.push_back("bool");
			types_.push_back("string");
			types_.push_back("texture1D");
			types_.push_back("texture2D");
			types_.push_back("texture3D");
			types_.push_back("textureCUBE");
			types_.push_back("texture1DArray");
			types_.push_back("texture2DArray");
			types_.push_back("texture3DArray");
			types_.push_back("textureCUBEArray");
			types_.push_back("sampler");
			types_.push_back("shader");
			types_.push_back("uint");
			types_.push_back("uint2");
			types_.push_back("uint3");
			types_.push_back("uint4");
			types_.push_back("int");
			types_.push_back("int2");
			types_.push_back("int3");
			types_.push_back("int4");
			types_.push_back("float");
			types_.push_back("float2");
			types_.push_back("float2x2");
			types_.push_back("float2x3");
			types_.push_back("float2x4");
			types_.push_back("float3");
			types_.push_back("float3x2");
			types_.push_back("float3x3");
			types_.push_back("float3x4");
			types_.push_back("float4");
			types_.push_back("float4x2");
			types_.push_back("float4x3");
			types_.push_back("float4x4");
			types_.push_back("buffer");
			types_.push_back("structured_buffer");
			types_.push_back("byte_address_buffer");
			types_.push_back("rw_buffer");
			types_.push_back("rw_structured_buffer");
			types_.push_back("rw_texture1D");
			types_.push_back("rw_texture2D");
			types_.push_back("rw_texture3D");
			types_.push_back("rw_texture1DArray");
			types_.push_back("rw_texture2DArray");
			types_.push_back("rw_byte_address_buffer");
			types_.push_back("append_structured_buffer");
			types_.push_back("consume_structured_buffer");
		}

	private:
		std::vector<std::string> types_;

		static shared_ptr<type_define> instance_;
	};
	shared_ptr<type_define> type_define::instance_;

	class shade_mode_define
	{
	public:
		static shade_mode_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<shade_mode_define>();
				}
			}
			return *instance_;
		}

		ShadeMode from_str(std::string const & name) const
		{
			for (uint32_t i = 0; i < sms_.size(); ++ i)
			{
				if (sms_[i] == name)
				{
					return static_cast<ShadeMode>(i);
				}
			}
			LogError("Wrong ShadeMode name: %s", name.c_str());
			return static_cast<ShadeMode>(0xFFFFFFFF);
		}

		shade_mode_define()
		{
			sms_.push_back("flat");
			sms_.push_back("gouraud");
		}

	private:
		std::vector<std::string> sms_;

		static shared_ptr<shade_mode_define> instance_;
	};
	shared_ptr<shade_mode_define> shade_mode_define::instance_;

	class compare_function_define
	{
	public:
		static compare_function_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<compare_function_define>();
				}
			}
			return *instance_;
		}

		CompareFunction from_str(std::string const & name) const
		{
			for (uint32_t i = 0; i < cfs_.size(); ++ i)
			{
				if (cfs_[i] == name)
				{
					return static_cast<CompareFunction>(i);
				}
			}
			LogError("Wrong CompareFunction name: %s", name.c_str());
			return static_cast<CompareFunction>(0xFFFFFFFF);
		}

		compare_function_define()
		{
			cfs_.push_back("always_fail");
			cfs_.push_back("always_pass");
			cfs_.push_back("less");
			cfs_.push_back("less_equal");
			cfs_.push_back("equal");
			cfs_.push_back("not_equal");
			cfs_.push_back("greater_equal");
			cfs_.push_back("greater");
		}

	private:
		std::vector<std::string> cfs_;

		static shared_ptr<compare_function_define> instance_;
	};
	shared_ptr<compare_function_define> compare_function_define::instance_;

	class cull_mode_define
	{
	public:
		static cull_mode_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<cull_mode_define>();
				}
			}
			return *instance_;
		}

		CullMode from_str(std::string const & name) const
		{
			for (uint32_t i = 0; i < cms_.size(); ++ i)
			{
				if (cms_[i] == name)
				{
					return static_cast<CullMode>(i);
				}
			}
			LogError("Wrong CullMode name: %s", name.c_str());
			return static_cast<CullMode>(0xFFFFFFFF);
		}

		cull_mode_define()
		{
			cms_.push_back("none");
			cms_.push_back("front");
			cms_.push_back("back");
		}

	private:
		std::vector<std::string> cms_;

		static shared_ptr<cull_mode_define> instance_;
	};
	shared_ptr<cull_mode_define> cull_mode_define::instance_;

	class polygon_mode_define
	{
	public:
		static polygon_mode_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<polygon_mode_define>();
				}
			}
			return *instance_;
		}

		PolygonMode from_str(std::string const & name) const
		{
			for (uint32_t i = 0; i < pms_.size(); ++ i)
			{
				if (pms_[i] == name)
				{
					return static_cast<PolygonMode>(i);
				}
			}
			LogError("Wrong PolygonMode name: %s", name.c_str());
			return static_cast<PolygonMode>(0xFFFFFFFF);
		}

		polygon_mode_define()
		{
			pms_.push_back("point");
			pms_.push_back("line");
			pms_.push_back("fill");
		}

	private:
		std::vector<std::string> pms_;

		static shared_ptr<polygon_mode_define> instance_;
	};
	shared_ptr<polygon_mode_define> polygon_mode_define::instance_;

	class alpha_blend_factor_define
	{
	public:
		static alpha_blend_factor_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<alpha_blend_factor_define>();
				}
			}
			return *instance_;
		}

		AlphaBlendFactor from_str(std::string const & name) const
		{
			for (uint32_t i = 0; i < abfs_.size(); ++ i)
			{
				if (abfs_[i] == name)
				{
					return static_cast<AlphaBlendFactor>(i);
				}
			}
			LogError("Wrong AlphaBlendFactor name: %s", name.c_str());
			return static_cast<AlphaBlendFactor>(0xFFFFFFFF);
		}

		alpha_blend_factor_define()
		{
			abfs_.push_back("zero");
			abfs_.push_back("one");
			abfs_.push_back("src_alpha");
			abfs_.push_back("dst_alpha");
			abfs_.push_back("inv_src_alpha");
			abfs_.push_back("inv_dst_alpha");
			abfs_.push_back("src_color");
			abfs_.push_back("dst_color");
			abfs_.push_back("inv_src_color");
			abfs_.push_back("inv_dst_color");
			abfs_.push_back("src_alpha_sat");
		}

	private:
		std::vector<std::string> abfs_;

		static shared_ptr<alpha_blend_factor_define> instance_;
	};
	shared_ptr<alpha_blend_factor_define> alpha_blend_factor_define::instance_;

	class blend_operation_define
	{
	public:
		static blend_operation_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<blend_operation_define>();
				}
			}
			return *instance_;
		}

		BlendOperation from_str(std::string const & name) const
		{
			for (uint32_t i = 0; i < bops_.size(); ++ i)
			{
				if (bops_[i] == name)
				{
					return static_cast<BlendOperation>(i + 1);
				}
			}
			LogError("Wrong BlendOperation name: %s", name.c_str());
			return static_cast<BlendOperation>(0xFFFFFFFF);
		}

		blend_operation_define()
		{
			bops_.push_back("add");
			bops_.push_back("sub");
			bops_.push_back("rev_sub");
			bops_.push_back("min");
			bops_.push_back("max");
		}

	private:
		std::vector<std::string> bops_;

		static shared_ptr<blend_operation_define> instance_;
	};
	shared_ptr<blend_operation_define> blend_operation_define::instance_;

	class stencil_operation_define
	{
	public:
		static stencil_operation_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<stencil_operation_define>();
				}
			}
			return *instance_;
		}

		StencilOperation from_str(std::string const & name) const
		{
			for (uint32_t i = 0; i < sops_.size(); ++ i)
			{
				if (sops_[i] == name)
				{
					return static_cast<StencilOperation>(i);
				}
			}
			LogError("Wrong StencilOperation name: %s", name.c_str());
			return static_cast<StencilOperation>(0xFFFFFFFF);
		}

		stencil_operation_define()
		{
			sops_.push_back("keep");
			sops_.push_back("zero");
			sops_.push_back("replace");
			sops_.push_back("incr");
			sops_.push_back("decr");
			sops_.push_back("invert");
			sops_.push_back("incr_wrap");
			sops_.push_back("decr_wrap");
		}

	private:
		std::vector<std::string> sops_;

		static shared_ptr<stencil_operation_define> instance_;
	};
	shared_ptr<stencil_operation_define> stencil_operation_define::instance_;

	class texture_filter_mode_define
	{
	public:
		static texture_filter_mode_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<texture_filter_mode_define>();
				}
			}
			return *instance_;
		}

		TexFilterOp from_str(std::string const & name) const
		{
			int cmp;
			std::string f;
			if (0 == name.find("cmp_"))
			{
				cmp = 1;
				f = name.substr(4);
			}
			else
			{
				cmp = 0;
				f = name;
			}
			for (uint32_t i = 0; i < tfs_.size(); ++ i)
			{
				if (tfs_[i] == f)
				{
					return static_cast<TexFilterOp>((cmp << 4) + i);
				}
			}
			if ("anisotropic" == f)
			{
				return static_cast<TexFilterOp>((cmp << 4) + TFO_Anisotropic);
			}
			LogError("Wrong TexFilterOp name: %s", name.c_str());
			return static_cast<TexFilterOp>(0xFFFFFFFF);
		}

		texture_filter_mode_define()
		{
			tfs_.push_back("min_mag_mip_point");
			tfs_.push_back("min_mag_point_mip_linear");
			tfs_.push_back("min_point_mag_linear_mip_point");
			tfs_.push_back("min_point_mag_mip_linear");
			tfs_.push_back("min_linear_mag_mip_point");
			tfs_.push_back("min_linear_mag_point_mip_linear");
			tfs_.push_back("min_mag_linear_mip_point");
			tfs_.push_back("min_mag_mip_linear");
		}

	private:
		std::vector<std::string> tfs_;

		static shared_ptr<texture_filter_mode_define> instance_;
	};
	shared_ptr<texture_filter_mode_define> texture_filter_mode_define::instance_;

	class texture_addr_mode_define
	{
	public:
		static texture_addr_mode_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<texture_addr_mode_define>();
				}
			}
			return *instance_;
		}

		TexAddressingMode from_str(std::string const & name) const
		{
			for (uint32_t i = 0; i < tams_.size(); ++ i)
			{
				if (tams_[i] == name)
				{
					return static_cast<TexAddressingMode>(i);
				}
			}
			LogError("Wrong TexAddressingMode name: %s", name.c_str());
			return static_cast<TexAddressingMode>(0xFFFFFFFF);
		}

		texture_addr_mode_define()
		{
			tams_.push_back("wrap");
			tams_.push_back("mirror");
			tams_.push_back("clamp");
			tams_.push_back("border");
		}

	private:
		std::vector<std::string> tams_;

		static shared_ptr<texture_addr_mode_define> instance_;
	};
	shared_ptr<texture_addr_mode_define> texture_addr_mode_define::instance_;

	class logic_operation_define
	{
	public:
		static logic_operation_define& instance()
		{
			if (!instance_)
			{
				unique_lock<mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeSharedPtr<logic_operation_define>();
				}
			}
			return *instance_;
		}

		LogicOperation from_str(std::string const & name) const
		{
			for (uint32_t i = 0; i < lops_.size(); ++ i)
			{
				if (lops_[i] == name)
				{
					return static_cast<LogicOperation>(i);
				}
			}
			LogError("Wrong LogicOperation name: %s", name.c_str());
			return static_cast<LogicOperation>(0xFFFFFFFF);
		}

		logic_operation_define()
		{
			lops_.push_back("clear");
			lops_.push_back("set");
			lops_.push_back("copy");
			lops_.push_back("copy_inverted");
			lops_.push_back("noop");
			lops_.push_back("invert");
			lops_.push_back("and");
			lops_.push_back("nand");
			lops_.push_back("or");
			lops_.push_back("nor");
			lops_.push_back("xor");
			lops_.push_back("equiv");
			lops_.push_back("and_reverse");
			lops_.push_back("and_inverted");
			lops_.push_back("or_reverse");
			lops_.push_back("or_inverted");
		}

	private:
		std::vector<std::string> lops_;

		static shared_ptr<logic_operation_define> instance_;
	};
	shared_ptr<logic_operation_define> logic_operation_define::instance_;

	bool bool_from_str(std::string const & name)
	{
		if (("true" == name) || ("1" == name))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	int get_index(XMLNodePtr const & node)
	{
		int index = 0;
		XMLAttributePtr attr = node->Attrib("index");
		if (attr)
		{
			index = attr->ValueInt();
		}
		return index;
	}

	std::string get_profile(XMLNodePtr const & node)
	{
		XMLAttributePtr attr = node->Attrib("profile");
		if (attr)
		{
			return attr->ValueString();
		}
		else
		{
			return "auto";
		}
	}

	std::string get_func_name(XMLNodePtr const & node)
	{
		std::string value = node->Attrib("value")->ValueString();
		return value.substr(0, value.find("("));
	}


	RenderVariablePtr read_var(XMLNodePtr const & node, uint32_t type, uint32_t array_size)
	{
		RenderVariablePtr var;
		XMLAttributePtr attr;

		switch (type)
		{
		case REDT_bool:
			if (0 == array_size)
			{
				attr = node->Attrib("value");
				bool tmp = false;
				if (attr)
				{
					std::string value_str = attr->ValueString();
					tmp = bool_from_str(value_str);
				}

				var = MakeSharedPtr<RenderVariableBool>();
				*var = tmp;
			}
			break;

		case REDT_uint:
			if (0 == array_size)
			{
				attr = node->Attrib("value");
				uint32_t tmp = 0;
				if (attr)
				{
					tmp = attr->ValueInt();
				}

				var = MakeSharedPtr<RenderVariableUInt>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableUIntArray>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<uint32_t> init_val(std::min(array_size, static_cast<uint32_t>(strs.size())), 0);
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							if (index < strs.size())
							{
								boost::algorithm::trim(strs[index]);
								init_val[index] = boost::lexical_cast<uint32_t>(strs[index]);
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_int:
			if (0 == array_size)
			{
				attr = node->Attrib("value");
				int32_t tmp = 0;
				if (attr)
				{
					tmp = attr->ValueInt();
				}

				var = MakeSharedPtr<RenderVariableInt>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableIntArray>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<int32_t> init_val(std::min(array_size, static_cast<uint32_t>(strs.size())), 0);
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							if (index < strs.size())
							{
								boost::algorithm::trim(strs[index]);
								init_val[index] = boost::lexical_cast<int32_t>(strs[index]);
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_string:
			{
				attr = node->Attrib("value");
				std::string tmp;
				if (attr)
				{
					tmp = attr->ValueString();
				}

				var = MakeSharedPtr<RenderVariableString>();
				*var = tmp;
			}
			break;

		case REDT_texture1D:
		case REDT_texture2D:
		case REDT_texture3D:
		case REDT_textureCUBE:
		case REDT_texture1DArray:
		case REDT_texture2DArray:
		case REDT_texture3DArray:
		case REDT_textureCUBEArray:
		case REDT_rw_texture1D:
		case REDT_rw_texture2D:
		case REDT_rw_texture3D:
		case REDT_rw_texture1DArray:
		case REDT_rw_texture2DArray:
			var = MakeSharedPtr<RenderVariableTexture>();
			*var = TexturePtr();
			attr = node->Attrib("elem_type");
			if (attr)
			{
				*var = attr->ValueString();
			}
			else
			{
				*var = std::string("float4");
			}
			break;

		case REDT_sampler:
			{
				SamplerStateDesc desc;

				for (XMLNodePtr state_node = node->FirstNode("state"); state_node; state_node = state_node->NextSibling("state"))
				{
					std::string name = state_node->Attrib("name")->ValueString();
					if ("filtering" == name)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.filter = texture_filter_mode_define::instance().from_str(value_str);
					}
					else if ("address_u" == name)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.addr_mode_u = texture_addr_mode_define::instance().from_str(value_str);
					}
					else if ("address_v" == name)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.addr_mode_v = texture_addr_mode_define::instance().from_str(value_str);
					}
					else if ("address_w" == name)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.addr_mode_w = texture_addr_mode_define::instance().from_str(value_str);
					}
					else if ("max_anisotropy" == name)
					{
						desc.max_anisotropy = static_cast<uint8_t>(state_node->Attrib("value")->ValueUInt());
					}
					else if ("min_lod" == name)
					{
						desc.min_lod = state_node->Attrib("value")->ValueFloat();
					}
					else if ("max_lod" == name)
					{
						desc.max_lod = state_node->Attrib("value")->ValueFloat();
					}
					else if ("mip_map_lod_bias" == name)
					{
						desc.mip_map_lod_bias = state_node->Attrib("value")->ValueFloat();
					}
					else if ("cmp_func" == name)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.cmp_func = compare_function_define::instance().from_str(value_str);
					}
					else if ("border_clr" == name)
					{
						attr = state_node->Attrib("r");
						if (attr)
						{
							desc.border_clr.r() = attr->ValueFloat();
						}
						attr = state_node->Attrib("g");
						if (attr)
						{
							desc.border_clr.g() = attr->ValueFloat();
						}
						attr = state_node->Attrib("b");
						if (attr)
						{
							desc.border_clr.b() = attr->ValueFloat();
						}
						attr = state_node->Attrib("a");
						if (attr)
						{
							desc.border_clr.a() = attr->ValueFloat();
						}
					}
					else
					{
						LogError("Wrong sampler state name: %s", name.c_str());
					}
				}

				var = MakeSharedPtr<RenderVariableSampler>();
				*var = Context::Instance().RenderFactoryInstance().MakeSamplerStateObject(desc);
			}
			break;

		case REDT_shader:
			{
				ShaderDesc desc;
				desc.profile = get_profile(node);
				desc.func_name = get_func_name(node);

				var = MakeSharedPtr<RenderVariableShader>();
				*var = desc;
			}
			break;

		case REDT_float:
			if (0 == array_size)
			{
				float tmp = 0;
				attr = node->Attrib("value");
				if (attr)
				{
					tmp = attr->ValueFloat();
				}

				var = MakeSharedPtr<RenderVariableFloat>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloatArray>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<float> init_val(std::min(array_size, static_cast<uint32_t>(strs.size())), 0.0f);
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							if (index < strs.size())
							{
								boost::algorithm::trim(strs[index]);
								init_val[index] = boost::lexical_cast<float>(strs[index]);
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_uint2:
			if (0 == array_size)
			{
				uint2 tmp(0, 0);
				attr = node->Attrib("x");
				if (attr)
				{
					tmp.x() = attr->ValueUInt();
				}
				attr = node->Attrib("y");
				if (attr)
				{
					tmp.y() = attr->ValueUInt();
				}

				var = MakeSharedPtr<RenderVariableUInt2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt2Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<uint2> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 1) / 2)), int2(0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 2; ++ j)
							{
								if (index * 2 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 2 + j]);
									init_val[index][j] = boost::lexical_cast<uint32_t>(strs[index * 2 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_uint3:
			if (0 == array_size)
			{
				uint3 tmp(0, 0, 0);
				attr = node->Attrib("x");
				if (attr)
				{
					tmp.x() = attr->ValueUInt();
				}
				attr = node->Attrib("y");
				if (attr)
				{
					tmp.y() = attr->ValueUInt();
				}
				attr = node->Attrib("z");
				if (attr)
				{
					tmp.z() = attr->ValueUInt();
				}

				var = MakeSharedPtr<RenderVariableUInt3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt3Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<uint3> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 2) / 3)), int3(0, 0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 3; ++ j)
							{
								if (index * 3 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 3 + j]);
									init_val[index][j] = boost::lexical_cast<uint32_t>(strs[index * 3 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_uint4:
			if (0 == array_size)
			{
				uint4 tmp(0, 0, 0, 0);
				attr = node->Attrib("x");
				if (attr)
				{
					tmp.x() = attr->ValueUInt();
				}
				attr = node->Attrib("y");
				if (attr)
				{
					tmp.y() = attr->ValueUInt();
				}
				attr = node->Attrib("z");
				if (attr)
				{
					tmp.z() = attr->ValueUInt();
				}
				attr = node->Attrib("w");
				if (attr)
				{
					tmp.w() = attr->ValueUInt();
				}

				var = MakeSharedPtr<RenderVariableUInt4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt4Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<int4> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 3) / 4)), int4(0, 0, 0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 4; ++ j)
							{
								if (index * 4 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 4 + j]);
									init_val[index][j] = boost::lexical_cast<uint32_t>(strs[index * 4 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_int2:
			if (0 == array_size)
			{
				int2 tmp(0, 0);
				attr = node->Attrib("x");
				if (attr)
				{
					tmp.x() = attr->ValueInt();
				}
				attr = node->Attrib("y");
				if (attr)
				{
					tmp.y() = attr->ValueInt();
				}

				var = MakeSharedPtr<RenderVariableInt2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt2Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<int2> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 1) / 2)), int2(0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 2; ++ j)
							{
								if (index * 2 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 2 + j]);
									init_val[index][j] = boost::lexical_cast<int32_t>(strs[index * 2 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_int3:
			if (0 == array_size)
			{
				int3 tmp(0, 0, 0);
				attr = node->Attrib("x");
				if (attr)
				{
					tmp.x() = attr->ValueInt();
				}
				attr = node->Attrib("y");
				if (attr)
				{
					tmp.y() = attr->ValueInt();
				}
				attr = node->Attrib("z");
				if (attr)
				{
					tmp.z() = attr->ValueInt();
				}

				var = MakeSharedPtr<RenderVariableInt3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt3Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<int3> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 2) / 3)), int3(0, 0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 3; ++ j)
							{
								if (index * 3 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 3 + j]);
									init_val[index][j] = boost::lexical_cast<int32_t>(strs[index * 3 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_int4:
			if (0 == array_size)
			{
				int4 tmp(0, 0, 0, 0);
				attr = node->Attrib("x");
				if (attr)
				{
					tmp.x() = attr->ValueInt();
				}
				attr = node->Attrib("y");
				if (attr)
				{
					tmp.y() = attr->ValueInt();
				}
				attr = node->Attrib("z");
				if (attr)
				{
					tmp.z() = attr->ValueInt();
				}
				attr = node->Attrib("w");
				if (attr)
				{
					tmp.w() = attr->ValueInt();
				}

				var = MakeSharedPtr<RenderVariableInt4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt4Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<int4> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 3) / 4)), int4(0, 0, 0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 4; ++ j)
							{
								if (index * 4 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 4 + j]);
									init_val[index][j] = boost::lexical_cast<int32_t>(strs[index * 4 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_float2:
			if (0 == array_size)
			{
				float2 tmp(0, 0);
				attr = node->Attrib("x");
				if (attr)
				{
					tmp.x() = attr->ValueFloat();
				}
				attr = node->Attrib("y");
				if (attr)
				{
					tmp.y() = attr->ValueFloat();
				}

				var = MakeSharedPtr<RenderVariableFloat2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat2Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<float2> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 1) / 2)), float2(0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 2; ++ j)
							{
								if (index * 2 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 2 + j]);
									init_val[index][j] = boost::lexical_cast<float>(strs[index * 2 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_float3:
			if (0 == array_size)
			{
				float3 tmp(0, 0, 0);
				attr = node->Attrib("x");
				if (attr)
				{
					tmp.x() = attr->ValueFloat();
				}
				attr = node->Attrib("y");
				if (attr)
				{
					tmp.y() = attr->ValueFloat();
				}
				attr = node->Attrib("z");
				if (attr)
				{
					tmp.z() = attr->ValueFloat();
				}

				var = MakeSharedPtr<RenderVariableFloat3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat3Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<float3> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 2) / 3)), float3(0, 0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 3; ++ j)
							{
								if (index * 3 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 3 + j]);
									init_val[index][j] = boost::lexical_cast<float>(strs[index * 3 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_float4:
			if (0 == array_size)
			{
				float4 tmp(0, 0, 0, 0);
				attr = node->Attrib("x");
				if (attr)
				{
					tmp.x() = attr->ValueFloat();
				}
				attr = node->Attrib("y");
				if (attr)
				{
					tmp.y() = attr->ValueFloat();
				}
				attr = node->Attrib("z");
				if (attr)
				{
					tmp.z() = attr->ValueFloat();
				}
				attr = node->Attrib("w");
				if (attr)
				{
					tmp.w() = attr->ValueFloat();
				}

				var = MakeSharedPtr<RenderVariableFloat4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat4Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<float4> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 3) / 4)), float4(0, 0, 0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 4; ++ j)
							{
								if (index * 4 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 4 + j]);
									init_val[index][j] = boost::lexical_cast<float>(strs[index * 4 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_float4x4:
			if (0 == array_size)
			{
				float4x4 tmp(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				for (int y = 0; y < 4; ++ y)
				{
					for (int x = 0; x < 4; ++ x)
					{
						std::stringstream oss;
						oss << "_" << static_cast<char>('0' + y) << static_cast<char>('0' + x);
						attr = node->Attrib(oss.str().c_str());
						if (attr)
						{
							tmp[y * 4 + x] = attr->ValueFloat();
						}
					}
				}

				var = MakeSharedPtr<RenderVariableFloat4x4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat4x4Array>();

				XMLNodePtr value_node = node->FirstNode("value");
				if (value_node)
				{
					value_node = value_node->FirstNode();
					if (value_node && (XNT_CData == value_node->Type()))
					{
						std::string value_str = value_node->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(","));
						std::vector<float4> init_val(std::min(array_size, static_cast<uint32_t>((strs.size() + 3) / 4)), float4(0, 0, 0, 0));
						for (size_t index = 0; index < init_val.size(); ++ index)
						{
							for (size_t j = 0; j < 16; ++ j)
							{
								if (index * 16 + j < strs.size())
								{
									boost::algorithm::trim(strs[index * 16 + j]);
									init_val[index][j] = boost::lexical_cast<float>(strs[index * 16 + j]);
								}
							}
						}
						*var = init_val;
					}
				}
			}
			break;

		case REDT_buffer:
		case REDT_structured_buffer:
		case REDT_rw_buffer:
		case REDT_rw_structured_buffer:
		case REDT_consume_structured_buffer:
		case REDT_append_structured_buffer:
			var = MakeSharedPtr<RenderVariableBuffer>();
			*var = GraphicsBufferPtr();
			attr = node->Attrib("elem_type");
			if (attr)
			{
				*var = attr->ValueString();
			}
			else
			{
				*var = std::string("float4");
			}
			break;

		case REDT_byte_address_buffer:
		case REDT_rw_byte_address_buffer:
			var = MakeSharedPtr<RenderVariableByteAddressBuffer>();
			*var = GraphicsBufferPtr();
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return var;
	}

	RenderVariablePtr stream_in_var(ResIdentifierPtr const & res, uint32_t type, uint32_t array_size)
	{
		RenderVariablePtr var;

		switch (type)
		{
		case REDT_bool:
			if (0 == array_size)
			{
				bool tmp;
				res->read(&tmp, sizeof(tmp));

				var = MakeSharedPtr<RenderVariableBool>();
				*var = tmp;
			}
			break;

		case REDT_uint:
			if (0 == array_size)
			{
				uint32_t tmp;
				res->read(&tmp, sizeof(tmp));
				LittleEndianToNative<sizeof(tmp)>(&tmp);

				var = MakeSharedPtr<RenderVariableUInt>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableUIntArray>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<uint32_t> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						LittleEndianToNative<sizeof(init_val[i])>(&init_val[i]);
					}
					*var = init_val;
				}
			}
			break;

		case REDT_int:
			if (0 == array_size)
			{
				int32_t tmp;
				res->read(&tmp, sizeof(tmp));
				LittleEndianToNative<sizeof(tmp)>(&tmp);

				var = MakeSharedPtr<RenderVariableInt>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableIntArray>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<int32_t> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						LittleEndianToNative<sizeof(init_val[i])>(&init_val[i]);
					}
					*var = init_val;
				}
			}
			break;

		case REDT_string:
			{
				var = MakeSharedPtr<RenderVariableString>();
				*var = ReadShortString(res);
			}
			break;

		case REDT_texture1D:
		case REDT_texture2D:
		case REDT_texture3D:
		case REDT_textureCUBE:
		case REDT_texture1DArray:
		case REDT_texture2DArray:
		case REDT_texture3DArray:
		case REDT_textureCUBEArray:
		case REDT_rw_texture1D:
		case REDT_rw_texture2D:
		case REDT_rw_texture3D:
		case REDT_rw_texture1DArray:
		case REDT_rw_texture2DArray:
			{
				var = MakeSharedPtr<RenderVariableTexture>();
				*var = TexturePtr();
				*var = ReadShortString(res);
			}
			break;

		case REDT_sampler:
			{
				SamplerStateDesc desc;
				res->read(&desc, sizeof(desc));
				LittleEndianToNative<sizeof(desc.border_clr[0])>(&desc.border_clr[0]);
				LittleEndianToNative<sizeof(desc.border_clr[1])>(&desc.border_clr[1]);
				LittleEndianToNative<sizeof(desc.border_clr[2])>(&desc.border_clr[2]);
				LittleEndianToNative<sizeof(desc.border_clr[3])>(&desc.border_clr[3]);
				LittleEndianToNative<sizeof(desc.addr_mode_u)>(&desc.addr_mode_u);
				LittleEndianToNative<sizeof(desc.addr_mode_v)>(&desc.addr_mode_v);
				LittleEndianToNative<sizeof(desc.addr_mode_w)>(&desc.addr_mode_w);
				LittleEndianToNative<sizeof(desc.filter)>(&desc.filter);
				LittleEndianToNative<sizeof(desc.min_lod)>(&desc.min_lod);
				LittleEndianToNative<sizeof(desc.max_lod)>(&desc.max_lod);
				LittleEndianToNative<sizeof(desc.mip_map_lod_bias)>(&desc.mip_map_lod_bias);
				LittleEndianToNative<sizeof(desc.cmp_func)>(&desc.cmp_func);

				var = MakeSharedPtr<RenderVariableSampler>();
				*var = Context::Instance().RenderFactoryInstance().MakeSamplerStateObject(desc);
			}
			break;

		case REDT_shader:
			{
				ShaderDesc desc;
				desc.profile = ReadShortString(res);
				desc.func_name = ReadShortString(res);

				var = MakeSharedPtr<RenderVariableShader>();
				*var = desc;
			}
			break;

		case REDT_float:
			if (0 == array_size)
			{
				float tmp;
				res->read(&tmp, sizeof(tmp));
				LittleEndianToNative<sizeof(tmp)>(&tmp);

				var = MakeSharedPtr<RenderVariableFloat>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloatArray>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<float> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						LittleEndianToNative<sizeof(init_val[i])>(&init_val[i]);
					}
					*var = init_val;
				}
			}
			break;

		case REDT_uint2:
			if (0 == array_size)
			{
				uint2 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 2; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableUInt2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt2Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<int2> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_uint3:
			if (0 == array_size)
			{
				uint3 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 3; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableUInt3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt3Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<int3> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_uint4:
			if (0 == array_size)
			{
				uint4 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 4; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableUInt4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt4Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<int4> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_int2:
			if (0 == array_size)
			{
				int2 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 2; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableInt2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt2Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<int2> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_int3:
			if (0 == array_size)
			{
				int3 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 3; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableInt3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt3Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<int3> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_int4:
			if (0 == array_size)
			{
				int4 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 4; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableInt4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt4Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<int4> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_float2:
			if (0 == array_size)
			{
				float2 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 2; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableFloat2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat2Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<float2> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_float3:
			if (0 == array_size)
			{
				float3 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 3; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableFloat3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat3Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<float3> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_float4:
			if (0 == array_size)
			{
				float4 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 4; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableFloat4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat4Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<float4> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_float4x4:
			if (0 == array_size)
			{
				float4x4 tmp;
				res->read(&tmp, sizeof(tmp));
				for (int i = 0; i < 16; ++ i)
				{
					LittleEndianToNative<sizeof(tmp[i])>(&tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableFloat4x4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat4x4Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				LittleEndianToNative<sizeof(len)>(&len);
				if (len > 0)
				{
					std::vector<float4x4> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 16; ++ j)
						{
							LittleEndianToNative<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					*var = init_val;
				}
			}
			break;

		case REDT_buffer:
		case REDT_structured_buffer:
		case REDT_rw_buffer:
		case REDT_rw_structured_buffer:
		case REDT_consume_structured_buffer:
		case REDT_append_structured_buffer:
			{
				var = MakeSharedPtr<RenderVariableBuffer>();
				*var = GraphicsBufferPtr();
				*var = ReadShortString(res);
			}
			break;

		case REDT_byte_address_buffer:
		case REDT_rw_byte_address_buffer:
			var = MakeSharedPtr<RenderVariableByteAddressBuffer>();
			*var = GraphicsBufferPtr();
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return var;
	}

	void stream_out_var(std::ostream& os, RenderVariablePtr const & var, uint32_t type, uint32_t array_size)
	{
		switch (type)
		{
		case REDT_bool:
			if (0 == array_size)
			{
				bool tmp;
				var->Value(tmp);

				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			break;

		case REDT_uint:
			if (0 == array_size)
			{
				uint32_t tmp;
				var->Value(tmp);

				NativeToLittleEndian<sizeof(tmp)>(&tmp);
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<uint32_t> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						NativeToLittleEndian<sizeof(init_val[i])>(&init_val[i]);
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_int:
			if (0 == array_size)
			{
				int32_t tmp;
				var->Value(tmp);

				NativeToLittleEndian<sizeof(tmp)>(&tmp);
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int32_t> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						NativeToLittleEndian<sizeof(init_val[i])>(&init_val[i]);
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_string:
			{
				std::string tmp;
				var->Value(tmp);
				WriteShortString(os, tmp);
			}
			break;

		case REDT_texture1D:
		case REDT_texture2D:
		case REDT_texture3D:
		case REDT_textureCUBE:
		case REDT_texture1DArray:
		case REDT_texture2DArray:
		case REDT_texture3DArray:
		case REDT_textureCUBEArray:
		case REDT_rw_texture1D:
		case REDT_rw_texture2D:
		case REDT_rw_texture3D:
		case REDT_rw_texture1DArray:
		case REDT_rw_texture2DArray:
			{
				std::string tmp;
				var->Value(tmp);
				WriteShortString(os, tmp);
			}
			break;

		case REDT_sampler:
			{
				SamplerStateObjectPtr tmp;
				var->Value(tmp);
				SamplerStateDesc desc = tmp->GetDesc();
				NativeToLittleEndian<sizeof(desc.border_clr[0])>(&desc.border_clr[0]);
				NativeToLittleEndian<sizeof(desc.border_clr[1])>(&desc.border_clr[1]);
				NativeToLittleEndian<sizeof(desc.border_clr[2])>(&desc.border_clr[2]);
				NativeToLittleEndian<sizeof(desc.border_clr[3])>(&desc.border_clr[3]);
				NativeToLittleEndian<sizeof(desc.addr_mode_u)>(&desc.addr_mode_u);
				NativeToLittleEndian<sizeof(desc.addr_mode_v)>(&desc.addr_mode_v);
				NativeToLittleEndian<sizeof(desc.addr_mode_w)>(&desc.addr_mode_w);
				NativeToLittleEndian<sizeof(desc.filter)>(&desc.filter);
				NativeToLittleEndian<sizeof(desc.min_lod)>(&desc.min_lod);
				NativeToLittleEndian<sizeof(desc.max_lod)>(&desc.max_lod);
				NativeToLittleEndian<sizeof(desc.mip_map_lod_bias)>(&desc.mip_map_lod_bias);
				NativeToLittleEndian<sizeof(desc.cmp_func)>(&desc.cmp_func);
				os.write(reinterpret_cast<char const *>(&desc), sizeof(desc));
			}
			break;

		case REDT_shader:
			{
				ShaderDesc tmp;
				var->Value(tmp);
				WriteShortString(os, tmp.profile);
				WriteShortString(os, tmp.func_name);
			}
			break;

		case REDT_float:
			if (0 == array_size)
			{
				float tmp;
				var->Value(tmp);

				NativeToLittleEndian<sizeof(tmp)>(&tmp);
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						NativeToLittleEndian<sizeof(init_val[i])>(&init_val[i]);
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_uint2:
			if (0 == array_size)
			{
				uint2 tmp;
				var->Value(tmp);

				for (int i = 0; i < 2; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int2> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_uint3:
			if (0 == array_size)
			{
				uint3 tmp;
				var->Value(tmp);

				for (int i = 0; i < 3; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int3> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_uint4:
			if (0 == array_size)
			{
				uint4 tmp;
				var->Value(tmp);

				for (int i = 0; i < 3; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int4> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_int2:
			if (0 == array_size)
			{
				int2 tmp;
				var->Value(tmp);

				for (int i = 0; i < 2; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int2> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_int3:
			if (0 == array_size)
			{
				int3 tmp;
				var->Value(tmp);

				for (int i = 0; i < 3; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int3> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_int4:
			if (0 == array_size)
			{
				int4 tmp;
				var->Value(tmp);

				for (int i = 0; i < 4; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int4> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_float2:
			if (0 == array_size)
			{
				float2 tmp;
				var->Value(tmp);

				for (int i = 0; i < 2; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float2> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_float3:
			if (0 == array_size)
			{
				float3 tmp;
				var->Value(tmp);

				for (int i = 0; i < 3; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float3> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char*>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_float4:
			if (0 == array_size)
			{
				float4 tmp;
				var->Value(tmp);

				for (int i = 0; i < 4; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float4> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_float4x4:
			if (0 == array_size)
			{
				float4x4 tmp;
				var->Value(tmp);

				for (int i = 0; i < 16; ++ i)
				{
					NativeToLittleEndian<sizeof(tmp[i])>(&tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float4x4> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 16; ++ j)
						{
							NativeToLittleEndian<sizeof(init_val[i][j])>(&init_val[i][j]);
						}
					}
					os.write(reinterpret_cast<char const *>(&init_val[0]), len * sizeof(init_val[0]));
				}
			}
			break;

		case REDT_buffer:
		case REDT_structured_buffer:
		case REDT_rw_buffer:
		case REDT_rw_structured_buffer:
		case REDT_consume_structured_buffer:
		case REDT_append_structured_buffer:
			{
				std::string tmp;
				var->Value(tmp);
				WriteShortString(os, tmp);
			}
			break;

		case REDT_byte_address_buffer:
		case REDT_rw_byte_address_buffer:
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}
}

namespace KlayGE
{
	void RenderEffectAnnotation::Load(XMLNodePtr const & node)
	{
		type_ = type_define::instance().type_code(node->Attrib("type")->ValueString());
		name_ = node->Attrib("name")->ValueString();
		var_ = read_var(node, type_, 0);
	}

	void RenderEffectAnnotation::StreamOut(std::ostream& os)
	{
		uint32_t t = type_;
		NativeToLittleEndian<sizeof(t)>(&t);
		os.write(reinterpret_cast<char const *>(&t), sizeof(t));
		WriteShortString(os, name_);
		stream_out_var(os, var_, type_, 0);
	}


	RenderEffect::RenderEffect()
	{
	}

	void RenderEffect::Load(std::string const & name)
	{
		std::string fxml_name = ResLoader::Instance().Locate(name);
		if (fxml_name.empty())
		{
			fxml_name = name;
		}
		std::string kfx_name = fxml_name.substr(0, fxml_name.rfind(".")) + ".kfx";

		ResIdentifierPtr source = ResLoader::Instance().Open(fxml_name);
		ResIdentifierPtr kfx_source = ResLoader::Instance().Open(kfx_name);

		res_name_ = MakeSharedPtr<std::string>(fxml_name);
		if (source)
		{
			timestamp_ = source->Timestamp();

			XMLDocument doc;
			XMLNodePtr root = doc.Parse(source);

			XMLAttributePtr attr;

			std::vector<XMLDocumentPtr> include_docs;
			for (XMLNodePtr node = root->FirstNode("include"); node; node = node->NextSibling("include"))
			{
				attr = node->Attrib("name");
				ResIdentifierPtr include_source = ResLoader::Instance().Open(attr->ValueString());
				if (include_source)
				{
					timestamp_ = std::max(timestamp_, include_source->Timestamp());
				}
			}
		}
		else
		{
			timestamp_ = 0;
		}

		shader_descs_.reset();
		cbuffers_.reset();
		macros_.reset();
		params_.clear();
		shaders_.reset();
		techniques_.clear();

		if (source)
		{
			shader_descs_ = MakeSharedPtr<KLAYGE_DECLTYPE(*shader_descs_)>(1);

			XMLDocument doc;
			XMLNodePtr root = doc.Parse(source);

			cbuffers_ = MakeSharedPtr<KLAYGE_DECLTYPE(*cbuffers_)>();

			XMLAttributePtr attr;

			std::vector<XMLDocumentPtr> include_docs;
			for (XMLNodePtr node = root->FirstNode("include"); node;)
			{
				attr = node->Attrib("name");
				include_docs.push_back(MakeSharedPtr<XMLDocument>());
				XMLNodePtr include_root = include_docs.back()->Parse(ResLoader::Instance().Open(attr->ValueString()));

				for (XMLNodePtr child_node = include_root->FirstNode(); child_node; child_node = child_node->NextSibling())
				{
					if (XNT_Element == child_node->Type())
					{
						root->InsertNode(node, doc.CloneNode(child_node));
					}
				}

				XMLNodePtr node_next = node->NextSibling("include");
				root->RemoveNode(node);
				node = node_next;
			}

			{
				XMLNodePtr macro_node = root->FirstNode("macro");
				if (macro_node)
				{
					macros_ = MakeSharedPtr<KLAYGE_DECLTYPE(*macros_)>();
				}
				for (; macro_node; macro_node = macro_node->NextSibling("macro"))
				{
					macros_->push_back(std::make_pair(std::make_pair(macro_node->Attrib("name")->ValueString(), macro_node->Attrib("value")->ValueString()), true));
				}
			}

			std::vector<XMLNodePtr> parameter_nodes;
			for (XMLNodePtr node = root->FirstNode(); node; node = node->NextSibling())
			{
				if ("parameter" == node->Name())
				{
					parameter_nodes.push_back(node);
				}
				else if ("cbuffer" == node->Name())
				{
					for (XMLNodePtr sub_node = node->FirstNode("parameter"); sub_node; sub_node = sub_node->NextSibling("parameter"))
					{
						parameter_nodes.push_back(sub_node);
					}
				}
			}

			for (uint32_t param_index = 0; param_index < parameter_nodes.size(); ++ param_index)
			{
				XMLNodePtr const & node = parameter_nodes[param_index];

				uint32_t type = type_define::instance().type_code(node->Attrib("type")->ValueString());
				if ((type != REDT_sampler)
					&& (type != REDT_texture1D) && (type != REDT_texture2D) && (type != REDT_texture3D) && (type != REDT_textureCUBE)
					&& (type != REDT_texture1DArray) && (type != REDT_texture2DArray) && (type != REDT_texture3DArray) && (type != REDT_textureCUBEArray))
				{
					XMLNodePtr parent_node = node->Parent();
					std::string cbuff_name = parent_node->AttribString("name", "global_cb");

					bool found = false;
					for (size_t i = 0; i < cbuffers_->size(); ++ i)
					{
						if ((*cbuffers_)[i].first == cbuff_name)
						{
							(*cbuffers_)[i].second.push_back(param_index);
							found = true;
							break;
						}
					}
					if (!found)
					{
						cbuffers_->push_back(std::make_pair(cbuff_name, std::vector<uint32_t>(1, param_index)));
					}
				}

				RenderEffectParameterPtr param = MakeSharedPtr<RenderEffectParameter>(*this);
				params_.push_back(param);

				param->Load(node);
			}

			{
				XMLNodePtr shader_node = root->FirstNode("shader");
				if (shader_node)
				{
					shaders_ = MakeSharedPtr<KLAYGE_DECLTYPE(*shaders_)>();
					for (; shader_node; shader_node = shader_node->NextSibling("shader"))
					{
						shaders_->push_back(RenderShaderFunc());
						shaders_->back().Load(shader_node);
					}
				}
			}

			uint32_t index = 0;
			for (XMLNodePtr node = root->FirstNode("technique"); node; node = node->NextSibling("technique"), ++ index)
			{
				RenderTechniquePtr technique = MakeSharedPtr<RenderTechnique>(*this);
				techniques_.push_back(technique);

				technique->Load(node, index);
			}
		}

		std::ofstream ofs(kfx_name.c_str(), std::ios_base::binary | std::ios_base::out);
		this->StreamOut(ofs);
	}

	void RenderEffect::StreamOut(std::ostream& os)
	{
		uint32_t fourcc = MakeFourCC<'K', 'F', 'X', ' '>::value;
		NativeToLittleEndian<sizeof(fourcc)>(&fourcc);
		os.write(reinterpret_cast<char const *>(&fourcc), sizeof(fourcc));

		uint32_t ver = KFX_VERSION;
		NativeToLittleEndian<sizeof(ver)>(&ver);
		os.write(reinterpret_cast<char const *>(&ver), sizeof(ver));

		uint64_t timestamp = timestamp_;
		NativeToLittleEndian<sizeof(timestamp)>(&timestamp);
		os.write(reinterpret_cast<char const *>(&timestamp), sizeof(timestamp));

		{
			uint16_t num_macros = 0;
			if (macros_)
			{
				for (uint32_t i = 0; i < macros_->size(); ++ i)
				{
					if ((*macros_)[i].second)
					{
						++ num_macros;
					}
				}
			}

			NativeToLittleEndian<sizeof(num_macros)>(&num_macros);
			os.write(reinterpret_cast<char const *>(&num_macros), sizeof(num_macros));

			if (macros_)
			{
				for (uint32_t i = 0; i < macros_->size(); ++ i)
				{
					if ((*macros_)[i].second)
					{
						WriteShortString(os, (*macros_)[i].first.first);
						WriteShortString(os, (*macros_)[i].first.second);
					}
				}
			}
		}

		{
			uint16_t num_cbufs = static_cast<uint16_t>(cbuffers_->size());
			NativeToLittleEndian<sizeof(num_cbufs)>(&num_cbufs);
			os.write(reinterpret_cast<char const *>(&num_cbufs), sizeof(num_cbufs));
			for (uint32_t i = 0; i < cbuffers_->size(); ++ i)
			{
				WriteShortString(os, (*cbuffers_)[i].first);

				uint16_t len = static_cast<uint16_t>((*cbuffers_)[i].second.size());
				NativeToLittleEndian<sizeof(len)>(&len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				for (size_t j = 0; j < (*cbuffers_)[i].second.size(); ++ j)
				{
					uint32_t tmp = (*cbuffers_)[i].second[j];
					NativeToLittleEndian<sizeof(tmp)>(&tmp);
					os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
				}
			}
		}

		{
			uint16_t num_params = static_cast<uint16_t>(params_.size());
			NativeToLittleEndian<sizeof(num_params)>(&num_params);
			os.write(reinterpret_cast<char const *>(&num_params), sizeof(num_params));
			for (uint32_t i = 0; i < params_.size(); ++ i)
			{
				params_[i]->StreamOut(os);
			}
		}

		{
			uint16_t num_shaders = shaders_ ? static_cast<uint16_t>(shaders_->size()) : 0;
			NativeToLittleEndian<sizeof(num_shaders)>(&num_shaders);
			os.write(reinterpret_cast<char const *>(&num_shaders), sizeof(num_shaders));
			if (shaders_)
			{
				for (uint32_t i = 0; i < shaders_->size(); ++ i)
				{
					(*shaders_)[i].StreamOut(os);
				}
			}
		}

		{
			uint16_t num_shader_descs = static_cast<uint16_t>(shader_descs_->size() - 1);
			NativeToLittleEndian<sizeof(num_shader_descs)>(&num_shader_descs);
			os.write(reinterpret_cast<char const *>(&num_shader_descs), sizeof(num_shader_descs));
			for (uint32_t i = 0; i < shader_descs_->size() - 1; ++ i)
			{
				WriteShortString(os, (*shader_descs_)[i + 1].profile);
				WriteShortString(os, (*shader_descs_)[i + 1].func_name);

				uint64_t tmp64 = (*shader_descs_)[i + 1].macros_hash;
				NativeToLittleEndian<sizeof(tmp64)>(&tmp64);
				os.write(reinterpret_cast<char const *>(&tmp64), sizeof(tmp64));

				uint32_t tmp32 = (*shader_descs_)[i + 1].tech_pass_type;
				NativeToLittleEndian<sizeof(tmp32)>(&tmp32);
				os.write(reinterpret_cast<char const *>(&tmp32), sizeof(tmp32));

				uint8_t len = static_cast<uint8_t>((*shader_descs_)[i + 1].so_decl.size());
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				for (uint32_t j = 0; j < len; ++ j)
				{
					ShaderDesc::StreamOutputDecl so_decl = (*shader_descs_)[i + 1].so_decl[j];
					NativeToLittleEndian<sizeof(so_decl.usage)>(&so_decl.usage);
					os.write(reinterpret_cast<char const *>(&so_decl), sizeof(so_decl));
				}
			}
		}

		{
			uint16_t num_techs = static_cast<uint16_t>(techniques_.size());
			NativeToLittleEndian<sizeof(num_techs)>(&num_techs);
			os.write(reinterpret_cast<char const *>(&num_techs), sizeof(num_techs));
			for (uint32_t i = 0; i < techniques_.size(); ++ i)
			{
				techniques_[i]->StreamOut(os, i);
			}
		}
	}

	RenderEffectPtr RenderEffect::Clone()
	{
		RenderEffectPtr ret = MakeSharedPtr<RenderEffect>();

		ret->res_name_ = res_name_;
		ret->timestamp_ = timestamp_;

		ret->prototype_effect_ = prototype_effect_;
		ret->macros_ = macros_;
		ret->shaders_ = shaders_;
		ret->shader_descs_ = shader_descs_;

		ret->params_.resize(params_.size());
		for (size_t i = 0; i < params_.size(); ++ i)
		{
			ret->params_[i] = params_[i]->Clone(*ret);
		}

		ret->cbuffers_ = cbuffers_;

		ret->techniques_.resize(techniques_.size());
		for (size_t i = 0; i < techniques_.size(); ++ i)
		{
			ret->techniques_[i] = techniques_[i]->Clone(*ret);
		}

		return ret;
	}

	RenderEffectParameterPtr RenderEffect::ParameterByName(std::string const & name) const
	{
		typedef KLAYGE_DECLTYPE(params_) ParamsType;
		KLAYGE_FOREACH(ParamsType::const_reference param, params_)
		{
			if (name == *param->Name())
			{
				return param;
			}
		}
		return RenderEffectParameterPtr();
	}

	RenderEffectParameterPtr RenderEffect::ParameterBySemantic(std::string const & semantic) const
	{
		typedef KLAYGE_DECLTYPE(params_) ParamsType;
		KLAYGE_FOREACH(ParamsType::const_reference param, params_)
		{
			if (semantic == *param->Semantic())
			{
				return param;
			}
		}
		return RenderEffectParameterPtr();
	}

	RenderTechniquePtr const & RenderEffect::TechniqueByName(std::string const & name) const
	{
		typedef KLAYGE_DECLTYPE(techniques_) TechsType;
		KLAYGE_FOREACH(TechsType::const_reference tech, techniques_)
		{
			if (name == tech->Name())
			{
				return tech;
			}
		}
		static RenderTechniquePtr null_tech;
		return null_tech;
	}

	uint32_t RenderEffect::AddShaderDesc(ShaderDesc const & sd)
	{
		for (uint32_t i = 0; i < shader_descs_->size(); ++ i)
		{
			if ((*shader_descs_)[i] == sd)
			{
				return i;
			}
		}

		uint32_t id = static_cast<uint32_t>(shader_descs_->size());
		shader_descs_->push_back(sd);
		return id;
	}

	ShaderDesc& RenderEffect::GetShaderDesc(uint32_t id)
	{
		BOOST_ASSERT(id < shader_descs_->size());
		return (*shader_descs_)[id];
	}

	ShaderDesc const & RenderEffect::GetShaderDesc(uint32_t id) const
	{
		BOOST_ASSERT(id < shader_descs_->size());
		return (*shader_descs_)[id];
	}

	std::string const & RenderEffect::TypeName(uint32_t code) const
	{
		return type_define::instance().type_name(code);
	}


	void RenderTechnique::Load(XMLNodePtr const & node, uint32_t tech_index)
	{
		name_ = MakeSharedPtr<KLAYGE_DECLTYPE(*name_)>(node->Attrib("name")->ValueString());

		RenderTechniquePtr parent_tech;
		XMLAttributePtr inherit_attr = node->Attrib("inherit");
		if (inherit_attr)
		{
			std::string inherit = inherit_attr->ValueString();
			BOOST_ASSERT(inherit != *name_);

			parent_tech = effect_.TechniqueByName(inherit);
			BOOST_ASSERT(parent_tech);
			annotations_ = parent_tech->annotations_;
		}

		{
			XMLNodePtr anno_node = node->FirstNode("annotation");
			if (anno_node)
			{
				if (!annotations_)
				{
					annotations_ = MakeSharedPtr<KLAYGE_DECLTYPE(*annotations_)>();
				}
				for (; anno_node; anno_node = anno_node->NextSibling("annotation"))
				{
					RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
					annotations_->push_back(annotation);

					annotation->Load(anno_node);
				}
			}
		}

		{
			XMLNodePtr macro_node = node->FirstNode("macro");
			if (macro_node)
			{
				if (!macros_)
				{
					macros_ = MakeSharedPtr<KLAYGE_DECLTYPE(*macros_)>();
				}				
				if (parent_tech && parent_tech->macros_)
				{
					*macros_ = *parent_tech->macros_;
				}
				for (; macro_node; macro_node = macro_node->NextSibling("macro"))
				{
					macros_->push_back(std::make_pair(macro_node->Attrib("name")->ValueString(), macro_node->Attrib("value")->ValueString()));
				}
			}
			else if (parent_tech)
			{
				macros_ = parent_tech->macros_;
			}
		}

		if (!node->FirstNode("pass") && parent_tech)
		{
			is_validate_ = parent_tech->is_validate_;
			has_discard_ = parent_tech->has_discard_;
			has_tessellation_ = parent_tech->has_tessellation_;
			transparent_ = parent_tech->transparent_;
			weight_ = parent_tech->weight_;

			if (macros_ == parent_tech->macros_)
			{
				passes_ = parent_tech->passes_;
			}
			else
			{
				for (uint32_t index = 0; index < parent_tech->passes_.size(); ++ index)
				{
					RenderPassPtr pass = MakeSharedPtr<RenderPass>(effect_);
					passes_.push_back(pass);

					RenderPassPtr inherit_pass = parent_tech->passes_[index];

					pass->Load(tech_index, index, inherit_pass);
					is_validate_ &= pass->Validate();
				}
			}
		}
		else
		{
			is_validate_ = true;

			has_discard_ = false;
			has_tessellation_ = false;
			transparent_ = false;
			if (parent_tech)
			{
				weight_ = parent_tech->Weight();
			}
			else
			{
				weight_ = 1;
			}
		
			uint32_t index = 0;
			for (XMLNodePtr pass_node = node->FirstNode("pass"); pass_node; pass_node = pass_node->NextSibling("pass"), ++ index)
			{
				RenderPassPtr pass = MakeSharedPtr<RenderPass>(effect_);
				passes_.push_back(pass);

				RenderPassPtr inherit_pass;
				if (parent_tech && (index < parent_tech->passes_.size()))
				{
					inherit_pass = parent_tech->passes_[index];
				}

				pass->Load(pass_node, tech_index, index, inherit_pass);

				is_validate_ &= pass->Validate();

				for (XMLNodePtr state_node = pass_node->FirstNode("state"); state_node; state_node = state_node->NextSibling("state"))
				{
					++ weight_;

					std::string state_name = state_node->Attrib("name")->ValueString();
					if ("blend_enable" == state_name)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						if (bool_from_str(value_str))
						{
							transparent_ = true;
						}
					}
				}

				has_discard_ |= pass->GetShaderObject()->HasDiscard();
				has_tessellation_ |= pass->GetShaderObject()->HasTessellation();
			}
			if (transparent_)
			{
				weight_ += 10000;
			}
		}
	}

	void RenderTechnique::StreamOut(std::ostream& os, uint32_t tech_index)
	{
		WriteShortString(os, *name_);

		uint8_t num_anno;
		if (annotations_)
		{
			num_anno = static_cast<uint8_t>(annotations_->size());
		}
		else
		{
			num_anno = 0;
		}
		os.write(reinterpret_cast<char const *>(&num_anno), sizeof(num_anno));
		for (uint32_t i = 0; i < num_anno; ++ i)
		{
			RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
			(*annotations_)[i] = annotation;
				
			annotation->StreamOut(os);
		}

		uint8_t num_macro;
		if (macros_)
		{
			num_macro = static_cast<uint8_t>(macros_->size());
		}
		else
		{
			num_macro = 0;
		}
		os.write(reinterpret_cast<char const *>(&num_macro), sizeof(num_macro));
		for (uint32_t i = 0; i < num_macro; ++ i)
		{
			WriteShortString(os, (*macros_)[i].first);
			WriteShortString(os, (*macros_)[i].second);
		}

		os.write(reinterpret_cast<char const *>(&transparent_), sizeof(transparent_));
		float w = weight_;
		NativeToLittleEndian<sizeof(w)>(&w);
		os.write(reinterpret_cast<char const *>(&w), sizeof(w));

		uint8_t num_passes = static_cast<uint8_t>(passes_.size());
		os.write(reinterpret_cast<char const *>(&num_passes), sizeof(num_passes));
		for (uint32_t pass_index = 0; pass_index < num_passes; ++ pass_index)
		{
			passes_[pass_index]->StreamOut(os, tech_index, pass_index);
		}
	}

	RenderTechniquePtr RenderTechnique::Clone(RenderEffect& effect)
	{
		RenderTechniquePtr ret = MakeSharedPtr<RenderTechnique>(effect);

		ret->name_ = name_;

		ret->annotations_ = annotations_;
		ret->macros_ = macros_;
		ret->weight_ = weight_;
		ret->transparent_ = transparent_;
		ret->is_validate_ = is_validate_;
		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;

		ret->passes_.resize(passes_.size());
		for (size_t i = 0; i < passes_.size(); ++ i)
		{
			ret->passes_[i] = passes_[i]->Clone(effect);
		}

		return ret;
	}


	void RenderPass::Load(XMLNodePtr const & node, uint32_t tech_index, uint32_t pass_index, RenderPassPtr const & inherit_pass)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		name_ = MakeSharedPtr<KLAYGE_DECLTYPE(*name_)>(node->Attrib("name")->ValueString());

		if (inherit_pass)
		{
			annotations_ = inherit_pass->annotations_;
		}

		{
			XMLNodePtr anno_node = node->FirstNode("annotation");
			if (anno_node)
			{
				annotations_ = MakeSharedPtr<KLAYGE_DECLTYPE(*annotations_)>();
				for (; anno_node; anno_node = anno_node->NextSibling("annotation"))
				{
					RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
					annotations_->push_back(annotation);

					annotation->Load(anno_node);
				}
			}
		}

		{
			XMLNodePtr macro_node = node->FirstNode("macro");
			if (macro_node)
			{
				if (!macros_)
				{
					macros_ = MakeSharedPtr<KLAYGE_DECLTYPE(*macros_)>();
				}
				if (inherit_pass && inherit_pass->macros_)
				{
					*macros_ = *inherit_pass->macros_;
				}
				for (; macro_node; macro_node = macro_node->NextSibling("macro"))
				{
					macros_->push_back(std::make_pair(macro_node->Attrib("name")->ValueString(), macro_node->Attrib("value")->ValueString()));
				}
			}
			else if (inherit_pass)
			{
				macros_ = inherit_pass->macros_;
			}
		}

		uint64_t macros_hash;
		{
			RenderTechniquePtr const & tech = effect_.TechniqueByIndex(tech_index);

			size_t hash_val = 0;
			for (uint32_t i = 0; i < tech->NumMacros(); ++ i)
			{
				std::pair<std::string, std::string> const & name_value = tech->MacroByIndex(i);
				boost::hash_range(hash_val, name_value.first.begin(), name_value.first.end());
				boost::hash_range(hash_val, name_value.second.begin(), name_value.second.end());
			}
			for (uint32_t i = 0; i < this->NumMacros(); ++ i)
			{
				std::pair<std::string, std::string> const & name_value = this->MacroByIndex(i);
				boost::hash_range(hash_val, name_value.first.begin(), name_value.first.end());
				boost::hash_range(hash_val, name_value.second.begin(), name_value.second.end());
			}
			macros_hash = static_cast<uint64_t>(hash_val);
		}

		RasterizerStateDesc rs_desc;
		DepthStencilStateDesc dss_desc;
		BlendStateDesc bs_desc;
		shader_obj_ = rf.MakeShaderObject();

		shader_desc_ids_ = MakeSharedPtr<KLAYGE_DECLTYPE(*shader_desc_ids_)>();
		shader_desc_ids_->resize(ShaderObject::ST_NumShaderTypes, 0);

		if (inherit_pass)
		{
			rs_desc = inherit_pass->rasterizer_state_obj_->GetDesc();
			dss_desc = inherit_pass->depth_stencil_state_obj_->GetDesc();
			bs_desc = inherit_pass->blend_state_obj_->GetDesc();
			front_stencil_ref_ = inherit_pass->front_stencil_ref_;
			back_stencil_ref_ = inherit_pass->back_stencil_ref_;
			blend_factor_ = inherit_pass->blend_factor_;
			sample_mask_ = inherit_pass->sample_mask_;

			for (size_t i = 0; i < shader_desc_ids_->size(); ++ i)
			{
				(*shader_desc_ids_)[i] = (*inherit_pass->shader_desc_ids_)[i];
			}
		}

		for (XMLNodePtr state_node = node->FirstNode("state"); state_node; state_node = state_node->NextSibling("state"))
		{
			std::string state_name = state_node->Attrib("name")->ValueString();

			if ("polygon_mode" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.polygon_mode = polygon_mode_define::instance().from_str(value_str);
			}
			else if ("shade_mode" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.shade_mode = shade_mode_define::instance().from_str(value_str);
			}
			else if ("cull_mode" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.cull_mode = cull_mode_define::instance().from_str(value_str);
			}
			else if ("front_face_ccw" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.front_face_ccw = bool_from_str(value_str);
			}
			else if ("polygon_offset_factor" == state_name)
			{
				rs_desc.polygon_offset_factor = state_node->Attrib("value")->ValueFloat();
			}
			else if ("polygon_offset_units" == state_name)
			{
				rs_desc.polygon_offset_units = state_node->Attrib("value")->ValueFloat();
			}
			else if ("depth_clip_enable" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.depth_clip_enable = bool_from_str(value_str);
			}
			else if ("scissor_enable" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.scissor_enable = bool_from_str(value_str);
			}
			else if ("multisample_enable" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.multisample_enable = bool_from_str(value_str);
			}
			else if ("alpha_to_coverage_enable" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.alpha_to_coverage_enable = bool_from_str(value_str);
			}
			else if ("independent_blend_enable" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.independent_blend_enable = bool_from_str(value_str);
			}
			else if ("blend_enable" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.blend_enable[index] = bool_from_str(value_str);
			}
			else if ("logic_op_enable" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.logic_op_enable[index] = bool_from_str(value_str);
			}
			else if ("blend_op" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.blend_op[index] = blend_operation_define::instance().from_str(value_str);
			}
			else if ("src_blend" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.src_blend[index] = alpha_blend_factor_define::instance().from_str(value_str);
			}
			else if ("dest_blend" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.dest_blend[index] = alpha_blend_factor_define::instance().from_str(value_str);
			}
			else if ("blend_op_alpha" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.blend_op_alpha[index] = blend_operation_define::instance().from_str(value_str);
			}
			else if ("src_blend_alpha" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.src_blend_alpha[index] = alpha_blend_factor_define::instance().from_str(value_str);
			}
			else if ("dest_blend_alpha" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.dest_blend_alpha[index] = alpha_blend_factor_define::instance().from_str(value_str);
			}
			else if ("logic_op" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.logic_op[index] = logic_operation_define::instance().from_str(value_str);
			}
			else if ("color_write_mask" == state_name)
			{
				int index = get_index(state_node);
				bs_desc.color_write_mask[index] = static_cast<uint8_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if ("blend_factor" == state_name)
			{
				XMLAttributePtr attr = state_node->Attrib("r");
				if (attr)
				{
					blend_factor_.r() = attr->ValueFloat();
				}
				attr = state_node->Attrib("g");
				if (attr)
				{
					blend_factor_.g() = attr->ValueFloat();
				}
				attr = state_node->Attrib("b");
				if (attr)
				{
					blend_factor_.b() = attr->ValueFloat();
				}
				attr = state_node->Attrib("a");
				if (attr)
				{
					blend_factor_.a() = attr->ValueFloat();
				}
			}
			else if ("sample_mask" == state_name)
			{
				sample_mask_ = state_node->Attrib("value")->ValueUInt();
			}
			else if ("depth_enable" == state_name)
			{
				dss_desc.depth_enable = bool_from_str(state_node->Attrib("value")->ValueString());
			}
			else if ("depth_write_mask" == state_name)
			{
				dss_desc.depth_write_mask = bool_from_str(state_node->Attrib("value")->ValueString());
			}
			else if ("depth_func" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.depth_func = compare_function_define::instance().from_str(value_str);
			}
			else if ("front_stencil_enable" == state_name)
			{
				dss_desc.front_stencil_enable = bool_from_str(state_node->Attrib("value")->ValueString());
			}
			else if ("front_stencil_func" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.front_stencil_func = compare_function_define::instance().from_str(value_str);
			}
			else if ("front_stencil_ref" == state_name)
			{
				front_stencil_ref_ = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if ("front_stencil_read_mask" == state_name)
			{
				dss_desc.front_stencil_read_mask = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if ("front_stencil_write_mask" == state_name)
			{
				dss_desc.front_stencil_write_mask = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if ("front_stencil_fail" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.front_stencil_fail = stencil_operation_define::instance().from_str(value_str);
			}
			else if ("front_stencil_depth_fail" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.front_stencil_depth_fail = stencil_operation_define::instance().from_str(value_str);
			}
			else if ("front_stencil_pass" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.front_stencil_pass = stencil_operation_define::instance().from_str(value_str);
			}
			else if ("back_stencil_enable" == state_name)
			{
				dss_desc.back_stencil_enable = bool_from_str(state_node->Attrib("value")->ValueString());
			}
			else if ("back_stencil_func" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.back_stencil_func = compare_function_define::instance().from_str(value_str);
			}
			else if ("back_stencil_ref" == state_name)
			{
				back_stencil_ref_ = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if ("back_stencil_read_mask" == state_name)
			{
				dss_desc.back_stencil_read_mask = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if ("back_stencil_write_mask" == state_name)
			{
				dss_desc.back_stencil_write_mask = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if ("back_stencil_fail" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.back_stencil_fail = stencil_operation_define::instance().from_str(value_str);
			}
			else if ("back_stencil_depth_fail" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.back_stencil_depth_fail = stencil_operation_define::instance().from_str(value_str);
			}
			else if ("back_stencil_pass" == state_name)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.back_stencil_pass = stencil_operation_define::instance().from_str(value_str);
			}
			else if (("vertex_shader" == state_name) || ("pixel_shader" == state_name) || ("geometry_shader" == state_name)
				|| ("compute_shader" == state_name) || ("hull_shader" == state_name) || ("domain_shader" == state_name))
			{
				ShaderObject::ShaderType type;
				if ("vertex_shader" == state_name)
				{
					type = ShaderObject::ST_VertexShader;
				}
				else if ("pixel_shader" == state_name)
				{
					type = ShaderObject::ST_PixelShader;
				}
				else if ("geometry_shader" == state_name)
				{
					type = ShaderObject::ST_GeometryShader;
				}
				else if ("compute_shader" == state_name)
				{
					type = ShaderObject::ST_ComputeShader;
				}
				else if ("hull_shader" == state_name)
				{
					type = ShaderObject::ST_HullShader;
				}
				else
				{
					BOOST_ASSERT("domain_shader" == state_name);
					type = ShaderObject::ST_DomainShader;
				}

				ShaderDesc sd;
				sd.profile = get_profile(state_node);
				sd.func_name = get_func_name(state_node);
				sd.macros_hash = macros_hash;

				if ((ShaderObject::ST_VertexShader == type) || (ShaderObject::ST_GeometryShader == type))
				{
					XMLNodePtr so_node = state_node->FirstNode("stream_output");
					if (so_node)
					{
						for (XMLNodePtr slot_node = so_node->FirstNode("slot"); slot_node; slot_node = slot_node->NextSibling("slot"))
						{
							ShaderDesc::StreamOutputDecl decl;

							std::string usage_str = slot_node->Attrib("usage")->ValueString();
							XMLAttributePtr attr = slot_node->Attrib("usage_index");
							if (attr)
							{
								decl.usage_index = static_cast<uint8_t>(attr->ValueInt());
							}
							else
							{
								decl.usage_index = 0;
							}

							if (("POSITION" == usage_str) || ("SV_Position" == usage_str))
							{
								decl.usage = VEU_Position;
							}
							else if ("NORMAL" == usage_str)
							{
								decl.usage = VEU_Normal;
							}
							else if ("COLOR" == usage_str)
							{
								if (0 == decl.usage_index)
								{
									decl.usage = VEU_Diffuse;
								}
								else
								{
									decl.usage = VEU_Specular;
								}
							}
							else if ("BLENDWEIGHT" == usage_str)
							{
								decl.usage = VEU_BlendWeight;
							}
							else if ("BLENDINDICES" == usage_str)
							{
								decl.usage = VEU_BlendIndex;
							}
							else if ("TEXCOORD" == usage_str)
							{
								decl.usage = VEU_TextureCoord;
							}
							else if ("TANGENT" == usage_str)
							{
								decl.usage = VEU_Tangent;
							}
							else if ("BINORMAL" == usage_str)
							{
								decl.usage = VEU_Binormal;
							}

							attr = slot_node->Attrib("component");
							std::string component_str;
							if (attr)
							{
								component_str = slot_node->Attrib("component")->ValueString();
							}
							else
							{
								component_str = "xyzw";
							}
							decl.start_component = static_cast<uint8_t>(component_str[0] - 'x');
							decl.component_count = static_cast<uint8_t>(std::min(static_cast<size_t>(4), component_str.size()));

							sd.so_decl.push_back(decl);
						}
					}
				}

				(*shader_desc_ids_)[type] = effect_.AddShaderDesc(sd);
			}
			else
			{
				LogError("Wrong state name: %s", state_name.c_str());
			}
		}

		rasterizer_state_obj_ = rf.MakeRasterizerStateObject(rs_desc);
		depth_stencil_state_obj_ = rf.MakeDepthStencilStateObject(dss_desc);
		blend_state_obj_ = rf.MakeBlendStateObject(bs_desc);

		for (int type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			ShaderDesc& sd = effect_.GetShaderDesc((*shader_desc_ids_)[type]);
			if (!sd.func_name.empty())
			{
				if (sd.tech_pass_type != 0xFFFFFFFF)
				{
					RenderTechniquePtr const & tech = effect_.TechniqueByIndex(sd.tech_pass_type >> 16);
					RenderPassPtr const & pass = tech->Pass((sd.tech_pass_type >> 8) & 0xFF);
					shader_obj_->AttachShader(static_cast<ShaderObject::ShaderType>(type),
						effect_, *tech, *pass, pass->GetShaderObject());
				}
				else
				{
					RenderTechniquePtr const & tech = effect_.TechniqueByIndex(tech_index);
					shader_obj_->AttachShader(static_cast<ShaderObject::ShaderType>(type),
						effect_, *tech, *this, *shader_desc_ids_);
					sd.tech_pass_type = (tech_index << 16) + (pass_index << 8) + type;
				}
			}
		}

		shader_obj_->LinkShaders(effect_);

		is_validate_ = shader_obj_->Validate();
	}

	void RenderPass::Load(uint32_t tech_index, uint32_t pass_index, RenderPassPtr const & inherit_pass)
	{
		BOOST_ASSERT(inherit_pass);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		name_ = inherit_pass->name_;
		annotations_ = inherit_pass->annotations_;
		macros_ = inherit_pass->macros_;

		uint64_t macros_hash;
		{
			RenderTechniquePtr const & tech = effect_.TechniqueByIndex(tech_index);

			size_t hash_val = 0;
			for (uint32_t i = 0; i < tech->NumMacros(); ++ i)
			{
				std::pair<std::string, std::string> const & name_value = tech->MacroByIndex(i);
				boost::hash_range(hash_val, name_value.first.begin(), name_value.first.end());
				boost::hash_range(hash_val, name_value.second.begin(), name_value.second.end());
			}
			for (uint32_t i = 0; i < this->NumMacros(); ++ i)
			{
				std::pair<std::string, std::string> const & name_value = this->MacroByIndex(i);
				boost::hash_range(hash_val, name_value.first.begin(), name_value.first.end());
				boost::hash_range(hash_val, name_value.second.begin(), name_value.second.end());
			}
			macros_hash = static_cast<uint64_t>(hash_val);
		}

		shader_obj_ = rf.MakeShaderObject();

		shader_desc_ids_ = MakeSharedPtr<KLAYGE_DECLTYPE(*shader_desc_ids_)>();
		shader_desc_ids_->resize(ShaderObject::ST_NumShaderTypes, 0);

		rasterizer_state_obj_ = inherit_pass->rasterizer_state_obj_;
		depth_stencil_state_obj_ = inherit_pass->depth_stencil_state_obj_;
		blend_state_obj_ = inherit_pass->blend_state_obj_;
		front_stencil_ref_ = inherit_pass->front_stencil_ref_;
		back_stencil_ref_ = inherit_pass->back_stencil_ref_;
		blend_factor_ = inherit_pass->blend_factor_;
		sample_mask_ = inherit_pass->sample_mask_;

		for (int type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			ShaderDesc sd = effect_.GetShaderDesc((*inherit_pass->shader_desc_ids_)[type]);
			if (!sd.func_name.empty())
			{
				sd.macros_hash = macros_hash;
				sd.tech_pass_type = (tech_index << 16) + (pass_index << 8) + type;
				(*shader_desc_ids_)[type] = effect_.AddShaderDesc(sd);
				
				RenderTechniquePtr const & tech = effect_.TechniqueByIndex(tech_index);
				shader_obj_->AttachShader(static_cast<ShaderObject::ShaderType>(type),
					effect_, *tech, *this, *shader_desc_ids_);
			}
		}

		shader_obj_->LinkShaders(effect_);

		is_validate_ = shader_obj_->Validate();
	}

	void RenderPass::StreamOut(std::ostream& os, uint32_t tech_index, uint32_t pass_index)
	{
		WriteShortString(os, *name_);

		uint8_t num_anno;
		if (annotations_)
		{
			num_anno = static_cast<uint8_t>(annotations_->size());
		}
		else
		{
			num_anno = 0;
		}
		os.write(reinterpret_cast<char const *>(&num_anno), sizeof(num_anno));
		for (uint32_t i = 0; i < num_anno; ++ i)
		{
			RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
			(*annotations_)[i] = annotation;
				
			annotation->StreamOut(os);
		}

		uint8_t num_macro;
		if (macros_)
		{
			num_macro = static_cast<uint8_t>(macros_->size());
		}
		else
		{
			num_macro = 0;
		}
		os.write(reinterpret_cast<char const *>(&num_macro), sizeof(num_macro));
		for (uint32_t i = 0; i < num_macro; ++ i)
		{
			WriteShortString(os, (*macros_)[i].first);
			WriteShortString(os, (*macros_)[i].second);
		}

		RasterizerStateDesc rs_desc = rasterizer_state_obj_->GetDesc();
		DepthStencilStateDesc dss_desc = depth_stencil_state_obj_->GetDesc();
		BlendStateDesc bs_desc = blend_state_obj_->GetDesc();

		NativeToLittleEndian<sizeof(rs_desc.polygon_mode)>(&rs_desc.polygon_mode);
		NativeToLittleEndian<sizeof(rs_desc.shade_mode)>(&rs_desc.shade_mode);
		NativeToLittleEndian<sizeof(rs_desc.cull_mode)>(&rs_desc.cull_mode);
		NativeToLittleEndian<sizeof(rs_desc.polygon_offset_factor)>(&rs_desc.polygon_offset_factor);
		NativeToLittleEndian<sizeof(rs_desc.polygon_offset_units)>(&rs_desc.polygon_offset_units);
		os.write(reinterpret_cast<char const *>(&rs_desc), sizeof(rs_desc));
		
		NativeToLittleEndian<sizeof(dss_desc.depth_func)>(&dss_desc.depth_func);
		NativeToLittleEndian<sizeof(dss_desc.front_stencil_func)>(&dss_desc.front_stencil_func);
		NativeToLittleEndian<sizeof(dss_desc.front_stencil_read_mask)>(&dss_desc.front_stencil_read_mask);
		NativeToLittleEndian<sizeof(dss_desc.front_stencil_write_mask)>(&dss_desc.front_stencil_write_mask);
		NativeToLittleEndian<sizeof(dss_desc.front_stencil_fail)>(&dss_desc.front_stencil_fail);
		NativeToLittleEndian<sizeof(dss_desc.front_stencil_depth_fail)>(&dss_desc.front_stencil_depth_fail);
		NativeToLittleEndian<sizeof(dss_desc.front_stencil_pass)>(&dss_desc.front_stencil_pass);
		NativeToLittleEndian<sizeof(dss_desc.back_stencil_func)>(&dss_desc.back_stencil_func);
		NativeToLittleEndian<sizeof(dss_desc.back_stencil_read_mask)>(&dss_desc.back_stencil_read_mask);
		NativeToLittleEndian<sizeof(dss_desc.back_stencil_write_mask)>(&dss_desc.back_stencil_write_mask);
		NativeToLittleEndian<sizeof(dss_desc.back_stencil_fail)>(&dss_desc.back_stencil_fail);
		NativeToLittleEndian<sizeof(dss_desc.back_stencil_depth_fail)>(&dss_desc.back_stencil_depth_fail);
		NativeToLittleEndian<sizeof(dss_desc.back_stencil_pass)>(&dss_desc.back_stencil_pass);
		os.write(reinterpret_cast<char const *>(&dss_desc), sizeof(dss_desc));

		for (size_t i = 0; i < bs_desc.blend_op.size(); ++ i)
		{
			NativeToLittleEndian<sizeof(bs_desc.blend_op[i])>(&bs_desc.blend_op[i]);
			NativeToLittleEndian<sizeof(bs_desc.src_blend[i])>(&bs_desc.src_blend[i]);
			NativeToLittleEndian<sizeof(bs_desc.dest_blend[i])>(&bs_desc.dest_blend[i]);
			NativeToLittleEndian<sizeof(bs_desc.blend_op_alpha[i])>(&bs_desc.blend_op_alpha[i]);
			NativeToLittleEndian<sizeof(bs_desc.src_blend_alpha[i])>(&bs_desc.src_blend_alpha[i]);
			NativeToLittleEndian<sizeof(bs_desc.dest_blend_alpha[i])>(&bs_desc.dest_blend_alpha[i]);
		}		
		os.write(reinterpret_cast<char const *>(&bs_desc), sizeof(bs_desc));

		{
			uint16_t tmp = front_stencil_ref_;
			NativeToLittleEndian<sizeof(tmp)>(&tmp);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			tmp = back_stencil_ref_;
			NativeToLittleEndian<sizeof(tmp)>(&tmp);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}
		{
			Color tmp = blend_factor_;
			NativeToLittleEndian<sizeof(tmp[0])>(&tmp[0]);
			NativeToLittleEndian<sizeof(tmp[1])>(&tmp[1]);
			NativeToLittleEndian<sizeof(tmp[2])>(&tmp[2]);
			NativeToLittleEndian<sizeof(tmp[3])>(&tmp[3]);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}
		{
			uint32_t tmp = sample_mask_;
			NativeToLittleEndian<sizeof(tmp)>(&tmp);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}

		for (uint32_t i = 0; i < shader_desc_ids_->size(); ++ i)
		{
			uint32_t tmp = (*shader_desc_ids_)[i];
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}

		for (int type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			ShaderObjectPtr shared_so;
			ShaderDesc const & sd = effect_.GetShaderDesc((*shader_desc_ids_)[type]);
			if (!sd.func_name.empty())
			{
				if (sd.tech_pass_type == (tech_index << 16) + (pass_index << 8) + type)
				{
					shader_obj_->StreamOut(os, static_cast<ShaderObject::ShaderType>(type));
				}
			}
		}
	}

	RenderPassPtr RenderPass::Clone(RenderEffect& effect)
	{
		RenderPassPtr ret = MakeSharedPtr<RenderPass>(effect);

		ret->name_ = name_;
		ret->annotations_ = annotations_;
		ret->macros_ = macros_;
		ret->shader_desc_ids_ = shader_desc_ids_;

		ret->rasterizer_state_obj_ = rasterizer_state_obj_;
		ret->depth_stencil_state_obj_ = depth_stencil_state_obj_;
		ret->front_stencil_ref_ = front_stencil_ref_;
		ret->back_stencil_ref_ = back_stencil_ref_;
		ret->blend_state_obj_ = blend_state_obj_;
		ret->blend_factor_ = blend_factor_;
		ret->sample_mask_ = sample_mask_;
		ret->shader_obj_ = shader_obj_->Clone(effect);

		ret->is_validate_ = is_validate_;

		return ret;
	}

	void RenderPass::Bind()
	{
		RenderEngine& render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		render_eng.SetStateObjects(rasterizer_state_obj_, depth_stencil_state_obj_,
			front_stencil_ref_, back_stencil_ref_, blend_state_obj_, blend_factor_, sample_mask_);

		shader_obj_->Bind();
	}

	void RenderPass::Unbind()
	{
		shader_obj_->Unbind();
	}


	RenderEffectParameter::RenderEffectParameter(RenderEffect& effect)
		: effect_(effect)
	{
	}

	RenderEffectParameter::~RenderEffectParameter()
	{
	}

	void RenderEffectParameter::Load(XMLNodePtr const & node)
	{
		type_ = type_define::instance().type_code(node->Attrib("type")->ValueString());
		name_ = MakeSharedPtr<KLAYGE_DECLTYPE(*name_)>(node->Attrib("name")->ValueString());

		XMLAttributePtr attr = node->Attrib("semantic");
		if (attr)
		{
			semantic_ = MakeSharedPtr<KLAYGE_DECLTYPE(*semantic_)>(attr->ValueString());
		}

		uint32_t as;
		attr = node->Attrib("array_size");
		if (attr)
		{
			array_size_ = MakeSharedPtr<std::string>(attr->ValueString());

			try
			{
				as = attr->ValueUInt();
			}
			catch (...)
			{
				as = 1;  // dummy array size
			}
		}
		else
		{
			as = 0;
		}
		var_ = read_var(node, type_, as);

		{
			XMLNodePtr anno_node = node->FirstNode("annotation");
			if (anno_node)
			{
				annotations_ = MakeSharedPtr<KLAYGE_DECLTYPE(*annotations_)>();
				for (; anno_node; anno_node = anno_node->NextSibling("annotation"))
				{
					RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
					annotations_->push_back(annotation);

					annotation->Load(anno_node);
				}
			}
		}

		if (annotations_ && ((REDT_texture1D == type_) || (REDT_texture2D == type_) || (REDT_texture3D == type_) || (REDT_textureCUBE == type_)
			|| (REDT_texture1DArray == type_) || (REDT_texture2DArray == type_) || (REDT_texture3DArray == type_) || (REDT_textureCUBEArray == type_)))
		{
			for (size_t i = 0; i < annotations_->size(); ++ i)
			{
				if (REDT_string == (*annotations_)[i]->Type())
				{
					if ("SasResourceAddress" == (*annotations_)[i]->Name())
					{
						std::string val;
						(*annotations_)[i]->Value(val);

						*var_ = SyncLoadTexture(val, EAH_GPU_Read | EAH_Immutable);
					}
				}
			}
		}
	}

	void RenderEffectParameter::StreamOut(std::ostream& os)
	{
		uint32_t t = type_;
		NativeToLittleEndian<sizeof(t)>(&t);
		os.write(reinterpret_cast<char const *>(&t), sizeof(t));
		WriteShortString(os, *name_);
		if (semantic_)
		{
			WriteShortString(os, *semantic_);
		}
		else
		{
			uint8_t len = 0;
			os.write(reinterpret_cast<char const *>(&len), sizeof(len));
		}

		if (array_size_)
		{
			WriteShortString(os, *array_size_);
		}
		else
		{
			uint8_t len = 0;
			os.write(reinterpret_cast<char const *>(&len), sizeof(len));
		}
		uint32_t as;
		if (array_size_)
		{
			try
			{
				as = boost::lexical_cast<uint32_t>(*array_size_);
			}
			catch (...)
			{
				as = 1;  // dummy array size
			}
		}
		else
		{
			as = 0;
		}
		stream_out_var(os, var_, type_, as);

		uint8_t num_anno;
		if (annotations_)
		{
			num_anno = static_cast<uint8_t>(annotations_->size());
		}
		else
		{
			num_anno = 0;
		}
		os.write(reinterpret_cast<char const *>(&num_anno), sizeof(num_anno));
		for (uint32_t i = 0; i < num_anno; ++ i)
		{
			(*annotations_)[i]->StreamOut(os);
		}
	}

	RenderEffectParameterPtr RenderEffectParameter::Clone(RenderEffect& effect)
	{
		RenderEffectParameterPtr ret = MakeSharedPtr<RenderEffectParameter>(effect);

		ret->name_ = name_;
		ret->semantic_ = semantic_;

		ret->type_ = type_;
		ret->var_ = var_->Clone();
		ret->array_size_ = array_size_;

		ret->annotations_ = annotations_;

		return ret;
	}


	void RenderShaderFunc::Load(XMLNodePtr const & node)
	{
		type_ = ShaderObject::ST_NumShaderTypes;
		XMLAttributePtr attr = node->Attrib("type");
		if (attr)
		{
			std::string type_str = attr->ValueString();
			if ("vertex_shader" == type_str)
			{
				type_ = ShaderObject::ST_VertexShader;
			}
			else if ("pixel_shader" == type_str)
			{
				type_ = ShaderObject::ST_PixelShader;
			}
			else if ("geometry_shader" == type_str)
			{
				type_ = ShaderObject::ST_GeometryShader;
			}
			else if ("compute_shader" == type_str)
			{
				type_ = ShaderObject::ST_ComputeShader;
			}
			else if ("hull_shader" == type_str)
			{
				type_ = ShaderObject::ST_HullShader;
			}
			else if ("domain_shader" == type_str)
			{
				type_ = ShaderObject::ST_DomainShader;
			}
		}
		
		version_ = 0;
		attr = node->Attrib("version");
		if (attr)
		{
			version_ = attr->ValueInt();
		}

		for (XMLNodePtr shader_text_node = node->FirstNode(); shader_text_node; shader_text_node = shader_text_node->NextSibling())
		{
			if ((XNT_Comment == shader_text_node->Type()) || (XNT_CData == shader_text_node->Type()))
			{
				str_ += shader_text_node->ValueString();
			}
		}
	}

	void RenderShaderFunc::StreamOut(std::ostream& os)
	{
		uint32_t tmp = type_;
		NativeToLittleEndian<sizeof(tmp)>(&tmp);
		os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		tmp = version_;
		NativeToLittleEndian<sizeof(tmp)>(&tmp);
		os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));

		uint32_t len = static_cast<uint32_t>(str_.size());
		tmp = len;
		NativeToLittleEndian<sizeof(tmp)>(&tmp);
		os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		os.write(&str_[0], len * sizeof(str_[0]));
	}


	RenderVariable::RenderVariable()
	{
	}

	RenderVariable::~RenderVariable()
	{
	}

	RenderVariable& RenderVariable::operator=(bool const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(uint32_t const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(int32_t const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(float const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(uint2 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(uint3 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(uint4 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(int2 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(int3 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(int4 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(float2 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(float3 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(float4 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(float4x4 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(TexturePtr const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(TextureSubresource const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(function<TexturePtr()> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(SamplerStateObjectPtr const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(GraphicsBufferPtr const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::string const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(ShaderDesc const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<bool> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<uint32_t> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<int32_t> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<float> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<uint2> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<uint3> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<uint4> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<int2> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<int3> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<int4> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<float2> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<float3> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<float4> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<float4x4> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	void RenderVariable::Value(bool& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(uint32_t& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(int32_t& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(float& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(uint2& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(uint3& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(uint4& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(int2& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(int3& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(int4& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(float2& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(float3& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(float4& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(float4x4& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(TexturePtr& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(TextureSubresource& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(SamplerStateObjectPtr& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(GraphicsBufferPtr& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::string& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(ShaderDesc& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<bool>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<uint32_t>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<int32_t>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<float>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<uint2>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<uint3>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<uint4>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<int2>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<int3>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<int4>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<float2>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<float3>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<float4>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<float4x4>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	RenderVariablePtr RenderVariableTexture::Clone()
	{
		RenderVariablePtr ret = MakeSharedPtr<RenderVariableTexture>();
		TexturePtr val;
		this->Value(val);
		*ret = val;
		std::string elem_type;
		this->Value(elem_type);
		*ret = elem_type;
		return ret;
	}

	RenderVariable& RenderVariableTexture::operator=(TexturePtr const & value)
	{
		uint32_t array_size = 1;
		uint32_t mipmap = 1;
		if (value)
		{
			array_size = value->ArraySize();
			mipmap = value->NumMipMaps();
		}
		return this->operator=(TextureSubresource(value, 0, array_size, 0, mipmap));
	}

	RenderVariable& RenderVariableTexture::operator=(TextureSubresource const & value)
	{
		val_ = value;
		return *this;
	}

	RenderVariable& RenderVariableTexture::operator=(function<TexturePtr()> const & value)
	{
		tl_ = value;
		if (tl_)
		{
			val_.tex = tl_();
			val_.first_array_index = 0;
			val_.first_level = 0;
			if (val_.tex)
			{
				val_.num_items = val_.tex->ArraySize();
				val_.num_levels = val_.tex->NumMipMaps();
			}
			else
			{
				val_.num_items = 1;
				val_.num_levels = 1;
			}
		}
		return *this;
	}

	void RenderVariableTexture::Value(TexturePtr& val) const
	{
		if (tl_ && !val_.tex)
		{
			val_.tex = tl_();
			if (val_.tex)
			{
				val_.num_items = val_.tex->ArraySize();
				val_.num_levels = val_.tex->NumMipMaps();
			}
		}
		val = val_.tex;
	}

	void RenderVariableTexture::Value(TextureSubresource& val) const
	{
		if (tl_ && !val_.tex)
		{
			val_.tex = tl_();
			if (val_.tex)
			{
				val_.num_items = val_.tex->ArraySize();
				val_.num_levels = val_.tex->NumMipMaps();
			}
		}
		val = val_;
	}

	RenderVariable& RenderVariableTexture::operator=(std::string const & value)
	{
		elem_type_ = value;
		return *this;
	}

	void RenderVariableTexture::Value(std::string& val) const
	{
		val = elem_type_;
	}


	RenderVariablePtr RenderVariableBuffer::Clone()
	{
		RenderVariablePtr ret = MakeSharedPtr<RenderVariableBuffer>();
		GraphicsBufferPtr val;
		this->Value(val);
		*ret = val;
		std::string elem_type;
		this->Value(elem_type);
		*ret = elem_type;
		return ret;
	}

	RenderVariable& RenderVariableBuffer::operator=(GraphicsBufferPtr const & value)
	{
		val_ = value;
		return *this;
	}

	void RenderVariableBuffer::Value(GraphicsBufferPtr& val) const
	{
		val = val_;
	}

	RenderVariable& RenderVariableBuffer::operator=(std::string const & value)
	{
		elem_type_ = value;
		return *this;
	}

	void RenderVariableBuffer::Value(std::string& val) const
	{
		val = elem_type_;
	}


	RenderVariablePtr RenderVariableByteAddressBuffer::Clone()
	{
		RenderVariablePtr ret = MakeSharedPtr<RenderVariableByteAddressBuffer>();
		GraphicsBufferPtr val;
		this->Value(val);
		*ret = val;
		std::string elem_type;
		this->Value(elem_type);
		*ret = elem_type;
		return ret;
	}

	RenderVariable& RenderVariableByteAddressBuffer::operator=(GraphicsBufferPtr const & value)
	{
		val_ = value;
		return *this;
	}

	void RenderVariableByteAddressBuffer::Value(GraphicsBufferPtr& val) const
	{
		val = val_;
	}

	RenderVariable& RenderVariableByteAddressBuffer::operator=(std::string const & value)
	{
		elem_type_ = value;
		return *this;
	}

	void RenderVariableByteAddressBuffer::Value(std::string& val) const
	{
		val = elem_type_;
	}
}
