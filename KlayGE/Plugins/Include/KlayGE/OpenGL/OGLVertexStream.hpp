// OGLVertexStream.h
// KlayGE OpenGL顶点数据流类 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 初次建立 (2004.4.3)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLVERTEXSTREAM_HPP
#define _OGLVERTEXSTREAM_HPP

#include <vector>

#include <KlayGE/RenderBuffer.hpp>

namespace KlayGE
{
	class OGLVertexStream : public VertexStream
	{
	public:
		OGLVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum);

		bool IsStatic() const
			{ return false; }
		size_t NumVertices() const
			{ return buffer_.size() / this->ElementSize() / this->ElementNum(); }

		void Assign(const void* src, size_t numVertices, size_t stride = 0);

		const std::vector<U8, alloc<U8> >& OGLBuffer() const
			{ return buffer_; }

	protected:
		std::vector<U8, alloc<U8> > buffer_;
	};
}

#endif			// _OGLVERTEXSTREAM_HPP
