// RenderEffect.cpp
// KlayGE 渲染效果类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/RenderEffect.hpp>

namespace
{
	using namespace KlayGE;

	std::string read_short_string(ResIdentifierPtr const & source)
	{
		uint8_t len;
		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		std::string ret(len, '\0');
		source->read(&ret[0], len);

		return ret;
	}

	boost::shared_ptr<RenderVariable> read_var(ResIdentifierPtr const & source, uint32_t type, uint32_t array_size)
	{
		boost::shared_ptr<RenderVariable> var(new RenderVariable);

		switch (type)
		{
		case type_define::TC_bool:
			if (0 == array_size)
			{
				bool tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				var->set_value(tmp);
			}
			break;

		case type_define::TC_dword:
		case type_define::TC_int:
			if (0 == array_size)
			{
				int tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				var->set_value(tmp);
			}
			break;

		case type_define::TC_string:
			var->set_value(read_short_string(source));
			break;

		case type_define::TC_sampler:
			break;

		case type_define::TC_shader:
			{
				shader_desc desc;

				desc.profile = read_short_string(source);

				std::string value = read_short_string(source);
				desc.func_name = value.substr(0, value.find("("));

				var->set_value(desc);
			}
			break;

		case type_define::TC_float:
			if (0 == array_size)
			{
				float tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				var->set_value(tmp);
			}
			break;

		case type_define::TC_float2:
			if (0 == array_size)
			{
				float2 tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				var->set_value(tmp);
			}
			break;

		case type_define::TC_float3:
			if (0 == array_size)
			{
				float3 tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				var->set_value(tmp);
			}
			break;

		case type_define::TC_float4:
			if (0 == array_size)
			{
				float4 tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				var->set_value(tmp);
			}
			break;

		case type_define::TC_float4x4:
			if (0 == array_size)
			{
				float4x4 tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				var->set_value(tmp);
			}
			break;

		default:
			assert(false);
			break;
		}

		return var;
	}
}

