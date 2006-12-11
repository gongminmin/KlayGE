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
#include <KlayGE/Context.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ShaderObject.hpp>

#include <sstream>

#include <KlayGE/RenderEffect.hpp>

namespace
{
	using namespace KlayGE;

	class type_define
	{
	public:
		enum code
		{
			TC_bool = 0,
			TC_dword,
			TC_string,
			TC_sampler1D,
			TC_sampler2D,
			TC_sampler3D,
			TC_samplerCUBE,
			TC_shader,
			TC_int,
			TC_int2,
			TC_int3,
			TC_int4,
			TC_float,
			TC_float2,
			TC_float2x2,
			TC_float2x3,
			TC_float2x4,
			TC_float3,
			TC_float3x2,
			TC_float3x3,
			TC_float3x4,
			TC_float4,
			TC_float4x2,
			TC_float4x3,
			TC_float4x4
		};

	public:
		static type_define& instance()
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
			BOOST_ASSERT(false);
			return 0xFFFFFFFF;
		}

		std::string const & type_define::type_name(uint32_t code) const
		{
			if (code < types_.size())
			{
				return types_[code];
			}
			BOOST_ASSERT(false);
			return "";
		}

	private:
		type_define()
		{
			types_.push_back("bool");
			types_.push_back("dword");
			types_.push_back("string");
			types_.push_back("sampler1D");
			types_.push_back("sampler2D");
			types_.push_back("sampler3D");
			types_.push_back("samplerCUBE");
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
		boost::shared_ptr<RenderVariable> var;

		switch (type)
		{
		case type_define::TC_bool:
			if (0 == array_size)
			{
				var.reset(new RenderVariableBool);

				bool tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				*var = tmp;
			}
			break;

		case type_define::TC_dword:
		case type_define::TC_int:
			if (0 == array_size)
			{
				var.reset(new RenderVariableInt);

				int tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				*var = tmp;
			}
			break;

		case type_define::TC_string:
			{
				var.reset(new RenderVariableString);
				*var = read_short_string(source);
			}
			break;

		case type_define::TC_sampler1D:
		case type_define::TC_sampler2D:
		case type_define::TC_sampler3D:
		case type_define::TC_samplerCUBE:
			{
				var.reset(new RenderVariableSampler);
				SamplerPtr s(new Sampler);

				uint32_t tmp_int;
				source->read(reinterpret_cast<char*>(&tmp_int), sizeof(tmp_int));
				s->filter = static_cast<Sampler::TexFilterOp>(tmp_int);

				source->read(reinterpret_cast<char*>(&tmp_int), sizeof(tmp_int));
				s->addr_mode_u = static_cast<Sampler::TexAddressingMode>(tmp_int);
				source->read(reinterpret_cast<char*>(&tmp_int), sizeof(tmp_int));
				s->addr_mode_v = static_cast<Sampler::TexAddressingMode>(tmp_int);
				source->read(reinterpret_cast<char*>(&tmp_int), sizeof(tmp_int));
				s->addr_mode_w = static_cast<Sampler::TexAddressingMode>(tmp_int);

				source->read(reinterpret_cast<char*>(&tmp_int), sizeof(tmp_int));
				s->anisotropy = static_cast<uint8_t>(tmp_int);

				source->read(reinterpret_cast<char*>(&tmp_int), sizeof(tmp_int));
				s->max_mip_level = static_cast<uint8_t>(tmp_int);

				float tmp_float;
				source->read(reinterpret_cast<char*>(&tmp_float), sizeof(tmp_float));
				s->mip_map_lod_bias = tmp_float;

				Color border_clr;
				source->read(reinterpret_cast<char*>(&border_clr), sizeof(border_clr));
				s->border_clr = border_clr;
				
				*var = s;
			}
			break;

		case type_define::TC_shader:
			{
				var.reset(new RenderVariableShader);

				shader_desc desc;

				desc.profile = read_short_string(source);

				std::string value = read_short_string(source);
				desc.func_name = value.substr(0, value.find("("));

				*var = desc;
			}
			break;

		case type_define::TC_float:
			if (0 == array_size)
			{
				var.reset(new RenderVariableFloat);

				float tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				*var = tmp;
			}
			else
			{
				var.reset(new RenderVariableFloatArray);
			}
			break;

		case type_define::TC_float2:
			{
				var.reset(new RenderVariableFloat2);

				float2 tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				*var = tmp;
			}
			break;

		case type_define::TC_float3:
			{
				var.reset(new RenderVariableFloat3);

				float3 tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				*var = tmp;
			}
			break;

		case type_define::TC_float4:
			if (0 == array_size)
			{
				var.reset(new RenderVariableFloat4);

				float4 tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				*var = tmp;
			}
			else
			{
				var.reset(new RenderVariableFloat4Array);
			}
			break;

		case type_define::TC_float4x4:
			if (0 == array_size)
			{
				var.reset(new RenderVariableFloat4x4);

				float4x4 tmp;
				source->read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
				*var = tmp;
			}
			else
			{
				var.reset(new RenderVariableFloat4x4Array);
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
	void RenderEffectAnnotation::Load(ResIdentifierPtr const & source)
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
				params_.push_back(RenderEffectParameterPtr(new RenderEffectParameter(*this)));
				params_[i]->Load(source);
			}

			for (uint32_t i = 0; i < header.num_shaders; ++ i)
			{
				shaders_.push_back(RenderShaderFunc());
				shaders_[i].Load(source);
			}

			for (uint32_t i = 0; i < header.num_techniques; ++ i)
			{
				techniques_.push_back(RenderTechniquePtr(new RenderTechnique(*this)));
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
			annotations_.push_back(boost::shared_ptr<RenderEffectAnnotation>(new RenderEffectAnnotation));
			annotations_[i]->Load(source);
		}

		is_validate_ = true;

		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		for (uint32_t i = 0; i < len; ++ i)
		{
			passes_.push_back(RenderPassPtr(new RenderPass(effect_)));
			passes_[i]->Load(source);

			is_validate_ &= passes_[i]->Validate();
		}
	}

	void RenderTechnique::Begin()
	{
	}

	void RenderTechnique::End()
	{
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

	RenderPassPtr RenderPass::NullObject()
	{
		static RenderPassPtr obj(new NullRenderPass);
		return obj;
	}

	void RenderPass::Load(ResIdentifierPtr const & source)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		name_ = read_short_string(source);

		uint32_t len;
		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		for (size_t i = 0; i < len; ++ i)
		{
			annotations_.push_back(boost::shared_ptr<RenderEffectAnnotation>(new RenderEffectAnnotation));
			annotations_[i]->Load(source);
		}

		render_state_obj_.reset(new RenderStateObject);
		shader_obj_ = rf.MakeShaderObject();

		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		for (size_t i = 0; i < len; ++ i)
		{
			uint32_t type;
			source->read(reinterpret_cast<char*>(&type), sizeof(type));
			std::string state_name = read_short_string(source);
			boost::shared_ptr<RenderVariable> var = read_var(source, type, 0);

			if (type_define::TC_shader != type)
			{
				uint32_t state_val;
				switch (type)
				{
				case type_define::TC_bool:
					{
						bool tmp;
						var->Value(tmp);
						state_val = tmp;
					}
					break;

				case type_define::TC_int:
					{
						int tmp;
						var->Value(tmp);
						state_val = tmp;
					}
					break;

				case type_define::TC_float:
					{
						float tmp;
						var->Value(tmp);
						state_val = float_to_uint32(tmp);
					}
					break;

				default:
					BOOST_ASSERT(false);
					state_val = 0;
					break;
				}

				if ("polygon_mode" == state_name)
				{
					render_state_obj_->polygon_mode = static_cast<RenderStateObject::PolygonMode>(state_val);
				}
				if ("shade_mode" == state_name)
				{
					render_state_obj_->shade_mode = static_cast<RenderStateObject::ShadeMode>(state_val);
				}
				if ("cull_mode" == state_name)
				{
					render_state_obj_->cull_mode = static_cast<RenderStateObject::CullMode>(state_val);
				}

				if ("alpha_to_coverage_enable" == state_name)
				{
					render_state_obj_->alpha_to_coverage_enable = state_val ? true : false;
				}
				if ("blend_enable" == state_name)
				{
					render_state_obj_->blend_enable = state_val ? true : false;
				}
				if ("blend_op" == state_name)
				{
					render_state_obj_->blend_op = static_cast<RenderStateObject::BlendOperation>(state_val);
				}
				if ("src_blend" == state_name)
				{
					render_state_obj_->src_blend = static_cast<RenderStateObject::AlphaBlendFactor>(state_val);
				}
				if ("dest_blend" == state_name)
				{
					render_state_obj_->dest_blend = static_cast<RenderStateObject::AlphaBlendFactor>(state_val);
				}
				if ("blend_op_alpha" == state_name)
				{
					render_state_obj_->blend_op_alpha = static_cast<RenderStateObject::BlendOperation>(state_val);
				}
				if ("src_blend_alpha" == state_name)
				{
					render_state_obj_->src_blend_alpha = static_cast<RenderStateObject::AlphaBlendFactor>(state_val);
				}
				if ("dest_blend_alpha" == state_name)
				{
					render_state_obj_->dest_blend_alpha = static_cast<RenderStateObject::AlphaBlendFactor>(state_val);
				}

				if ("depth_enable" == state_name)
				{
					render_state_obj_->depth_enable = state_val ? true : false;
				}
				if ("depth_mask" == state_name)
				{
					render_state_obj_->depth_mask = state_val ? true : false;
				}
				if ("depth_func" == state_name)
				{
					render_state_obj_->depth_func = static_cast<RenderStateObject::CompareFunction>(state_val);
				}
				if ("polygon_offset_factor" == state_name)
				{
					render_state_obj_->polygon_offset_factor = uint32_to_float(state_val);
				}
				if ("polygon_offset_units" == state_name)
				{
					render_state_obj_->polygon_offset_units = uint32_to_float(state_val);
				}

				if ("front_stencil_enable" == state_name)
				{
					render_state_obj_->front_stencil_enable = state_val ? true : false;
				}
				if ("front_stencil_func" == state_name)
				{
					render_state_obj_->front_stencil_func = static_cast<RenderStateObject::CompareFunction>(state_val);
				}
				if ("front_stencil_ref" == state_name)
				{
					render_state_obj_->front_stencil_ref = static_cast<uint16_t>(state_val);
				}
				if ("front_stencil_mask" == state_name)
				{
					render_state_obj_->front_stencil_mask = static_cast<uint16_t>(state_val);
				}
				if ("front_stencil_fail" == state_name)
				{
					render_state_obj_->front_stencil_fail = static_cast<RenderStateObject::StencilOperation>(state_val);
				}
				if ("front_stencil_depth_fail" == state_name)
				{
					render_state_obj_->front_stencil_depth_fail = static_cast<RenderStateObject::StencilOperation>(state_val);
				}
				if ("front_stencil_pass" == state_name)
				{
					render_state_obj_->front_stencil_pass = static_cast<RenderStateObject::StencilOperation>(state_val);
				}
				if ("front_stencil_write_mask" == state_name)
				{
					render_state_obj_->front_stencil_write_mask = static_cast<uint16_t>(state_val);
				}
				if ("back_stencil_enable" == state_name)
				{
					render_state_obj_->back_stencil_enable = state_val ? true : false;
				}
				if ("back_stencil_func" == state_name)
				{
					render_state_obj_->back_stencil_func = static_cast<RenderStateObject::CompareFunction>(state_val);
				}
				if ("back_stencil_ref" == state_name)
				{
					render_state_obj_->back_stencil_ref = static_cast<uint16_t>(state_val);
				}
				if ("back_stencil_mask" == state_name)
				{
					render_state_obj_->back_stencil_mask = static_cast<uint16_t>(state_val);
				}
				if ("back_stencil_fail" == state_name)
				{
					render_state_obj_->back_stencil_fail = static_cast<RenderStateObject::StencilOperation>(state_val);
				}
				if ("back_stencil_depth_fail" == state_name)
				{
					render_state_obj_->back_stencil_depth_fail = static_cast<RenderStateObject::StencilOperation>(state_val);
				}
				if ("back_stencil_pass" == state_name)
				{
					render_state_obj_->back_stencil_pass = static_cast<RenderStateObject::StencilOperation>(state_val);
				}
				if ("back_stencil_write_mask" == state_name)
				{
					render_state_obj_->back_stencil_write_mask = static_cast<uint16_t>(state_val);
				}

				if ("scissor_enable" == state_name)
				{
					render_state_obj_->scissor_enable = state_val ? true : false;
				}

				if ("color_mask_0" == state_name)
				{
					render_state_obj_->color_mask_0 = static_cast<uint8_t>(state_val);
				}
				if ("color_mask_1" == state_name)
				{
					render_state_obj_->color_mask_1 = static_cast<uint8_t>(state_val);
				}
				if ("color_mask_2" == state_name)
				{
					render_state_obj_->color_mask_2 = static_cast<uint8_t>(state_val);
				}
				if ("color_mask_3" == state_name)
				{
					render_state_obj_->color_mask_3 = static_cast<uint8_t>(state_val);
				}
			}
			else
			{
				if ("vertex_shader" == state_name)
				{
					shader_desc sd;
					var->Value(sd);

					shader_obj_->SetShader(ShaderObject::ST_VertexShader, sd.profile, sd.func_name, this->GenShaderText());
				}
				if ("pixel_shader" == state_name)
				{
					shader_desc sd;
					var->Value(sd);

					shader_obj_->SetShader(ShaderObject::ST_PixelShader, sd.profile, sd.func_name, this->GenShaderText());
				}
			}
		}

		is_validate_ = shader_obj_->Validate();

		for (uint32_t i = 0; i < effect_.NumParameters(); ++ i)
		{
			for (size_t j = 0; j < ShaderObject::ST_NumShaderTypes; ++ j)
			{
				ShaderObject::ShaderType type = static_cast<ShaderObject::ShaderType>(j);

				RenderEffectParameterPtr param = effect_.ParameterByIndex(i);
				if (shader_obj_->HasParameter(type, param->Name()))
				{
					param_descs_[type].push_back(param);
				}
			}
		}
	}

	void RenderPass::Begin()
	{
		for (size_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			for (size_t j = 0; j < param_descs_[i].size(); ++ j)
			{
				RenderEffectParameterPtr param = param_descs_[i][j];
				if (param->IsDirty())
				{
					switch (param->type())
					{
					case type_define::TC_bool:
						if (param->ArraySize() != 0)
						{
							std::vector<bool> tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						else
						{
							bool tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						break;

					case type_define::TC_dword:
					case type_define::TC_int:
						if (param->ArraySize() != 0)
						{
							std::vector<int> tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						else
						{
							int tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						break;

					case type_define::TC_float:
						if (param->ArraySize() != 0)
						{
							std::vector<float> tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						else
						{
							float tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						break;

					case type_define::TC_float2:
						{
							float2 tmp;
							param->Value(tmp);
							float4 v4(tmp.x(), tmp.y(), 0, 0);
							shader_obj_->SetParameter(param->Name(), v4);
						}
						break;

					case type_define::TC_float3:
						{
							float3 tmp;
							param->Value(tmp);
							float4 v4(tmp.x(), tmp.y(), tmp.z(), 0);
							shader_obj_->SetParameter(param->Name(), v4);
						}
						break;

					case type_define::TC_float4:
						if (param->ArraySize() != 0)
						{
							std::vector<float4> tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						else
						{
							float4 tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						break;

					case type_define::TC_float4x4:
						if (param->ArraySize() != 0)
						{
							std::vector<float4x4> tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						else
						{
							float4x4 tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						break;

					case type_define::TC_sampler1D:
					case type_define::TC_sampler2D:
					case type_define::TC_sampler3D:
					case type_define::TC_samplerCUBE:
						{
							SamplerPtr tmp;
							param->Value(tmp);
							shader_obj_->SetParameter(param->Name(), tmp);
						}
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}

					//param->Dirty(false);
				}
			}
		}

		RenderEngine& render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		render_eng.SetStateObjects(*render_state_obj_, *shader_obj_);
	}

	void RenderPass::End()
	{
	}

	std::string RenderPass::GenShaderText() const
	{
		std::stringstream ss;
		for (uint32_t i = 0; i < effect_.NumParameters(); ++ i)
		{
			RenderEffectParameter& param = *effect_.ParameterByIndex(i);

			ss << type_define::instance().type_name(param.type()) << " " << param.Name();
			if (param.ArraySize() != 0)
			{
				ss << "[" << param.ArraySize() << "]";
			}

			ss << ";" << std::endl;
		}

		for (uint32_t i = 0; i < effect_.NumShaders(); ++ i)
		{
			ss << effect_.ShaderByIndex(i).str() << std::endl;
		}

		return ss.str();
	}



	class NullRenderEffectParameter : public RenderEffectParameter
	{
	public:
		NullRenderEffectParameter()
			: RenderEffectParameter(*RenderEffect::NullObject())
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


	RenderEffectParameter::RenderEffectParameter(RenderEffect& effect)
		: effect_(effect)
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
			annotations_.push_back(boost::shared_ptr<RenderEffectAnnotation>(new RenderEffectAnnotation));
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
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(int const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float2 const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float3 const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float4 const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float4x4 const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(TexturePtr const & value)
	{
		SamplerPtr s;
		var_->Value(s);
		BOOST_ASSERT(s);

		s->texture = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<bool> const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<int> const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float> const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float4> const & value)
	{
		*var_ = value;
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float4x4> const & value)
	{
		*var_ = value;
		return *this;
	}

	void RenderEffectParameter::Value(bool& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(int& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(float& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(float2& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(float3& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(float4& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(float4x4& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(SamplerPtr& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(std::vector<bool>& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(std::vector<int>& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(std::vector<float>& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(std::vector<float4>& val) const
	{
		var_->Value(val);
	}

	void RenderEffectParameter::Value(std::vector<float4x4>& val) const
	{
		var_->Value(val);
	}


	void RenderShaderFunc::Load(ResIdentifierPtr const & source)
	{
		uint32_t len;
		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		str_.resize(len);
		source->read(&str_[0], len);
	}


	RenderVariable::RenderVariable()
		: dirty_(true)
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

	RenderVariable& RenderVariable::operator=(int const & /*value*/)
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

	RenderVariable& RenderVariable::operator=(SamplerPtr const & /*value*/)
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

	RenderVariable& RenderVariable::operator=(std::vector<int> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderVariable& RenderVariable::operator=(std::vector<float> const & /*value*/)
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

	void RenderVariable::Value(int& /*value*/) const
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

	void RenderVariable::Value(SamplerPtr& /*value*/) const
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

	void RenderVariable::Value(std::vector<int>& /*value*/) const
	{
		BOOST_ASSERT(false);
	}

	void RenderVariable::Value(std::vector<float>& /*value*/) const
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
