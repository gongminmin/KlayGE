// OGLRenderEffect.cpp
// KlayGE OpenGL渲染效果类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/D3D9/D3D9Texture.hpp>

#include <cassert>

#include <KlayGE/OpenGL/OGLRenderEffect.hpp>

namespace KlayGE
{
	OGLRenderEffect::OGLRenderEffect(const String& srcData, UINT flags)
	{
	}

	OGLRenderEffect::OGLRenderEffect(const OGLRenderEffect& rhs)
	{
	}

	RenderEffectPtr OGLRenderEffect::Clone() const
	{
		return RenderEffectPtr(new OGLRenderEffect(*this));
	}

	void OGLRenderEffect::Desc(UINT& parameters, UINT& techniques, UINT& functions)
	{
	}

	void OGLRenderEffect::SetValue(const String& name, const void* data, UINT bytes)
	{
	}

	void* OGLRenderEffect::GetValue(const String& name, UINT bytes) const
	{
		return NULL;
	}

	void OGLRenderEffect::SetFloat(const String& name, float value)
	{
	}

	float OGLRenderEffect::GetFloat(const String& name) const
	{
		return 0;
	}

	void OGLRenderEffect::SetVector(const String& name, const Vector4& value)
	{
	}

	Vector4 OGLRenderEffect::GetVector(const String& name) const
	{
		return Vector4::Zero();
	}

	void OGLRenderEffect::SetMatrix(const String& name, const Matrix4& value)
	{
	}

	Matrix4 OGLRenderEffect::GetMatrix(const String& name) const
	{
		return Matrix4::Identity();
	}

	void OGLRenderEffect::SetMatrixArray(const String& name, const std::vector<Matrix4, alloc<Matrix4> >& matrices)
	{
	}

	void OGLRenderEffect::GetMatrixArray(const String& name, std::vector<Matrix4, alloc<Matrix4> >& matrices)
	{
	}	

	void OGLRenderEffect::SetInt(const String& name, int value)
	{
	}

	int OGLRenderEffect::GetInt(const String& name) const
	{
		return 0;
	}

	void OGLRenderEffect::SetBool(const String& name, bool value)
	{
	}

	bool OGLRenderEffect::GetBool(const String& name) const
	{
		return false;
	}

	void OGLRenderEffect::SetString(const String& name, const String& value)
	{
	}

	String OGLRenderEffect::GetString(const String& name) const
	{
		return String();
	}

	void OGLRenderEffect::SetTexture(const String& name, const TexturePtr& tex)
	{
	}

	void OGLRenderEffect::SetVertexShader(const String& name, const VertexShaderPtr& vs)
	{
	}

	void OGLRenderEffect::SetPixelShader(const String& name, const PixelShaderPtr& ps)
	{
	}

	void OGLRenderEffect::SetTechnique(const String& technique)
	{
	}

	void OGLRenderEffect::SetTechnique(UINT technique)
	{
	}

	UINT OGLRenderEffect::Begin(UINT flags)
	{
		return 1;
	}

	void OGLRenderEffect::Pass(UINT passNum)
	{
	}

	void OGLRenderEffect::End()
	{
	}
}
