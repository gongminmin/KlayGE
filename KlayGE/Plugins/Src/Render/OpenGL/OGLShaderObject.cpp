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
			for (parameter_descs_t::iterator iter = param_descs_[i].begin();
				iter != param_descs_[i].end(); ++ iter)
			{
				cgDestroyParameter(iter->second);
			}

			cgDestroyProgram(shaders_[i]);
		}
	}

	void OGLShaderObject::SetShader(ShaderType type, std::string const & profile, std::string const & name, std::string const & text)
	{
		is_validate_ = true;

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
				CG_SOURCE, text.c_str(), profiles_[type], name.c_str(), NULL);

		CGerror error;
		char const * err_string = cgGetLastErrorString(&error);
		if (error != CG_NO_ERROR)
		{
#ifdef KLAYGE_DEBUG
			std::cerr << text << std::endl;
			std::cerr << err_string << std::endl;
			if (CG_COMPILER_ERROR == error)
			{
				std::cerr << cgGetLastListing(render_factory.CGContext()) << std::endl;
			}
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
				std::pair<std::string, CGparameter> p_desc;
				param_descs_[type].insert(std::make_pair(cgGetParameterName(cg_param), cg_param));
			}

			cg_param = cgGetNextParameter(cg_param);
		}

		if (ST_VertexShader == type)
		{
			using namespace boost::assign;
			std::vector<std::pair<std::string, VertexElementUsage> > pre_define_semantics; 
			pre_define_semantics += std::make_pair(std::string("BLENDWEIGHT"), VEU_BlendWeight),
				std::make_pair(std::string("BLENDINDICES"), VEU_BlendIndex),
				std::make_pair(std::string("TANGENT"), VEU_Tangent),
				std::make_pair(std::string("BINORMAL"), VEU_Binormal);

			CGparameter cg_param = cgGetFirstParameter(shaders_[ST_VertexShader], CG_PROGRAM);
			while (cg_param)
			{
				if ((CG_VARYING == cgGetParameterVariability(cg_param))
					&& (CG_IN == cgGetParameterDirection(cg_param)))
				{
					std::string semantic = cgGetParameterSemantic(cg_param);

					for (size_t j = 0; j < pre_define_semantics.size(); ++ j)
					{
						if (0 == semantic.find(pre_define_semantics[j].first))
						{
							VertexElementUsage usage = pre_define_semantics[j].second;
							uint8_t usage_index = 0;

							semantic.erase(0, pre_define_semantics[j].first.size());
							if (!semantic.empty())
							{
								usage_index = static_cast<uint8_t>(boost::lexical_cast<int>(semantic));
							}

							vertex_varyings_.insert(std::make_pair(std::make_pair(usage, usage_index),
								static_cast<uint8_t>(cgGetParameterResourceIndex(cg_param))));

							break;
						}
					}
				}

				cg_param = cgGetNextParameter(cg_param);
			}
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

	bool OGLShaderObject::HasParameter(ShaderType type, std::string const & name) const
	{
		return param_descs_[type].find(name) != param_descs_[type].end();
	}

	int32_t OGLShaderObject::AttribIndex(VertexElementUsage usage, uint8_t usage_index)
	{
		std::pair<VertexElementUsage, uint8_t> p = std::make_pair(usage, usage_index);

		vertex_varyings_t::iterator iter = vertex_varyings_.find(p);
		if (iter != vertex_varyings_.end())
		{
			return iter->second;
		}
		else
		{
			return -1;
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, bool value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				cgSetParameter1i(iter->second, value);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, int value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				cgSetParameter1i(iter->second, value);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, float value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				cgSetParameter1f(iter->second, value);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, float4 const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				cgSetParameter4fv(iter->second, &value[0]);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, float4x4 const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				cgGLSetMatrixParameterfr(iter->second, &value[0]);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, std::vector<bool> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				std::vector<int> tmp(value.begin(), value.end());
				cgSetParameterValueir(iter->second, static_cast<int>(tmp.size()), &tmp[0]);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, std::vector<int> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				cgSetParameterValueir(iter->second, static_cast<int>(value.size()), &value[0]);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, std::vector<float> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				cgGLSetParameterArray1f(iter->second, 0, static_cast<int>(value.size()), &value[0]);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, std::vector<float4> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				cgGLSetParameterArray4f(iter->second, 0, static_cast<long>(value.size()), &value[0][0]);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, std::vector<float4x4> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				cgGLSetMatrixParameterArrayfr(iter->second, 0, static_cast<long>(value.size()), &value[0][0]);
			}
		}
	}

	void OGLShaderObject::SetParameter(std::string const & name, SamplerPtr const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				uint32_t index = cgGLGetTextureEnum(iter->second) - GL_TEXTURE0;

				BOOST_ASSERT(index < samplers_[type].size());
				samplers_[type][index] = value;
			}
		}
	}
}
