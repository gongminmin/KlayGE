// RenderEffect.cpp
// KlayGE 渲染效果类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2003-2009
// Homepage: http://klayge.sourceforge.net
//
// 3.9.0
// 直接从fxml文件读取特效脚本 (2009.4.21)
// 支持Buffer类型 (2009.5.14)
// 支持macro标签 (2009.5.22)
//
// 3.8.0
// 支持CBuffer (2008.10.6)
//
// 3.6.0
// 增加了Clone (2007.6.11)
//
// 3.5.0
// 改用基于xml的特效格式 (2006.10.21)
//
// 3.2.0
// 支持了bool类型 (2006.3.8)
//
// 3.0.0
// 增加了RenderTechnique和RenderPass (2005.9.4)
//
// 2.8.0
// 增加了Do*函数，使用模板方法模式 (2005.7.24)
// 使用新的自动更新参数的方法 (2005.7.25)
//
// 2.2.0
// 统一使用istream作为资源标示符 (2004.10.26)
//
// 2.1.2
// 增加了Parameter (2004.5.26)
//
// 2.0.3
// 初次建立 (2003.3.2)
// 修改了SetTexture的参数 (2004.3.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/XMLDom.hpp>

#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
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

#include <KlayGE/RenderEffect.hpp>

using namespace std;

namespace
{
	using namespace KlayGE;

	class type_define
	{
	public:
		static type_define& instance()
		{
			static type_define ret;
			return ret;
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

	private:
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
	};

	class shade_mode_define
	{
	public:
		static shade_mode_define& instance()
		{
			static shade_mode_define ret;
			return ret;
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
			cerr << "Wrong ShadeMode name: " << name << endl;
			return static_cast<ShadeMode>(0xFFFFFFFF);
		}

	private:
		shade_mode_define()
		{
			sms_.push_back("flat");
			sms_.push_back("gouraud");
		}

	private:
		std::vector<std::string> sms_;
	};

	class compare_function_define
	{
	public:
		static compare_function_define& instance()
		{
			static compare_function_define ret;
			return ret;
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
			cerr << "Wrong CompareFunction name: " << name << endl;
			return static_cast<CompareFunction>(0xFFFFFFFF);
		}

	private:
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
	};

	class cull_mode_define
	{
	public:
		static cull_mode_define& instance()
		{
			static cull_mode_define ret;
			return ret;
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
			cerr << "Wrong CullMode name: " << name << endl;
			return static_cast<CullMode>(0xFFFFFFFF);
		}

	private:
		cull_mode_define()
		{
			cms_.push_back("none");
			cms_.push_back("front");
			cms_.push_back("back");
		}

	private:
		std::vector<std::string> cms_;
	};

	class polygon_mode_define
	{
	public:
		static polygon_mode_define& instance()
		{
			static polygon_mode_define ret;
			return ret;
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
			cerr << "Wrong PolygonMode name: " << name << endl;
			return static_cast<PolygonMode>(0xFFFFFFFF);
		}

	private:
		polygon_mode_define()
		{
			pms_.push_back("point");
			pms_.push_back("line");
			pms_.push_back("fill");
		}

	private:
		std::vector<std::string> pms_;
	};

	class alpha_blend_factor_define
	{
	public:
		static alpha_blend_factor_define& instance()
		{
			static alpha_blend_factor_define ret;
			return ret;
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
			cerr << "Wrong AlphaBlendFactor name: " << name << endl;
			return static_cast<AlphaBlendFactor>(0xFFFFFFFF);
		}

	private:
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
	};

	class blend_operation_define
	{
	public:
		static blend_operation_define& instance()
		{
			static blend_operation_define ret;
			return ret;
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
			cerr << "Wrong BlendOperation name: " << name << endl;
			return static_cast<BlendOperation>(0xFFFFFFFF);
		}

	private:
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
	};

