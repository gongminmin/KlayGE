// RenderVertexStream.cpp
// KlayGE 渲染到顶点流类 实现文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.1.0
// 初次建立 (2005.10.29)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderVertexStream.hpp>

namespace KlayGE
{
	class NullRenderVertexStream : public RenderVertexStream
	{
	public:
		void Attach(VertexStreamPtr vs)
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

	RenderVertexStreamPtr RenderVertexStream::NullObject()
	{
		static RenderVertexStreamPtr obj(new NullRenderVertexStream);
		return obj;
	}
}
