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
	OGLRenderEffect::OGLRenderEffect(std::string const & srcData, uint32_t flags)
	{
	}

	OGLRenderEffect::OGLRenderEffect(OGLRenderEffect const & rhs)
	{
	}

	RenderEffectPtr OGLRenderEffect::Clone() const
	{
		return RenderEffectPtr(new OGLRenderEffect(*this));
	}

	void OGLRenderEffect::Desc(uint32_t& parameters, uint32_t& techniques, uint32_t& functions)
	{
	}

	RenderEffectParameterPtr OGLRenderEffect::Parameter(uint32_t index)
	{
		return RenderEffectParameterPtr();
	}

	RenderEffectParameterPtr OGLRenderEffect::ParameterByName(std::string const & name)
	{
		return RenderEffectParameterPtr();
	}

	RenderEffectParameterPtr OGLRenderEffect::ParameterBySemantic(std::string const & semantic)
	{
		return RenderEffectParameterPtr();
	}

	void OGLRenderEffect::SetTechnique(std::string const & technique)
	{
	}

	void OGLRenderEffect::SetTechnique(uint32_t technique)
	{
	}

	uint32_t OGLRenderEffect::Begin(uint32_t flags)
	{
		return 1;
	}

	void OGLRenderEffect::BeginPass(uint32_t passNum)
	{
	}

	void OGLRenderEffect::EndPass()
	{
	}

	void OGLRenderEffect::End()
	{
	}


	RenderEffectParameter const & OGLRenderEffectParameter::operator=(float value)
	{
		return *this;
	}
	
	RenderEffectParameter const & OGLRenderEffectParameter::operator=(Vector4 const & value)
	{
		return *this;
	}

	RenderEffectParameter const & OGLRenderEffectParameter::operator=(Matrix4 const & value)
	{
		return *this;
	}

	RenderEffectParameter const & OGLRenderEffectParameter::operator=(int value)
	{
		return *this;
	}

	RenderEffectParameter const & OGLRenderEffectParameter::operator=(TexturePtr const & tex)
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

	void OGLRenderEffectParameter::SetFloatArray(float const * matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::GetFloatArray(float* matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::SetVectorArray(Vector4 const * matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::GetVectorArray(Vector4* matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::SetMatrixArray(Matrix4 const * matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::GetMatrixArray(Matrix4* matrices, size_t count)
	{
	}

	void SetIntArray(int const * matrices, size_t count)
	{
	}

	void GetIntArray(int* matrices, size_t count)
	{
	}
}
