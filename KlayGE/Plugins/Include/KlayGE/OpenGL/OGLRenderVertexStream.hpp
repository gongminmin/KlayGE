// OGLRenderVertexStream.hpp
// KlayGE OpenGL渲染到顶点流类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERVERTEXSTREAM_HPP
#define _OGLRENDERVERTEXSTREAM_HPP

#include <KlayGE/RenderVertexStream.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLRenderVertexStream : public RenderVertexStream
	{
	public:
		OGLRenderVertexStream();

		void Attach(VertexStreamPtr vs);
		void Detach();

		virtual void CustomAttribute(std::string const & name, void* pData);

		bool RequiresTextureFlipping() const
			{ return false; }
	};

	typedef boost::shared_ptr<OGLRenderVertexStream> OGLRenderVertexStreamPtr;
}

#endif			// _OGLRENDERVERTEXSTREAM_HPP
