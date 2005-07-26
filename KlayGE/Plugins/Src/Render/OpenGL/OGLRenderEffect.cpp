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
	OGLRenderEffect::OGLRenderEffect(std::string const & srcData)
	{
	}

	void OGLRenderEffect::Desc(uint32_t& parameters, uint32_t& techniques, uint32_t& functions)
	{
	}

	bool OGLRenderEffect::Validate(std::string const & technique)
	{
		return false;
	}

	void OGLRenderEffect::SetTechnique(std::string const & technique)
	{
	}

	std::string OGLRenderEffect::DoNameBySemantic(std::string const & semantic)
	{
		return "";
	}

	RenderEffectParameterPtr OGLRenderEffect::DoParameterByName(std::string const & name)
	{
		return RenderEffectParameterPtr();
	}

	uint32_t OGLRenderEffect::Begin(uint32_t flags)
	{
		return 0;
	}

	void OGLRenderEffect::End()
	{
	}

	void OGLRenderEffect::BeginPass(uint32_t passNum)
	{
	}

	void OGLRenderEffect::EndPass()
	{
	}


	bool OGLRenderEffectParameter::DoTestType(RenderEffectParameterType type)
	{
		return true;
	}

	void OGLRenderEffectParameter::DoFloat(float value)
	{
	}
	
	void OGLRenderEffectParameter::DoVector4(Vector4 const & value)
	{
	}

	void OGLRenderEffectParameter::DoMatrix4(Matrix4 const & value)
	{
	}

	void OGLRenderEffectParameter::DoInt(int value)
	{
	}

	void OGLRenderEffectParameter::DoTexture(TexturePtr const & tex)
	{
	}

	void OGLRenderEffectParameter::DoSetFloatArray(float const * matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::DoSetVector4Array(Vector4 const * matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::DoSetMatrix4Array(Matrix4 const * matrices, size_t count)
	{
	}

	void OGLRenderEffectParameter::DoSetIntArray(int const * matrices, size_t count)
	{
	}
}
