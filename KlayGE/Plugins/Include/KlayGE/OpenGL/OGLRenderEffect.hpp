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
		OGLRenderEffect(const std::string& srcData, UINT flags = 0);
		OGLRenderEffect(const OGLRenderEffect& rhs);

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

		void SetTechnique(const std::string& technique);
		void SetTechnique(UINT technique);

		UINT Begin(UINT flags = 0);
		void Pass(UINT passNum);
		void End();
	};
}

#endif		// _OGLRENDEREFFECT_HPP
