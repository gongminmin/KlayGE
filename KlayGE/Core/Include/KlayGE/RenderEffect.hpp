// RenderEffect.hpp
// KlayGE 渲染效果类 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/SharedPtr.hpp>
#include <vector>

#pragma comment(lib, "KlayGE_Core.lib")

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

		virtual void Desc(UINT& parameters, UINT& techniques, UINT& functions) = 0;

		virtual void SetValue(const std::string& name, const void* data, UINT bytes) = 0;
		virtual void* GetValue(const std::string& name, UINT bytes) const = 0;

		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual float GetFloat(const std::string& name) const = 0;
		virtual void SetVector(const std::string& name, const Vector4& value) = 0;
		virtual Vector4 GetVector(const std::string& name) const = 0;
		virtual void SetMatrix(const std::string& name, const Matrix4& value) = 0;
		virtual Matrix4 GetMatrix(const std::string& name) const = 0;
		virtual void SetMatrixArray(const std::string& name, const Matrix4* matrices, size_t count) = 0;
		virtual void GetMatrixArray(const std::string& name, Matrix4* matrices, size_t count) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;
		virtual int GetInt(const std::string& name) const = 0;
		virtual void SetBool(const std::string& name, bool value) = 0;
		virtual bool GetBool(const std::string& name) const = 0;
		virtual void SetString(const std::string& name, const std::string& value) = 0;
		virtual std::string GetString(const std::string& name) const = 0;

		virtual void SetTexture(const std::string& name, const TexturePtr& tex) = 0;

		virtual void SetVertexShader(const std::string& name, const VertexShaderPtr& vs) = 0;
		virtual void SetPixelShader(const std::string& name, const PixelShaderPtr& ps) = 0;

		virtual void SetTechnique(const std::string& technique) = 0;
		virtual void SetTechnique(UINT technique) = 0;

		virtual UINT Begin(UINT flags = 0) = 0;
		virtual void Pass(UINT passNum) = 0;
		virtual void End() = 0;
	};

	RenderEffectPtr LoadRenderEffect(const std::string& effectName, bool fromPack = false);
}

#endif		// _RENDEREFFECT_HPP
