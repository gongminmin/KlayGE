// D3D9RenderEffect.hpp
// KlayGE D3D9渲染效果类 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 增加了D3D9RenderTechnique (2004.3.16)
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

#ifndef _D3D9RENDEREFFECT_HPP
#define _D3D9RENDEREFFECT_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/COMPtr.hpp>

#include <d3dx9effect.h>

#include <KlayGE/RenderEffect.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class D3D9RenderEffect : public RenderEffect
	{
	public:
		D3D9RenderEffect(const std::string& srcData, UINT flags = 0);
		D3D9RenderEffect(const D3D9RenderEffect& rhs);

		const COMPtr<ID3DXEffect>& D3DXEffect() const
			{ return effect_; }

		RenderEffectPtr Clone() const;

		void Desc(UINT& parameters, UINT& techniques, UINT& functions);

		RenderEffectParameterPtr Parameter(UINT index);
		RenderEffectParameterPtr ParameterByName(const std::string& name);
		RenderEffectParameterPtr ParameterBySemantic(const std::string& semantic);

		void SetTechnique(const std::string& technique);
		void SetTechnique(UINT technique);

		bool Validate(D3DXHANDLE handle);

		UINT Begin(UINT flags = 0);
		void Pass(UINT passNum);
		void End();

	private:
		COMPtr<ID3DXEffect> effect_;
	};

	class D3D9RenderEffectParameter : public RenderEffectParameter
	{
	public:
		D3D9RenderEffectParameter(COMPtr<ID3DXEffect> effect, D3DXHANDLE parameter);

		void SetFloat(float value);
		float GetFloat() const;
		void SetVector(const Vector4& value);
		Vector4 GetVector() const;
		void SetMatrix(const Matrix4& value);
		Matrix4 GetMatrix() const;
		void SetMatrixArray(const Matrix4* matrices, size_t count);
		void GetMatrixArray(Matrix4* matrices, size_t count);
		void SetInt(int value);
		int GetInt() const;
		void SetBool(bool value);
		bool GetBool() const;
		void SetString(const std::string& value);
		std::string GetString() const;

		void SetTexture(const TexturePtr& tex);

	private:
		COMPtr<ID3DXEffect> effect_;
		D3DXHANDLE parameter_;
	};
}

#endif		// _D3D9RENDEREFFECT_HPP
