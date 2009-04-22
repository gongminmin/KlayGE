// RenderEffect.cpp
// KlayGE 渲染效果类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2003-2009
// Homepage: http://klayge.sourceforge.net
//
// 3.9.0
// 直接从fxml文件读取特效脚本 (2009.4.21)
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

#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#pragma warning(disable: 4702)
#include <boost/lexical_cast.hpp>
#pragma warning(pop)
#include <boost/filesystem.hpp>

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
			types_.push_back("dword");
			types_.push_back("string");
			types_.push_back("texture1D");
			types_.push_back("texture2D");
			types_.push_back("texture3D");
			types_.push_back("textureCUBE");
			types_.push_back("sampler");
			types_.push_back("shader");
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

	int get_index(rapidxml::xml_node<>* node)
	{
		int index = 0;
		rapidxml::xml_attribute<>* attr = node->first_attribute("index");
		if (attr != NULL)
		{
			index = boost::lexical_cast<int>(attr->value());
		}
		return index;
	}

	std::string get_profile(rapidxml::xml_node<>* node)
	{
		rapidxml::xml_attribute<>* attr = node->first_attribute("profile");
		if (attr != NULL)
		{
			return attr->value();
		}
		else
		{
			return "auto";
		}
	}

	std::string get_func_name(rapidxml::xml_node<>* node)
	{
		std::string value = node->first_attribute("value")->value();
		return value.substr(0, value.find("("));
	}


	RenderVariablePtr read_var(rapidxml::xml_node<>* node, uint32_t type, uint32_t array_size)
	{
		using boost::lexical_cast;

		RenderVariablePtr var;
		rapidxml::xml_attribute<>* attr;

		switch (type)
		{
		case REDT_bool:
			if (0 == array_size)
			{
				attr = node->first_attribute("value");
				bool tmp = false;
				if (attr != NULL)
				{
					std::string value_str = attr->value();
					tmp = bool_from_str(value_str);
				}

				var = MakeSharedPtr<RenderVariableBool>();
				*var = tmp;
			}
			break;

		case REDT_dword:
		case REDT_int:
			if (0 == array_size)
			{
				attr = node->first_attribute("value");
				int32_t tmp = 0;
				if (attr != NULL)
				{
					tmp = lexical_cast<int32_t>(attr->value());
				}

				var = MakeSharedPtr<RenderVariableInt>();
				*var = tmp;
			}
			break;

		case REDT_string:
			{
				attr = node->first_attribute("value");
				std::string tmp;
				if (attr != NULL)
				{
					tmp = attr->value();
				}

				var = MakeSharedPtr<RenderVariableString>();
				*var = tmp;
			}
			break;

		case REDT_texture1D:
		case REDT_texture2D:
		case REDT_texture3D:
		case REDT_textureCUBE:
			var = MakeSharedPtr<RenderVariableTexture>();
			*var = TexturePtr();
			break;

		case REDT_sampler:
			{
				SamplerStateDesc desc;

				for (rapidxml::xml_node<>* state_node = node->first_node("state"); state_node; state_node = state_node->next_sibling("state"))
				{
					std::string name = state_node->first_attribute("name")->value();
					if ("filtering" == name)
					{
						std::string value_str = state_node->first_attribute("value")->value();
						desc.filter = texture_filter_mode_define::instance().from_str(value_str);
						if (0xFFFFFFFF == desc.filter)
						{
							cerr << "Wrong filtering name: " << value_str << endl;
						}
					}
					else if ("address_u" == name)
					{
						std::string value_str = state_node->first_attribute("value")->value();
						desc.addr_mode_u = texture_addr_mode_define::instance().from_str(value_str);
						if (0xFFFFFFFF == desc.addr_mode_u)
						{
							cerr << "Wrong address u name: " << value_str << endl;
						}
					}
					else if ("address_v" == name)
					{
						std::string value_str = state_node->first_attribute("value")->value();
						desc.addr_mode_v = texture_addr_mode_define::instance().from_str(value_str);
						if (0xFFFFFFFF == desc.addr_mode_v)
						{
							cerr << "Wrong address u name: " << value_str << endl;
						}
					}
					else if ("address_w" == name)
					{
						std::string value_str = state_node->first_attribute("value")->value();
						desc.addr_mode_w = texture_addr_mode_define::instance().from_str(value_str);
						if (0xFFFFFFFF == desc.addr_mode_w)
						{
							cerr << "Wrong address u name: " << value_str << endl;
						}
					}
					else if ("anisotropy" == name)
					{
						std::string value_str = state_node->first_attribute("value")->value();
						desc.anisotropy = static_cast<uint8_t>(lexical_cast<uint32_t>(value_str));
					}
					else if ("max_mip_level" == name)
					{
						std::string value_str = state_node->first_attribute("value")->value();
						desc.max_mip_level = static_cast<uint8_t>(lexical_cast<uint32_t>(value_str));
					}
					else if ("mip_map_lod_bias" == name)
					{
						std::string value_str = state_node->first_attribute("value")->value();
						desc.mip_map_lod_bias = lexical_cast<float>(value_str);
					}
					else if ("border_clr" == name)
					{
						attr = state_node->first_attribute("r");
						if (attr != NULL)
						{
							desc.border_clr.r() = lexical_cast<float>(attr->value());
						}
						attr = state_node->first_attribute("g");
						if (attr != NULL)
						{
							desc.border_clr.g() = lexical_cast<float>(attr->value());
						}
						attr = state_node->first_attribute("b");
						if (attr != NULL)
						{
							desc.border_clr.b() = lexical_cast<float>(attr->value());
						}
						attr = state_node->first_attribute("a");
						if (attr != NULL)
						{
							desc.border_clr.a() = lexical_cast<float>(attr->value());
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
				attr = node->first_attribute("value");
				if (attr != NULL)
				{
					tmp = lexical_cast<float>(attr->value());
				}

				var = MakeSharedPtr<RenderVariableFloat>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloatArray>();
			}
			break;

		case REDT_float2:
			if (0 == array_size)
			{
				float2 tmp(0, 0);
				attr = node->first_attribute("x");
				if (attr != NULL)
				{
					tmp.x() = lexical_cast<float>(attr->value());
				}
				attr = node->first_attribute("y");
				if (attr != NULL)
				{
					tmp.y() = lexical_cast<float>(attr->value());
				}

				var = MakeSharedPtr<RenderVariableFloat2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat2Array>();
			}
			break;

		case REDT_float3:
			if (0 == array_size)
			{
				float3 tmp(0, 0, 0);
				attr = node->first_attribute("x");
				if (attr != NULL)
				{
					tmp.x() = lexical_cast<float>(attr->value());
				}
				attr = node->first_attribute("y");
				if (attr != NULL)
				{
					tmp.y() = lexical_cast<float>(attr->value());
				}
				attr = node->first_attribute("z");
				if (attr != NULL)
				{
					tmp.z() = lexical_cast<float>(attr->value());
				}

				var = MakeSharedPtr<RenderVariableFloat3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat3Array>();
			}
			break;

		case REDT_float4:
			if (0 == array_size)
			{
				float4 tmp(0, 0, 0, 0);
				attr = node->first_attribute("x");
				if (attr != NULL)
				{
					tmp.x() = lexical_cast<float>(attr->value());
				}
				attr = node->first_attribute("y");
				if (attr != NULL)
				{
					tmp.y() = lexical_cast<float>(attr->value());
				}
				attr = node->first_attribute("z");
				if (attr != NULL)
				{
					tmp.z() = lexical_cast<float>(attr->value());
				}
				attr = node->first_attribute("w");
				if (attr != NULL)
				{
					tmp.w() = lexical_cast<float>(attr->value());
				}

				var = MakeSharedPtr<RenderVariableFloat4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat4Array>();
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
						attr = node->first_attribute(oss.str().c_str());
						if (attr != NULL)
						{
							tmp[y * 4 + x] = lexical_cast<float>(attr->value());
						}
					}
				}

				var = MakeSharedPtr<RenderVariableFloat4x4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat4x4Array>();
			}
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
	void RenderEffectAnnotation::Load(rapidxml::xml_node<>* node)
	{
		type_ = type_define::instance().type_code(node->first_attribute("type")->value());
		name_ = node->first_attribute("name")->value();
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

	void RenderEffect::Load(ResIdentifierPtr const & source)
	{
		if (source)
		{
			using boost::lexical_cast;

			source->seekg(0, std::ios_base::end);
			int len = static_cast<int>(source->tellg());
			source->seekg(0, std::ios_base::beg);
			std::vector<char> str(len + 1, 0);
			source->read(&str[0], len);

			using namespace rapidxml;
			xml_document<> doc;
			doc.parse<0>(&str[0]);

			cbuffers_ = MakeSharedPtr<BOOST_TYPEOF(*cbuffers_)>();

			xml_node<>* root = doc.first_node("effect");
			xml_attribute<>* attr;

			std::vector<std::vector<char> > include_strs;
			for (xml_node<>* node = root->first_node("include"); node;)
			{
				attr = node->first_attribute("name");
				std::ifstream include_file(ResLoader::Instance().Locate(attr->value()).c_str());
				include_file.seekg(0, std::ios_base::end);
				int len = static_cast<int>(include_file.tellg());
				include_file.seekg(0, std::ios_base::beg);
				include_strs.push_back(std::vector<char>(len + 1, 0));
				std::vector<char>& str = include_strs.back();
				include_file.read(&str[0], len);

				xml_document<> include_doc;
				include_doc.parse<0>(&str[0]);

				xml_node<>* include_root = include_doc.first_node("effect");
				for (xml_node<>* child_node = include_root->first_node(); child_node; child_node = child_node->next_sibling())
				{
					if (node_element == child_node->type())
					{
						root->insert_node(node, doc.clone_node(child_node));
					}
				}

				xml_node<>* node_next = node->next_sibling("include");
				root->remove_node(node);
				node = node_next;
			}

			std::vector<xml_node<>*> parameter_nodes;
			for (xml_node<>* node = root->first_node(); node; node = node->next_sibling())
			{
				if (std::string("parameter") == node->name())
				{
					parameter_nodes.push_back(node);
				}
				else if (std::string("cbuffer") == node->name())
				{
					for (xml_node<>* sub_node = node->first_node("parameter"); sub_node; sub_node = sub_node->next_sibling("parameter"))
					{
						parameter_nodes.push_back(sub_node);
					}
				}
			}

			for (uint32_t param_index = 0; param_index < parameter_nodes.size(); ++ param_index)
			{
				xml_node<>* node = parameter_nodes[param_index];

				{
					xml_node<>* parent_node = node->parent();
					std::string cbuff_name;
					attr = parent_node->first_attribute("name");
					if (attr != NULL)
					{
						cbuff_name = attr->value();
					}
					else
					{
						cbuff_name = "global_cb";
					}

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
				xml_node<>* shader_node = root->first_node("shader");
				shaders_ = MakeSharedPtr<BOOST_TYPEOF(*shaders_)>();
				for (; shader_node; shader_node = shader_node->next_sibling("shader"))
				{
					shaders_->push_back(RenderShaderFunc());
					shaders_->back().Load(shader_node);
				}
			}

			for (xml_node<>* node = root->first_node("technique"); node; node = node->next_sibling("technique"))
			{
				RenderTechniquePtr technique = MakeSharedPtr<RenderTechnique>(*this);
				techniques_.push_back(technique);

				technique->Load(node);
			}
		}
	}

	RenderEffectPtr RenderEffect::Clone()
	{
		RenderEffectPtr ret = MakeSharedPtr<RenderEffect>();

		ret->prototype_effect_ = prototype_effect_;
		ret->shaders_ = shaders_;

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

	void RenderTechnique::Load(rapidxml::xml_node<>* node)
	{
		using namespace rapidxml;

		name_ = MakeSharedPtr<BOOST_TYPEOF(*name_)>(std::string(node->first_attribute("name")->value()));

		{
			xml_node<>* anno_node = node->first_node("annotation");
			if (anno_node != NULL)
			{
				annotations_ = MakeSharedPtr<BOOST_TYPEOF(*annotations_)>();
				for (; anno_node; anno_node = anno_node->next_sibling("annotation"))
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
		for (xml_node<>* pass_node = node->first_node("pass"); pass_node; pass_node = pass_node->next_sibling("pass"))
		{
			RenderPassPtr pass = MakeSharedPtr<RenderPass>(effect_);
			passes_.push_back(pass);

			pass->Load(pass_node);

			is_validate_ &= pass->Validate();

			for (xml_node<>* state_node = pass_node->first_node("state"); state_node; state_node = state_node->next_sibling("state"))
			{
				++ weight_;

				std::string state_name = state_node->first_attribute("name")->value();
				if ("blend_enable" == state_name)
				{
					std::string value_str = state_node->first_attribute("value")->value();
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

	void RenderPass::Load(rapidxml::xml_node<>* node)
	{
		using boost::lexical_cast;
		using namespace rapidxml;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		name_ = MakeSharedPtr<BOOST_TYPEOF(*name_)>(std::string(node->first_attribute("name")->value()));

		{
			xml_node<>* anno_node = node->first_node("annotation");
			if (anno_node != NULL)
			{
				annotations_ = MakeSharedPtr<BOOST_TYPEOF(*annotations_)>();
				for (; anno_node; anno_node = anno_node->next_sibling("annotation"))
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

		shader_descs_ = MakeSharedPtr<BOOST_TYPEOF(*shader_descs_)>();
		shader_descs_->resize(ShaderObject::ST_NumShaderTypes);

		for (xml_node<>* state_node = node->first_node("state"); state_node; state_node = state_node->next_sibling("state"))
		{
			std::string state_name = state_node->first_attribute("name")->value();

			if ("polygon_mode" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				rs_desc.polygon_mode = polygon_mode_define::instance().from_str(value_str);
				if (0xFFFFFFFF == rs_desc.polygon_mode)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("shade_mode" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				rs_desc.shade_mode = shade_mode_define::instance().from_str(value_str);
				if (0xFFFFFFFF == rs_desc.shade_mode)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("cull_mode" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				rs_desc.cull_mode = cull_mode_define::instance().from_str(value_str);
				if (0xFFFFFFFF == rs_desc.cull_mode)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("front_face_ccw" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				rs_desc.front_face_ccw = bool_from_str(value_str);
			}
			else if ("polygon_offset_factor" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				rs_desc.polygon_offset_factor = lexical_cast<float>(value_str);
			}
			else if ("polygon_offset_units" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				rs_desc.polygon_offset_units = lexical_cast<float>(value_str);
			}
			else if ("scissor_enable" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				rs_desc.scissor_enable = bool_from_str(value_str);
			}
			else if ("multisample_enable" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				rs_desc.multisample_enable = bool_from_str(value_str);
			}
			else if ("alpha_to_coverage_enable" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.alpha_to_coverage_enable = bool_from_str(value_str);
			}
			else if ("independent_blend_enable" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.independent_blend_enable = bool_from_str(value_str);
			}
			else if ("blend_enable" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.blend_enable[index] = bool_from_str(value_str);
			}
			else if ("blend_op" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.blend_op[index] = blend_operation_define::instance().from_str(value_str);
				if (0xFFFFFFFF == bs_desc.blend_op[index])
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("src_blend" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.src_blend[index] = alpha_blend_factor_define::instance().from_str(value_str);
				if (0xFFFFFFFF == bs_desc.src_blend[index])
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("dest_blend" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.dest_blend[index] = alpha_blend_factor_define::instance().from_str(value_str);
				if (0xFFFFFFFF == bs_desc.dest_blend[index])
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("blend_op_alpha" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.blend_op_alpha[index] = blend_operation_define::instance().from_str(value_str);
				if (0xFFFFFFFF == bs_desc.blend_op_alpha[index])
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("src_blend_alpha" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.src_blend_alpha[index] = alpha_blend_factor_define::instance().from_str(value_str);
				if (0xFFFFFFFF == bs_desc.src_blend_alpha[index])
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("dest_blend_alpha" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.dest_blend_alpha[index] = alpha_blend_factor_define::instance().from_str(value_str);
				if (0xFFFFFFFF == bs_desc.dest_blend_alpha[index])
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("color_write_mask" == state_name)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->first_attribute("value")->value();
				bs_desc.color_write_mask[index] = static_cast<uint8_t>(lexical_cast<uint32_t>(value_str));
			}
			else if ("blend_factor" == state_name)
			{
				xml_attribute<>* attr = state_node->first_attribute("r");
				if (attr != NULL)
				{
					blend_factor_.r() = lexical_cast<float>(attr->value());
				}
				attr = state_node->first_attribute("g");
				if (attr != NULL)
				{
					blend_factor_.g() = lexical_cast<float>(attr->value());
				}
				attr = state_node->first_attribute("b");
				if (attr != NULL)
				{
					blend_factor_.b() = lexical_cast<float>(attr->value());
				}
				attr = state_node->first_attribute("a");
				if (attr != NULL)
				{
					blend_factor_.a() = lexical_cast<float>(attr->value());
				}
			}
			else if ("sample_mask" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				sample_mask_ = lexical_cast<uint32_t>(value_str);
			}
			else if ("depth_enable" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.depth_enable = bool_from_str(value_str);
			}
			else if ("depth_write_mask" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.depth_write_mask = bool_from_str(value_str);
			}
			else if ("depth_func" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.depth_func = compare_function_define::instance().from_str(value_str);
				if (0xFFFFFFFF == dss_desc.depth_func)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("front_stencil_enable" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.front_stencil_enable = bool_from_str(value_str);
			}
			else if ("front_stencil_func" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.front_stencil_func = compare_function_define::instance().from_str(value_str);
				if (0xFFFFFFFF == dss_desc.front_stencil_func)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("front_stencil_ref" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				front_stencil_ref_ = lexical_cast<uint16_t>(value_str);
			}
			else if ("front_stencil_read_mask" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.front_stencil_read_mask = lexical_cast<uint16_t>(value_str);
			}
			else if ("front_stencil_write_mask" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.front_stencil_write_mask = lexical_cast<uint16_t>(value_str);
			}
			else if ("front_stencil_fail" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.front_stencil_fail = stencil_operation_define::instance().from_str(value_str);
				if (0xFFFFFFFF == dss_desc.front_stencil_fail)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("front_stencil_depth_fail" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.front_stencil_depth_fail = stencil_operation_define::instance().from_str(value_str);
				if (0xFFFFFFFF == dss_desc.front_stencil_depth_fail)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("front_stencil_pass" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.front_stencil_pass = stencil_operation_define::instance().from_str(value_str);
				if (0xFFFFFFFF == dss_desc.front_stencil_pass)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("back_stencil_enable" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.back_stencil_enable = bool_from_str(value_str);
			}
			else if ("back_stencil_func" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.back_stencil_func = compare_function_define::instance().from_str(value_str);
				if (0xFFFFFFFF == dss_desc.back_stencil_func)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("back_stencil_ref" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				back_stencil_ref_ = lexical_cast<uint16_t>(value_str);
			}
			else if ("back_stencil_read_mask" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.back_stencil_read_mask = lexical_cast<uint16_t>(value_str);
			}
			else if ("back_stencil_write_mask" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.back_stencil_write_mask = lexical_cast<uint16_t>(value_str);
			}
			else if ("back_stencil_fail" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.back_stencil_fail = stencil_operation_define::instance().from_str(value_str);
				if (0xFFFFFFFF == dss_desc.back_stencil_fail)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("back_stencil_depth_fail" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.back_stencil_depth_fail = stencil_operation_define::instance().from_str(value_str);
				if (0xFFFFFFFF == dss_desc.back_stencil_depth_fail)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("back_stencil_pass" == state_name)
			{
				std::string value_str = state_node->first_attribute("value")->value();
				dss_desc.back_stencil_pass = stencil_operation_define::instance().from_str(value_str);
				if (0xFFFFFFFF == dss_desc.back_stencil_pass)
				{
					cerr << "Wrong " << state_name <<  " state: " << value_str << endl;
				}
			}
			else if ("vertex_shader" == state_name)
			{
				shader_desc& sd = (*shader_descs_)[ShaderObject::ST_VertexShader];
				sd.profile = get_profile(state_node);
				sd.func_name = get_func_name(state_node);
			}
			else if ("pixel_shader" == state_name)
			{
				shader_desc& sd = (*shader_descs_)[ShaderObject::ST_PixelShader];
				sd.profile = get_profile(state_node);
				sd.func_name = get_func_name(state_node);
			}
			else if ("geometry_shader" == state_name)
			{
				shader_desc& sd = (*shader_descs_)[ShaderObject::ST_GeometryShader];
				sd.profile = get_profile(state_node);
				sd.func_name = get_func_name(state_node);
			}
			else
			{
				cerr << "Wrong state name: " << state_name << endl;
			}
		}

		rasterizer_state_obj_ = rf.MakeRasterizerStateObject(rs_desc);
		depth_stencil_state_obj_ = rf.MakeDepthStencilStateObject(dss_desc);
		blend_state_obj_ = rf.MakeBlendStateObject(bs_desc);

		shader_obj_->SetShader(effect_, shader_descs_);

		is_validate_ = shader_obj_->Validate();
	}

	RenderPassPtr RenderPass::Clone(RenderEffect& effect)
	{
		RenderPassPtr ret = MakeSharedPtr<RenderPass>(effect);

		ret->name_ = name_;
		ret->annotations_ = annotations_;
		ret->shader_descs_ = shader_descs_;

		ret->rasterizer_state_obj_ = rasterizer_state_obj_;
		ret->depth_stencil_state_obj_ = depth_stencil_state_obj_;
		ret->blend_state_obj_ = blend_state_obj_;
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

	void RenderEffectParameter::Load(rapidxml::xml_node<>* node)
	{
		using boost::lexical_cast;
		using namespace rapidxml;

		type_ = type_define::instance().type_code(node->first_attribute("type")->value());
		name_ = MakeSharedPtr<BOOST_TYPEOF(*name_)>(std::string(node->first_attribute("name")->value()));

		xml_attribute<>* attr = node->first_attribute("semantic");
		if (attr != NULL)
		{
			semantic_ = MakeSharedPtr<BOOST_TYPEOF(*semantic_)>(std::string(attr->value()));
		}

		attr = node->first_attribute("array_size");
		if (attr != NULL)
		{
			array_size_ = lexical_cast<uint32_t>(attr->value());
		}
		else
		{
			array_size_ = 0;
		}
		var_ = read_var(node, type_, array_size_);

		{
			xml_node<>* anno_node = node->first_node("annotation");
			if (anno_node != NULL)
			{
				annotations_ = MakeSharedPtr<BOOST_TYPEOF(*annotations_)>();
				for (; anno_node; anno_node = anno_node->next_sibling("annotation"))
				{
					RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
					annotations_->push_back(annotation);

					annotation->Load(anno_node);
				}
			}
		}

		if (annotations_ && ((REDT_texture1D == type_) || (REDT_texture2D == type_) || (REDT_texture3D == type_) || (REDT_textureCUBE == type_)))
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


	void RenderShaderFunc::Load(rapidxml::xml_node<>* node)
	{
		using namespace rapidxml;
		for (xml_node<>* shader_text_node = node->first_node(); shader_text_node; shader_text_node = shader_text_node->next_sibling())
		{
			if ((node_comment == shader_text_node->type()) || (node_cdata == shader_text_node->type()))
			{
				str_ += std::string(shader_text_node->value(), shader_text_node->value_size());
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

	void RenderVariable::Value(int32_t& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(float& /*value*/) const
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

	void RenderVariable::Value(std::vector<int32_t>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<float>& /*value*/) const
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
