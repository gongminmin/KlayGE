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

		RenderEffectParameterPtr Parameter(UINT index);
		RenderEffectParameterPtr ParameterByName(const std::string& name);
		RenderEffectParameterPtr ParameterBySemantic(const std::string& semantic);

		void SetTechnique(const std::string& technique);
		void SetTechnique(UINT technique);

		UINT Begin(UINT flags = 0);
		void BeginPass(UINT passNum);
		void EndPass();
		void End();
	};

	class OGLRenderEffectParameter : public RenderEffectParameter
	{
	public:
		RenderEffectParameter& operator=(float value);
		RenderEffectParameter& operator=(const Vector4& value);
		RenderEffectParameter& operator=(const Matrix4& value);
		RenderEffectParameter& operator=(int value);
		RenderEffectParameter& operator=(const TexturePtr& tex);

		operator float() const;
		operator Vector4() const;
		operator Matrix4() const;
		operator int() const;

		void SetFloatArray(const float* value, size_t count);
		void GetFloatArray(float* value, size_t count);
		void SetVectorArray(const Vector4* value, size_t count);
		void GetVectorArray(Vector4* value, size_t count);
		void SetMatrixArray(const Matrix4* matrices, size_t count);
		void GetMatrixArray(Matrix4* matrices, size_t count);
		void SetIntArray(const int* value, size_t count);
		void GetIntArray(int* value, size_t count);
	};
}

#endif		// _OGLRENDEREFFECT_HPP
