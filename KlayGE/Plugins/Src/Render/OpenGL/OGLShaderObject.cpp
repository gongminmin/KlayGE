// OGLShaderObject.cpp
// KlayGE OpenGL shader对象类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Sampler.hpp>

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

	void OGLShaderObject::SetShader(ShaderType type, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs,
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
				param_descs_[type].insert(std::make_pair(new std::string(cgGetParameterName(cg_param)), cg_param));
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

	ShaderObjectPtr OGLShaderObject::Clone()
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
					ret->param_descs_[i].insert(std::make_pair(new std::string(cgGetParameterName(cg_param)), cg_param));
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

	OGLShaderObject::parameter_descs_t::const_iterator OGLShaderObject::FindParam(ShaderType type, boost::shared_ptr<std::string> const & name) const
	{
		return param_descs_[type].find(name);
	}

	bool OGLShaderObject::HasParameter(ShaderType type, boost::shared_ptr<std::string> const & name) const
	{
		return (this->FindParam(type, name) != param_descs_[type].end());
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, bool value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::const_iterator iter = this->FindParam(type, name);
			if (iter != param_descs_[type].end())
			{
				cgSetParameter1i(iter->second, value);
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, int value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::const_iterator iter = this->FindParam(type, name);
			if (iter != param_descs_[type].end())
			{
				cgSetParameter1i(iter->second, value);
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, float value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::const_iterator iter = this->FindParam(type, name);
			if (iter != param_descs_[type].end())
			{
				cgSetParameter1f(iter->second, value);
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, float4 const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::const_iterator iter = this->FindParam(type, name);
			if (iter != param_descs_[type].end())
			{
				cgSetParameter4fv(iter->second, &value[0]);
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, float4x4 const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::const_iterator iter = this->FindParam(type, name);
			if (iter != param_descs_[type].end())
			{
				cgGLSetMatrixParameterfr(iter->second, &value[0]);
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, std::vector<bool> const & value)
	{
		if (!value.empty())
		{
			for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
			{
				ShaderType type = static_cast<ShaderType>(i);

				parameter_descs_t::const_iterator iter = this->FindParam(type, name);
				if (iter != param_descs_[type].end())
				{
					std::vector<int> tmp(value.begin(), value.end());
					cgSetParameterValueir(iter->second, static_cast<int>(tmp.size()), &tmp[0]);
				}
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, std::vector<int> const & value)
	{
		if (!value.empty())
		{
			for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
			{
				ShaderType type = static_cast<ShaderType>(i);

				parameter_descs_t::const_iterator iter = this->FindParam(type, name);
				if (iter != param_descs_[type].end())
				{
					cgSetParameterValueir(iter->second, static_cast<int>(value.size()), &value[0]);
				}
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float> const & value)
	{
		if (!value.empty())
		{
			for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
			{
				ShaderType type = static_cast<ShaderType>(i);

				parameter_descs_t::const_iterator iter = this->FindParam(type, name);
				if (iter != param_descs_[type].end())
				{
					cgGLSetParameterArray1f(iter->second, 0, static_cast<int>(value.size()), &value[0]);
				}
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float4> const & value)
	{
		if (!value.empty())
		{
			for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
			{
				ShaderType type = static_cast<ShaderType>(i);

				parameter_descs_t::const_iterator iter = this->FindParam(type, name);
				if (iter != param_descs_[type].end())
				{
					cgGLSetParameterArray4f(iter->second, 0, static_cast<long>(value.size()), &value[0][0]);
				}
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float4x4> const & value)
	{
		if (!value.empty())
		{
			for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
			{
				ShaderType type = static_cast<ShaderType>(i);

				parameter_descs_t::const_iterator iter = this->FindParam(type, name);
				if (iter != param_descs_[type].end())
				{
					cgGLSetMatrixParameterArrayfr(iter->second, 0, static_cast<long>(value.size()), &value[0][0]);
				}
			}
		}
	}

	void OGLShaderObject::SetParameter(boost::shared_ptr<std::string> const & name, SamplerPtr const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::const_iterator iter = this->FindParam(type, name);
			if (iter != param_descs_[type].end())
			{
				uint32_t index = cgGLGetTextureEnum(iter->second) - GL_TEXTURE0;

				BOOST_ASSERT(index < samplers_[type].size());
				samplers_[type][index] = value;
			}
		}
	}

	void OGLShaderObject::Active()
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		cgGLBindProgram(this->VertexShader());
		cgGLEnableProfile(this->VertexShaderProfile());
		cgGLBindProgram(this->PixelShader());
		cgGLEnableProfile(this->PixelShaderProfile());

		for (int i = 0; i < ST_NumShaderTypes; ++ i)
		{
			std::vector<SamplerPtr> const & samplers = this->Samplers(static_cast<ShaderType>(i));

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
