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
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
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
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

namespace KlayGE
{
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
		is_validate_ = true;

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

			is_validate_ = false;
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
	}

	ShaderObjectPtr OGLShaderObject::Clone()
	{
		OGLShaderObjectPtr ret(new OGLShaderObject);

		ret->shader_descs_ = shader_descs_;
		ret->shader_text_ = shader_text_;
		ret->profiles_ = profiles_;

		ret->is_validate_ = true;

		OGLRenderFactory& render_factory(*checked_cast<OGLRenderFactory*>(&Context::Instance().RenderFactoryInstance()));
		for (size_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
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

				ret->is_validate_ = false;
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
}
