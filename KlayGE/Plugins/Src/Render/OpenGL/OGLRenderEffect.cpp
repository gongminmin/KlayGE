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
#include <KlayGE/OpenGL/OGLTexture.hpp>

#include <cassert>

#include <KlayGE/OpenGL/OGLRenderEffect.hpp>

namespace KlayGE
{
	OGLRenderEffect::OGLRenderEffect(const std::string& srcData, UINT flags)
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

	RenderEffectParameterPtr OGLRenderEffect::Parameter(UINT index)
	{
		return RenderEffectParameterPtr();
	}

	RenderEffectParameterPtr OGLRenderEffect::ParameterByName(const std::string& name)
	{
		return RenderEffectParameterPtr();
	}

	RenderEffectParameterPtr OGLRenderEffect::ParameterBySemantic(const std::string& semantic)
	{
		return RenderEffectParameterPtr();
	}

	void OGLRenderEffect::SetTechnique(const std::string& technique)
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


	void OGLRenderEffectParameter::SetFloat(float value)
	{
	}

	float OGLRenderEffectParameter::GetFloat() const
	{
		return 0;
	}

	void OGLRenderEffectParameter::SetVector(const Vector4& value)
	{
	}

	Vector4 OGLRenderEffectParameter::GetVector() const
	{
		return Vector4::Zero();
	}

	void OGLRenderEffectParameter::SetMatrix(const Matrix4& value)
	{
	}

	Matrix4 OGLRenderEffectParameter::GetMatrix() const
	{
		return Matrix4::Identity();
	}

	void OGLRenderEffectParameter::SetMatrixArray(const Matrix4* matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::GetMatrixArray(Matrix4* matrices, size_t count)
	{
	}	

	void OGLRenderEffectParameter::SetInt(int value)
	{
	}

	int OGLRenderEffectParameter::GetInt() const
	{
		return 0;
	}

	void OGLRenderEffectParameter::SetBool(bool value)
	{
	}

	bool OGLRenderEffectParameter::GetBool() const
	{
		return false;
	}

	void OGLRenderEffectParameter::SetString(const std::string& value)
	{
	}

	std::string OGLRenderEffectParameter::GetString() const
	{
		return std::string();
	}

	void OGLRenderEffectParameter::SetTexture(const TexturePtr& tex)
	{
	}
}
