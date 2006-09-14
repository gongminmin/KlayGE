// OGLRenderEffect.hpp
// KlayGE OpenGL渲染效果类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 使用Cg实现 (2005.7.30)
//
// 2.5.0
// 去掉了Clone (2005.4.16)
//
// 2.0.4
// 初次建立 (2004.4.4)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDEREFFECT_HPP
#define _OGLRENDEREFFECT_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_OpenGL
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

namespace KlayGE
{
	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class OGLRenderEffect : public RenderEffect
	{
	public:
		explicit OGLRenderEffect(std::string const & srcData);
		~OGLRenderEffect();

	private:
		std::string DoNameBySemantic(std::string const & semantic);
		RenderEffectParameterPtr DoParameterByName(std::string const & name);

		RenderTechniquePtr MakeRenderTechnique(CGtechnique tech);

	private:
		CGeffect effect_;
	};

	class OGLRenderTechnique : public RenderTechnique
	{
	public:
		OGLRenderTechnique(RenderEffect& effect, std::string const & name, CGtechnique tech);

		bool Validate();

	private:
		RenderPassPtr MakeRenderPass(uint32_t index, CGpass pass);

		uint32_t DoBegin(uint32_t flags);
		void DoEnd();

	private:
		CGtechnique technique_;
	};

	class OGLRenderPass : public RenderPass
	{
	public:
		OGLRenderPass(RenderEffect& effect, uint32_t index, CGpass pass);

		void Begin();
		void End();

	private:
		CGpass pass_;
	};

	template <typename T>
	class OGLRenderEffectParameter : public RenderEffectParameterConcrete<T>
	{
	public:
		OGLRenderEffectParameter(RenderEffect& effect, std::string const & name, CGparameter param)
				: RenderEffectParameterConcrete<T>(effect, name),
					param_(param)
		{
		}
		virtual ~OGLRenderEffectParameter()
		{
			cgDestroyParameter(param_);
		}

	protected:
		CGparameter param_;
	};

	class OGLRenderEffectParameterBool : public OGLRenderEffectParameter<bool>
	{
	public:
		OGLRenderEffectParameterBool(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<bool>(effect, name, param)
		{
		}

	private:
		void DoFlush(bool const & value);

	private:
		OGLRenderEffectParameterBool(OGLRenderEffectParameterBool const & rhs);
		OGLRenderEffectParameterBool& operator=(OGLRenderEffectParameterBool const & rhs);
	};

	class OGLRenderEffectParameterInt : public OGLRenderEffectParameter<int>
	{
	public:
		OGLRenderEffectParameterInt(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<int>(effect, name, param)
		{
		}

	private:
		void DoFlush(int const & value);

	private:
		OGLRenderEffectParameterInt(OGLRenderEffectParameterInt const & rhs);
		OGLRenderEffectParameterInt& operator=(OGLRenderEffectParameterInt const & rhs);
	};

	class OGLRenderEffectParameterFloat : public OGLRenderEffectParameter<float>
	{
	public:
		OGLRenderEffectParameterFloat(RenderEffect& effect, std::string const & name, CGparameter param)
			: OGLRenderEffectParameter<float>(effect, name, param)
		{
		}

	private:
		void DoFlush(float const & value);

	private:
		OGLRenderEffectParameterFloat(OGLRenderEffectParameterFloat const & rhs);
		OGLRenderEffectParameterFloat& operator=(OGLRenderEffectParameterFloat const & rhs);
	};

	class OGLRenderEffectParameterFloat2 : public OGLRenderEffectParameter<float2>
	{
	public:
		OGLRenderEffectParameterFloat2(RenderEffect& effect, std::string const & name, CGparameter param)
			: OGLRenderEffectParameter<float2>(effect, name, param)
		{
		}

	private:
		void DoFlush(float2 const & value);

	private:
		OGLRenderEffectParameterFloat2(OGLRenderEffectParameterFloat2 const & rhs);
		OGLRenderEffectParameterFloat2& operator=(OGLRenderEffectParameterFloat2 const & rhs);
	};

	class OGLRenderEffectParameterFloat3 : public OGLRenderEffectParameter<float3>
	{
	public:
		OGLRenderEffectParameterFloat3(RenderEffect& effect, std::string const & name, CGparameter param)
			: OGLRenderEffectParameter<float3>(effect, name, param)
		{
		}

	private:
		void DoFlush(float3 const & value);

	private:
		OGLRenderEffectParameterFloat3(OGLRenderEffectParameterFloat3 const & rhs);
		OGLRenderEffectParameterFloat3& operator=(OGLRenderEffectParameterFloat3 const & rhs);
	};

