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
			std::cerr << cgGetLastListing(renderFactory.CGContext()) << std::endl;
		}
		BOOST_ASSERT(effect_);

		CGprogram program = cgGetFirstProgram(renderFactory.CGContext());
		while (program)
		{
			CGparameter param = cgGetFirstParameter(program, CG_PROGRAM);
			while (param)
			{
				RenderEffectParameterPtr reparam;

				std::string const name = cgGetParameterName(param);
				std::string semantic;
				if (cgGetParameterSemantic(param))
				{
					semantic = cgGetParameterSemantic(param);
				}

				CGtype param_type = cgGetParameterType(param);
				CGtype param_base_type = cgGetParameterBaseType(param);
				CGparameterclass param_class = cgGetParameterClass(param);

				switch (param_class)
				{
				case CG_PARAMETERCLASS_SCALAR:
					switch (param_type)
					{
					case CG_BOOL:
						reparam.reset(new OGLRenderEffectParameterBool(*this, name, semantic, param));
						break;

					case CG_INT:
						reparam.reset(new OGLRenderEffectParameterInt(*this, name, semantic, param));
						break;

					case CG_FLOAT:
						reparam.reset(new OGLRenderEffectParameterFloat(*this, name, semantic, param));
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
					break;

				case CG_PARAMETERCLASS_VECTOR:
					switch (param_type)
					{
					case CG_FLOAT2:
						reparam.reset(new OGLRenderEffectParameterFloat2(*this, name, semantic, param));
						break;

					case CG_FLOAT3:
						reparam.reset(new OGLRenderEffectParameterFloat3(*this, name, semantic, param));
						break;

					case CG_FLOAT4:
						reparam.reset(new OGLRenderEffectParameterFloat4(*this, name, semantic, param));
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
					break;

				case CG_PARAMETERCLASS_MATRIX:
					switch (param_type)
					{
					case CG_FLOAT4x4:
						reparam.reset(new OGLRenderEffectParameterFloat4x4(*this, name, semantic, param));
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
					break;

				case CG_PARAMETERCLASS_SAMPLER:
					reparam.reset(new OGLRenderEffectParameterSampler(*this, name, semantic, param));
					break;

				case CG_PARAMETERCLASS_ARRAY:
					switch (param_type)
					{
					case CG_ARRAY:
						switch (param_base_type)
						{
						case CG_BOOL:
							reparam.reset(new OGLRenderEffectParameterBoolArray(*this, name, semantic, param));
							break;

						case CG_INT:
							reparam.reset(new OGLRenderEffectParameterIntArray(*this, name, semantic, param));
							break;

						case CG_FLOAT:
							reparam.reset(new OGLRenderEffectParameterFloatArray(*this, name, semantic, param));
							break;

						case CG_FLOAT4:
							reparam.reset(new OGLRenderEffectParameterFloat4Array(*this, name, semantic, param));
							break;

						case CG_FLOAT4x4:
							reparam.reset(new OGLRenderEffectParameterFloat4x4Array(*this, name, semantic, param));
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}

				params_.push_back(reparam);
				param = cgGetNextParameter(param);
			}

			program = cgGetNextProgram(program);
		}

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

	void OGLRenderEffectParameterFloat2::DoFlush(float2 const & value)
	{
		cgSetParameter2fv(param_, &value[0]);
	}

	void OGLRenderEffectParameterFloat3::DoFlush(float3 const & value)
	{
		cgSetParameter3fv(param_, &value[0]);
	}

	void OGLRenderEffectParameterFloat4::DoFlush(float4 const & value)
	{
		cgSetParameter4fv(param_, &value[0]);
	}

	void OGLRenderEffectParameterFloat4x4::DoFlush(float4x4 const & value)
	{
		cgSetParameterValuefr(param_, 4 * 4, &value[0]);
	}

	void OGLRenderEffectParameterInt::DoFlush(int const & value)
	{
		cgSetParameter1i(param_, value);
	}

	void OGLRenderEffectParameterSampler::DoFlush(SamplerPtr const & value)
	{
		OGLTexture& ogl_tex = *checked_pointer_cast<OGLTexture>(value->GetTexture());
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().SetSampler(0, value);
		cgGLSetupSampler(param_, ogl_tex.GLTexture());
	}

	void OGLRenderEffectParameterFloatArray::DoFlush(std::vector<float> const & value)
	{
		cgGLSetParameterArray1f(param_, 0, static_cast<long>(value.size()), &value[0]);
	}

	void OGLRenderEffectParameterFloat4Array::DoFlush(std::vector<float4> const & value)
	{
		cgGLSetParameterArray4f(param_, 0, static_cast<long>(value.size()), &value[0][0]);
	}

	void OGLRenderEffectParameterFloat4x4Array::DoFlush(std::vector<float4x4> const & value)
	{
		cgGLSetMatrixParameterArrayfr(param_, 0, static_cast<long>(value.size()), &value[0][0]);
	}

	void OGLRenderEffectParameterIntArray::DoFlush(std::vector<int> const & value)
	{
		cgSetParameterValueir(param_, static_cast<long>(value.size()), &value[0]);
	}
}
