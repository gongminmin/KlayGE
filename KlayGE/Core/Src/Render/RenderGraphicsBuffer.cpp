// RenderGraphicsBuffer.hpp
// KlayGE 渲染图形缓冲区类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderGraphicsBuffer.hpp>

namespace KlayGE
{
	class NullRenderGraphicsBuffer : public RenderGraphicsBuffer
	{
	public:
		void Attach(GraphicsBufferPtr vs)
		{
		}
		void Detach()
		{
		}

		void CustomAttribute(std::string const & /*name*/, void* /*data*/)
		{
		}
		bool RequiresTextureFlipping() const
		{
			return false;
		}
	};

	RenderGraphicsBufferPtr RenderGraphicsBuffer::NullObject()
	{
		static RenderGraphicsBufferPtr obj(new NullRenderGraphicsBuffer);
		return obj;
	}
}
