// OGLIndexStream.cpp
// KlayGE OpenGL索引数据流类 实现文件
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
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <algorithm>

#include <KlayGE/OpenGL/OGLIndexStream.hpp>

namespace KlayGE
{
	OGLIndexStream::OGLIndexStream()
	{
	}

	void OGLIndexStream::Assign(void const * src, size_t numIndices)
	{
		buffer_.assign(static_cast<U16 const *>(src),
			static_cast<U16 const *>(src) + numIndices);
	}
}
