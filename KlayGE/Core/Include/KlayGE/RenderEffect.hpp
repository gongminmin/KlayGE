// RenderEffect.hpp
// KlayGE 渲染效果类 头文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.2
// 增加了Parameter (2004.5.26)
//
// 2.0.3
// 修改了SetTexture的参数 (2004.3.6)
// 增加了SetMatrixArray/GetMatrixArray (2004.3.11)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDEREFFECT_HPP
#define _RENDEREFFECT_HPP

#include <KlayGE/PreDeclare.hpp>
#include <vector>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class RenderEffect
	{
	public:
		virtual ~RenderEffect()
			{ }

		static RenderEffectPtr NullObject();

		virtual RenderEffectPtr Clone() const = 0;

		virtual void Desc(uint32_t& parameters, uint32_t& techniques, uint32_t& functions) = 0;

		virtual RenderEffectParameterPtr Parameter(uint32_t index) = 0;
		virtual RenderEffectParameterPtr ParameterByName(std::string const & name) = 0;
		virtual RenderEffectParameterPtr ParameterBySemantic(std::string const & semantic) = 0;

		virtual void SetTechnique(std::string const & technique) = 0;
		virtual void SetTechnique(uint32_t technique) = 0;

		virtual uint32_t Begin(uint32_t flags = 0) = 0;
		virtual void BeginPass(uint32_t passNum) = 0;
		virtual void EndPass() = 0;
		virtual void End() = 0;
	};

	class RenderEffectParameter
	{
	public:
		virtual ~RenderEffectParameter()
			{ }

		static RenderEffectParameterPtr NullObject();

		virtual RenderEffectParameter& operator=(float value) = 0;
		virtual RenderEffectParameter& operator=(Vector4 const & value) = 0;
		virtual RenderEffectParameter& operator=(Matrix4 const & value) = 0;
		virtual RenderEffectParameter& operator=(int value) = 0;
		virtual RenderEffectParameter& operator=(TexturePtr const & tex) = 0;

		virtual operator float() const = 0;
		virtual operator Vector4() const = 0;
		virtual operator Matrix4() const = 0;
		virtual operator int() const = 0;

		virtual void SetFloatArray(float const * value, size_t count) = 0;
		virtual void GetFloatArray(float* value, size_t count) = 0;
		virtual void SetVectorArray(Vector4 const * value, size_t count) = 0;
		virtual void GetVectorArray(Vector4* value, size_t count) = 0;
		virtual void SetMatrixArray(Matrix4 const * matrices, size_t count) = 0;
		virtual void GetMatrixArray(Matrix4* matrices, size_t count) = 0;
		virtual void SetIntArray(int const * value, size_t count) = 0;
		virtual void GetIntArray(int* value, size_t count) = 0;
	};

	RenderEffectPtr LoadRenderEffect(std::string const & effectName);
}

#endif		// _RENDEREFFECT_HPP
