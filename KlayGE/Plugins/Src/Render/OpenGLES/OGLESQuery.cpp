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
#include <KFL/ThrowErr.hpp>
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
		BOOST_ASSERT(glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_occlusion_query_boolean());

		if (glloader_GLES_VERSION_3_0())
		{
			glGenQueries(1, &query_);
		}
		else
		{
			glGenQueriesEXT(1, &query_);
		}
	}

	OGLESConditionalRender::~OGLESConditionalRender()
	{
		if (glloader_GLES_VERSION_3_0())
		{
			glDeleteQueries(1, &query_);
		}
		else
		{
			glDeleteQueriesEXT(1, &query_);
		}
	}

	void OGLESConditionalRender::Begin()
	{
		if (glloader_GLES_VERSION_3_0())
		{
			glBeginQuery(GL_ANY_SAMPLES_PASSED, query_);
		}
		else
		{
			glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query_);
		}
	}

	void OGLESConditionalRender::End()
	{
		if (glloader_GLES_VERSION_3_0())
		{
			glEndQuery(GL_ANY_SAMPLES_PASSED);
		}
		else
		{
			glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
		}
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
		if (glloader_GLES_VERSION_3_0())
		{
			GLuint available = 0;
			while (!available)
			{
				glGetQueryObjectuiv(query_, GL_QUERY_RESULT_AVAILABLE, &available);
			}

			glGetQueryObjectuiv(query_, GL_QUERY_RESULT, &ret);
		}
		else
		{
			GLuint available = 0;
			while (!available)
			{
				glGetQueryObjectuivEXT(query_, GL_QUERY_RESULT_AVAILABLE_EXT, &available);
			}

			glGetQueryObjectuivEXT(query_, GL_QUERY_RESULT_EXT, &ret);
		}
		return (ret != 0);
	}


	OGLESTimerQuery::OGLESTimerQuery()
	{
		BOOST_ASSERT(glloader_GLES_EXT_disjoint_timer_query());

		glGenQueriesEXT(1, &query_);
	}

	OGLESTimerQuery::~OGLESTimerQuery()
	{
		glDeleteQueriesEXT(1, &query_);
	}

	void OGLESTimerQuery::Begin()
	{
		glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query_);
	}

	void OGLESTimerQuery::End()
	{
		glEndQueryEXT(GL_TIME_ELAPSED_EXT);
	}

	double OGLESTimerQuery::TimeElapsed()
	{
		GLuint available = 0;
		while (!available)
		{
			glGetQueryObjectuivEXT(query_, GL_QUERY_RESULT_AVAILABLE_EXT, &available);
		}

		OGLESRenderEngine const & re = *checked_cast<OGLESRenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (!re.GPUDisjointOccurred())
		{
			GLuint64 ret;
			glGetQueryObjectui64vEXT(query_, GL_QUERY_RESULT_EXT, &ret);
			return static_cast<uint64_t>(ret)* 1e-9;
		}
		else
		{
			return -1;
		}
	}
}