	class stencil_operation_define
	{
	public:
		static stencil_operation_define& instance()
		{
			static stencil_operation_define ret;
			return ret;
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
			cerr << "Wrong StencilOperation name: " << name << endl;
			return static_cast<StencilOperation>(0xFFFFFFFF);
		}

	private:
		stencil_operation_define()
		{
			sops_.push_back("keep");
			sops_.push_back("zero");
			sops_.push_back("replace");
			sops_.push_back("increment");
			sops_.push_back("decrement");
			sops_.push_back("invert");
		}

	private:
		std::vector<std::string> sops_;
	};

	class texture_filter_mode_define
	{
	public:
		static texture_filter_mode_define& instance()
		{
			static texture_filter_mode_define ret;
			return ret;
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
			cerr << "Wrong TexFilterOp name: " << name << endl;
			return static_cast<TexFilterOp>(0xFFFFFFFF);
		}

	private:
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
	};

	class texture_addr_mode_define
	{
	public:
		static texture_addr_mode_define& instance()
		{
			static texture_addr_mode_define ret;
			return ret;
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
			cerr << "Wrong TexAddressingMode name: " << name << endl;
			return static_cast<TexAddressingMode>(0xFFFFFFFF);
		}

	private:
		texture_addr_mode_define()
		{
			tams_.push_back("wrap");
			tams_.push_back("mirror");
			tams_.push_back("clamp");
			tams_.push_back("border");
		}

	private:
		std::vector<std::string> tams_;
	};

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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
						std::vector<int32_t> init_val(std::min(array_size, static_cast<uint32_t>(strs.size())), 0);
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
						cerr << "Wrong sampler state name:" << name << endl;
					}
				}

				var = MakeSharedPtr<RenderVariableSampler>();
				*var = Context::Instance().RenderFactoryInstance().MakeSamplerStateObject(desc);
			}
			break;

		case REDT_shader:
			{
				shader_desc desc;
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
				int2 tmp(0, 0);
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
				int3 tmp(0, 0, 0);
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
				int4 tmp(0, 0, 0, 0);
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
						boost::algorithm::split(strs, value_str, boost::bind(std::equal_to<char>(), ',', _1));
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
}

namespace KlayGE
{
	void RenderEffectAnnotation::Load(XMLNodePtr const & node)
	{
		type_ = type_define::instance().type_code(node->Attrib("type")->ValueString());
		name_ = node->Attrib("name")->ValueString();
		var_ = read_var(node, type_, 0);
	}


	class NullRenderEffect : public RenderEffect
	{
	public:
		NullRenderEffect()
		{
		}

		RenderTechniquePtr MakeRenderTechnique()
		{
			return RenderTechnique::NullObject();
		}
	};

	RenderEffect::RenderEffect()
	{
	}

