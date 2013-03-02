// VertexBuffer.cpp
// KlayGE 顶点缓冲区类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://www.klayge.org
//
// 3.2.0
// 支持32位的IndexStream (2006.1.4)
//
// 3.1.0
// 分离了实例和几何流 (2005.10.31)
//
// 3.0.0
// 支持instancing (2005.10.22)
//
// 2.4.0
// 改名为VertexBuffer (2005.3.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderView.hpp>

#include <KlayGE/GraphicsBuffer.hpp>

namespace KlayGE
{
	class NullGraphicsBuffer : public GraphicsBuffer
	{
	public:
		NullGraphicsBuffer()
			: GraphicsBuffer(BU_Static, 0)
		{
		}

		void* Map(BufferAccess /*ba*/)
		{
			return nullptr;
		}

		void Unmap()
		{
		}

		void CopyToBuffer(GraphicsBuffer& /*rhs*/)
		{
		}

		void DoResize()
		{
		}

		RenderViewPtr CreateRenderView(uint32_t /*width*/, uint32_t /*height*/)
		{
			return RenderView::NullObject();
		}
	};

	GraphicsBuffer::GraphicsBuffer(BufferUsage usage, uint32_t access_hint)
			: usage_(usage), access_hint_(access_hint), size_in_byte_(0)
	{
	}

	GraphicsBuffer::~GraphicsBuffer()
	{
	}

	GraphicsBufferPtr GraphicsBuffer::NullObject()
	{
		static GraphicsBufferPtr obj = MakeSharedPtr<NullGraphicsBuffer>();
		return obj;
	}

	void GraphicsBuffer::Resize(uint32_t size_in_byte)
	{
		size_in_byte_ = size_in_byte;
		this->DoResize();
	}
}
