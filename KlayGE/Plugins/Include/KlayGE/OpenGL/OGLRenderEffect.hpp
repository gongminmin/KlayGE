// OGLRenderEffect.hpp
// KlayGE OpenGL渲染效果类 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
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

#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")

namespace KlayGE
{
	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class OGLRenderEffect : public RenderEffect
	{
	public:
		OGLRenderEffect(const String& srcData, UINT flags = 0);
		OGLRenderEffect(const OGLRenderEffect& rhs);

		RenderEffectPtr Clone() const;

		void Desc(UINT& parameters, UINT& techniques, UINT& functions);

		void SetValue(const String& name, const void* data, UINT bytes);
		void* GetValue(const String& name, UINT bytes) const;

		void SetFloat(const String& name, float value);
		float GetFloat(const String& name) const;
		void SetVector(const String& name, const Vector4& value);
		Vector4 GetVector(const String& name) const;
		void SetMatrix(const String& name, const Matrix4& value);
		Matrix4 GetMatrix(const String& name) const;
		void SetMatrixArray(const String& name, const std::vector<Matrix4, alloc<Matrix4> >& matrices);
		void GetMatrixArray(const String& name, std::vector<Matrix4, alloc<Matrix4> >& matrices);
		void SetInt(const String& name, int value);
		int GetInt(const String& name) const;
		void SetBool(const String& name, bool value);
		bool GetBool(const String& name) const;
		void SetString(const String& name, const String& value);
		String GetString(const String& name) const;

		void SetTexture(const String& name, const TexturePtr& tex);
		void SetVertexShader(const String& name, U32 vsHandle);
		void SetPixelShader(const String& name, U32 psHandle);

		void SetTechnique(const String& technique);
		void SetTechnique(UINT technique);

		UINT Begin(UINT flags = 0);
		void Pass(UINT passNum);
		void End();
	};
}

#endif		// _OGLRENDEREFFECT_HPP