	void RenderEffect::Load(ResIdentifierPtr const & source, std::pair<std::string, std::string>* predefined_macros)
	{
		if (source)
		{
			shader_descs_.resize(1);

			XMLDocument doc;
			XMLNodePtr root = doc.Parse(source);

			cbuffers_ = MakeSharedPtr<BOOST_TYPEOF(*cbuffers_)>();

			XMLAttributePtr attr;

			std::vector<XMLDocumentPtr> include_docs;
			for (XMLNodePtr node = root->FirstNode("include"); node;)
			{
				attr = node->Attrib("name");
				include_docs.push_back(MakeSharedPtr<XMLDocument>());
				XMLNodePtr include_root = include_docs.back()->Parse(ResLoader::Instance().Load(attr->ValueString()));

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
				if (macro_node || predefined_macros)
				{
					macros_ = MakeSharedPtr<BOOST_TYPEOF(*macros_)>();
				}
				if (predefined_macros)
				{
					size_t m = 0;
					while (!predefined_macros[m].first.empty())
					{
						macros_->push_back(std::make_pair(predefined_macros[m].first, predefined_macros[m].second));
						++ m;
					}
				}
				for (; macro_node; macro_node = macro_node->NextSibling("macro"))
				{
					macros_->push_back(std::make_pair(macro_node->Attrib("name")->ValueString(), macro_node->Attrib("value")->ValueString()));
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
					shaders_ = MakeSharedPtr<BOOST_TYPEOF(*shaders_)>();
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
	}

	RenderEffectPtr RenderEffect::Clone()
	{
		RenderEffectPtr ret = MakeSharedPtr<RenderEffect>();

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

	RenderEffectPtr const & RenderEffect::NullObject()
	{
		static RenderEffectPtr obj = MakeSharedPtr<NullRenderEffect>();
		return obj;
	}

	RenderEffectParameterPtr RenderEffect::ParameterByName(std::string const & name) const
	{
		BOOST_FOREACH(BOOST_TYPEOF(params_)::const_reference param, params_)
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
		BOOST_FOREACH(BOOST_TYPEOF(params_)::const_reference param, params_)
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
		BOOST_FOREACH(BOOST_TYPEOF(techniques_)::const_reference tech, techniques_)
		{
			if (name == tech->Name())
			{
				return tech;
			}
		}
		return RenderTechnique::NullObject();
	}

	uint32_t RenderEffect::AddShaderDesc(shader_desc const & sd)
	{
		for (uint32_t i = 0; i < shader_descs_.size(); ++ i)
		{
			if (shader_descs_[i] == sd)
			{
				return i;
			}
		}

		uint32_t id = static_cast<uint32_t>(shader_descs_.size());
		shader_descs_.push_back(sd);
		return id;
	}

	shader_desc& RenderEffect::GetShaderDesc(uint32_t id)
	{
		BOOST_ASSERT(id < shader_descs_.size());
		return shader_descs_[id];
	}

	shader_desc const & RenderEffect::GetShaderDesc(uint32_t id) const
	{
		BOOST_ASSERT(id < shader_descs_.size());
		return shader_descs_[id];
	}

	std::string const & RenderEffect::TypeName(uint32_t code) const
	{
		return type_define::instance().type_name(code);
	}


	class NullRenderTechnique : public RenderTechnique
	{
	public:
		NullRenderTechnique()
			: RenderTechnique(*RenderEffect::NullObject())
		{
			is_validate_ = true;
		}

	private:
		void DoBegin()
		{
		}
		void DoEnd()
		{
		}

		RenderPassPtr MakeRenderPass()
		{
			return RenderPass::NullObject();
		}
	};

	RenderTechniquePtr const & RenderTechnique::NullObject()
	{
		static RenderTechniquePtr obj = MakeSharedPtr<NullRenderTechnique>();
		return obj;
	}

	void RenderTechnique::Load(XMLNodePtr const & node, uint32_t tech_index)
	{
		name_ = MakeSharedPtr<BOOST_TYPEOF(*name_)>(node->Attrib("name")->ValueString());

		{
			XMLNodePtr anno_node = node->FirstNode("annotation");
			if (anno_node)
			{
				annotations_ = MakeSharedPtr<BOOST_TYPEOF(*annotations_)>();
				for (; anno_node; anno_node = anno_node->NextSibling("annotation"))
				{
					RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
					annotations_->push_back(annotation);

					annotation->Load(anno_node);
				}
			}
		}

		is_validate_ = true;

		bool blend = false;
		weight_ = 1;
		uint32_t index = 0;
		for (XMLNodePtr pass_node = node->FirstNode("pass"); pass_node; pass_node = pass_node->NextSibling("pass"), ++ index)
		{
			RenderPassPtr pass = MakeSharedPtr<RenderPass>(effect_);
			passes_.push_back(pass);

			pass->Load(pass_node, tech_index, index);

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
						blend = true;
					}
				}
			}
		}
		if (blend)
		{
			weight_ += 10000;
		}
	}

	RenderTechniquePtr RenderTechnique::Clone(RenderEffect& effect)
	{
		RenderTechniquePtr ret = MakeSharedPtr<RenderTechnique>(effect);

		ret->name_ = name_;

		ret->annotations_ = annotations_;
		ret->weight_ = weight_;
		ret->is_validate_ = is_validate_;

		ret->passes_.resize(passes_.size());
		for (size_t i = 0; i < passes_.size(); ++ i)
		{
			ret->passes_[i] = passes_[i]->Clone(effect);
		}

		return ret;
	}


	class NullRenderPass : public RenderPass
	{
	public:
		NullRenderPass()
			: RenderPass(*RenderEffect::NullObject())
		{
			is_validate_ = true;
		}
	};

	RenderPassPtr const & RenderPass::NullObject()
	{
		static RenderPassPtr obj = MakeSharedPtr<NullRenderPass>();
		return obj;
	}

	void RenderPass::Load(XMLNodePtr const & node, uint32_t tech_index, uint32_t pass_index)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		name_ = MakeSharedPtr<BOOST_TYPEOF(*name_)>(std::string(node->Attrib("name")->ValueString()));

		{
			XMLNodePtr anno_node = node->FirstNode("annotation");
			if (anno_node)
			{
				annotations_ = MakeSharedPtr<BOOST_TYPEOF(*annotations_)>();
				for (; anno_node; anno_node = anno_node->NextSibling("annotation"))
				{
					RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
					annotations_->push_back(annotation);

					annotation->Load(anno_node);
				}
			}
		}

		RasterizerStateDesc rs_desc;
		DepthStencilStateDesc dss_desc;
		BlendStateDesc bs_desc;
		shader_obj_ = rf.MakeShaderObject();

		shader_desc_ids_ = MakeSharedPtr<BOOST_TYPEOF(*shader_desc_ids_)>();
		shader_desc_ids_->resize(ShaderObject::ST_NumShaderTypes, 0);

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

				shader_desc sd;
				sd.profile = get_profile(state_node);
				sd.func_name = get_func_name(state_node);

				if ((ShaderObject::ST_VertexShader == type) || (ShaderObject::ST_GeometryShader == type))
				{
					XMLNodePtr so_node = state_node->FirstNode("stream_output");
					if (so_node)
					{
						for (XMLNodePtr slot_node = so_node->FirstNode("slot"); slot_node; slot_node = slot_node->NextSibling("slot"))
						{
							shader_desc::stream_output_decl decl;

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
				cerr << "Wrong state name: " << state_name << endl;
			}
		}

		rasterizer_state_obj_ = rf.MakeRasterizerStateObject(rs_desc);
		depth_stencil_state_obj_ = rf.MakeDepthStencilStateObject(dss_desc);
		blend_state_obj_ = rf.MakeBlendStateObject(bs_desc);

		shader_obj_->SetShader(effect_, shader_desc_ids_, tech_index, pass_index);

		is_validate_ = shader_obj_->Validate();
	}

	RenderPassPtr RenderPass::Clone(RenderEffect& effect)
	{
		RenderPassPtr ret = MakeSharedPtr<RenderPass>(effect);

		ret->name_ = name_;
		ret->annotations_ = annotations_;
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
		name_ = MakeSharedPtr<BOOST_TYPEOF(*name_)>(std::string(node->Attrib("name")->ValueString()));

		XMLAttributePtr attr = node->Attrib("semantic");
		if (attr)
		{
			semantic_ = MakeSharedPtr<BOOST_TYPEOF(*semantic_)>(attr->ValueString());
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
				annotations_ = MakeSharedPtr<BOOST_TYPEOF(*annotations_)>();
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

						*var_ = LoadTexture(val, EAH_GPU_Read)();
					}
				}
			}
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

	RenderVariable& RenderVariable::operator=(shader_desc const & /*value*/)
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

	void RenderVariable::Value(shader_desc& /*value*/) const
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
}
