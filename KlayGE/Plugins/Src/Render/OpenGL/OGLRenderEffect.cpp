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


	RenderEffectParameter& OGLRenderEffectParameter::operator=(float value)
	{
		return *this;
	}
	
	RenderEffectParameter& OGLRenderEffectParameter::operator=(const Vector4& value)
	{
		return *this;
	}

	RenderEffectParameter& OGLRenderEffectParameter::operator=(const Matrix4& value)
	{
		return *this;
	}

	RenderEffectParameter& OGLRenderEffectParameter::operator=(int value)
	{
		return *this;
	}

	RenderEffectParameter& OGLRenderEffectParameter::operator=(const TexturePtr& tex)
	{
		return *this;
	}

	OGLRenderEffectParameter::operator float() const
	{
		return 0;
	}

	OGLRenderEffectParameter::operator Vector4() const
	{
		return Vector4::Zero();
	}

	OGLRenderEffectParameter::operator Matrix4() const
	{
		return Matrix4::Identity();
	}

	OGLRenderEffectParameter::operator int() const
	{
		return 0;
	}

	void OGLRenderEffectParameter::SetFloatArray(const float* matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::GetFloatArray(float* matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::SetVectorArray(const Vector4* matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::GetVectorArray(Vector4* matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::SetMatrixArray(const Matrix4* matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::GetMatrixArray(Matrix4* matrices, size_t count)
	{
	}

	void SetIntArray(const int* matrices, size_t count)
	{
	}

	void GetIntArray(int* matrices, size_t count)
	{
	}
}
