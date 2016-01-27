// RenderEffect.cpp
// KlayGE 渲染效果类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Thead-safe singleton (2010.7.6)
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
#include <KFL/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KFL/XMLDom.hpp>
#include <KFL/Thread.hpp>

#include <fstream>
#include <boost/assert.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>

#include <KlayGE/RenderEffect.hpp>

namespace
{
	using namespace KlayGE;

	uint32_t const KFX_VERSION = 0x0107;

	std::mutex singleton_mutex;

	class type_define
	{
	public:
		static type_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<type_define>();
				}
			}
			return *instance_;
		}

		uint32_t type_code(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < types_hash_.size(); ++ i)
			{
				if (types_hash_[i] == name_hash)
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

			types_hash_.resize(types_.size());
			for (size_t i = 0; i < types_.size(); ++ i)
			{
				types_hash_[i] = RT_HASH(types_[i].c_str());
			}
		}

	private:
		std::vector<std::string> types_;
		std::vector<size_t> types_hash_;

		static std::unique_ptr<type_define> instance_;
	};
	std::unique_ptr<type_define> type_define::instance_;

	class shade_mode_define
	{
	public:
		static shade_mode_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<shade_mode_define>();
				}
			}
			return *instance_;
		}

		ShadeMode from_str(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < sms_hash_.size(); ++ i)
			{
				if (sms_hash_[i] == name_hash)
				{
					return static_cast<ShadeMode>(i);
				}
			}
			BOOST_ASSERT(false);
			LogError("Wrong ShadeMode name: %s", name.c_str());
			return static_cast<ShadeMode>(0xFFFFFFFF);
		}

		shade_mode_define()
		{
			sms_hash_.push_back(CT_HASH("flat"));
			sms_hash_.push_back(CT_HASH("gouraud"));
		}

	private:
		std::vector<size_t> sms_hash_;

		static std::unique_ptr<shade_mode_define> instance_;
	};
	std::unique_ptr<shade_mode_define> shade_mode_define::instance_;

	class compare_function_define
	{
	public:
		static compare_function_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<compare_function_define>();
				}
			}
			return *instance_;
		}

		CompareFunction from_str(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < cfs_hash_.size(); ++ i)
			{
				if (cfs_hash_[i] == name_hash)
				{
					return static_cast<CompareFunction>(i);
				}
			}
			BOOST_ASSERT(false);
			LogError("Wrong CompareFunction name: %s", name.c_str());
			return static_cast<CompareFunction>(0xFFFFFFFF);
		}

		compare_function_define()
		{
			cfs_hash_.push_back(CT_HASH("always_fail"));
			cfs_hash_.push_back(CT_HASH("always_pass"));
			cfs_hash_.push_back(CT_HASH("less"));
			cfs_hash_.push_back(CT_HASH("less_equal"));
			cfs_hash_.push_back(CT_HASH("equal"));
			cfs_hash_.push_back(CT_HASH("not_equal"));
			cfs_hash_.push_back(CT_HASH("greater_equal"));
			cfs_hash_.push_back(CT_HASH("greater"));
		}

	private:
		std::vector<size_t> cfs_hash_;

		static std::unique_ptr<compare_function_define> instance_;
	};
	std::unique_ptr<compare_function_define> compare_function_define::instance_;

	class cull_mode_define
	{
	public:
		static cull_mode_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<cull_mode_define>();
				}
			}
			return *instance_;
		}

		CullMode from_str(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < cms_hash_.size(); ++ i)
			{
				if (cms_hash_[i] == name_hash)
				{
					return static_cast<CullMode>(i);
				}
			}
			BOOST_ASSERT(false);
			LogError("Wrong CullMode name: %s", name.c_str());
			return static_cast<CullMode>(0xFFFFFFFF);
		}

		cull_mode_define()
		{
			cms_hash_.push_back(CT_HASH("none"));
			cms_hash_.push_back(CT_HASH("front"));
			cms_hash_.push_back(CT_HASH("back"));
		}

	private:
		std::vector<size_t> cms_hash_;

		static std::unique_ptr<cull_mode_define> instance_;
	};
	std::unique_ptr<cull_mode_define> cull_mode_define::instance_;

	class polygon_mode_define
	{
	public:
		static polygon_mode_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<polygon_mode_define>();
				}
			}
			return *instance_;
		}

		PolygonMode from_str(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < pms_hash_.size(); ++ i)
			{
				if (pms_hash_[i] == name_hash)
				{
					return static_cast<PolygonMode>(i);
				}
			}
			BOOST_ASSERT(false);
			LogError("Wrong PolygonMode name: %s", name.c_str());
			return static_cast<PolygonMode>(0xFFFFFFFF);
		}

		polygon_mode_define()
		{
			pms_hash_.push_back(CT_HASH("point"));
			pms_hash_.push_back(CT_HASH("line"));
			pms_hash_.push_back(CT_HASH("fill"));
		}

	private:
		std::vector<size_t> pms_hash_;

		static std::unique_ptr<polygon_mode_define> instance_;
	};
	std::unique_ptr<polygon_mode_define> polygon_mode_define::instance_;

	class alpha_blend_factor_define
	{
	public:
		static alpha_blend_factor_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<alpha_blend_factor_define>();
				}
			}
			return *instance_;
		}

		AlphaBlendFactor from_str(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < abfs_hash_.size(); ++ i)
			{
				if (abfs_hash_[i] == name_hash)
				{
					return static_cast<AlphaBlendFactor>(i);
				}
			}
			BOOST_ASSERT(false);
			LogError("Wrong AlphaBlendFactor name: %s", name.c_str());
			return static_cast<AlphaBlendFactor>(0xFFFFFFFF);
		}

		alpha_blend_factor_define()
		{
			abfs_hash_.push_back(CT_HASH("zero"));
			abfs_hash_.push_back(CT_HASH("one"));
			abfs_hash_.push_back(CT_HASH("src_alpha"));
			abfs_hash_.push_back(CT_HASH("dst_alpha"));
			abfs_hash_.push_back(CT_HASH("inv_src_alpha"));
			abfs_hash_.push_back(CT_HASH("inv_dst_alpha"));
			abfs_hash_.push_back(CT_HASH("src_color"));
			abfs_hash_.push_back(CT_HASH("dst_color"));
			abfs_hash_.push_back(CT_HASH("inv_src_color"));
			abfs_hash_.push_back(CT_HASH("inv_dst_color"));
			abfs_hash_.push_back(CT_HASH("src_alpha_sat"));
			abfs_hash_.push_back(CT_HASH("blend_factor"));
			abfs_hash_.push_back(CT_HASH("inv_blend_factor"));
			abfs_hash_.push_back(CT_HASH("src1_alpha"));
			abfs_hash_.push_back(CT_HASH("inv_src1_alpha"));
			abfs_hash_.push_back(CT_HASH("src1_color"));
			abfs_hash_.push_back(CT_HASH("inv_src1_color"));
		}

	private:
		std::vector<size_t> abfs_hash_;

		static std::unique_ptr<alpha_blend_factor_define> instance_;
	};
	std::unique_ptr<alpha_blend_factor_define> alpha_blend_factor_define::instance_;

	class blend_operation_define
	{
	public:
		static blend_operation_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<blend_operation_define>();
				}
			}
			return *instance_;
		}

		BlendOperation from_str(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < bops_hash_.size(); ++ i)
			{
				if (bops_hash_[i] == name_hash)
				{
					return static_cast<BlendOperation>(i + 1);
				}
			}
			BOOST_ASSERT(false);
			LogError("Wrong BlendOperation name: %s", name.c_str());
			return static_cast<BlendOperation>(0xFFFFFFFF);
		}

		blend_operation_define()
		{
			bops_hash_.push_back(CT_HASH("add"));
			bops_hash_.push_back(CT_HASH("sub"));
			bops_hash_.push_back(CT_HASH("rev_sub"));
			bops_hash_.push_back(CT_HASH("min"));
			bops_hash_.push_back(CT_HASH("max"));
		}

	private:
		std::vector<size_t> bops_hash_;

		static std::unique_ptr<blend_operation_define> instance_;
	};
	std::unique_ptr<blend_operation_define> blend_operation_define::instance_;

	class stencil_operation_define
	{
	public:
		static stencil_operation_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<stencil_operation_define>();
				}
			}
			return *instance_;
		}

		StencilOperation from_str(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < sops_hash_.size(); ++ i)
			{
				if (sops_hash_[i] == name_hash)
				{
					return static_cast<StencilOperation>(i);
				}
			}
			BOOST_ASSERT(false);
			LogError("Wrong StencilOperation name: %s", name.c_str());
			return static_cast<StencilOperation>(0xFFFFFFFF);
		}

		stencil_operation_define()
		{
			sops_hash_.push_back(CT_HASH("keep"));
			sops_hash_.push_back(CT_HASH("zero"));
			sops_hash_.push_back(CT_HASH("replace"));
			sops_hash_.push_back(CT_HASH("incr"));
			sops_hash_.push_back(CT_HASH("decr"));
			sops_hash_.push_back(CT_HASH("invert"));
			sops_hash_.push_back(CT_HASH("incr_wrap"));
			sops_hash_.push_back(CT_HASH("decr_wrap"));
		}

	private:
		std::vector<size_t> sops_hash_;

		static std::unique_ptr<stencil_operation_define> instance_;
	};
	std::unique_ptr<stencil_operation_define> stencil_operation_define::instance_;

	class texture_filter_mode_define
	{
	public:
		static texture_filter_mode_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<texture_filter_mode_define>();
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
			size_t const f_hash = RT_HASH(f.c_str());
			for (uint32_t i = 0; i < tfs_hash_.size(); ++ i)
			{
				if (tfs_hash_[i] == f_hash)
				{
					return static_cast<TexFilterOp>((cmp << 4) + i);
				}
			}
			if (CT_HASH("anisotropic") == f_hash)
			{
				return static_cast<TexFilterOp>((cmp << 4) + TFO_Anisotropic);
			}
			BOOST_ASSERT(false);
			LogError("Wrong TexFilterOp name: %s", name.c_str());
			return static_cast<TexFilterOp>(0xFFFFFFFF);
		}

		texture_filter_mode_define()
		{
			tfs_hash_.push_back(CT_HASH("min_mag_mip_point"));
			tfs_hash_.push_back(CT_HASH("min_mag_point_mip_linear"));
			tfs_hash_.push_back(CT_HASH("min_point_mag_linear_mip_point"));
			tfs_hash_.push_back(CT_HASH("min_point_mag_mip_linear"));
			tfs_hash_.push_back(CT_HASH("min_linear_mag_mip_point"));
			tfs_hash_.push_back(CT_HASH("min_linear_mag_point_mip_linear"));
			tfs_hash_.push_back(CT_HASH("min_mag_linear_mip_point"));
			tfs_hash_.push_back(CT_HASH("min_mag_mip_linear"));
		}

	private:
		std::vector<size_t> tfs_hash_;

		static std::unique_ptr<texture_filter_mode_define> instance_;
	};
	std::unique_ptr<texture_filter_mode_define> texture_filter_mode_define::instance_;

	class texture_addr_mode_define
	{
	public:
		static texture_addr_mode_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<texture_addr_mode_define>();
				}
			}
			return *instance_;
		}

		TexAddressingMode from_str(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < tams_hash_.size(); ++ i)
			{
				if (tams_hash_[i] == name_hash)
				{
					return static_cast<TexAddressingMode>(i);
				}
			}
			BOOST_ASSERT(false);
			LogError("Wrong TexAddressingMode name: %s", name.c_str());
			return static_cast<TexAddressingMode>(0xFFFFFFFF);
		}

		texture_addr_mode_define()
		{
			tams_hash_.push_back(CT_HASH("wrap"));
			tams_hash_.push_back(CT_HASH("mirror"));
			tams_hash_.push_back(CT_HASH("clamp"));
			tams_hash_.push_back(CT_HASH("border"));
		}

	private:
		std::vector<size_t> tams_hash_;

		static std::unique_ptr<texture_addr_mode_define> instance_;
	};
	std::unique_ptr<texture_addr_mode_define> texture_addr_mode_define::instance_;

	class logic_operation_define
	{
	public:
		static logic_operation_define& instance()
		{
			if (!instance_)
			{
				std::lock_guard<std::mutex> lock(singleton_mutex);
				if (!instance_)
				{
					instance_ = MakeUniquePtr<logic_operation_define>();
				}
			}
			return *instance_;
		}

		LogicOperation from_str(std::string const & name) const
		{
			size_t const name_hash = RT_HASH(name.c_str());
			for (uint32_t i = 0; i < lops_hash_.size(); ++ i)
			{
				if (lops_hash_[i] == name_hash)
				{
					return static_cast<LogicOperation>(i);
				}
			}
			BOOST_ASSERT(false);
			LogError("Wrong LogicOperation name: %s", name.c_str());
			return static_cast<LogicOperation>(0xFFFFFFFF);
		}

		logic_operation_define()
		{
			lops_hash_.push_back(CT_HASH("clear"));
			lops_hash_.push_back(CT_HASH("set"));
			lops_hash_.push_back(CT_HASH("copy"));
			lops_hash_.push_back(CT_HASH("copy_inverted"));
			lops_hash_.push_back(CT_HASH("noop"));
			lops_hash_.push_back(CT_HASH("invert"));
			lops_hash_.push_back(CT_HASH("and"));
			lops_hash_.push_back(CT_HASH("nand"));
			lops_hash_.push_back(CT_HASH("or"));
			lops_hash_.push_back(CT_HASH("nor"));
			lops_hash_.push_back(CT_HASH("xor"));
			lops_hash_.push_back(CT_HASH("equiv"));
			lops_hash_.push_back(CT_HASH("and_reverse"));
			lops_hash_.push_back(CT_HASH("and_inverted"));
			lops_hash_.push_back(CT_HASH("or_reverse"));
			lops_hash_.push_back(CT_HASH("or_inverted"));
		}

	private:
		std::vector<size_t> lops_hash_;

		static std::unique_ptr<logic_operation_define> instance_;
	};
	std::unique_ptr<logic_operation_define> logic_operation_define::instance_;

