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

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class OGLRenderEffect : public RenderEffect
	{
	public:
		explicit OGLRenderEffect(std::string const & srcData);
		~OGLRenderEffect();

		bool Validate(std::string const & technique);
		void SetTechnique(std::string const & technique);

		void BeginPass(uint32_t passNum);
		void EndPass();

	private:
		std::string DoNameBySemantic(std::string const & semantic);
		RenderEffectParameterPtr DoParameterByName(std::string const & name);

		uint32_t DoBegin(uint32_t flags);
		void DoEnd();

	private:
		CGeffect effect_;
		CGtechnique technique_;
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

	class OGLRenderEffectParameterVector4 : public OGLRenderEffectParameter<Vector4>
	{
	public:
		OGLRenderEffectParameterVector4(RenderEffect& effect, std::string const & name, CGparameter param)
			: OGLRenderEffectParameter<Vector4>(effect, name, param)
		{
		}

	private:
		void DoFlush(Vector4 const & value);

	private:
		OGLRenderEffectParameterVector4(OGLRenderEffectParameterVector4 const & rhs);
		OGLRenderEffectParameterVector4& operator=(OGLRenderEffectParameterVector4 const & rhs);
	};

	class OGLRenderEffectParameterMatrix4 : public OGLRenderEffectParameter<Matrix4>
	{
	public:
		OGLRenderEffectParameterMatrix4(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<Matrix4>(effect, name, param)
		{
		}

	private:
		void DoFlush(Matrix4 const & value);

	private:
		OGLRenderEffectParameterMatrix4(OGLRenderEffectParameterMatrix4 const & rhs);
		OGLRenderEffectParameterMatrix4& operator=(OGLRenderEffectParameterMatrix4 const & rhs);
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

	class OGLRenderEffectParameterVector4Array : public OGLRenderEffectParameter<std::vector<Vector4> >
	{
	public:
		OGLRenderEffectParameterVector4Array(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<std::vector<Vector4> >(effect, name, param)
		{
		}

	private:
		void DoFlush(std::vector<Vector4> const & value);

	private:
		OGLRenderEffectParameterVector4Array(OGLRenderEffectParameterVector4Array const & rhs);
		OGLRenderEffectParameterVector4Array& operator=(OGLRenderEffectParameterVector4Array const & rhs);
	};

	class OGLRenderEffectParameterMatrix4Array : public OGLRenderEffectParameter<std::vector<Matrix4> >
	{
	public:
		OGLRenderEffectParameterMatrix4Array(RenderEffect& effect, std::string const & name, CGparameter param)
				: OGLRenderEffectParameter<std::vector<Matrix4> >(effect, name, param)
		{
		}

	private:
		void DoFlush(std::vector<Matrix4> const & value);

	private:
		OGLRenderEffectParameterMatrix4Array(OGLRenderEffectParameterMatrix4Array const & rhs);
		OGLRenderEffectParameterMatrix4Array& operator=(OGLRenderEffectParameterMatrix4Array const & rhs);
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
}

#endif		// _OGLRENDEREFFECT_HPP
