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

	void OGLRenderEffect::SetValue(const std::string& name, const void* data, UINT bytes)
	{
	}

	void* OGLRenderEffect::GetValue(const std::string& name, UINT bytes) const
	{
		return NULL;
	}

	void OGLRenderEffect::SetFloat(const std::string& name, float value)
	{
	}

	float OGLRenderEffect::GetFloat(const std::string& name) const
	{
		return 0;
	}

	void OGLRenderEffect::SetVector(const std::string& name, const Vector4& value)
	{
	}

	Vector4 OGLRenderEffect::GetVector(const std::string& name) const
	{
		return Vector4::Zero();
	}

	void OGLRenderEffect::SetMatrix(const std::string& name, const Matrix4& value)
	{
	}

	Matrix4 OGLRenderEffect::GetMatrix(const std::string& name) const
	{
		return Matrix4::Identity();
	}

	void OGLRenderEffect::SetMatrixArray(const std::string& name, const Matrix4* matrices, size_t count)
	{
	}

	void OGLRenderEffect::GetMatrixArray(const std::string& name, Matrix4* matrices, size_t count)
	{
	}	

	void OGLRenderEffect::SetInt(const std::string& name, int value)
	{
	}

	int OGLRenderEffect::GetInt(const std::string& name) const
	{
		return 0;
	}

	void OGLRenderEffect::SetBool(const std::string& name, bool value)
	{
	}

	bool OGLRenderEffect::GetBool(const std::string& name) const
	{
		return false;
	}

	void OGLRenderEffect::SetString(const std::string& name, const std::string& value)
	{
	}

	std::string OGLRenderEffect::GetString(const std::string& name) const
	{
		return std::string();
	}

	void OGLRenderEffect::SetTexture(const std::string& name, const TexturePtr& tex)
	{
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
}
