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

		void SetValue(const std::string& name, const void* data, UINT bytes);
		void* GetValue(const std::string& name, UINT bytes) const;

		void SetFloat(const std::string& name, float value);
		float GetFloat(const std::string& name) const;
		void SetVector(const std::string& name, const Vector4& value);
		Vector4 GetVector(const std::string& name) const;
		void SetMatrix(const std::string& name, const Matrix4& value);
		Matrix4 GetMatrix(const std::string& name) const;
		void SetMatrixArray(const std::string& name, const Matrix4* matrices, size_t count);
		void GetMatrixArray(const std::string& name, Matrix4* matrices, size_t count);
		void SetInt(const std::string& name, int value);
		int GetInt(const std::string& name) const;
		void SetBool(const std::string& name, bool value);
		bool GetBool(const std::string& name) const;
		void SetString(const std::string& name, const std::string& value);
		std::string GetString(const std::string& name) const;

		void SetTexture(const std::string& name, const TexturePtr& tex);

		void SetVertexShader(const std::string& name, const VertexShaderPtr& vs);
		void SetPixelShader(const std::string& name, const PixelShaderPtr& ps);

		void SetTechnique(const std::string& technique);
		void SetTechnique(UINT technique);

		bool Validate(D3DXHANDLE handle);

		UINT Begin(UINT flags = 0);
		void Pass(UINT passNum);
		void End();

	private:
		COMPtr<ID3DXEffect> effect_;
	};
}

#endif		// _D3D9RENDEREFFECT_HPP
