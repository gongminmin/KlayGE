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
		if (glloader_GLES_VERSION_3_0())
		{
			glGenQueries(1, &query_);
		}
		else if (glloader_GLES_EXT_occlusion_query_boolean())
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
		else if (glloader_GLES_EXT_occlusion_query_boolean())
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
		else if (glloader_GLES_EXT_occlusion_query_boolean())
		{
			glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query_);
		}
	}

	void OGLESConditionalRender::End()
	{
		if (glloader_GLES_VERSION_3_0())
		{
			glEndQueryEXT(GL_ANY_SAMPLES_PASSED);
		}
		else if (glloader_GLES_EXT_occlusion_query_boolean())
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
			glGetQueryObjectuiv(query_, GL_QUERY_RESULT, &ret);
		}
		else if (glloader_GLES_EXT_occlusion_query_boolean())
		{
			glGetQueryObjectuivEXT(query_, GL_QUERY_RESULT_EXT, &ret);
		}
		return (ret != 0);
	}
}