	class OGLRenderEffectParameterFloat4 : public OGLRenderEffectParameter<float4>
	{
	public:
		OGLRenderEffectParameterFloat4(RenderEffect& effect, std::string const & name, CGparameter param)
			: OGLRenderEffectParameter<float4>(effect, name, param)
		{
		}

	private:
		void DoFlush(float4 const & value);

	private:
		OGLRenderEffectParameterFloat4(OGLRenderEffectParameterFloat4 const & rhs);
		OGLRenderEffectParameterFloat4& operator=(OGLRenderEffectParameterFloat4 const & rhs);
	};

	class OGLRenderEffectParameterFloat4x4 : public OGLRenderEffectParameter<float4x4>
	{
	public:
		OGLRenderEffectParameterFloat4x4(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<float4x4>(effect, name, param)
		{
		}

	private:
		void DoFlush(float4x4 const & value);

	private:
		OGLRenderEffectParameterFloat4x4(OGLRenderEffectParameterFloat4x4 const & rhs);
		OGLRenderEffectParameterFloat4x4& operator=(OGLRenderEffectParameterFloat4x4 const & rhs);
	};

	class OGLRenderEffectParameterSampler : public OGLRenderEffectParameter<SamplerPtr>
	{
	public:
		OGLRenderEffectParameterSampler(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<SamplerPtr>(effect, name, param)
		{
		}

	private:
		void DoFlush(SamplerPtr const & value);

	private:
		OGLRenderEffectParameterSampler(OGLRenderEffectParameterSampler const & rhs);
		OGLRenderEffectParameterSampler& operator=(OGLRenderEffectParameterSampler const & rhs);
	};
	
	class OGLRenderEffectParameterBoolArray : public OGLRenderEffectParameter<std::vector<bool> >
	{
	public:
		OGLRenderEffectParameterBoolArray(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<std::vector<bool> >(effect, name, param)
		{
		}

	private:
		void DoFlush(std::vector<bool> const & value);

	private:
		OGLRenderEffectParameterBoolArray(OGLRenderEffectParameterBoolArray const & rhs);
		OGLRenderEffectParameterBoolArray& operator=(OGLRenderEffectParameterBoolArray const & rhs);
	};
	
	class OGLRenderEffectParameterIntArray : public OGLRenderEffectParameter<std::vector<int> >
	{
	public:
		OGLRenderEffectParameterIntArray(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<std::vector<int> >(effect, name, param)
		{
		}

	private:
		void DoFlush(std::vector<int> const & value);

	private:
		OGLRenderEffectParameterIntArray(OGLRenderEffectParameterIntArray const & rhs);
		OGLRenderEffectParameterIntArray& operator=(OGLRenderEffectParameterIntArray const & rhs);
	};

	class OGLRenderEffectParameterFloatArray : public OGLRenderEffectParameter<std::vector<float> >
	{
	public:
		OGLRenderEffectParameterFloatArray(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<std::vector<float> >(effect, name, param)
		{
		}

	private:
		void DoFlush(std::vector<float> const & value);

	private:
		OGLRenderEffectParameterFloatArray(OGLRenderEffectParameterFloatArray const & rhs);
		OGLRenderEffectParameterFloatArray& operator=(OGLRenderEffectParameterFloatArray const & rhs);
	};

	class OGLRenderEffectParameterFloat4Array : public OGLRenderEffectParameter<std::vector<float4> >
	{
	public:
		OGLRenderEffectParameterFloat4Array(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<std::vector<float4> >(effect, name, param)
		{
		}

	private:
		void DoFlush(std::vector<float4> const & value);

	private:
		OGLRenderEffectParameterFloat4Array(OGLRenderEffectParameterFloat4Array const & rhs);
		OGLRenderEffectParameterFloat4Array& operator=(OGLRenderEffectParameterFloat4Array const & rhs);
	};

	class OGLRenderEffectParameterFloat4x4Array : public OGLRenderEffectParameter<std::vector<float4x4> >
	{
	public:
		OGLRenderEffectParameterFloat4x4Array(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<std::vector<float4x4> >(effect, name, param)
		{
		}

	private:
		void DoFlush(std::vector<float4x4> const & value);

	private:
		OGLRenderEffectParameterFloat4x4Array(OGLRenderEffectParameterFloat4x4Array const & rhs);
		OGLRenderEffectParameterFloat4x4Array& operator=(OGLRenderEffectParameterFloat4x4Array const & rhs);
	};
}

#endif		// _OGLRENDEREFFECT_HPP
