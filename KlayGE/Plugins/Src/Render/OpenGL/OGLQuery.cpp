// OGLQuery.hpp
// KlayGE OpenGL查询类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2005-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 加入了ConditionalRender (2008.10.11)
//
// 3.0.0
// 初次建立 (2005.9.27)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLQuery.hpp>

namespace KlayGE
{
	OGLOcclusionQuery::OGLOcclusionQuery()
	{
		glGenQueries(1, &query_);
	}

	OGLOcclusionQuery::~OGLOcclusionQuery()
	{
		glDeleteQueries(1, &query_);
	}

	void OGLOcclusionQuery::Begin()
	{
		glBeginQuery(GL_SAMPLES_PASSED, query_);
	}

	void OGLOcclusionQuery::End()
	{
		glEndQuery(GL_SAMPLES_PASSED);
	}

	uint64_t OGLOcclusionQuery::SamplesPassed()
	{
		GLuint ret;
		glGetQueryObjectuiv(query_, GL_QUERY_RESULT, &ret);
		return static_cast<uint64_t>(ret);
	}


	OGLConditionalRender::OGLConditionalRender()
	{
		glGenQueries(1, &query_);
	}

	OGLConditionalRender::~OGLConditionalRender()
	{
		glDeleteQueries(1, &query_);
	}

	void OGLConditionalRender::Begin()
	{
		glBeginQuery(GL_SAMPLES_PASSED, query_);
	}

	void OGLConditionalRender::End()
	{
		glEndQuery(GL_SAMPLES_PASSED);
	}

	void OGLConditionalRender::BeginConditionalRender()
	{
		glBeginConditionalRenderNV(query_, GL_QUERY_WAIT_NV);
	}

	void OGLConditionalRender::EndConditionalRender()
	{
		glEndConditionalRenderNV();
	}
}
