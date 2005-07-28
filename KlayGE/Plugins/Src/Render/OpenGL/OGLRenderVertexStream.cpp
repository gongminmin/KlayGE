// OGLRenderVertexStream.hpp
// KlayGE OpenGL渲染到顶点流类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLVertexStream.hpp>
#include <KlayGE/OpenGL/OGLRenderVertexStream.hpp>

namespace KlayGE
{
	OGLRenderVertexStream::OGLRenderVertexStream(uint32_t width, uint32_t height)
	{
		left_ = 0;
		top_ = 0;
		width_ = width;
		height_ = height;
	}

	void OGLRenderVertexStream::CustomAttribute(std::string const & /*name*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLRenderVertexStream::Attach(VertexStreamPtr vs)
	{
		BOOST_ASSERT(false);
	}

	void OGLRenderVertexStream::Detach()
	{
		BOOST_ASSERT(false);
	}
}
