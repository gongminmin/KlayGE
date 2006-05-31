// FrameBuffer.cpp
// KlayGE 渲染到纹理类 实现文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.1.0
// 初次建立 (2005.10.29)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/FrameBuffer.hpp>

namespace KlayGE
{
	class NullFrameBuffer : public FrameBuffer
	{
	public:
		void Attach(uint32_t /*att*/, RenderViewPtr /*view*/)
		{
		}
		void Detach(uint32_t /*att*/)
		{
		}
	};

	FrameBufferPtr FrameBuffer::NullObject()
	{
		static FrameBufferPtr obj(new NullFrameBuffer);
		return obj;
	}
}
