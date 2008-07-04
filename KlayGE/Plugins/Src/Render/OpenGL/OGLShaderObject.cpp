// OGLShaderObject.cpp
// KlayGE OpenGL shader对象类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 改为直接传入RenderEffect (2008.7.4)
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4512)
#endif
#include <boost/assign.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/assign/std/vector.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

namespace KlayGE
{
	OGLShaderObject::OGLShaderObject()
	{
		is_shader_validate_.assign(true);
	}

	OGLShaderObject::~OGLShaderObject()
	{
		for (int i = 0; i < ST_NumShaderTypes; ++ i)
		{
			BOOST_FOREACH(BOOST_TYPEOF(param_descs_[i])::reference desc, param_descs_[i])
			{
				cgDestroyParameter(desc.second);
			}

			cgDestroyProgram(shaders_[i]);
		}
	}

	void OGLShaderObject::SetShader(RenderEffect& effect, ShaderType type, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs,
			boost::shared_ptr<std::string> const & shader_text)
	{
		is_shader_validate_[type] = true;

		shader_descs_ = shader_descs;
		shader_text_ = shader_text;

		std::string profile = (*shader_descs_)[type].profile;
		switch (type)
		{
		case ST_VertexShader:
			if ("auto" == profile)
			{
				profiles_[type] = cgGLGetLatestProfile(CG_GL_VERTEX);
			}
			else
			{
				profiles_[type] = cgGetProfile(profile.c_str());
			}
			break;

		case ST_PixelShader:
			if ("auto" == profile)
			{
				profiles_[type] = cgGLGetLatestProfile(CG_GL_FRAGMENT);
			}
			else
			{
				profiles_[type] = cgGetProfile(profile.c_str());
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		OGLRenderFactory& render_factory(*checked_cast<OGLRenderFactory*>(&Context::Instance().RenderFactoryInstance()));

		shaders_[type] = cgCreateProgram(render_factory.CGContext(),
				CG_SOURCE, shader_text->c_str(), profiles_[type], (*shader_descs_)[type].func_name.c_str(), NULL);

		CGerror error;
		char const * err_string = cgGetLastErrorString(&error);
		if (error != CG_NO_ERROR)
		{
#ifdef KLAYGE_DEBUG
			std::cerr << *shader_text << std::endl;
			std::cerr << err_string << std::endl;
			if (CG_COMPILER_ERROR == error)
			{
				std::cerr << cgGetLastListing(render_factory.CGContext()) << std::endl;
			}
#else
			UNREF_PARAM(err_string);
#endif

			is_shader_validate_[type] = false;
		}

		cgGLLoadProgram(shaders_[type]);

		CGparameter cg_param = cgGetFirstParameter(shaders_[type], CG_GLOBAL);
		while (cg_param)
		{
			if (cgIsParameterUsed(cg_param, shaders_[type])
				&& (CG_PARAMETERCLASS_OBJECT != cgGetParameterClass(cg_param)))
			{
				RenderEffectParameterPtr const & p = effect.ParameterByName(cgGetParameterName(cg_param));
				if (p != RenderEffectParameter::NullObject())
				{
					param_descs_[type].insert(std::make_pair(p, cg_param));
				}
			}

			cg_param = cgGetNextParameter(cg_param);
		}

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		switch (type)
		{
		case ST_VertexShader:
			samplers_[type].resize(re.DeviceCaps().max_vertex_texture_units);
			break;

		case ST_PixelShader:
			samplers_[type].resize(re.DeviceCaps().max_texture_units);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		is_validate_ = true;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			is_validate_ &= is_shader_validate_[i];
		}
	}

	ShaderObjectPtr OGLShaderObject::Clone(RenderEffect& effect)
	{
		OGLShaderObjectPtr ret(new OGLShaderObject);

		ret->shader_descs_ = shader_descs_;
		ret->shader_text_ = shader_text_;
		ret->profiles_ = profiles_;

		OGLRenderFactory& render_factory(*checked_cast<OGLRenderFactory*>(&Context::Instance().RenderFactoryInstance()));
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->is_shader_validate_[i] = true;

			ret->shaders_[i] = cgCreateProgram(render_factory.CGContext(),
					CG_SOURCE, ret->shader_text_->c_str(), ret->profiles_[i],
					(*ret->shader_descs_)[i].func_name.c_str(), NULL);

			CGerror error;
			char const * err_string = cgGetLastErrorString(&error);
			if (error != CG_NO_ERROR)
			{
#ifdef KLAYGE_DEBUG
				std::cerr << *ret->shader_text_ << std::endl;
				std::cerr << err_string << std::endl;
				if (CG_COMPILER_ERROR == error)
				{
					std::cerr << cgGetLastListing(render_factory.CGContext()) << std::endl;
				}
#else
				UNREF_PARAM(err_string);
#endif

				ret->is_shader_validate_[i] = false;
			}

			cgGLLoadProgram(ret->shaders_[i]);

			CGparameter cg_param = cgGetFirstParameter(ret->shaders_[i], CG_GLOBAL);
			while (cg_param)
			{
				if (cgIsParameterUsed(cg_param, ret->shaders_[i])
					&& (CG_PARAMETERCLASS_OBJECT != cgGetParameterClass(cg_param)))
				{
					RenderEffectParameterPtr const & p = effect.ParameterByName(cgGetParameterName(cg_param));
					if (p != RenderEffectParameter::NullObject())
					{
						ret->param_descs_[i].insert(std::make_pair(p, cg_param));
					}
				}

				cg_param = cgGetNextParameter(cg_param);
			}

			ret->samplers_[i].resize(samplers_[i].size());
		}

		ret->is_validate_ = true;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->is_validate_ &= ret->is_shader_validate_[i];
		}

		return ret;
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, bool value)
	{
		cgSetParameter1i(cg_param, value);
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, int value)
	{
		cgSetParameter1i(cg_param, value);
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, float value)
	{
		cgSetParameter1f(cg_param, value);
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, float4 const & value)
	{
		cgSetParameter4fv(cg_param, &value[0]);
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, float4x4 const & value)
	{
		cgGLSetMatrixParameterfr(cg_param, &value[0]);
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, std::vector<bool> const & value)
	{
		if (!value.empty())
		{
			std::vector<int> tmp(value.begin(), value.end());
			cgSetParameterValueir(cg_param, static_cast<int>(tmp.size()), &tmp[0]);
		}
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, std::vector<int> const & value)
	{
		if (!value.empty())
		{
			cgSetParameterValueir(cg_param, static_cast<int>(value.size()), &value[0]);
		}
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, std::vector<float> const & value)
	{
		if (!value.empty())
		{
			cgGLSetParameterArray1f(cg_param, 0, static_cast<int>(value.size()), &value[0]);
		}
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, std::vector<float4> const & value)
	{
		if (!value.empty())
		{
			cgGLSetParameterArray4f(cg_param, 0, static_cast<long>(value.size()), &value[0][0]);
		}
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, std::vector<float4x4> const & value)
	{
		if (!value.empty())
		{
			cgGLSetMatrixParameterArrayfr(cg_param, 0, static_cast<long>(value.size()), &value[0][0]);
		}
	}

