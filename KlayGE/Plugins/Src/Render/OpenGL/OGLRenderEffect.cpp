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
#include <KlayGE/Sampler.hpp>
#include <KlayGE/Util.hpp>

#include <iostream>
using namespace std;
#include <boost/assert.hpp>

#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLRenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderEffect.hpp>

namespace KlayGE
{
	OGLRenderEffect::OGLRenderEffect(std::string const & srcData)
	{
		OGLRenderFactory& renderFactory(*checked_cast<OGLRenderFactory*>(&Context::Instance().RenderFactoryInstance()));

		effect_ = cgCreateEffect(renderFactory.CGContext(), srcData.c_str(), NULL);
		if (0 == effect_)
		{
			cerr << cgGetLastListing(renderFactory.CGContext()) << endl;
		}
		BOOST_ASSERT(effect_);

		CGtechnique tech = cgGetFirstTechnique(effect_);
		while (tech)
		{
			techniques_.push_back(this->MakeRenderTechnique(tech));
			tech = cgGetNextTechnique(tech);
		}
	}

	OGLRenderEffect::~OGLRenderEffect()
	{
		cgDestroyEffect(effect_);
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

		if ((CG_PARAMETERCLASS_SCALAR == param_class) && (CG_BOOL == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterBool(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_SCALAR == param_class) && (CG_INT == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterInt(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_SCALAR == param_class) && (CG_FLOAT == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterFloat(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_VECTOR == param_class) && (CG_FLOAT2 == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterVector2(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_VECTOR == param_class) && (CG_FLOAT3 == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterVector3(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_VECTOR == param_class) && (CG_FLOAT4 == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterVector4(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_MATRIX == param_class) && (CG_FLOAT4x4 == param_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterMatrix4(*this, name, param));
		}

		if (CG_PARAMETERCLASS_SAMPLER == param_class)
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterSampler(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_ARRAY == param_class) && (CG_ARRAY == param_type) && (CG_BOOL == param_base_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterBoolArray(*this, name, param));
		}

		if ((CG_PARAMETERCLASS_ARRAY == param_class) && (CG_ARRAY == param_type) && (CG_INT == param_base_type))
		{
			return RenderEffectParameterPtr(new OGLRenderEffectParameterIntArray(*this, name, param));
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

		BOOST_ASSERT(false);
		return RenderEffectParameterPtr();
	}

	RenderTechniquePtr OGLRenderEffect::MakeRenderTechnique(CGtechnique tech)
	{
		BOOST_ASSERT(tech != 0);
		RenderTechniquePtr ret(new OGLRenderTechnique(*this, cgGetTechniqueName(tech), tech));
		return ret;
	}


	OGLRenderTechnique::OGLRenderTechnique(RenderEffect& effect, std::string const & name, CGtechnique tech)
		: RenderTechnique(effect, name),
			technique_(tech)
	{
		CGpass pass = cgGetFirstPass(technique_);
		while (pass)
		{
			passes_.push_back(this->MakeRenderPass(static_cast<uint32_t>(passes_.size()), pass));
			pass = cgGetNextPass(pass);
		}
	}

	RenderPassPtr OGLRenderTechnique::MakeRenderPass(uint32_t index, CGpass pass)
	{
		RenderPassPtr ret(new OGLRenderPass(effect_, index, pass));
		return ret;
	}

	uint32_t OGLRenderTechnique::DoBegin(uint32_t /*flags*/)
	{
		return this->NumPasses();
	}

	void OGLRenderTechnique::DoEnd()
	{
	}

	bool OGLRenderTechnique::Validate()
	{
		return CG_TRUE == cgValidateTechnique(technique_);
	}


	OGLRenderPass::OGLRenderPass(RenderEffect& effect, uint32_t index, CGpass pass)
		: RenderPass(effect, index),
			pass_(pass)
	{
	}

	void OGLRenderPass::Begin()
	{
		cgSetPassState(pass_);
	}

	void OGLRenderPass::End()
	{
	}


	void OGLRenderEffectParameterFloat::DoFlush(float const & value)
	{
		cgSetParameter1f(param_, value);
	}

	void OGLRenderEffectParameterVector2::DoFlush(float2 const & value)
	{
		cgSetParameter2fv(param_, &value[0]);
	}

	void OGLRenderEffectParameterVector3::DoFlush(float3 const & value)
	{
		cgSetParameter3fv(param_, &value[0]);
	}

	void OGLRenderEffectParameterVector4::DoFlush(float4 const & value)
	{
		cgSetParameter4fv(param_, &value[0]);
	}

	void OGLRenderEffectParameterMatrix4::DoFlush(float4x4 const & value)
	{
		cgSetParameterValuefr(param_, 4 * 4, &value[0]);
	}

	void OGLRenderEffectParameterInt::DoFlush(int const & value)
	{
		cgSetParameter1i(param_, value);
	}

	void OGLRenderEffectParameterSampler::DoFlush(SamplerPtr const & value)
	{
		OGLTexture& ogl_tex = *checked_cast<OGLTexture*>(value->GetTexture().get());
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().SetSampler(0, value);
		cgGLSetupSampler(param_, ogl_tex.GLTexture());
	}

	void OGLRenderEffectParameterFloatArray::DoFlush(std::vector<float> const & value)
	{
		cgGLSetParameterArray1f(param_, 0, static_cast<long>(value.size()), &value[0]);
	}

	void OGLRenderEffectParameterVector4Array::DoFlush(std::vector<float4> const & value)
	{
		cgGLSetParameterArray4f(param_, 0, static_cast<long>(value.size()), &value[0][0]);
	}

	void OGLRenderEffectParameterMatrix4Array::DoFlush(std::vector<float4x4> const & value)
	{
		cgGLSetMatrixParameterArrayfr(param_, 0, static_cast<long>(value.size()), &value[0][0]);
	}

	void OGLRenderEffectParameterIntArray::DoFlush(std::vector<int> const & value)
	{
		cgSetParameterValueir(param_, static_cast<long>(value.size()), &value[0]);
	}
}
