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
#pragma warning(disable : 4244)
#pragma warning(disable : 4245)
#include <boost/crc.hpp>

#include <KlayGE/OpenGL/OGLRenderEffect.hpp>

namespace KlayGE
{
	OGLRenderEffect::OGLRenderEffect(std::string const & srcData)
	{
		boost::crc_32_type crc32;
		crc32.process_bytes(&srcData[0], srcData.size());
		crc32_ = crc32.checksum();
	}

	uint32_t OGLRenderEffect::HashCode() const
	{
		return crc32_;
	}

	void OGLRenderEffect::Desc(uint32_t& parameters, uint32_t& techniques, uint32_t& functions)
	{
	}

	bool OGLRenderEffect::SetTechnique(std::string const & technique)
	{
		return false;
	}

	bool OGLRenderEffect::SetTechnique(uint32_t technique)
	{
		return false;
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
