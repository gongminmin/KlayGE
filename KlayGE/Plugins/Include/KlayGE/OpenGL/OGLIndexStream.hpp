// OGLIndexStream.hpp
// KlayGE OpenGL索引数据流类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了CopyToMemory (2005.7.24)
// 只支持vbo (2005.7.31)
//
// 2.7.0
// 支持vertex_buffer_object (2005.6.19)
//
// 2.0.4
// 初次建立 (2004.4.3)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLINDEXSTREAM_HPP
#define _OGLINDEXSTREAM_HPP

#include <vector>
#include <glloader/glloader.h>

#include <KlayGE/VertexBuffer.hpp>

namespace KlayGE
{
	class OGLIndexStream : public IndexStream
	{
	public:
		OGLIndexStream(bool staticStream);
		~OGLIndexStream();

		bool IsStatic() const
		{
			return static_stream_;
		}
		uint32_t NumIndices() const
		{
			return numIndices_;
		}

		void Assign(void const * src, uint32_t numIndices);
		void CopyToMemory(void* data);

		void Active();

		GLuint OGLvbo() const
		{
			return ib_;
		}

	protected:
		uint32_t numIndices_;

		bool static_stream_;

		GLuint ib_;
	};
}

#endif			// _OGLINDEXSTREAM_HPP
