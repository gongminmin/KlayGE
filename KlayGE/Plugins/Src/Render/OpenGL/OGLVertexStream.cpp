// OGLVertexStream.cpp
// KlayGE OpenGL顶点数据流类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 初次建立 (2004.4.3)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/OpenGL/OGLVertexStream.hpp>

namespace KlayGE
{
	OGLVertexStream::OGLVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum)
			: VertexStream(type, elementSize, elementNum)
	{
	}

	void OGLVertexStream::Assign(const void* src, size_t numVertices, size_t stride)
	{
		const size_t vertexSize(this->ElementSize() * this->ElementNum());
		const size_t size(vertexSize * numVertices);

		buffer_.resize(size);

		if (stride != 0)
		{
			U8* destPtr(&buffer_[0]);
			const U8* srcPtr(static_cast<const U8*>(static_cast<const void*>(src)));
			for (size_t i = 0; i < numVertices; ++ i)
			{
				memcpy(destPtr, srcPtr, vertexSize);

				destPtr += vertexSize;
				srcPtr += vertexSize + stride;
			}
		}
		else
		{
			Engine::MemoryInstance().Cpy(&buffer_[0], src, size);
		}
	}
}
