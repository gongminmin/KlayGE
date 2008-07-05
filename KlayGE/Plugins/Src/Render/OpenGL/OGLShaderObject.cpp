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
#include <boost/bind.hpp>

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
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::reference pb, param_binds_[i])
			{
				cgDestroyParameter(pb.cg_param);
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
					param_binds_[type].push_back(this->GetBindFunc(cg_param, p, type));
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
						ret->param_binds_[i].push_back(ret->GetBindFunc(cg_param, p, static_cast<ShaderType>(i)));
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

	void OGLShaderObject::SetBool(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		bool v;
		param->Value(v);

		cgSetParameter1i(cg_param, v);
	}

	void OGLShaderObject::SetInt(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		int v;
		param->Value(v);

		cgSetParameter1i(cg_param, v);
	}

	void OGLShaderObject::SetFloat(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		float v;
		param->Value(v);

		cgSetParameter1f(cg_param, v);
	}

	void OGLShaderObject::SetFloat2(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		float2 v;
		param->Value(v);

		cgSetParameter2fv(cg_param, &v[0]);
	}
	
	void OGLShaderObject::SetFloat3(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		float3 v;
		param->Value(v);

		cgSetParameter3fv(cg_param, &v[0]);
	}

	void OGLShaderObject::SetFloat4(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		float4 v;
		param->Value(v);

		cgSetParameter4fv(cg_param, &v[0]);
	}

	void OGLShaderObject::SetFloat4x4(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		float4x4 v;
		param->Value(v);

		cgGLSetMatrixParameterfr(cg_param, &v[0]);
	}

	void OGLShaderObject::SetBoolArray(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		std::vector<bool> v;
		param->Value(v);

		if (!v.empty())
		{
			std::vector<int> tmp(v.begin(), v.end());
			cgSetParameterValueir(cg_param, static_cast<int>(tmp.size()), &tmp[0]);
		}
	}

	void OGLShaderObject::SetIntArray(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		std::vector<int> v;
		param->Value(v);

		if (!v.empty())
		{
			cgSetParameterValueir(cg_param, static_cast<int>(v.size()), &v[0]);
		}
	}

	void OGLShaderObject::SetFloatArray(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		std::vector<float> v;
		param->Value(v);

		if (!v.empty())
		{
			cgGLSetParameterArray1f(cg_param, 0, static_cast<int>(v.size()), &v[0]);
		}
	}

	void OGLShaderObject::SetFloat4Array(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		std::vector<float4> v;
		param->Value(v);

		if (!v.empty())
		{
			cgGLSetParameterArray4f(cg_param, 0, static_cast<long>(v.size()), &v[0][0]);
		}
	}

	void OGLShaderObject::SetFloat4x4Array(CGparameter cg_param, RenderEffectParameterPtr const & param)
	{
		std::vector<float4x4> v;
		param->Value(v);

		if (!v.empty())
		{
			cgGLSetMatrixParameterArrayfr(cg_param, 0, static_cast<long>(v.size()), &v[0][0]);
		}
	}

	void OGLShaderObject::SetSampler(CGparameter cg_param, ShaderType type, RenderEffectParameterPtr const & param)
	{
		uint32_t index = cgGLGetTextureEnum(cg_param) - GL_TEXTURE0;
		BOOST_ASSERT(index < samplers_[type].size());

		SamplerPtr v;
		param->Value(v);

		samplers_[type][index] = v;
	}

	OGLShaderObject::parameter_bind_t OGLShaderObject::GetBindFunc(CGparameter cg_param, RenderEffectParameterPtr const & param, ShaderType type)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.cg_param = cg_param;
		ret.type = type;

		switch (param->type())
		{
		case REDT_bool:
			if (param->ArraySize() != 0)
			{
				ret.func = boost::bind(&OGLShaderObject::SetBoolArray, this, _1, _2);
			}
			else
			{
				ret.func = boost::bind(&OGLShaderObject::SetBool, this, _1, _2);
			}
			break;

		case REDT_dword:
		case REDT_int:
			if (param->ArraySize() != 0)
			{
				ret.func = boost::bind(&OGLShaderObject::SetIntArray, this, _1, _2);
			}
			else
			{
				ret.func = boost::bind(&OGLShaderObject::SetInt, this, _1, _2);
			}
			break;

		case REDT_float:
			if (param->ArraySize() != 0)
			{
				ret.func = boost::bind(&OGLShaderObject::SetFloatArray, this, _1, _2);
			}
			else
			{
				ret.func = boost::bind(&OGLShaderObject::SetFloat, this, _1, _2);
			}
			break;

		case REDT_float2:
			ret.func = boost::bind(&OGLShaderObject::SetFloat2, this, _1, _2);
			break;

		case REDT_float3:
			ret.func = boost::bind(&OGLShaderObject::SetFloat3, this, _1, _2);
			break;

		case REDT_float4:
			if (param->ArraySize() != 0)
			{
				ret.func = boost::bind(&OGLShaderObject::SetFloat4Array, this, _1, _2);
			}
			else
			{
				ret.func = boost::bind(&OGLShaderObject::SetFloat4, this, _1, _2);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize() != 0)
			{
				ret.func = boost::bind(&OGLShaderObject::SetFloat4x4Array, this, _1, _2);
			}
			else
			{
				ret.func = boost::bind(&OGLShaderObject::SetFloat4x4, this, _1, _2);
			}
			break;

		case REDT_sampler1D:
		case REDT_sampler2D:
		case REDT_sampler3D:
		case REDT_samplerCUBE:
			ret.func = boost::bind(&OGLShaderObject::SetSampler, this, _1, type, _2);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void OGLShaderObject::Active()
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		cgGLBindProgram(shaders_[ST_VertexShader]);
		cgGLEnableProfile(profiles_[ST_VertexShader]);
		cgGLBindProgram(shaders_[ST_PixelShader]);
		cgGLEnableProfile(profiles_[ST_PixelShader]);

		for (int i = 0; i < ST_NumShaderTypes; ++ i)
		{
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::reference pb, param_binds_[i])
			{
				pb.func(pb.cg_param, pb.param);
			}

			std::vector<SamplerPtr> const & samplers = samplers_[i];

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
