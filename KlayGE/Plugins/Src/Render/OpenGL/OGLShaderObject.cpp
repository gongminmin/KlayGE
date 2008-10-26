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
#include <KlayGE/OpenGL/OGLRenderFactoryInternal.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

namespace
{
	using namespace KlayGE;

	template <typename SrcType>
	class SetOGLShaderParameter
	{
	};

	template <>
	class SetOGLShaderParameter<bool>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			bool v;
			param_->Value(v);

			cgSetParameter1i(cg_param_, v);
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			int v;
			param_->Value(v);

			cgSetParameter1i(cg_param_, v);
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			float v;
			param_->Value(v);

			cgSetParameter1f(cg_param_, v);
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float2>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			float2 v;
			param_->Value(v);

			cgSetParameter2fv(cg_param_, &v.x());
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float3>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			float3 v;
			param_->Value(v);

			cgSetParameter3fv(cg_param_, &v.x());
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float4>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			float4 v;
			param_->Value(v);

			cgSetParameter4fv(cg_param_, &v.x());
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float4x4>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			float4x4 v;
			param_->Value(v);

			cgGLSetMatrixParameterfr(cg_param_, &v[0]);
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<bool*>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			std::vector<bool> v;
			param_->Value(v);

			if (!v.empty())
			{
				std::vector<int> tmp(v.begin(), v.end());
				cgSetParameterValueir(cg_param_, static_cast<int>(tmp.size()), &tmp[0]);
			}
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int*>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int> v;
			param_->Value(v);

			if (!v.empty())
			{
				cgSetParameterValueir(cg_param_, static_cast<int>(v.size()), &v[0]);
			}
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float*>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float> v;
			param_->Value(v);

			if (!v.empty())
			{
				cgGLSetParameterArray1f(cg_param_, 0, static_cast<int>(v.size()), &v[0]);
			}
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float4*>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float4> v;
			param_->Value(v);

			if (!v.empty())
			{
				cgGLSetParameterArray4f(cg_param_, 0, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float4x4*>
	{
	public:
		SetOGLShaderParameter(CGparameter cg_param, RenderEffectParameterPtr const & param)
			: cg_param_(cg_param), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float4x4> v;
			param_->Value(v);

			if (!v.empty())
			{
				cgGLSetMatrixParameterArrayfr(cg_param_, 0, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		CGparameter cg_param_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >
	{
	public:
		SetOGLShaderParameter(std::pair<TexturePtr, SamplerStateObjectPtr>& sampler, CGparameter cg_param, GLuint stage, RenderEffectParameterPtr const & param)
			: sampler_(&sampler), cg_param_(cg_param), stage_(stage), param_(param)
		{
		}

		void operator()()
		{
			param_->Value(*sampler_);

			if (sampler_ && sampler_->first)
			{
				GLuint const gl_tex = checked_pointer_cast<OGLTexture>(sampler_->first)->GLTexture();
				checked_pointer_cast<OGLSamplerStateObject>(sampler_->second)->Active(stage_, sampler_->first);

				cgGLSetTextureParameter(cg_param_, gl_tex);
			}
		}

	private:
		std::pair<TexturePtr, SamplerStateObjectPtr>* sampler_;
		CGparameter cg_param_;
		GLuint stage_;
		RenderEffectParameterPtr param_;
	};
}

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
		*shader_text_ = "#define OGL_EXPLICIT_TEXUNIT\n\n" + *shader_text_;

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

		is_validate_ = true;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			is_validate_ &= is_shader_validate_[i];
		}
	}

	ShaderObjectPtr OGLShaderObject::Clone(RenderEffect& effect)
	{
		OGLShaderObjectPtr ret = MakeSharedPtr<OGLShaderObject>();

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

			ret->samplers_[i].resize(samplers_[i].size());

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
		}

		ret->is_validate_ = true;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->is_validate_ &= ret->is_shader_validate_[i];
		}

		return ret;
	}

	OGLShaderObject::parameter_bind_t OGLShaderObject::GetBindFunc(CGparameter cg_param, RenderEffectParameterPtr const & param, ShaderType type)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.cg_param = cg_param;

		switch (param->type())
		{
		case REDT_bool:
			if (param->ArraySize() != 0)
			{
				ret.func = SetOGLShaderParameter<bool*>(cg_param, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<bool>(cg_param, param);
			}
			break;

		case REDT_dword:
		case REDT_int:
			if (param->ArraySize() != 0)
			{
				ret.func = SetOGLShaderParameter<int*>(cg_param, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<int>(cg_param, param);
			}
			break;

		case REDT_float:
			if (param->ArraySize() != 0)
			{
				ret.func = SetOGLShaderParameter<float*>(cg_param, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<float>(cg_param, param);
			}
			break;

		case REDT_float2:
			ret.func = SetOGLShaderParameter<float2>(cg_param, param);
			break;

		case REDT_float3:
			ret.func = SetOGLShaderParameter<float3>(cg_param, param);
			break;

		case REDT_float4:
			if (param->ArraySize() != 0)
			{
				ret.func = SetOGLShaderParameter<float4*>(cg_param, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<float4>(cg_param, param);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize() != 0)
			{
				ret.func = SetOGLShaderParameter<float4x4*>(cg_param, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<float4x4>(cg_param, param);
			}
			break;

		case REDT_sampler1D:
		case REDT_sampler2D:
		case REDT_sampler3D:
		case REDT_samplerCUBE:
			{
				uint32_t index = cgGLGetTextureEnum(cg_param) - GL_TEXTURE0;
				BOOST_ASSERT(index < samplers_[type].size());

				ret.func = SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >(samplers_[type][index], cg_param, index, param);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void OGLShaderObject::Bind()
	{
		for (int i = 0; i < ST_NumShaderTypes; ++ i)
		{
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::reference pb, param_binds_[i])
			{
				pb.func();
			}

			cgGLBindProgram(shaders_[i]);
			cgGLEnableProfile(profiles_[i]);

			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::reference pb, param_binds_[i])
			{
				switch (pb.param->type())
				{
				case REDT_sampler1D:
				case REDT_sampler2D:
				case REDT_sampler3D:
				case REDT_samplerCUBE:
					cgGLEnableTextureParameter(pb.cg_param);
					break;
				}
			}
		}
	}

	void OGLShaderObject::Unbind()
	{
		for (int i = 0; i < ST_NumShaderTypes; ++ i)
		{
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::reference pb, param_binds_[i])
			{
				switch (pb.param->type())
				{
				case REDT_sampler1D:
				case REDT_sampler2D:
				case REDT_sampler3D:
				case REDT_samplerCUBE:
					cgGLDisableTextureParameter(pb.cg_param);
					break;
				}
			}

			cgGLUnbindProgram(profiles_[i]);
		}
	}
}