#if KLAYGE_IS_DEV_PLATFORM
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
					size_t const name_hash = RT_HASH(name.c_str());

					if (CT_HASH("filtering") == name_hash)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.filter = texture_filter_mode_define::instance().from_str(value_str);
					}
					else if (CT_HASH("address_u") == name_hash)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.addr_mode_u = texture_addr_mode_define::instance().from_str(value_str);
					}
					else if (CT_HASH("address_v") == name_hash)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.addr_mode_v = texture_addr_mode_define::instance().from_str(value_str);
					}
					else if (CT_HASH("address_w") == name_hash)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.addr_mode_w = texture_addr_mode_define::instance().from_str(value_str);
					}
					else if (CT_HASH("max_anisotropy") == name_hash)
					{
						desc.max_anisotropy = static_cast<uint8_t>(state_node->Attrib("value")->ValueUInt());
					}
					else if (CT_HASH("min_lod") == name_hash)
					{
						desc.min_lod = state_node->Attrib("value")->ValueFloat();
					}
					else if (CT_HASH("max_lod") == name_hash)
					{
						desc.max_lod = state_node->Attrib("value")->ValueFloat();
					}
					else if (CT_HASH("mip_map_lod_bias") == name_hash)
					{
						desc.mip_map_lod_bias = state_node->Attrib("value")->ValueFloat();
					}
					else if (CT_HASH("cmp_func") == name_hash)
					{
						std::string value_str = state_node->Attrib("value")->ValueString();
						desc.cmp_func = compare_function_define::instance().from_str(value_str);
					}
					else if (CT_HASH("border_clr") == name_hash)
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
						BOOST_ASSERT(false);
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
						attr = node->Attrib(std::string("_")
							+ static_cast<char>('0' + y) + static_cast<char>('0' + x));
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
#endif

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

				var = MakeSharedPtr<RenderVariableUInt>();
				*var = LE2Native(tmp);
			}
			else
			{
				var = MakeSharedPtr<RenderVariableUIntArray>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<uint32_t> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						init_val[i] = LE2Native(init_val[i]);
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

				var = MakeSharedPtr<RenderVariableInt>();
				*var = LE2Native(tmp);
			}
			else
			{
				var = MakeSharedPtr<RenderVariableIntArray>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<int32_t> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						init_val[i] = LE2Native(init_val[i]);
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
				desc.border_clr[0] = LE2Native(desc.border_clr[0]);
				desc.border_clr[1] = LE2Native(desc.border_clr[1]);
				desc.border_clr[2] = LE2Native(desc.border_clr[2]);
				desc.border_clr[3] = LE2Native(desc.border_clr[3]);
				desc.addr_mode_u = LE2Native(desc.addr_mode_u);
				desc.addr_mode_v = LE2Native(desc.addr_mode_v);
				desc.addr_mode_w = LE2Native(desc.addr_mode_w);
				desc.filter = LE2Native(desc.filter);
				desc.min_lod = LE2Native(desc.min_lod);
				desc.max_lod = LE2Native(desc.max_lod);
				desc.mip_map_lod_bias = LE2Native(desc.mip_map_lod_bias);
				desc.cmp_func = LE2Native(desc.cmp_func);

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

				var = MakeSharedPtr<RenderVariableFloat>();
				*var = LE2Native(tmp);
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloatArray>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<float> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						init_val[i] = LE2Native(init_val[i]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableUInt2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt2Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<int2> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableUInt3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt3Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<int3> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableUInt4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt4Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<int4> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableInt2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt2Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<int2> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableInt3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt3Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<int3> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableInt4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableInt4Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<int4> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableFloat2>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat2Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<float2> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableFloat3>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat3Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<float3> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableFloat4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat4Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<float4> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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
					tmp[i] = LE2Native(tmp[i]);
				}

				var = MakeSharedPtr<RenderVariableFloat4x4>();
				*var = tmp;
			}
			else
			{
				var = MakeSharedPtr<RenderVariableFloat4x4Array>();

				uint32_t len;
				res->read(&len, sizeof(len));
				len = LE2Native(len);
				if (len > 0)
				{
					std::vector<float4x4> init_val(len);
					res->read(&init_val[0], len * sizeof(init_val[0]));
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 16; ++ j)
						{
							init_val[i][j] = LE2Native(init_val[i][j]);
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

#if KLAYGE_IS_DEV_PLATFORM
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

				tmp = Native2LE(tmp);
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<uint32_t> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						init_val[i] = Native2LE(init_val[i]);
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

				tmp = Native2LE(tmp);
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int32_t> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						init_val[i] = Native2LE(init_val[i]);
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
				desc.border_clr[0] = Native2LE(desc.border_clr[0]);
				desc.border_clr[1] = Native2LE(desc.border_clr[1]);
				desc.border_clr[2] = Native2LE(desc.border_clr[2]);
				desc.border_clr[3] = Native2LE(desc.border_clr[3]);
				desc.addr_mode_u = Native2LE(desc.addr_mode_u);
				desc.addr_mode_v = Native2LE(desc.addr_mode_v);
				desc.addr_mode_w = Native2LE(desc.addr_mode_w);
				desc.filter = Native2LE(desc.filter);
				desc.min_lod = Native2LE(desc.min_lod);
				desc.max_lod = Native2LE(desc.max_lod);
				desc.mip_map_lod_bias = Native2LE(desc.mip_map_lod_bias);
				desc.cmp_func = Native2LE(desc.cmp_func);
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

				tmp = Native2LE(tmp);
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						init_val[i] = Native2LE(init_val[i]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int2> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int3> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int4> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int2> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int3> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<int4> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float2> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 2; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float3> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 3; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float4> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 4; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
					tmp[i] = Native2LE(tmp[i]);
				}
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			else
			{
				std::vector<float4x4> init_val;
				var->Value(init_val);

				uint32_t len = static_cast<uint32_t>(init_val.size());
				len = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				if (len > 0)
				{
					for (uint32_t i = 0; i < len; ++ i)
					{
						for (int j = 0; j < 16; ++ j)
						{
							init_val[i][j] = Native2LE(init_val[i][j]);
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
#endif
}

namespace KlayGE
{
	class EffectLoadingDesc : public ResLoadingDesc
	{
	private:
		struct EffectDesc
		{
			std::string res_name;

			RenderEffectPtr effect;
		};

	public:
		explicit EffectLoadingDesc(std::string const & name)
		{
			effect_desc_.res_name = name;
		}

		uint64_t Type() const
		{
			static uint64_t const type = CT_HASH("EffectLoadingDesc");
			return type;
		}

		bool StateLess() const
		{
			return false;
		}

		void SubThreadStage()
		{
		}

		std::shared_ptr<void> MainThreadStage()
		{
			effect_desc_.effect = MakeSharedPtr<RenderEffect>();
			effect_desc_.effect->Load(effect_desc_.res_name);
			return std::static_pointer_cast<void>(effect_desc_.effect);
		}

		bool HasSubThreadStage() const
		{
			return false;
		}

		bool Match(ResLoadingDesc const & rhs) const
		{
			if (this->Type() == rhs.Type())
			{
				EffectLoadingDesc const & eld = static_cast<EffectLoadingDesc const &>(rhs);
				return (effect_desc_.res_name == eld.effect_desc_.res_name);
			}
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs)
		{
			BOOST_ASSERT(this->Type() == rhs.Type());

			EffectLoadingDesc const & eld = static_cast<EffectLoadingDesc const &>(rhs);
			effect_desc_.res_name = eld.effect_desc_.res_name;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource)
		{
			effect_desc_.effect = std::static_pointer_cast<RenderEffect>(resource)->Clone();
			return std::static_pointer_cast<void>(effect_desc_.effect);
		}

		virtual std::shared_ptr<void> Resource() const override
		{
			return effect_desc_.effect;
		}

	private:
		EffectDesc effect_desc_;
	};


#if KLAYGE_IS_DEV_PLATFORM
	void RenderEffectAnnotation::Load(XMLNodePtr const & node)
	{
		type_ = type_define::instance().type_code(node->Attrib("type")->ValueString());
		name_ = node->Attrib("name")->ValueString();
		var_ = read_var(node, type_, 0);
	}
#endif

	void RenderEffectAnnotation::StreamIn(ResIdentifierPtr const & res)
	{
		res->read(&type_, sizeof(type_));
		type_ = LE2Native(type_);
		name_ = ReadShortString(res);
		var_ = stream_in_var(res, type_, 0);
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderEffectAnnotation::StreamOut(std::ostream& os)
	{
		uint32_t t = Native2LE(type_);
		os.write(reinterpret_cast<char const *>(&t), sizeof(t));
		WriteShortString(os, name_);
		stream_out_var(os, var_, type_, 0);
	}
#endif


	RenderEffect::RenderEffect()
	{
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderEffect::RecursiveIncludeNode(XMLNodePtr const & root, std::vector<std::string>& include_names) const
	{
		for (XMLNodePtr node = root->FirstNode("include"); node; node = node->NextSibling("include"))
		{
			XMLAttributePtr attr = node->Attrib("name");
			BOOST_ASSERT(attr);

			std::string include_name = attr->ValueString();

			XMLDocument include_doc;
			XMLNodePtr include_root = include_doc.Parse(ResLoader::Instance().Open(include_name));
			this->RecursiveIncludeNode(include_root, include_names);

			bool found = false;
			for (size_t i = 0; i < include_names.size(); ++ i)
			{
				if (include_name == include_names[i])
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				include_names.push_back(include_name);
			}
		}
	}

	void RenderEffect::InsertIncludeNodes(XMLDocument& target_doc, XMLNodePtr const & target_root,
		XMLNodePtr const & target_place, XMLNodePtr const & include_root) const
	{
		for (XMLNodePtr child_node = include_root->FirstNode(); child_node; child_node = child_node->NextSibling())
		{
			if ((XNT_Element == child_node->Type()) && (child_node->Name() != "include"))
			{
				target_root->InsertNode(target_place, target_doc.CloneNode(child_node));
			}
		}
	}
#endif

	void RenderEffect::Load(std::string const & name)
	{
		std::string fxml_name = ResLoader::Instance().Locate(name);
		if (fxml_name.empty())
		{
			fxml_name = name;
		}
		std::string kfx_name = fxml_name.substr(0, fxml_name.rfind(".")) + ".kfx";

#if KLAYGE_IS_DEV_PLATFORM
		ResIdentifierPtr source = ResLoader::Instance().Open(fxml_name);
#endif
		ResIdentifierPtr kfx_source = ResLoader::Instance().Open(kfx_name);

#if KLAYGE_IS_DEV_PLATFORM
		XMLDocumentPtr doc;
		XMLNodePtr root;
#endif

		res_name_ = MakeSharedPtr<std::remove_reference<decltype(*res_name_)>::type>();
		res_name_->first = fxml_name;
		res_name_->second = boost::hash_range(fxml_name.begin(), fxml_name.end());
#if KLAYGE_IS_DEV_PLATFORM
		if (source)
		{
			timestamp_ = source->Timestamp();

			doc = MakeSharedPtr<XMLDocument>();
			root = doc->Parse(source);

			std::vector<std::string> include_names;
			this->RecursiveIncludeNode(root, include_names);

			for (auto const & include_name : include_names)
			{
				ResIdentifierPtr include_source = ResLoader::Instance().Open(include_name);
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
#endif

		if (!this->StreamIn(kfx_source))
		{
#if KLAYGE_IS_DEV_PLATFORM
			if (source)
			{
				shader_descs_.reset();
				macros_.reset();
				cbuffers_.clear();
				params_.clear();
				shader_frags_.reset();
				hlsl_shader_.reset();
				techniques_.clear();

				shader_descs_ = MakeSharedPtr<std::remove_reference<decltype(*shader_descs_)>::type>(1);

				XMLAttributePtr attr;

				std::vector<XMLDocumentPtr> include_docs;
				std::vector<std::string> whole_include_names;
				for (XMLNodePtr node = root->FirstNode("include"); node;)
				{
					attr = node->Attrib("name");
					BOOST_ASSERT(attr);
					std::string include_name = attr->ValueString();

					include_docs.push_back(MakeSharedPtr<XMLDocument>());
					XMLNodePtr include_root = include_docs.back()->Parse(ResLoader::Instance().Open(include_name));

					std::vector<std::string> include_names;
					this->RecursiveIncludeNode(include_root, include_names);

					if (!include_names.empty())
					{
						for (auto iter = include_names.begin(); iter != include_names.end();)
						{
							bool found = false;
							for (auto iter_w = whole_include_names.begin(); iter_w != whole_include_names.end(); ++ iter_w)
							{
								if (*iter == *iter_w)
								{
									found = true;
									break;
								}
							}

							if (found)
							{
								iter = include_names.erase(iter);
							}
							else
							{
								include_docs.push_back(MakeSharedPtr<XMLDocument>());
								XMLNodePtr recursive_include_root = include_docs.back()->Parse(ResLoader::Instance().Open(*iter));
								this->InsertIncludeNodes(*doc, root, node, recursive_include_root);

								whole_include_names.push_back(*iter);
								++ iter;
							}
						}
					}

					bool found = false;
					for (auto iter_w = whole_include_names.begin(); iter_w != whole_include_names.end(); ++ iter_w)
					{
						if (include_name == *iter_w)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						this->InsertIncludeNodes(*doc, root, node, include_root);
						whole_include_names.push_back(include_name);
					}

					XMLNodePtr node_next = node->NextSibling("include");
					root->RemoveNode(node);
					node = node_next;
				}

				{
					XMLNodePtr macro_node = root->FirstNode("macro");
					if (macro_node)
					{
						macros_ = MakeSharedPtr<std::remove_reference<decltype(*macros_)>::type>();
					}
					for (; macro_node; macro_node = macro_node->NextSibling("macro"))
					{
						macros_->emplace_back(std::make_pair(macro_node->Attrib("name")->ValueString(), macro_node->Attrib("value")->ValueString()), true);
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
						&& (type != REDT_texture1D) && (type != REDT_texture2D) && (type != REDT_texture3D)
						&& (type != REDT_textureCUBE)
						&& (type != REDT_texture1DArray) && (type != REDT_texture2DArray)
						&& (type != REDT_texture3DArray) && (type != REDT_textureCUBEArray)
						&& (type != REDT_buffer) && (type != REDT_structured_buffer)
						&& (type != REDT_byte_address_buffer) && (type != REDT_rw_buffer)
						&& (type != REDT_rw_structured_buffer) && (type != REDT_rw_texture1D)
						&& (type != REDT_rw_texture2D) && (type != REDT_rw_texture3D)
						&& (type != REDT_rw_texture1DArray) && (type != REDT_rw_texture2DArray)
						&& (type != REDT_rw_byte_address_buffer) && (type != REDT_append_structured_buffer)
						&& (type != REDT_consume_structured_buffer))
					{
						RenderEffectConstantBufferPtr cbuff;
						XMLNodePtr parent_node = node->Parent();
						std::string cbuff_name = parent_node->AttribString("name", "global_cb");
						size_t const cbuff_name_hash = RT_HASH(cbuff_name.c_str());

						bool found = false;
						for (size_t i = 0; i < cbuffers_.size(); ++ i)
						{
							if (cbuffers_[i]->NameHash() == cbuff_name_hash)
							{
								cbuff = cbuffers_[i];
								found = true;
								break;
							}
						}
						if (!found)
						{
							cbuff = MakeSharedPtr<RenderEffectConstantBuffer>();
							cbuff->Load(cbuff_name);
							cbuffers_.push_back(cbuff);
						}

						cbuff->AddParameter(param_index);
					}

					RenderEffectParameterPtr param = MakeSharedPtr<RenderEffectParameter>();
					params_.push_back(param);

					param->Load(node);
				}

				{
					XMLNodePtr shader_node = root->FirstNode("shader");
					if (shader_node)
					{
						shader_frags_ = MakeSharedPtr<std::remove_reference<decltype(*shader_frags_)>::type>();
						for (; shader_node; shader_node = shader_node->NextSibling("shader"))
						{
							shader_frags_->push_back(RenderShaderFragment());
							shader_frags_->back().Load(shader_node);
						}
					}
				}

				this->GenHLSLShaderText();

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
#endif
		}
	}

	bool RenderEffect::StreamIn(ResIdentifierPtr const & source)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		bool ret = false;
		if (source)
		{
			uint32_t fourcc;
			source->read(&fourcc, sizeof(fourcc));
			fourcc = LE2Native(fourcc);
			
			uint32_t ver;
			source->read(&ver, sizeof(ver));
			ver = LE2Native(ver);
			
			if ((MakeFourCC<'K', 'F', 'X', ' '>::value == fourcc) && (KFX_VERSION == ver))
			{
				uint32_t shader_fourcc;
				source->read(&shader_fourcc, sizeof(shader_fourcc));
				shader_fourcc = LE2Native(shader_fourcc);

				uint32_t shader_ver;
				source->read(&shader_ver, sizeof(shader_ver));
				shader_ver = LE2Native(shader_ver);

				if ((re.NativeShaderFourCC() == shader_fourcc) && (re.NativeShaderVersion() == shader_ver))
				{
					uint64_t timestamp;
					source->read(&timestamp, sizeof(timestamp));
#if KLAYGE_IS_DEV_PLATFORM
					timestamp = LE2Native(timestamp);
					if (timestamp_ <= timestamp)
#endif
					{
						shader_descs_ = MakeSharedPtr<std::remove_reference<decltype(*shader_descs_)>::type>(1);

						{
							uint16_t num_macros;
							source->read(&num_macros, sizeof(num_macros));
							num_macros = LE2Native(num_macros);

							if (num_macros > 0)
							{
								macros_ = MakeSharedPtr<std::remove_reference<decltype(*macros_)>::type>();
							}
							for (uint32_t i = 0; i < num_macros; ++ i)
							{
								std::string name = ReadShortString(source);
								std::string value = ReadShortString(source);
								macros_->emplace_back(std::make_pair(name, value), true);
							}
						}

						{
							uint16_t num_cbufs;
							source->read(&num_cbufs, sizeof(num_cbufs));
							num_cbufs = LE2Native(num_cbufs);
							cbuffers_.resize(num_cbufs);
							for (uint32_t i = 0; i < num_cbufs; ++ i)
							{
								cbuffers_[i] = MakeSharedPtr<RenderEffectConstantBuffer>();
								cbuffers_[i]->StreamIn(source);
							}
						}

						{
							uint16_t num_params;
							source->read(&num_params, sizeof(num_params));
							num_params = LE2Native(num_params);
							params_.resize(num_params);
							for (uint32_t i = 0; i < num_params; ++ i)
							{
								RenderEffectParameterPtr param = MakeSharedPtr<RenderEffectParameter>();
								params_[i] = param;

								param->StreamIn(source);
							}
						}

						{
							uint16_t num_shader_frags;
							source->read(&num_shader_frags, sizeof(num_shader_frags));
							num_shader_frags = LE2Native(num_shader_frags);
							if (num_shader_frags > 0)
							{
								shader_frags_ = MakeSharedPtr<std::remove_reference<decltype(*shader_frags_)>::type>(num_shader_frags);
								for (uint32_t i = 0; i < num_shader_frags; ++ i)
								{
									(*shader_frags_)[i].StreamIn(source);
								}
							}
						}

						{
							uint16_t num_shader_descs;
							source->read(&num_shader_descs, sizeof(num_shader_descs));
							num_shader_descs = LE2Native(num_shader_descs);
							shader_descs_->resize(num_shader_descs + 1);
							for (uint32_t i = 0; i < num_shader_descs; ++ i)
							{
								(*shader_descs_)[i + 1].profile = ReadShortString(source);
								(*shader_descs_)[i + 1].func_name = ReadShortString(source);
								source->read(&(*shader_descs_)[i + 1].macros_hash, sizeof((*shader_descs_)[i + 1].macros_hash));

								source->read(&(*shader_descs_)[i + 1].tech_pass_type, sizeof((*shader_descs_)[i + 1].tech_pass_type));
								(*shader_descs_)[i + 1].tech_pass_type = LE2Native((*shader_descs_)[i + 1].tech_pass_type);

								uint8_t len;
								source->read(&len, sizeof(len));
								if (len > 0)
								{
									(*shader_descs_)[i + 1].so_decl.resize(len);
									source->read(&(*shader_descs_)[i + 1].so_decl[0], len * sizeof((*shader_descs_)[i + 1].so_decl[0]));
									for (uint32_t j = 0; j < len; ++ j)
									{
										(*shader_descs_)[i + 1].so_decl[j].usage = LE2Native((*shader_descs_)[i + 1].so_decl[j].usage);
									}
								}
							}
						}

						ret = true;
						{
							uint16_t num_techs;
							source->read(&num_techs, sizeof(num_techs));
							num_techs = LE2Native(num_techs);
							techniques_.resize(num_techs);
							for (uint32_t i = 0; i < num_techs; ++ i)
							{
								RenderTechniquePtr technique = MakeSharedPtr<RenderTechnique>(*this);
								techniques_[i] = technique;

								ret &= technique->StreamIn(source, i);
							}
						}
					}
				}
			}
		}

		return ret;
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderEffect::StreamOut(std::ostream& os)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		uint32_t fourcc = Native2LE(MakeFourCC<'K', 'F', 'X', ' '>::value);
		os.write(reinterpret_cast<char const *>(&fourcc), sizeof(fourcc));

		uint32_t ver = Native2LE(KFX_VERSION);
		os.write(reinterpret_cast<char const *>(&ver), sizeof(ver));

		uint32_t shader_fourcc = Native2LE(re.NativeShaderFourCC());
		os.write(reinterpret_cast<char const *>(&shader_fourcc), sizeof(shader_fourcc));

		uint32_t shader_ver = Native2LE(re.NativeShaderVersion());
		os.write(reinterpret_cast<char const *>(&shader_ver), sizeof(shader_ver));

		uint64_t timestamp = Native2LE(timestamp_);
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

			num_macros = Native2LE(num_macros);
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
			uint16_t num_cbufs = Native2LE(static_cast<uint16_t>(cbuffers_.size()));
			os.write(reinterpret_cast<char const *>(&num_cbufs), sizeof(num_cbufs));
			for (uint32_t i = 0; i < cbuffers_.size(); ++ i)
			{
				cbuffers_[i]->StreamOut(os);
			}
		}

		{
			uint16_t num_params = Native2LE(static_cast<uint16_t>(params_.size()));
			os.write(reinterpret_cast<char const *>(&num_params), sizeof(num_params));
			for (uint32_t i = 0; i < params_.size(); ++ i)
			{
				params_[i]->StreamOut(os);
			}
		}

		{
			uint16_t num_shader_frags = Native2LE(static_cast<uint16_t>(shader_frags_ ? shader_frags_->size() : 0));
			os.write(reinterpret_cast<char const *>(&num_shader_frags), sizeof(num_shader_frags));
			if (shader_frags_)
			{
				for (uint32_t i = 0; i < shader_frags_->size(); ++ i)
				{
					(*shader_frags_)[i].StreamOut(os);
				}
			}
		}

		{
			uint16_t num_shader_descs = Native2LE(static_cast<uint16_t>(shader_descs_->size() - 1));
			os.write(reinterpret_cast<char const *>(&num_shader_descs), sizeof(num_shader_descs));
			for (uint32_t i = 0; i < shader_descs_->size() - 1; ++ i)
			{
				WriteShortString(os, (*shader_descs_)[i + 1].profile);
				WriteShortString(os, (*shader_descs_)[i + 1].func_name);

				uint64_t tmp64 = Native2LE((*shader_descs_)[i + 1].macros_hash);
				os.write(reinterpret_cast<char const *>(&tmp64), sizeof(tmp64));

				uint32_t tmp32 = Native2LE((*shader_descs_)[i + 1].tech_pass_type);
				os.write(reinterpret_cast<char const *>(&tmp32), sizeof(tmp32));

				uint8_t len = static_cast<uint8_t>((*shader_descs_)[i + 1].so_decl.size());
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				for (uint32_t j = 0; j < len; ++ j)
				{
					ShaderDesc::StreamOutputDecl so_decl = (*shader_descs_)[i + 1].so_decl[j];
					so_decl.usage = Native2LE(so_decl.usage);
					os.write(reinterpret_cast<char const *>(&so_decl), sizeof(so_decl));
				}
			}
		}

		{
			uint16_t num_techs = Native2LE(static_cast<uint16_t>(techniques_.size()));
			os.write(reinterpret_cast<char const *>(&num_techs), sizeof(num_techs));
			for (uint32_t i = 0; i < techniques_.size(); ++ i)
			{
				techniques_[i]->StreamOut(os, i);
			}
		}
	}
#endif

	RenderEffectPtr RenderEffect::Clone()
	{
		RenderEffectPtr ret = MakeSharedPtr<RenderEffect>();

		ret->res_name_ = res_name_;
#if KLAYGE_IS_DEV_PLATFORM
		ret->timestamp_ = timestamp_;
#endif

		ret->macros_ = macros_;
		ret->shader_frags_ = shader_frags_;
#if KLAYGE_IS_DEV_PLATFORM
		ret->hlsl_shader_ = hlsl_shader_;
#endif
		ret->shader_descs_ = shader_descs_;

		ret->params_.resize(params_.size());
		for (size_t i = 0; i < params_.size(); ++ i)
		{
			ret->params_[i] = params_[i]->Clone();
		}

		ret->cbuffers_.resize(cbuffers_.size());
		for (size_t i = 0; i < cbuffers_.size(); ++ i)
		{
			ret->cbuffers_[i] = cbuffers_[i]->Clone(*this, *ret);
		}

		ret->techniques_.resize(techniques_.size());
		for (size_t i = 0; i < techniques_.size(); ++ i)
		{
			ret->techniques_[i] = techniques_[i]->Clone(*ret);
		}

		return ret;
	}

	RenderEffectParameterPtr const & RenderEffect::ParameterByName(std::string const & name) const
	{
		size_t const name_hash = boost::hash_range(name.begin(), name.end());
		for (auto const & param : params_)
		{
			if (name_hash == param->NameHash())
			{
				return param;
			}
		}
		static RenderEffectParameterPtr null_param;
		return null_param;
	}

	RenderEffectParameterPtr const & RenderEffect::ParameterBySemantic(std::string const & semantic) const
	{
		size_t const semantic_hash = boost::hash_range(semantic.begin(), semantic.end());
		for (auto const & param : params_)
		{
			if (semantic_hash == param->SemanticHash())
			{
				return param;
			}
		}
		static RenderEffectParameterPtr null_param;
		return null_param;
	}

	RenderEffectConstantBufferPtr const & RenderEffect::CBufferByName(std::string const & name) const
	{
		size_t const name_hash = boost::hash_range(name.begin(), name.end());
		for (auto const & cbuffer : cbuffers_)
		{
			if (name_hash == cbuffer->NameHash())
			{
				return cbuffer;
			}
		}
		static RenderEffectConstantBufferPtr null_cbuffer;
		return null_cbuffer;
	}

	RenderTechniquePtr const & RenderEffect::TechniqueByName(std::string const & name) const
	{
		size_t const name_hash = boost::hash_range(name.begin(), name.end());
		for (auto const & tech : techniques_)
		{
			if (name_hash == tech->NameHash())
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

#if KLAYGE_IS_DEV_PLATFORM
	void RenderEffect::GenHLSLShaderText()
	{
		hlsl_shader_ = MakeSharedPtr<std::string>();
		std::string& str = *hlsl_shader_;

		str += "#define SHADER_MODEL(major, minor) ((major) * 4 + (minor))\n\n";

		for (uint32_t i = 0; i < this->NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = this->MacroByIndex(i);
			str += "#define " + name_value.first + " " + name_value.second + "\n";
		}
		str += '\n';

		for (uint32_t i = 0; i < this->NumCBuffers(); ++ i)
		{
			RenderEffectConstantBufferPtr const & cbuff = this->CBufferByIndex(i);
			str += "cbuffer " + cbuff->Name() + "\n";
			str += "{\n";

			for (uint32_t j = 0; j < cbuff->NumParameters(); ++ j)
			{
				RenderEffectParameter& param = *this->ParameterByIndex(cbuff->ParameterIndex(j));
				switch (param.Type())
				{
				case REDT_texture1D:
				case REDT_texture2D:
				case REDT_texture3D:
				case REDT_textureCUBE:
				case REDT_texture1DArray:
				case REDT_texture2DArray:
				case REDT_texture3DArray:
				case REDT_textureCUBEArray:
				case REDT_sampler:
				case REDT_buffer:
				case REDT_structured_buffer:
				case REDT_byte_address_buffer:
				case REDT_rw_buffer:
				case REDT_rw_structured_buffer:
				case REDT_rw_texture1D:
				case REDT_rw_texture2D:
				case REDT_rw_texture3D:
				case REDT_rw_texture1DArray:
				case REDT_rw_texture2DArray:
				case REDT_rw_byte_address_buffer:
				case REDT_append_structured_buffer:
				case REDT_consume_structured_buffer:
					break;

				default:
					str += this->TypeName(param.Type()) + " " + param.Name();
					if (param.ArraySize())
					{
						str += "[" + *param.ArraySize() + "]";
					}
					str += ";\n";
					break;
				}
			}

			str += "};\n";
		}

		for (uint32_t i = 0; i < this->NumParameters(); ++ i)
		{
			RenderEffectParameter& param = *this->ParameterByIndex(i);

			std::string elem_type;
			switch (param.Type())
			{
			case REDT_texture1D:
			case REDT_texture2D:
			case REDT_texture3D:
			case REDT_textureCUBE:
			case REDT_texture1DArray:
			case REDT_texture2DArray:
			case REDT_textureCUBEArray:
			case REDT_buffer:
			case REDT_structured_buffer:
			case REDT_rw_buffer:
			case REDT_rw_structured_buffer:
			case REDT_rw_texture1D:
			case REDT_rw_texture2D:
			case REDT_rw_texture3D:
			case REDT_rw_texture1DArray:
			case REDT_rw_texture2DArray:
			case REDT_append_structured_buffer:
			case REDT_consume_structured_buffer:
				param.Var()->Value(elem_type);
				break;

			default:
				break;
			}

			std::string const & param_name = param.Name();
			switch (param.Type())
			{
			case REDT_texture1D:
				str += "Texture1D<" + elem_type + "> " + param_name + ";\n";
				break;

			case REDT_texture2D:
				str += "Texture2D<" + elem_type + "> " + param_name + ";\n";
				break;

			case REDT_texture3D:
				str += "#if KLAYGE_MAX_TEX_DEPTH <= 1\n";
				str += "Texture2D<" + elem_type + "> " + param_name + ";\n";
				str += "#else\n";
				str += "Texture3D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_textureCUBE:
				str += "TextureCube<" + elem_type + "> " + param_name + ";\n";
				break;

			case REDT_texture1DArray:
				str += "#if KLAYGE_MAX_TEX_ARRAY_LEN > 1\n";
				str += "Texture1DArray<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_texture2DArray:
				str += "#if KLAYGE_MAX_TEX_ARRAY_LEN > 1\n";
				str += "Texture2DArray<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_textureCUBEArray:
				str += "#if (KLAYGE_MAX_TEX_ARRAY_LEN > 1) && (KLAYGE_SHADER_MODEL >= SHADER_MODEL(4, 1))\n";
				str += "TextureCubeArray<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(4, 0)\n";
				str += "Buffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_sampler:
				str += "sampler " + param_name + ";\n";
				break;

			case REDT_structured_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(4, 0)\n";
				str += "StructuredBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_byte_address_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(4, 0)\n";
				str += "ByteAddressBuffer " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_structured_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(4, 0)\n";
				str += "RWStructuredBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_texture1D:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture1D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_texture2D:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture2D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_texture3D:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture3D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_texture1DArray:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture1DArray<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_texture2DArray:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture2DArray<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_byte_address_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(4, 0)\n";
				str += "RWByteAddressBuffer " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_append_structured_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "AppendStructuredBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_consume_structured_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "ConsumeStructuredBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			default:
				break;
			}
		}

		for (uint32_t i = 0; i < this->NumShaderFragments(); ++ i)
		{
			RenderShaderFragment const & effect_shader_frag = this->ShaderFragmentByIndex(i);
			ShaderObject::ShaderType const shader_type = effect_shader_frag.Type();
			switch (shader_type)
			{
			case ShaderObject::ST_VertexShader:
				str += "#if KLAYGE_VERTEX_SHADER\n";
				break;

			case ShaderObject::ST_PixelShader:
				str += "#if KLAYGE_PIXEL_SHADER\n";
				break;

			case ShaderObject::ST_GeometryShader:
				str += "#if KLAYGE_GEOMETRY_SHADER\n";
				break;

			case ShaderObject::ST_ComputeShader:
				str += "#if KLAYGE_COMPUTE_SHADER\n";
				break;

			case ShaderObject::ST_HullShader:
				str += "#if KLAYGE_HULL_SHADER\n";
				break;

			case ShaderObject::ST_DomainShader:
				str += "#if KLAYGE_DOMAIN_SHADER\n";
				break;

			case ShaderObject::ST_NumShaderTypes:
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			ShaderModel const & ver = effect_shader_frag.Version();
			if ((ver.major_ver != 0) || (ver.minor_ver != 0))
			{
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL("
					+ boost::lexical_cast<std::string>(static_cast<int>(ver.major_ver)) + ", "
					+ boost::lexical_cast<std::string>(static_cast<int>(ver.minor_ver)) + ")\n";
			}

			str += effect_shader_frag.str() + "\n";

			if ((ver.major_ver != 0) || (ver.minor_ver != 0))
			{
				str += "#endif\n";
			}
			if (shader_type != ShaderObject::ST_NumShaderTypes)
			{
				str += "#endif\n";
			}
		}
	}

	std::string const & RenderEffect::HLSLShaderText() const
	{
		if (hlsl_shader_)
		{
			return *hlsl_shader_;
		}
		else
		{
			static std::string empty;
			return empty;
		}
	}
#endif


#if KLAYGE_IS_DEV_PLATFORM
	void RenderTechnique::Load(XMLNodePtr const & node, uint32_t tech_index)
	{
		name_ = MakeSharedPtr<std::remove_reference<decltype(*name_)>::type>();
		name_->first = node->Attrib("name")->ValueString();
		name_->second = boost::hash_range(name_->first.begin(), name_->first.end());

		tech_hash_ = effect_.ResNameHash();
		boost::hash_combine(tech_hash_, name_->second);

		RenderTechniquePtr parent_tech;
		XMLAttributePtr inherit_attr = node->Attrib("inherit");
		if (inherit_attr)
		{
			std::string inherit = inherit_attr->ValueString();
			BOOST_ASSERT(inherit != name_->first);

			parent_tech = effect_.TechniqueByName(inherit);
			BOOST_ASSERT(parent_tech);
		}

		{
			XMLNodePtr anno_node = node->FirstNode("annotation");
			if (anno_node)
			{
				annotations_ = MakeSharedPtr<std::remove_reference<decltype(*annotations_)>::type>();
				if (parent_tech && parent_tech->annotations_)
				{
					*annotations_ = *parent_tech->annotations_;
				}
				for (; anno_node; anno_node = anno_node->NextSibling("annotation"))
				{
					RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
					annotations_->push_back(annotation);

					annotation->Load(anno_node);
				}
			}
			else if (parent_tech)
			{
				annotations_ = parent_tech->annotations_;
			}
		}

		{
			XMLNodePtr macro_node = node->FirstNode("macro");
			if (macro_node)
			{
				macros_ = MakeSharedPtr<std::remove_reference<decltype(*macros_)>::type>();
				if (parent_tech && parent_tech->macros_)
				{
					*macros_ = *parent_tech->macros_;
				}
				for (; macro_node; macro_node = macro_node->NextSibling("macro"))
				{
					std::string name = macro_node->Attrib("name")->ValueString();
					std::string value = macro_node->Attrib("value")->ValueString();
					bool found = false;
					for (size_t i = 0; i < macros_->size(); ++ i)
					{
						if ((*macros_)[i].first == name)
						{
							(*macros_)[i].second = value;
							found = true;
							break;
						}
					}
					if (!found)
					{
						macros_->emplace_back(name, value);
					}
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
#endif

	bool RenderTechnique::StreamIn(ResIdentifierPtr const & res, uint32_t tech_index)
	{
		name_ = MakeSharedPtr<std::remove_reference<decltype(*name_)>::type>();
		name_->first = ReadShortString(res);
		name_->second = boost::hash_range(name_->first.begin(), name_->first.end());

		tech_hash_ = effect_.ResNameHash();
		boost::hash_combine(tech_hash_, name_->second);

		uint8_t num_anno;
		res->read(&num_anno, sizeof(num_anno));
		if (num_anno > 0)
		{
			annotations_ = MakeSharedPtr<std::remove_reference<decltype(*annotations_)>::type>();
			annotations_->resize(num_anno);
			for (uint32_t i = 0; i < num_anno; ++ i)
			{
				RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
				(*annotations_)[i] = annotation;
				
				annotation->StreamIn(res);
			}
		}

		uint8_t num_macro;
		res->read(&num_macro, sizeof(num_macro));
		if (num_macro > 0)
		{
			macros_ = MakeSharedPtr<std::remove_reference<decltype(*macros_)>::type>();
			macros_->resize(num_macro);
			for (uint32_t i = 0; i < num_macro; ++ i)
			{
				std::string name = ReadShortString(res);
				std::string value = ReadShortString(res);
				(*macros_)[i] = std::make_pair(name, value);
			}
		}

		is_validate_ = true;

		has_discard_ = false;
		has_tessellation_ = false;
		
		res->read(&transparent_, sizeof(transparent_));
		res->read(&weight_, sizeof(weight_));
		weight_ = LE2Native(weight_);

		bool ret = true;
		uint8_t num_passes;
		res->read(&num_passes, sizeof(num_passes));
		passes_.resize(num_passes);
		for (uint32_t pass_index = 0; pass_index < num_passes; ++ pass_index)
		{
			RenderPassPtr pass = MakeSharedPtr<RenderPass>(effect_);
			passes_[pass_index] = pass;

			ret &= pass->StreamIn(res, tech_index, pass_index);

			is_validate_ &= pass->Validate();

			has_discard_ |= pass->GetShaderObject()->HasDiscard();
			has_tessellation_ |= pass->GetShaderObject()->HasTessellation();
		}

		return ret;
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderTechnique::StreamOut(std::ostream& os, uint32_t tech_index)
	{
		WriteShortString(os, name_->first);

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
		float w = Native2LE(weight_);
		os.write(reinterpret_cast<char const *>(&w), sizeof(w));

		uint8_t num_passes = static_cast<uint8_t>(passes_.size());
		os.write(reinterpret_cast<char const *>(&num_passes), sizeof(num_passes));
		for (uint32_t pass_index = 0; pass_index < num_passes; ++ pass_index)
		{
			passes_[pass_index]->StreamOut(os, tech_index, pass_index);
		}
	}
#endif

	RenderTechniquePtr RenderTechnique::Clone(RenderEffect& effect)
	{
		RenderTechniquePtr ret = MakeSharedPtr<RenderTechnique>(effect);

		ret->tech_hash_ = tech_hash_;

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


#if KLAYGE_IS_DEV_PLATFORM
	void RenderPass::Load(XMLNodePtr const & node, uint32_t tech_index, uint32_t pass_index, RenderPassPtr const & inherit_pass)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		name_ = MakeSharedPtr<std::remove_reference<decltype(*name_)>::type>();
		name_->first = node->Attrib("name")->ValueString();
		name_->second = boost::hash_range(name_->first.begin(), name_->first.end());

		{
			XMLNodePtr anno_node = node->FirstNode("annotation");
			if (anno_node)
			{
				annotations_ = MakeSharedPtr<std::remove_reference<decltype(*annotations_)>::type>();
				if (inherit_pass && inherit_pass->annotations_)
				{
					*annotations_ = *inherit_pass->annotations_;
				}
				for (; anno_node; anno_node = anno_node->NextSibling("annotation"))
				{
					RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
					annotations_->push_back(annotation);

					annotation->Load(anno_node);
				}
			}
			else if (inherit_pass)
			{
				annotations_ = inherit_pass->annotations_;
			}
		}

		{
			XMLNodePtr macro_node = node->FirstNode("macro");
			if (macro_node)
			{
				macros_ = MakeSharedPtr<std::remove_reference<decltype(*macros_)>::type>();
				if (inherit_pass && inherit_pass->macros_)
				{
					*macros_ = *inherit_pass->macros_;
				}
				for (; macro_node; macro_node = macro_node->NextSibling("macro"))
				{
					std::string name = macro_node->Attrib("name")->ValueString();
					std::string value = macro_node->Attrib("value")->ValueString();
					bool found = false;
					for (size_t i = 0; i < macros_->size(); ++ i)
					{
						if ((*macros_)[i].first == name)
						{
							(*macros_)[i].second = value;
							found = true;
							break;
						}
					}
					if (!found)
					{
						macros_->emplace_back(name, value);
					}
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

		shader_desc_ids_ = MakeSharedPtr<std::remove_reference<decltype(*shader_desc_ids_)>::type>();
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
		else
		{
			front_stencil_ref_ = 0;
			back_stencil_ref_ = 0;
			blend_factor_ = Color(1, 1, 1, 1);
			sample_mask_ = 0xFFFFFFFF;
		}

		for (XMLNodePtr state_node = node->FirstNode("state"); state_node; state_node = state_node->NextSibling("state"))
		{
			std::string state_name = state_node->Attrib("name")->ValueString();
			size_t const state_name_hash = RT_HASH(state_name.c_str());

			if (CT_HASH("polygon_mode") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.polygon_mode = polygon_mode_define::instance().from_str(value_str);
			}
			else if (CT_HASH("shade_mode") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.shade_mode = shade_mode_define::instance().from_str(value_str);
			}
			else if (CT_HASH("cull_mode") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.cull_mode = cull_mode_define::instance().from_str(value_str);
			}
			else if (CT_HASH("front_face_ccw") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.front_face_ccw = bool_from_str(value_str);
			}
			else if (CT_HASH("polygon_offset_factor") == state_name_hash)
			{
				rs_desc.polygon_offset_factor = state_node->Attrib("value")->ValueFloat();
			}
			else if (CT_HASH("polygon_offset_units") == state_name_hash)
			{
				rs_desc.polygon_offset_units = state_node->Attrib("value")->ValueFloat();
			}
			else if (CT_HASH("depth_clip_enable") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.depth_clip_enable = bool_from_str(value_str);
			}
			else if (CT_HASH("scissor_enable") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.scissor_enable = bool_from_str(value_str);
			}
			else if (CT_HASH("multisample_enable") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				rs_desc.multisample_enable = bool_from_str(value_str);
			}
			else if (CT_HASH("alpha_to_coverage_enable") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.alpha_to_coverage_enable = bool_from_str(value_str);
			}
			else if (CT_HASH("independent_blend_enable") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.independent_blend_enable = bool_from_str(value_str);
			}
			else if (CT_HASH("blend_enable") == state_name_hash)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.blend_enable[index] = bool_from_str(value_str);
			}
			else if (CT_HASH("logic_op_enable") == state_name_hash)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.logic_op_enable[index] = bool_from_str(value_str);
			}
			else if (CT_HASH("blend_op") == state_name_hash)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.blend_op[index] = blend_operation_define::instance().from_str(value_str);
			}
			else if (CT_HASH("src_blend") == state_name_hash)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.src_blend[index] = alpha_blend_factor_define::instance().from_str(value_str);
			}
			else if (CT_HASH("dest_blend") == state_name_hash)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.dest_blend[index] = alpha_blend_factor_define::instance().from_str(value_str);
			}
			else if (CT_HASH("blend_op_alpha") == state_name_hash)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.blend_op_alpha[index] = blend_operation_define::instance().from_str(value_str);
			}
			else if (CT_HASH("src_blend_alpha") == state_name_hash)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.src_blend_alpha[index] = alpha_blend_factor_define::instance().from_str(value_str);
			}
			else if (CT_HASH("dest_blend_alpha") == state_name_hash)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.dest_blend_alpha[index] = alpha_blend_factor_define::instance().from_str(value_str);
			}
			else if (CT_HASH("logic_op") == state_name_hash)
			{
				int index = get_index(state_node);
				std::string value_str = state_node->Attrib("value")->ValueString();
				bs_desc.logic_op[index] = logic_operation_define::instance().from_str(value_str);
			}
			else if (CT_HASH("color_write_mask") == state_name_hash)
			{
				int index = get_index(state_node);
				bs_desc.color_write_mask[index] = static_cast<uint8_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if (CT_HASH("blend_factor") == state_name_hash)
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
			else if (CT_HASH("sample_mask") == state_name_hash)
			{
				sample_mask_ = state_node->Attrib("value")->ValueUInt();
			}
			else if (CT_HASH("depth_enable") == state_name_hash)
			{
				dss_desc.depth_enable = bool_from_str(state_node->Attrib("value")->ValueString());
			}
			else if (CT_HASH("depth_write_mask") == state_name_hash)
			{
				dss_desc.depth_write_mask = bool_from_str(state_node->Attrib("value")->ValueString());
			}
			else if (CT_HASH("depth_func") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.depth_func = compare_function_define::instance().from_str(value_str);
			}
			else if (CT_HASH("front_stencil_enable") == state_name_hash)
			{
				dss_desc.front_stencil_enable = bool_from_str(state_node->Attrib("value")->ValueString());
			}
			else if (CT_HASH("front_stencil_func") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.front_stencil_func = compare_function_define::instance().from_str(value_str);
			}
			else if (CT_HASH("front_stencil_ref") == state_name_hash)
			{
				front_stencil_ref_ = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if (CT_HASH("front_stencil_read_mask") == state_name_hash)
			{
				dss_desc.front_stencil_read_mask = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if (CT_HASH("front_stencil_write_mask") == state_name_hash)
			{
				dss_desc.front_stencil_write_mask = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if (CT_HASH("front_stencil_fail") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.front_stencil_fail = stencil_operation_define::instance().from_str(value_str);
			}
			else if (CT_HASH("front_stencil_depth_fail") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.front_stencil_depth_fail = stencil_operation_define::instance().from_str(value_str);
			}
			else if (CT_HASH("front_stencil_pass") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.front_stencil_pass = stencil_operation_define::instance().from_str(value_str);
			}
			else if (CT_HASH("back_stencil_enable") == state_name_hash)
			{
				dss_desc.back_stencil_enable = bool_from_str(state_node->Attrib("value")->ValueString());
			}
			else if (CT_HASH("back_stencil_func") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.back_stencil_func = compare_function_define::instance().from_str(value_str);
			}
			else if (CT_HASH("back_stencil_ref") == state_name_hash)
			{
				back_stencil_ref_ = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if (CT_HASH("back_stencil_read_mask") == state_name_hash)
			{
				dss_desc.back_stencil_read_mask = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if (CT_HASH("back_stencil_write_mask") == state_name_hash)
			{
				dss_desc.back_stencil_write_mask = static_cast<uint16_t>(state_node->Attrib("value")->ValueUInt());
			}
			else if (CT_HASH("back_stencil_fail") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.back_stencil_fail = stencil_operation_define::instance().from_str(value_str);
			}
			else if (CT_HASH("back_stencil_depth_fail") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.back_stencil_depth_fail = stencil_operation_define::instance().from_str(value_str);
			}
			else if (CT_HASH("back_stencil_pass") == state_name_hash)
			{
				std::string value_str = state_node->Attrib("value")->ValueString();
				dss_desc.back_stencil_pass = stencil_operation_define::instance().from_str(value_str);
			}
			else if ((CT_HASH("vertex_shader") == state_name_hash) || (CT_HASH("pixel_shader") == state_name_hash)
				|| (CT_HASH("geometry_shader") == state_name_hash) || (CT_HASH("compute_shader") == state_name_hash)
				|| (CT_HASH("hull_shader") == state_name_hash) || (CT_HASH("domain_shader") == state_name_hash))
			{
				ShaderObject::ShaderType type;
				if (CT_HASH("vertex_shader") == state_name_hash)
				{
					type = ShaderObject::ST_VertexShader;
				}
				else if (CT_HASH("pixel_shader") == state_name_hash)
				{
					type = ShaderObject::ST_PixelShader;
				}
				else if (CT_HASH("geometry_shader") == state_name_hash)
				{
					type = ShaderObject::ST_GeometryShader;
				}
				else if (CT_HASH("compute_shader") == state_name_hash)
				{
					type = ShaderObject::ST_ComputeShader;
				}
				else if (CT_HASH("hull_shader") == state_name_hash)
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
							size_t const usage_str_hash = RT_HASH(usage_str.c_str());
							XMLAttributePtr attr = slot_node->Attrib("usage_index");
							if (attr)
							{
								decl.usage_index = static_cast<uint8_t>(attr->ValueInt());
							}
							else
							{
								decl.usage_index = 0;
							}

							if ((CT_HASH("POSITION") == usage_str_hash) || (CT_HASH("SV_Position") == usage_str_hash))
							{
								decl.usage = VEU_Position;
							}
							else if (CT_HASH("NORMAL") == usage_str_hash)
							{
								decl.usage = VEU_Normal;
							}
							else if (CT_HASH("COLOR") == usage_str_hash)
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
							else if (CT_HASH("BLENDWEIGHT") == usage_str_hash)
							{
								decl.usage = VEU_BlendWeight;
							}
							else if (CT_HASH("BLENDINDICES") == usage_str_hash)
							{
								decl.usage = VEU_BlendIndex;
							}
							else if (CT_HASH("TEXCOORD") == usage_str_hash)
							{
								decl.usage = VEU_TextureCoord;
							}
							else if (CT_HASH("TANGENT") == usage_str_hash)
							{
								decl.usage = VEU_Tangent;
							}
							else if (CT_HASH("BINORMAL") == usage_str_hash)
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
				BOOST_ASSERT(false);
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

		shader_desc_ids_ = MakeSharedPtr<std::remove_reference<decltype(*shader_desc_ids_)>::type>();
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
#endif

	bool RenderPass::StreamIn(ResIdentifierPtr const & res, uint32_t tech_index, uint32_t pass_index)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		name_ = MakeSharedPtr<std::remove_reference<decltype(*name_)>::type>();
		name_->first = ReadShortString(res);
		name_->second = boost::hash_range(name_->first.begin(), name_->first.end());

		uint8_t num_anno;
		res->read(&num_anno, sizeof(num_anno));
		if (num_anno > 0)
		{
			annotations_ = MakeSharedPtr<std::remove_reference<decltype(*annotations_)>::type>();
			annotations_->resize(num_anno);
			for (uint32_t i = 0; i < num_anno; ++ i)
			{
				RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
				(*annotations_)[i] = annotation;
				
				annotation->StreamIn(res);
			}
		}

		uint8_t num_macro;
		res->read(&num_macro, sizeof(num_macro));
		if (num_macro > 0)
		{
			macros_ = MakeSharedPtr<std::remove_reference<decltype(*macros_)>::type>();
			macros_->resize(num_macro);
			for (uint32_t i = 0; i < num_macro; ++ i)
			{
				std::string name = ReadShortString(res);
				std::string value = ReadShortString(res);
				(*macros_)[i] = std::make_pair(name, value);
			}
		}

		RasterizerStateDesc rs_desc;
		DepthStencilStateDesc dss_desc;
		BlendStateDesc bs_desc;

		res->read(&rs_desc, sizeof(rs_desc));
		rs_desc.polygon_mode = LE2Native(rs_desc.polygon_mode);
		rs_desc.shade_mode = LE2Native(rs_desc.shade_mode);
		rs_desc.cull_mode = LE2Native(rs_desc.cull_mode);
		rs_desc.polygon_offset_factor = LE2Native(rs_desc.polygon_offset_factor);
		rs_desc.polygon_offset_units = LE2Native(rs_desc.polygon_offset_units);
		
		res->read(&dss_desc, sizeof(dss_desc));
		dss_desc.depth_func = LE2Native(dss_desc.depth_func);
		dss_desc.front_stencil_func = LE2Native(dss_desc.front_stencil_func);
		dss_desc.front_stencil_read_mask = LE2Native(dss_desc.front_stencil_read_mask);
		dss_desc.front_stencil_write_mask = LE2Native(dss_desc.front_stencil_write_mask);
		dss_desc.front_stencil_fail = LE2Native(dss_desc.front_stencil_fail);
		dss_desc.front_stencil_depth_fail = LE2Native(dss_desc.front_stencil_depth_fail);
		dss_desc.front_stencil_pass = LE2Native(dss_desc.front_stencil_pass);
		dss_desc.back_stencil_func = LE2Native(dss_desc.back_stencil_func);
		dss_desc.back_stencil_read_mask = LE2Native(dss_desc.back_stencil_read_mask);
		dss_desc.back_stencil_write_mask = LE2Native(dss_desc.back_stencil_write_mask);
		dss_desc.back_stencil_fail = LE2Native(dss_desc.back_stencil_fail);
		dss_desc.back_stencil_depth_fail = LE2Native(dss_desc.back_stencil_depth_fail);
		dss_desc.back_stencil_pass = LE2Native(dss_desc.back_stencil_pass);

		res->read(&bs_desc, sizeof(bs_desc));
		for (size_t i = 0; i < bs_desc.blend_op.size(); ++ i)
		{
			bs_desc.blend_op[i] = LE2Native(bs_desc.blend_op[i]);
			bs_desc.src_blend[i] = LE2Native(bs_desc.src_blend[i]);
			bs_desc.dest_blend[i] = LE2Native(bs_desc.dest_blend[i]);
			bs_desc.blend_op_alpha[i] = LE2Native(bs_desc.blend_op_alpha[i]);
			bs_desc.src_blend_alpha[i] = LE2Native(bs_desc.src_blend_alpha[i]);
			bs_desc.dest_blend_alpha[i] = LE2Native(bs_desc.dest_blend_alpha[i]);
		}
		
		rasterizer_state_obj_ = rf.MakeRasterizerStateObject(rs_desc);
		depth_stencil_state_obj_ = rf.MakeDepthStencilStateObject(dss_desc);
		blend_state_obj_ = rf.MakeBlendStateObject(bs_desc);

		res->read(&front_stencil_ref_, sizeof(front_stencil_ref_));
		front_stencil_ref_ = LE2Native(front_stencil_ref_);
		res->read(&back_stencil_ref_, sizeof(back_stencil_ref_));
		back_stencil_ref_ = LE2Native(back_stencil_ref_);
		res->read(&blend_factor_, sizeof(blend_factor_));
		for (int i = 0; i < 4; ++ i)
		{
			blend_factor_[i] = LE2Native(blend_factor_[i]);
		}
		res->read(&sample_mask_, sizeof(sample_mask_));
		sample_mask_ = LE2Native(sample_mask_);

		shader_desc_ids_ = MakeSharedPtr<std::remove_reference<decltype(*shader_desc_ids_)>::type>();
		shader_desc_ids_->resize(ShaderObject::ST_NumShaderTypes, 0);
		res->read(&(*shader_desc_ids_)[0], shader_desc_ids_->size() * sizeof((*shader_desc_ids_)[0]));
		for (int i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			(*shader_desc_ids_)[i] = LE2Native((*shader_desc_ids_)[i]);
		}

		
		shader_obj_ = rf.MakeShaderObject();

		bool native_accepted = true;

		for (int type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			ShaderDesc const & sd = effect_.GetShaderDesc((*shader_desc_ids_)[type]);
			if (!sd.func_name.empty())
			{
				ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(type);

				bool this_native_accepted;
				if (sd.tech_pass_type != (tech_index << 16) + (pass_index << 8) + type)
				{
					RenderTechniquePtr const & tech = effect_.TechniqueByIndex(sd.tech_pass_type >> 16);
					RenderPassPtr const & pass = tech->Pass((sd.tech_pass_type >> 8) & 0xFF);
					shader_obj_->AttachShader(st, effect_, *tech, *pass, pass->GetShaderObject());
					this_native_accepted = true;
				}
				else
				{
					this_native_accepted = shader_obj_->StreamIn(res, static_cast<ShaderObject::ShaderType>(type),
						effect_, *shader_desc_ids_);
				}

				native_accepted &= this_native_accepted;
			}
		}

		shader_obj_->LinkShaders(effect_);

		is_validate_ = shader_obj_->Validate();

		return native_accepted;
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderPass::StreamOut(std::ostream& os, uint32_t tech_index, uint32_t pass_index)
	{
		WriteShortString(os, name_->first);

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

		rs_desc.polygon_mode = Native2LE(rs_desc.polygon_mode);
		rs_desc.shade_mode = Native2LE(rs_desc.shade_mode);
		rs_desc.cull_mode = Native2LE(rs_desc.cull_mode);
		rs_desc.polygon_offset_factor = Native2LE(rs_desc.polygon_offset_factor);
		rs_desc.polygon_offset_units = Native2LE(rs_desc.polygon_offset_units);
		os.write(reinterpret_cast<char const *>(&rs_desc), sizeof(rs_desc));
		
		dss_desc.depth_func = Native2LE(dss_desc.depth_func);
		dss_desc.front_stencil_func = Native2LE(dss_desc.front_stencil_func);
		dss_desc.front_stencil_read_mask = Native2LE(dss_desc.front_stencil_read_mask);
		dss_desc.front_stencil_write_mask = Native2LE(dss_desc.front_stencil_write_mask);
		dss_desc.front_stencil_fail = Native2LE(dss_desc.front_stencil_fail);
		dss_desc.front_stencil_depth_fail = Native2LE(dss_desc.front_stencil_depth_fail);
		dss_desc.front_stencil_pass = Native2LE(dss_desc.front_stencil_pass);
		dss_desc.back_stencil_func = Native2LE(dss_desc.back_stencil_func);
		dss_desc.back_stencil_read_mask = Native2LE(dss_desc.back_stencil_read_mask);
		dss_desc.back_stencil_write_mask = Native2LE(dss_desc.back_stencil_write_mask);
		dss_desc.back_stencil_fail = Native2LE(dss_desc.back_stencil_fail);
		dss_desc.back_stencil_depth_fail = Native2LE(dss_desc.back_stencil_depth_fail);
		dss_desc.back_stencil_pass = Native2LE(dss_desc.back_stencil_pass);
		os.write(reinterpret_cast<char const *>(&dss_desc), sizeof(dss_desc));

		for (size_t i = 0; i < bs_desc.blend_op.size(); ++ i)
		{
			bs_desc.blend_op[i] = Native2LE(bs_desc.blend_op[i]);
			bs_desc.src_blend[i] = Native2LE(bs_desc.src_blend[i]);
			bs_desc.dest_blend[i] = Native2LE(bs_desc.dest_blend[i]);
			bs_desc.blend_op_alpha[i] = Native2LE(bs_desc.blend_op_alpha[i]);
			bs_desc.src_blend_alpha[i] = Native2LE(bs_desc.src_blend_alpha[i]);
			bs_desc.dest_blend_alpha[i] = Native2LE(bs_desc.dest_blend_alpha[i]);
		}		
		os.write(reinterpret_cast<char const *>(&bs_desc), sizeof(bs_desc));

		{
			uint16_t tmp;
			tmp = Native2LE(front_stencil_ref_);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			tmp = Native2LE(back_stencil_ref_);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}
		{
			Color tmp;
			tmp.r() = Native2LE(blend_factor_.r());
			tmp.g() = Native2LE(blend_factor_.g());
			tmp.b() = Native2LE(blend_factor_.b());
			tmp.a() = Native2LE(blend_factor_.a());
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}
		{
			uint32_t tmp = Native2LE(sample_mask_);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}

		for (uint32_t i = 0; i < shader_desc_ids_->size(); ++ i)
		{
			uint32_t tmp = Native2LE((*shader_desc_ids_)[i]);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}

		for (int type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
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
#endif

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


	RenderEffectConstantBuffer::RenderEffectConstantBuffer()
		: dirty_(true)
	{
	}

	RenderEffectConstantBuffer::~RenderEffectConstantBuffer()
	{
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderEffectConstantBuffer::Load(std::string const & name)
	{
		name_ = MakeSharedPtr<std::remove_reference<decltype(*name_)>::type>();
		name_->first = name;
		name_->second = boost::hash_range(name_->first.begin(), name_->first.end());
		param_indices_ = MakeSharedPtr<std::remove_reference<decltype(*param_indices_)>::type>();
	}
#endif

	void RenderEffectConstantBuffer::StreamIn(ResIdentifierPtr const & res)
	{
		name_ = MakeSharedPtr<std::remove_reference<decltype(*name_)>::type>();
		name_->first = ReadShortString(res);
		name_->second = boost::hash_range(name_->first.begin(), name_->first.end());
		param_indices_ = MakeSharedPtr<std::remove_reference<decltype(*param_indices_)>::type>();

		uint16_t len;
		res->read(&len, sizeof(len));
		len = LE2Native(len);
		param_indices_->resize(len);
		res->read(&(*param_indices_)[0], len * sizeof((*param_indices_)[0]));
		for (uint32_t i = 0; i < len; ++ i)
		{
			(*param_indices_)[i] = LE2Native((*param_indices_)[i]);
		}
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderEffectConstantBuffer::StreamOut(std::ostream& os)
	{
		WriteShortString(os, name_->first);

		uint16_t len = Native2LE(static_cast<uint16_t>(param_indices_->size()));
		os.write(reinterpret_cast<char const *>(&len), sizeof(len));
		for (size_t i = 0; i < param_indices_->size(); ++ i)
		{
			uint32_t tmp = Native2LE((*param_indices_)[i]);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}
	}
#endif

	RenderEffectConstantBufferPtr RenderEffectConstantBuffer::Clone(RenderEffect& src_effect, RenderEffect& dst_effect)
	{
		RenderEffectConstantBufferPtr ret = MakeSharedPtr<RenderEffectConstantBuffer>();

		ret->name_ = name_;
		ret->param_indices_ = param_indices_;
		ret->buff_ = buff_;
		ret->Resize(static_cast<uint32_t>(buff_.size()));

		for (size_t i = 0; i < param_indices_->size(); ++ i)
		{
			RenderEffectParameterPtr const & src_param = src_effect.ParameterByIndex((*param_indices_)[i]);
			if (src_param->InCBuffer())
			{
				RenderEffectParameterPtr const & dst_param = dst_effect.ParameterByIndex((*param_indices_)[i]);
				dst_param->RebindToCBuffer(ret);
			}
		}

		return ret;
	}

	void RenderEffectConstantBuffer::AddParameter(uint32_t index)
	{
		param_indices_->push_back(index);
	}

	void RenderEffectConstantBuffer::Resize(uint32_t size)
	{
		buff_.resize(size);
		if (size > 0)
		{
			if (!hw_buff_ || (size > hw_buff_->Size()))
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				hw_buff_ = rf.MakeConstantBuffer(BU_Dynamic, EAH_CPU_Write, size, nullptr);
			}
		}

		dirty_ = true;
	}

	void RenderEffectConstantBuffer::Update()
	{
		if (dirty_)
		{
			GraphicsBuffer::Mapper mapper(*hw_buff_, BA_Write_Only);
			std::memcpy(mapper.Pointer<uint8_t>(), &buff_[0], buff_.size());

			dirty_ = false;
		}
	}

	void RenderEffectConstantBuffer::BindHWBuff(GraphicsBufferPtr const & buff)
	{
		hw_buff_ = buff;
		buff_.resize(buff->Size());
	}


	RenderEffectParameter::RenderEffectParameter()
	{
	}

	RenderEffectParameter::~RenderEffectParameter()
	{
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderEffectParameter::Load(XMLNodePtr const & node)
	{
		type_ = type_define::instance().type_code(node->Attrib("type")->ValueString());
		name_ = MakeSharedPtr<std::remove_reference<decltype(*name_)>::type>();
		name_->first = node->Attrib("name")->ValueString();
		name_->second = boost::hash_range(name_->first.begin(), name_->first.end());

		XMLAttributePtr attr = node->Attrib("semantic");
		if (attr)
		{
			semantic_ = MakeSharedPtr<std::remove_reference<decltype(*semantic_)>::type>();
			semantic_->first = attr->ValueString();
			semantic_->second = boost::hash_range(semantic_->first.begin(), semantic_->first.end());
		}

		uint32_t as;
		attr = node->Attrib("array_size");
		if (attr)
		{
			array_size_ = MakeSharedPtr<std::string>(attr->ValueString());

			if (!attr->TryConvert(as))
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
				annotations_ = MakeSharedPtr<std::remove_reference<decltype(*annotations_)>::type>();
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

						if (ResLoader::Instance().Locate(val).empty())
						{
							LogError("%s NOT found", val.c_str());
						}
						else
						{
							*var_ = SyncLoadTexture(val, EAH_GPU_Read | EAH_Immutable);
						}
					}
				}
			}
		}
	}
#endif

	void RenderEffectParameter::StreamIn(ResIdentifierPtr const & res)
	{
		res->read(&type_, sizeof(type_));
		type_ = LE2Native(type_);
		name_ = MakeSharedPtr<std::remove_reference<decltype(*name_)>::type>();
		name_->first = ReadShortString(res);
		name_->second = boost::hash_range(name_->first.begin(), name_->first.end());

		std::string sem = ReadShortString(res);
		if (!sem.empty())
		{
			semantic_ = MakeSharedPtr<std::remove_reference<decltype(*semantic_)>::type>();
			semantic_->first = sem;
			semantic_->second = boost::hash_range(sem.begin(), sem.end());
		}

		uint32_t as;
		std::string as_str = ReadShortString(res);
		if (as_str.empty())
		{
			as = 0;
		}
		else
		{
			array_size_ = MakeSharedPtr<std::string>(as_str);

			if (!boost::conversion::try_lexical_convert(as_str, as))
			{
				as = 1;  // dummy array size
			}
		}
		var_ = stream_in_var(res, type_, as);

		uint8_t num_anno;
		res->read(&num_anno, sizeof(num_anno));
		if (num_anno > 0)
		{
			annotations_ = MakeSharedPtr<std::remove_reference<decltype(*annotations_)>::type>();
			annotations_->resize(num_anno);
			for (uint32_t i = 0; i < num_anno; ++ i)
			{
				RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
				(*annotations_)[i] = annotation;
				
				annotation->StreamIn(res);
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

						if (ResLoader::Instance().Locate(val).empty())
						{
							LogError("%s NOT found", val.c_str());
						}
						else
						{
							*var_ = SyncLoadTexture(val, EAH_GPU_Read | EAH_Immutable);
						}
					}
				}
			}
		}
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderEffectParameter::StreamOut(std::ostream& os)
	{
		uint32_t t = Native2LE(type_);
		os.write(reinterpret_cast<char const *>(&t), sizeof(t));
		WriteShortString(os, name_->first);
		if (semantic_)
		{
			WriteShortString(os, semantic_->first);
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
			if (!boost::conversion::try_lexical_convert(*array_size_, as))
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
#endif

	RenderEffectParameterPtr RenderEffectParameter::Clone()
	{
		RenderEffectParameterPtr ret = MakeSharedPtr<RenderEffectParameter>();

		ret->name_ = name_;
		ret->semantic_ = semantic_;

		ret->type_ = type_;
		ret->var_ = var_->Clone();
		ret->array_size_ = array_size_;

		ret->annotations_ = annotations_;

		return ret;
	}

	std::string const & RenderEffectParameter::Semantic() const
	{
		if (this->HasSemantic())
		{
			return semantic_->first;
		}
		else
		{
			static std::string empty("");
			return empty;
		}
	}

	size_t RenderEffectParameter::SemanticHash() const
	{
		return this->HasSemantic() ? semantic_->second : 0;
	}

	void RenderEffectParameter::BindToCBuffer(RenderEffectConstantBufferPtr const & cbuff, uint32_t offset,
		uint32_t stride)
	{
		cbuff_ = cbuff;
		var_->BindToCBuffer(cbuff.get(), offset, stride);
	}

	void RenderEffectParameter::RebindToCBuffer(RenderEffectConstantBufferPtr const & cbuff)
	{
		cbuff_ = cbuff;
		var_->RebindToCBuffer(cbuff.get());
	}


#if KLAYGE_IS_DEV_PLATFORM
	void RenderShaderFragment::Load(XMLNodePtr const & node)
	{
		type_ = ShaderObject::ST_NumShaderTypes;
		XMLAttributePtr attr = node->Attrib("type");
		if (attr)
		{
			std::string type_str = attr->ValueString();
			size_t const type_str_hash = RT_HASH(type_str.c_str());
			if (CT_HASH("vertex_shader") == type_str_hash)
			{
				type_ = ShaderObject::ST_VertexShader;
			}
			else if (CT_HASH("pixel_shader") == type_str_hash)
			{
				type_ = ShaderObject::ST_PixelShader;
			}
			else if (CT_HASH("geometry_shader") == type_str_hash)
			{
				type_ = ShaderObject::ST_GeometryShader;
			}
			else if (CT_HASH("compute_shader") == type_str_hash)
			{
				type_ = ShaderObject::ST_ComputeShader;
			}
			else if (CT_HASH("hull_shader") == type_str_hash)
			{
				type_ = ShaderObject::ST_HullShader;
			}
			else
			{
				BOOST_ASSERT("domain_shader" == type_str);
				type_ = ShaderObject::ST_DomainShader;
			}
		}
		
		ver_ = ShaderModel(0, 0);
		attr = node->Attrib("major_version");
		if (attr)
		{
			uint8_t minor_ver = 0;
			XMLAttributePtr minor_attr = node->Attrib("minor_version");
			if (minor_attr)
			{
				minor_ver = static_cast<uint8_t>(minor_attr->ValueInt());
			}
			ver_ = ShaderModel(static_cast<uint8_t>(attr->ValueInt()), minor_ver);
		}
		else
		{
			attr = node->Attrib("version");
			if (attr)
			{
				ver_ = ShaderModel(static_cast<uint8_t>(attr->ValueInt()), 0);
			}
		}

		for (XMLNodePtr shader_text_node = node->FirstNode(); shader_text_node; shader_text_node = shader_text_node->NextSibling())
		{
			if ((XNT_Comment == shader_text_node->Type()) || (XNT_CData == shader_text_node->Type()))
			{
				str_ += shader_text_node->ValueString();
			}
		}
	}
#endif

	void RenderShaderFragment::StreamIn(ResIdentifierPtr const & res)
	{
		res->read(&type_, sizeof(type_));
		type_ = LE2Native(type_);
		res->read(&ver_, sizeof(ver_));

		uint32_t len;
		res->read(&len, sizeof(len));
		len = LE2Native(len);
		str_.resize(len);
		res->read(&str_[0], len * sizeof(str_[0]));
	}

#if KLAYGE_IS_DEV_PLATFORM
	void RenderShaderFragment::StreamOut(std::ostream& os)
	{
		uint32_t tmp;
		tmp = Native2LE(type_);
		os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		os.write(reinterpret_cast<char const *>(&ver_), sizeof(ver_));

		uint32_t len = static_cast<uint32_t>(str_.size());
		tmp = Native2LE(len);
		os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		os.write(&str_[0], len * sizeof(str_[0]));
	}
#endif


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

	void RenderVariable::BindToCBuffer(RenderEffectConstantBuffer* cbuff, uint32_t offset,
			uint32_t stride)
	{
		KFL_UNUSED(cbuff);
		KFL_UNUSED(offset);
		KFL_UNUSED(stride);

		BOOST_ASSERT(false);
	}

	void RenderVariable::RebindToCBuffer(RenderEffectConstantBuffer* cbuff)
	{
		KFL_UNUSED(cbuff);

		BOOST_ASSERT(false);
	}

	RenderVariablePtr RenderVariableFloat4x4::Clone()
	{
		std::shared_ptr<RenderVariableFloat4x4> ret = MakeSharedPtr<RenderVariableFloat4x4>();
		ret->in_cbuff_ = in_cbuff_;
		if (in_cbuff_)
		{
			ret->data_ = data_;
		}
		float4x4 val;
		this->Value(val);
		*ret = val;
		return ret;
	}

	RenderVariable& RenderVariableFloat4x4::operator=(float4x4 const & value)
	{
		return RenderVariableConcrete<float4x4>::operator=(MathLib::transpose(value));
	}

	void RenderVariableFloat4x4::Value(float4x4& val) const
	{
		RenderVariableConcrete<float4x4>::Value(val);
		val = MathLib::transpose(val);
	}

	RenderVariablePtr RenderVariableFloat4x4Array::Clone()
	{
		std::shared_ptr<RenderVariableFloat4x4Array> ret = MakeSharedPtr<RenderVariableFloat4x4Array>();
		if (in_cbuff_)
		{
			if (!ret->in_cbuff_)
			{
				ret->RetriveT().~vector();
			}
			ret->data_ = data_;
		}
		ret->in_cbuff_ = in_cbuff_;
		std::vector<float4x4> val;
		this->Value(val);
		*ret = val;
		return ret;
	}

	RenderVariable& RenderVariableFloat4x4Array::operator=(std::vector<float4x4> const & value)
	{
		if (in_cbuff_)
		{
			float4x4* target = data_.cbuff_desc.cbuff->VariableInBuff<float4x4>(data_.cbuff_desc.offset);

			size_ = static_cast<uint32_t>(value.size());
			for (size_t i = 0; i < value.size(); ++ i)
			{
				target[i] = MathLib::transpose(value[i]);
			}

			data_.cbuff_desc.cbuff->Dirty(true);
		}
		else
		{
			this->RetriveT() = value;
		}
		return *this;
	}

	void RenderVariableFloat4x4Array::Value(std::vector<float4x4>& val) const
	{
		if (in_cbuff_)
		{
			float4x4 const * src = data_.cbuff_desc.cbuff->VariableInBuff<float4x4>(data_.cbuff_desc.offset);

			val.resize(size_);
			for (size_t i = 0; i < size_; ++ i)
			{
				val[i] = MathLib::transpose(src[i]);
			}
		}
		else
		{
			val = this->RetriveT();
		}
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

	void RenderVariableTexture::Value(TexturePtr& val) const
	{
		if (val_.tex)
		{
			val_.num_items = val_.tex->ArraySize();
			val_.num_levels = val_.tex->NumMipMaps();
		}
		val = val_.tex;
	}

	void RenderVariableTexture::Value(TextureSubresource& val) const
	{
		if (val_.tex)
		{
			val_.num_items = val_.tex->ArraySize();
			val_.num_levels = val_.tex->NumMipMaps();
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


	RenderEffectPtr SyncLoadRenderEffect(std::string const & effect_name)
	{
		return ResLoader::Instance().SyncQueryT<RenderEffect>(MakeSharedPtr<EffectLoadingDesc>(effect_name));
	}

	RenderEffectPtr ASyncLoadRenderEffect(std::string const & effect_name)
	{
		// TODO: Make it really async
		return ResLoader::Instance().SyncQueryT<RenderEffect>(MakeSharedPtr<EffectLoadingDesc>(effect_name));
	}
}