namespace KlayGE
{
	type_define::type_define()
	{
		types_.push_back("bool");
		types_.push_back("dword");
		types_.push_back("string");
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

	type_define& type_define::instance()
	{
		static type_define ret;
		return ret;
	}

	uint32_t type_define::type_code(std::string const & name) const
	{
		for (uint32_t i = 0; i < types_.size(); ++ i)
		{
			if (types_[i] == name)
			{
				return i;
			}
		}
		assert(false);
		return 0xFFFFFFFF;
	}

	std::string const & type_define::type_name(uint32_t code) const
	{
		if (code < types_.size())
		{
			return types_[code];
		}
		assert(false);
		return "";
	}

	states_define::states_define()
	{
		states_.push_back(std::make_pair("polygon_mode", "int"));
		states_.push_back(std::make_pair("shade_mode", "int"));
		states_.push_back(std::make_pair("cull_mode", "int"));

		states_.push_back(std::make_pair("alpha_to_coverage_enable", "bool"));
		states_.push_back(std::make_pair("blend_enable", "bool"));
		states_.push_back(std::make_pair("blend_op", "int"));
		states_.push_back(std::make_pair("src_blend", "int"));
		states_.push_back(std::make_pair("dest_blend", "int"));
		states_.push_back(std::make_pair("blend_op_alpha", "int"));
		states_.push_back(std::make_pair("src_blend_alpha", "int"));
		states_.push_back(std::make_pair("dest_blend_alpha", "int"));

		states_.push_back(std::make_pair("depth_enable", "bool"));
		states_.push_back(std::make_pair("depth_write_enable", "bool"));
		states_.push_back(std::make_pair("depth_func", "int"));
		states_.push_back(std::make_pair("slope_scale_depth_bias", "float"));
		states_.push_back(std::make_pair("depth_bias", "float"));

		states_.push_back(std::make_pair("front_stencil_enable", "bool"));
		states_.push_back(std::make_pair("front_stencil_func", "int"));
		states_.push_back(std::make_pair("front_stencil_ref", "int"));
		states_.push_back(std::make_pair("front_stencil_mask", "int"));
		states_.push_back(std::make_pair("front_stencil_fail", "int"));
		states_.push_back(std::make_pair("front_stencil_depth_fail", "int"));
		states_.push_back(std::make_pair("front_stencil_pass", "int"));
		states_.push_back(std::make_pair("front_stencil_write_mask", "int"));
		states_.push_back(std::make_pair("back_stencil_enable", "bool"));
		states_.push_back(std::make_pair("back_stencil_func", "int"));
		states_.push_back(std::make_pair("back_stencil_ref", "int"));
		states_.push_back(std::make_pair("back_stencil_mask", "int"));
		states_.push_back(std::make_pair("back_stencil_fail", "int"));
		states_.push_back(std::make_pair("back_stencil_depth_fail", "int"));
		states_.push_back(std::make_pair("back_stencil_pass", "int"));
		states_.push_back(std::make_pair("back_stencil_write_mask", "int"));

		states_.push_back(std::make_pair("scissor_enable", "bool"));

		states_.push_back(std::make_pair("color_mask_0", "int"));
		states_.push_back(std::make_pair("color_mask_1", "int"));
		states_.push_back(std::make_pair("color_mask_2", "int"));
		states_.push_back(std::make_pair("color_mask_3", "int"));

		states_.push_back(std::make_pair("pixel_shader", "shader"));
		states_.push_back(std::make_pair("vertex_shader", "shader"));
	}

	states_define& states_define::instance()
	{
		static states_define ret;
		return ret;
	}

	uint32_t states_define::state_code(std::string const & name) const
	{
		for (std::vector<std::pair<std::string, std::string> >::const_iterator iter = states_.begin();
			iter != states_.end(); ++ iter)
		{
			if (iter->first == name)
			{
				return static_cast<uint32_t>(std::distance(states_.begin(), iter));
			}
		}
		assert(false);

		return 0xFFFFFFFF;
	}

	std::string const & states_define::state_name(uint32_t code) const
	{
		if (code < states_.size())
		{
			return states_[code].first;
		}
		assert(false);
		return "";
	}

	void RenderAnnotation::Load(ResIdentifierPtr const & source)
	{
		source->read(reinterpret_cast<char*>(&type_), sizeof(type_));
		name_ = read_short_string(source);
		var_ = read_var(source, type_, 0);
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
	#pragma pack(push, 1)
		struct fxml_header
		{
			uint32_t fourcc;
			uint32_t ver;
			uint32_t num_parameters;
			uint32_t num_shaders;
			uint32_t num_techniques;
		};
	#pragma pack(pop)

		if (source)
		{
			fxml_header header;
			source->read(reinterpret_cast<char*>(&header), sizeof(header));
			BOOST_ASSERT((MakeFourCC<'F', 'X', 'M', 'L'>::value == header.fourcc));
			BOOST_ASSERT(1 == header.ver);

			for (uint32_t i = 0; i < header.num_parameters; ++ i)
			{
				params_.push_back(RenderEffectParameterPtr(new RenderEffectParameter(*this, "", "")));
				params_[i]->Load(source);
			}

			for (uint32_t i = 0; i < header.num_shaders; ++ i)
			{
				shaders_.push_back(RenderShaderFunc());
				shaders_[i].Load(source);
			}

			for (uint32_t i = 0; i < header.num_techniques; ++ i)
			{
				techniques_.push_back(this->MakeRenderTechnique());
				techniques_[i]->Load(source);
			}
		}
	}

	RenderEffectPtr RenderEffect::NullObject()
	{
		static RenderEffectPtr obj(new NullRenderEffect);
		return obj;
	}

	RenderEffectParameterPtr RenderEffect::ParameterByName(std::string const & name)
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			if (name == (*iter)->Name())
			{
				return *iter;
			}
		}
		return RenderEffectParameter::NullObject();
	}

	RenderEffectParameterPtr RenderEffect::ParameterBySemantic(std::string const & semantic)
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			if (semantic == (*iter)->Semantic())
			{
				return *iter;
			}
		}
		return RenderEffectParameter::NullObject();
	}

	RenderTechniquePtr RenderEffect::TechniqueByName(std::string const & name)
	{
		for (techniques_type::iterator iter = techniques_.begin(); iter != techniques_.end(); ++ iter)
		{
			if (name == (*iter)->Name())
			{
				return *iter;
			}
		}
		return RenderTechnique::NullObject();
	}

	
	class NullRenderTechnique : public RenderTechnique
	{
	public:
		NullRenderTechnique()
			: RenderTechnique(*RenderEffect::NullObject())
		{
		}

	private:
		void DoBegin(uint32_t /*flags*/)
		{
		}
		void DoEnd()
		{
		}

		RenderPassPtr MakeRenderPass(uint32_t /*index*/)
		{
			return RenderPass::NullObject();
		}
	};

	RenderTechniquePtr RenderTechnique::NullObject()
	{
		static RenderTechniquePtr obj(new NullRenderTechnique);
		return obj;
	}

	void RenderTechnique::Load(ResIdentifierPtr const & source)
	{
		name_ = read_short_string(source);
		source->read(reinterpret_cast<char*>(&weight_), sizeof(weight_));

		uint32_t len;
		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		for (size_t i = 0; i < len; ++ i)
		{
			annotations_.push_back(boost::shared_ptr<RenderAnnotation>(new RenderAnnotation));
			annotations_[i]->Load(source);
		}

		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		for (uint32_t i = 0; i < len; ++ i)
		{
			passes_.push_back(this->MakeRenderPass(i));
			passes_[i]->Load(source);
		}
	}

	void RenderTechnique::Begin(uint32_t flags)
	{
		this->DoBegin(flags);
	}

	void RenderTechnique::End()
	{
		return this->DoEnd();
	}

	bool RenderTechnique::Validate()
	{
		for (size_t i = 0; i < passes_.size(); ++ i)
		{
			if (!passes_[i]->Validate())
			{
				return false;
			}
		}
		return true;
	}


	class NullRenderPass : public RenderPass
	{
	public:
		NullRenderPass()
			: RenderPass(*RenderEffect::NullObject(), 0)
		{
			is_validate_ = true;
		}

		void Begin()
		{
		}
		void End()
		{
		}

		void DoRead()
		{
		}
	};

	RenderPassPtr RenderPass::NullObject()
	{
		static RenderPassPtr obj(new NullRenderPass);
		return obj;
	}

	void RenderPass::Load(ResIdentifierPtr const & source)
	{
		name_ = read_short_string(source);

		uint32_t len;
		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		for (size_t i = 0; i < len; ++ i)
		{
			annotations_.push_back(boost::shared_ptr<RenderAnnotation>(new RenderAnnotation));
			annotations_[i]->Load(source);
		}

		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		for (size_t i = 0; i < len; ++ i)
		{
			render_states_.push_back(boost::shared_ptr<RenderState>(new RenderState));
			render_states_[i]->Load(source);
		}

		this->DoRead();
	}

	uint32_t RenderPass::NumStates() const
	{
		return static_cast<uint32_t>(render_states_.size());
	}

	RenderState const & RenderPass::State(uint32_t state_id) const
	{
		return *render_states_[state_id];
	}


	class NullRenderEffectParameter : public RenderEffectParameter
	{
	public:
		NullRenderEffectParameter()
			: RenderEffectParameter(*RenderEffect::NullObject(), "", "")
		{
		}

		RenderEffectParameter& operator=(bool const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(int const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float2 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float3 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float4 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float4x4 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(SamplerPtr const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<bool> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<int> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<float> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<float4> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<float4x4> const & /*value*/)
		{
			return *this;
		}

		void Value(bool& val) const
		{
			val = false;
		}
		void Value(int& val) const
		{
			val = 0;
		}
		void Value(float& val) const
		{
			val = 0;
		}
		void Value(float2& val) const
		{
			val = float2::Zero();
		}
		void Value(float3& val) const
		{
			val = float3::Zero();
		}
		void Value(float4& val) const
		{
			val = float4::Zero();
		}
		void Value(float4x4& val) const
		{
			val = float4x4::Identity();
		}
		void Value(SamplerPtr& val) const
		{
			val = SamplerPtr();
		}
		void Value(std::vector<bool>& val) const
		{
			val.clear();
		}
		void Value(std::vector<int>& val) const
		{
			val.clear();
		}
		void Value(std::vector<float>& val) const
		{
			val.clear();
		}
		void Value(std::vector<float4>& val) const
		{
			val.clear();
		}
		void Value(std::vector<float4x4>& val) const
		{
			val.clear();
		}

		void Flush()
		{
		}

	private:
		NullRenderEffectParameter(NullRenderEffectParameter const & rhs);
		NullRenderEffectParameter& operator=(NullRenderEffectParameter const & rhs);
	};


	RenderEffectParameter::RenderEffectParameter(RenderEffect& effect,
						std::string const & name, std::string const & semantic)
		: effect_(effect), name_(name), semantic_(semantic),
			dirty_(true)
	{
	}

	RenderEffectParameter::~RenderEffectParameter()
	{
	}

	void RenderEffectParameter::Load(ResIdentifierPtr const & source)
	{
		source->read(reinterpret_cast<char*>(&array_size_), sizeof(array_size_));
		source->read(reinterpret_cast<char*>(&type_), sizeof(type_));
		name_ = read_short_string(source);
		var_ = read_var(source, type_, array_size_);

		uint32_t len;
		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		for (size_t i = 0; i < len; ++ i)
		{
			annotations_.push_back(boost::shared_ptr<RenderAnnotation>(new RenderAnnotation));
			annotations_[i]->Load(source);
		}

		semantic_ = read_short_string(source);
	}

	RenderEffectParameterPtr RenderEffectParameter::NullObject()
	{
		static RenderEffectParameterPtr obj(new NullRenderEffectParameter);
		return obj;
	}
	
	RenderEffectParameter& RenderEffectParameter::operator=(bool const & value)
	{
		if (type_define::TC_bool == type_)
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(int const & value)
	{
		if (type_define::TC_int == type_)
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float const & value)
	{
		if (type_define::TC_float == type_)
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float2 const & value)
	{
		if (type_define::TC_float2 == type_)
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float3 const & value)
	{
		if (type_define::TC_float3 == type_)
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float4 const & value)
	{
		if (type_define::TC_float4 == type_)
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float4x4 const & value)
	{
		if (type_define::TC_float4x4 == type_)
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(SamplerPtr const & value)
	{
		if (type_define::TC_sampler == type_)
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<bool> const & value)
	{
		if ((array_size_ != 0) && (type_define::TC_bool == type_))
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<int> const & value)
	{
		if ((array_size_ != 0) && (type_define::TC_int == type_))
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float> const & value)
	{
		if ((array_size_ != 0) && (type_define::TC_float == type_))
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float4> const & value)
	{
		if ((array_size_ != 0) && (type_define::TC_float4 == type_))
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float4x4> const & value)
	{
		if ((array_size_ != 0) && (type_define::TC_float4x4 == type_))
		{
			var_->set_value(value);
			this->Dirty(true);
		}
		else
		{
			BOOST_ASSERT(false);
		}
		return *this;
	}

	void RenderEffectParameter::Value(bool& val) const
	{
		if (type_define::TC_bool == type_)
		{
			val = boost::any_cast<bool>(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}

	void RenderEffectParameter::Value(int& val) const
	{
		if (type_define::TC_int == type_)
		{
			val = boost::any_cast<int>(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}

	void RenderEffectParameter::Value(float& val) const
	{
		if (type_define::TC_float == type_)
		{
			val = boost::any_cast<float>(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(float2& val) const
	{
		if (type_define::TC_float2 == type_)
		{
			val = boost::any_cast<float2>(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(float3& val) const
	{
		if (type_define::TC_float3 == type_)
		{
			val = boost::any_cast<float3>(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(float4& val) const
	{
		if (type_define::TC_float4 == type_)
		{
			val = boost::any_cast<float4>(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(float4x4& val) const
	{
		if (type_define::TC_float4x4 == type_)
		{
			val = boost::any_cast<float4x4>(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(SamplerPtr& val) const
	{
		if (type_define::TC_sampler == type_)
		{
			val = boost::any_cast<SamplerPtr>(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(std::vector<bool>& val) const
	{
		if ((array_size_ != 0) && (type_define::TC_bool == type_))
		{
			val = boost::any_cast<std::vector<bool> >(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(std::vector<int>& val) const
	{
		if ((array_size_ != 0) && (type_define::TC_int == type_))
		{
			val = boost::any_cast<std::vector<int> >(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(std::vector<float>& val) const
	{
		if ((array_size_ != 0) && (type_define::TC_float == type_))
		{
			val = boost::any_cast<std::vector<float> >(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(std::vector<float4>& val) const
	{
		if ((array_size_ != 0) && (type_define::TC_float4 == type_))
		{
			val = boost::any_cast<std::vector<float4> >(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}
	
	void RenderEffectParameter::Value(std::vector<float4x4>& val) const
	{
		if ((array_size_ != 0) && (type_define::TC_float4x4 == type_))
		{
			val = boost::any_cast<std::vector<float4x4> >(var_->get_value());
		}
		else
		{
			BOOST_ASSERT(false);
		}
	}


	void RenderShaderFunc::Load(ResIdentifierPtr const & source)
	{
		uint32_t len;
		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		str_.resize(len);
		source->read(&str_[0], len);
	}


	void RenderState::Load(ResIdentifierPtr const & source)
	{
		source->read(reinterpret_cast<char*>(&type_), sizeof(type_));
		state_ = states_define::instance().state_code(read_short_string(source));
		var_ = read_var(source, type_, 0);
	}
}
