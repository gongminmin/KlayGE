// OGLOcclusionQuery.hpp
// KlayGE OpenGL遮挡检测类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 初次建立 (2005.9.27)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLOCCLUSIONQUERY_HPP
#define _OGLOCCLUSIONQUERY_HPP

#include <KlayGE/OcclusionQuery.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLOcclusionQuery : public OcclusionQuery
	{
	public:
		OGLOcclusionQuery();
		~OGLOcclusionQuery();

		void Begin();
		void End();

		uint32_t SamplesPassed();

	private:
		GLuint query_;
	};
}

#endif		// _OGLOCCLUSIONQUERY_HPP
