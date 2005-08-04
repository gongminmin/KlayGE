// OGLRenderEffect.cpp
// KlayGE OpenGL渲染效果类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 使用Cg实现 (2005.7.30)
//
// 2.0.4
// 初次建立 (2004.4.4)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#include <iostream>
using namespace std;
#include <boost/assert.hpp>

#include <KlayGE/OpenGL/OGLRenderEffect.hpp>

namespace KlayGE
{
	OGLRenderEffect::OGLRenderEffect(std::string const & srcData)
	{
		BOOST_ASSERT(dynamic_cast<OGLRenderFactory*>(&Context::Instance().RenderFactoryInstance()) != NULL);

		OGLRenderFactory& renderFactory(static_cast<OGLRenderFactory&>(Context::Instance().RenderFactoryInstance()));

		effect_ = cgCreateEffect(renderFactory.CGContext(), srcData.c_str(), NULL);
		if (0 == effect_)
		{
			cerr << cgGetLastListing(renderFactory.CGContext()) << endl;
		}
		BOOST_ASSERT(effect_);
	}

	OGLRenderEffect::~OGLRenderEffect()
	{
		cgDestroyEffect(effect_);
	}

	bool OGLRenderEffect::Validate(std::string const & technique)
	{
		return CG_TRUE == cgValidateTechnique(cgGetNamedTechnique(effect_, technique.c_str()));
	}

	void OGLRenderEffect::SetTechnique(std::string const & technique)
	{
		technique_ = cgGetNamedTechnique(effect_, technique.c_str());
		BOOST_ASSERT(technique_);
	}

	std::string OGLRenderEffect::DoNameBySemantic(std::string const & semantic)
	{
		return cgGetParameterName(cgGetEffectParameterBySemantic(effect_, semantic.c_str()));
	}

	RenderEffectParameterPtr OGLRenderEffect::DoParameterByName(std::string const & name)
	{
		CGparameter param = cgGetNamedEffectParameter(effect_, name.c_str());

		CGtype param_type = cgGetParameterType(param);
		CGtype param_base_type = cgGetParameterBaseType(param);
		CGparameterclass param_class = cgGetParameterClass(param);

		if ((CG_PARAMETERCLASS_SCALAR == param_class) && (CG_FLOAT == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterFloat(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_VECTOR == param_class) && (CG_FLOAT4 == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterVector4(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_MATRIX == param_class) && (CG_FLOAT4x4 == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterMatrix4(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_SCALAR == param_class) && (CG_INT == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterInt(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_OBJECT == param_class) && (CG_TEXTURE == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterTexture(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_ARRAY == param_class) && (CG_ARRAY == param_type) && (CG_FLOAT == param_base_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterFloatArray(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_ARRAY == param_class) && (CG_ARRAY == param_type) && (CG_FLOAT4 == param_base_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterVector4Array(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_ARRAY == param_class) && (CG_ARRAY == param_type) && (CG_FLOAT4x4 == param_base_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterMatrix4Array(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_ARRAY == param_class) && (CG_ARRAY == param_type) && (CG_INT == param_base_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterIntArray(*this, name, param));
		}

		BOOST_ASSERT(false);
		return RenderEffectParameterPtr();
	}

	uint32_t OGLRenderEffect::DoBegin(uint32_t flags)
	{
		uint32_t ret = 0;

		CGpass pass = cgGetFirstPass(technique_);
		while (pass)
		{
			++ ret;
			pass = cgGetNextPass(pass);
		}

		return ret;
	}

	void OGLRenderEffect::DoEnd()
	{
	}

	void OGLRenderEffect::BeginPass(uint32_t passNum)
	{
		CGpass pass = cgGetFirstPass(technique_);
		while (pass && (passNum > 0))
		{
			-- passNum;
			pass = cgGetNextPass(pass);
		}

		cgSetPassState(pass);
	}

	void OGLRenderEffect::EndPass()
	{
	}


	void OGLRenderEffectParameterFloat::DoFlush(float const & value)
	{
		cgSetParameter1f(param_, value);
	}
	
	void OGLRenderEffectParameterVector4::DoFlush(Vector4 const & value)
	{
		cgSetParameter4fv(param_, &value[0]);
	}

	void OGLRenderEffectParameterMatrix4::DoFlush(Matrix4 const & value)
	{
		cgSetParameterValuefr(param_, 4 * 4, &value[0]);
	}

	void OGLRenderEffectParameterInt::DoFlush(int const & value)
	{
		cgSetParameter1i(param_, value);
	}

	void OGLRenderEffectParameterTexture::DoFlush(TexturePtr const & tex)
	{
		BOOST_ASSERT(dynamic_cast<OGLTexture*>(tex.get()) != NULL);

		OGLTexture& ogl_tex = static_cast<OGLTexture&>(*tex);
		cgGLSetTextureParameter(param_, ogl_tex.GLTexture());
	}

	void OGLRenderEffectParameterFloatArray::DoFlush(std::vector<float> const & value)
	{
		cgGLSetParameterArray1f(param_, 0, static_cast<long>(value.size()), &value[0]);
	}

	void OGLRenderEffectParameterVector4Array::DoFlush(std::vector<Vector4> const & value)
	{
		cgGLSetParameterArray4f(param_, 0, static_cast<long>(value.size()), &value[0][0]);
	}

	void OGLRenderEffectParameterMatrix4Array::DoFlush(std::vector<Matrix4> const & value)
	{
		cgGLSetMatrixParameterArrayfr(param_, 0, static_cast<long>(value.size()), &value[0][0]);
	}

	void OGLRenderEffectParameterIntArray::DoFlush(std::vector<int> const & value)
	{
		cgSetParameterValueir(param_, static_cast<long>(value.size()), &value[0]);
	}
}