	void OGLShaderObject::SetParameter(CGparameter cg_param, ShaderType type, SamplerPtr const & value)
	{
		uint32_t index = cgGLGetTextureEnum(cg_param) - GL_TEXTURE0;

		BOOST_ASSERT(index < samplers_[type].size());
		samplers_[type][index] = value;
	}

	void OGLShaderObject::Active()
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			for (parameter_descs_t::iterator iter = param_descs_[i].begin(); iter != param_descs_[i].end(); ++ iter)
			{
				RenderEffectParameterPtr const & param = iter->first;
				if (param->IsDirty())
				{
					switch (param->type())
					{
					case REDT_bool:
						if (param->ArraySize() != 0)
						{
							std::vector<bool> tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						else
						{
							bool tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						break;

					case REDT_dword:
					case REDT_int:
						if (param->ArraySize() != 0)
						{
							std::vector<int> tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						else
						{
							int tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						break;

					case REDT_float:
						if (param->ArraySize() != 0)
						{
							std::vector<float> tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						else
						{
							float tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						break;

					case REDT_float2:
						{
							float2 tmp;
							param->Value(tmp);
							float4 v4(tmp.x(), tmp.y(), 0, 0);
							this->SetParameter(iter->second, v4);
						}
						break;

					case REDT_float3:
						{
							float3 tmp;
							param->Value(tmp);
							float4 v4(tmp.x(), tmp.y(), tmp.z(), 0);
							this->SetParameter(iter->second, v4);
						}
						break;

					case REDT_float4:
						if (param->ArraySize() != 0)
						{
							std::vector<float4> tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						else
						{
							float4 tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						break;

					case REDT_float4x4:
						if (param->ArraySize() != 0)
						{
							std::vector<float4x4> tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						else
						{
							float4x4 tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, tmp);
						}
						break;

					case REDT_sampler1D:
					case REDT_sampler2D:
					case REDT_sampler3D:
					case REDT_samplerCUBE:
						{
							SamplerPtr tmp;
							param->Value(tmp);
							this->SetParameter(iter->second, static_cast<ShaderType>(i), tmp);
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

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		cgGLBindProgram(shaders_[ST_VertexShader]);
		cgGLEnableProfile(profiles_[ST_VertexShader]);
		cgGLBindProgram(shaders_[ST_PixelShader]);
		cgGLEnableProfile(profiles_[ST_PixelShader]);

		for (int i = 0; i < ST_NumShaderTypes; ++ i)
		{
			std::vector<SamplerPtr> const & samplers = samplers_[static_cast<ShaderType>(i)];

			for (uint32_t stage = 0, num_stage = static_cast<uint32_t>(samplers.size()); stage < num_stage; ++ stage)
			{
				SamplerPtr const & sampler = samplers[stage];
				if (!sampler || !sampler->texture)
				{
					glActiveTexture(GL_TEXTURE0 + stage);

					glBindTexture(GL_TEXTURE_2D, 0);
				}
				else
				{
					glActiveTexture(GL_TEXTURE0 + stage);

					OGLTexture& gl_tex = *checked_pointer_cast<OGLTexture>(sampler->texture);
					GLenum tex_type = gl_tex.GLType();

					glBindTexture(tex_type, gl_tex.GLTexture());

					re.TexParameter(tex_type, GL_TEXTURE_WRAP_S, OGLMapping::Mapping(sampler->addr_mode_u));
					re.TexParameter(tex_type, GL_TEXTURE_WRAP_T, OGLMapping::Mapping(sampler->addr_mode_v));
					re.TexParameter(tex_type, GL_TEXTURE_WRAP_R, OGLMapping::Mapping(sampler->addr_mode_w));

					{
						float tmp[4];
						glGetTexParameterfv(tex_type, GL_TEXTURE_BORDER_COLOR, tmp);
						if ((tmp[0] != sampler->border_clr.r())
							|| (tmp[1] != sampler->border_clr.g())
							|| (tmp[2] != sampler->border_clr.b())
							|| (tmp[3] != sampler->border_clr.a()))
						{
							glTexParameterfv(tex_type, GL_TEXTURE_BORDER_COLOR, &sampler->border_clr.r());
						}
					}

					switch (sampler->filter)
					{
					case Sampler::TFO_Point:
						re.TexParameter(tex_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						re.TexParameter(tex_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						break;

					case Sampler::TFO_Bilinear:
						re.TexParameter(tex_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						re.TexParameter(tex_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
						break;

					case Sampler::TFO_Trilinear:
					case Sampler::TFO_Anisotropic:
						re.TexParameter(tex_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						re.TexParameter(tex_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}

					re.TexParameter(tex_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, sampler->anisotropy);
					re.TexParameter(tex_type, GL_TEXTURE_MAX_LEVEL, sampler->max_mip_level);
					re.TexEnv(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, sampler->mip_map_lod_bias);
				}
			}
		}
	}
}
