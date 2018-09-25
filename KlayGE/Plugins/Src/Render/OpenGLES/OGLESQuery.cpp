// OGLESQuery.hpp
// KlayGE OpenGL ES查询类 实现文件
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

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESQuery.hpp>

namespace KlayGE
{
	OGLESConditionalRender::OGLESConditionalRender()
	{
		glGenQueries(1, &query_);
	}

	OGLESConditionalRender::~OGLESConditionalRender()
	{
		glDeleteQueries(1, &query_);
	}

	void OGLESConditionalRender::Begin()
	{
		glBeginQuery(GL_ANY_SAMPLES_PASSED, query_);
	}

	void OGLESConditionalRender::End()
	{
		glEndQuery(GL_ANY_SAMPLES_PASSED);
	}

	void OGLESConditionalRender::BeginConditionalRender()
	{
	}

	void OGLESConditionalRender::EndConditionalRender()
	{
	}

	bool OGLESConditionalRender::AnySamplesPassed()
	{
		GLuint ret = 0;
		GLuint available = 0;
		while (!available)
		{
			glGetQueryObjectuiv(query_, GL_QUERY_RESULT_AVAILABLE, &available);
		}

		glGetQueryObjectuiv(query_, GL_QUERY_RESULT, &ret);
		return (ret != 0);
	}


	OGLESTimerQuery::OGLESTimerQuery()
	{
		BOOST_ASSERT(glloader_GLES_EXT_disjoint_timer_query());

		glGenQueries(1, &query_);
	}

	OGLESTimerQuery::~OGLESTimerQuery()
	{
		glDeleteQueries(1, &query_);
	}

	void OGLESTimerQuery::Begin()
	{
		glBeginQuery(GL_TIME_ELAPSED_EXT, query_);
	}

	void OGLESTimerQuery::End()
	{
		glEndQuery(GL_TIME_ELAPSED_EXT);
	}

	double OGLESTimerQuery::TimeElapsed()
	{
		GLuint available = 0;
		while (!available)
		{
			glGetQueryObjectuiv(query_, GL_QUERY_RESULT_AVAILABLE, &available);
		}

		GLint disjoint_occurred = 0;
		glGetIntegerv(GL_GPU_DISJOINT_EXT, &disjoint_occurred);
		if (disjoint_occurred)
		{
			return -1;
		}
		else
		{
			GLuint64 ret;
			glGetQueryObjectui64vEXT(query_, GL_QUERY_RESULT_EXT, &ret);
			return static_cast<uint64_t>(ret) * 1e-9;
		}
	}


	OGLESSOStatisticsQuery::OGLESSOStatisticsQuery()
	{
		glGenQueries(1, &primitive_written_query_);
		if (glloader_GLES_VERSION_3_2() || glloader_GLES_EXT_geometry_shader())
		{
			glGenQueries(1, &primitive_generated_query_);
		}
		else
		{
			primitive_generated_query_ = 0;
		}
	}

	OGLESSOStatisticsQuery::~OGLESSOStatisticsQuery()
	{
		glDeleteQueries(1, &primitive_written_query_);
		if (glloader_GLES_VERSION_3_2() || glloader_GLES_EXT_geometry_shader())
		{
			glDeleteQueries(1, &primitive_generated_query_);
		}
	}

	void OGLESSOStatisticsQuery::Begin()
	{
		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitive_written_query_);
		if (primitive_generated_query_ != 0)
		{
			glBeginQuery(GL_PRIMITIVES_GENERATED, primitive_generated_query_);
		}
	}

	void OGLESSOStatisticsQuery::End()
	{
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
		if (primitive_generated_query_ != 0)
		{
			glEndQuery(GL_PRIMITIVES_GENERATED);
		}
	}

	uint64_t OGLESSOStatisticsQuery::NumPrimitivesWritten()
	{
		GLuint available = 0;
		while (!available)
		{
			glGetQueryObjectuiv(primitive_written_query_, GL_QUERY_RESULT_AVAILABLE, &available);
		}

		GLuint ret;
		glGetQueryObjectuiv(primitive_written_query_, GL_QUERY_RESULT, &ret);
		return ret;
	}

	uint64_t OGLESSOStatisticsQuery::PrimitivesGenerated()
	{
		GLuint ret;
		if (primitive_generated_query_ != 0)
		{
			GLuint available = 0;
			while (!available)
			{
				glGetQueryObjectuiv(primitive_generated_query_, GL_QUERY_RESULT_AVAILABLE, &available);
			}

			glGetQueryObjectuiv(primitive_generated_query_, GL_QUERY_RESULT, &ret);
		}
		else
		{
			ret = 0;
		}
		return ret;
	}
}