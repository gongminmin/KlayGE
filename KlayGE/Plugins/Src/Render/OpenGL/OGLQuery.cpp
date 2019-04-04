// OGLQuery.hpp
// KlayGE OpenGL查询类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2005-2008
// Homepage: http://www.klayge.org
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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
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
		GLuint available = 0;
		while (!available)
		{
			glGetQueryObjectuiv(query_, GL_QUERY_RESULT_AVAILABLE, &available);
		}

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
		glBeginQuery(GL_ANY_SAMPLES_PASSED, query_);
	}

	void OGLConditionalRender::End()
	{
		glEndQuery(GL_ANY_SAMPLES_PASSED);
	}

	void OGLConditionalRender::BeginConditionalRender()
	{
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (!re.HackForAMD())
		{
			glBeginConditionalRender(query_, GL_QUERY_WAIT);
		}
	}

	void OGLConditionalRender::EndConditionalRender()
	{
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (!re.HackForAMD())
		{
			glEndConditionalRender();
		}
	}

	bool OGLConditionalRender::AnySamplesPassed()
	{
		GLuint available = 0;
		while (!available)
		{
			glGetQueryObjectuiv(query_, GL_QUERY_RESULT_AVAILABLE, &available);
		}

		GLuint ret;
		glGetQueryObjectuiv(query_, GL_QUERY_RESULT, &ret);
		return (ret != 0);
	}


	OGLTimerQuery::OGLTimerQuery()
	{
		glGenQueries(1, &query_);
	}

	OGLTimerQuery::~OGLTimerQuery()
	{
		glDeleteQueries(1, &query_);
	}

	void OGLTimerQuery::Begin()
	{
		glBeginQuery(GL_TIME_ELAPSED, query_);
	}

	void OGLTimerQuery::End()
	{
		glEndQuery(GL_TIME_ELAPSED);
	}

	double OGLTimerQuery::TimeElapsed()
	{
		GLuint available = 0;
		while (!available)
		{
			glGetQueryObjectuiv(query_, GL_QUERY_RESULT_AVAILABLE, &available);
		}

		GLuint64 ret;
		glGetQueryObjectui64v(query_, GL_QUERY_RESULT, &ret);
		return static_cast<uint64_t>(ret) * 1e-9;
	}


	OGLSOStatisticsQuery::OGLSOStatisticsQuery()
	{
		glGenQueries(1, &primitive_written_query_);
		glGenQueries(1, &primitive_generated_query_);
	}

	OGLSOStatisticsQuery::~OGLSOStatisticsQuery()
	{
		glDeleteQueries(1, &primitive_written_query_);
		glDeleteQueries(1, &primitive_generated_query_);
	}

	void OGLSOStatisticsQuery::Begin()
	{
		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitive_written_query_);
		glBeginQuery(GL_PRIMITIVES_GENERATED, primitive_generated_query_);
	}

	void OGLSOStatisticsQuery::End()
	{
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
		glEndQuery(GL_PRIMITIVES_GENERATED);
	}

	uint64_t OGLSOStatisticsQuery::NumPrimitivesWritten()
	{
		GLuint available = 0;
		while (!available)
		{
			glGetQueryObjectuiv(primitive_written_query_, GL_QUERY_RESULT_AVAILABLE, &available);
		}

		GLuint64 ret;
		glGetQueryObjectui64v(primitive_written_query_, GL_QUERY_RESULT, &ret);
		return ret;
	}

	uint64_t OGLSOStatisticsQuery::PrimitivesGenerated()
	{
		GLuint available = 0;
		while (!available)
		{
			glGetQueryObjectuiv(primitive_generated_query_, GL_QUERY_RESULT_AVAILABLE, &available);
		}

		GLuint64 ret;
		glGetQueryObjectui64v(primitive_generated_query_, GL_QUERY_RESULT, &ret);
		return ret;
	}
}