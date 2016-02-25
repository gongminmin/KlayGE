// VertexBuffer.cpp
// KlayGE ���㻺������ ʵ���ļ�
// Ver 3.2.0
// ��Ȩ����(C) ������, 2003-2006
// Homepage: http://www.klayge.org
//
// 3.2.0
// ֧��32λ��IndexStream (2006.1.4)
//
// 3.1.0
// ������ʵ���ͼ����� (2005.10.31)
//
// 3.0.0
// ֧��instancing (2005.10.22)
//
// 2.4.0
// ����ΪVertexBuffer (2005.3.7)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderView.hpp>

#include <KlayGE/GraphicsBuffer.hpp>

namespace KlayGE
{
	GraphicsBuffer::GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte)
			: usage_(usage), access_hint_(access_hint), size_in_byte_(size_in_byte)
	{
	}

	GraphicsBuffer::~GraphicsBuffer()
	{
	}
}
