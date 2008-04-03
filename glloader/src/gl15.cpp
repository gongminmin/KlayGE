// glloader
// Copyright (C) 2004-2005 Minmin Gong
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#include <glloader/glloader.h>
#include <glloader/gl15.h>
#include <glloader/gl14.h>
#include "utils.hpp"

#ifdef GLLOADER_GL

using glloader::load_funcs;
using glloader::gl_features_extractor;

namespace
{
	bool _GL_VERSION_1_5 = false;

	char APIENTRY _glloader_GL_VERSION_1_5()
	{
		return _GL_VERSION_1_5;
	}

	void init_GL_VERSION_1_5()
	{
		glloader_GL_VERSION_1_5 = _glloader_GL_VERSION_1_5;

		if (glloader_is_supported("GL_VERSION_1_5"))
		{
			_GL_VERSION_1_5 = true;

			{
				glGenQueries = NULL;
				glDeleteQueries = NULL;
				glIsQuery = NULL;
				glBeginQuery = NULL;
				glEndQuery = NULL;
				glGetQueryiv = NULL;
				glGetQueryObjectiv = NULL;
				glGetQueryObjectuiv = NULL;
				glBindBuffer = NULL;
				glDeleteBuffers = NULL;
				glGenBuffers = NULL;
				glIsBuffer = NULL;
				glBufferData = NULL;
				glBufferSubData = NULL;
				glGetBufferSubData = NULL;
				glMapBuffer = NULL;
				glUnmapBuffer = NULL;
				glGetBufferParameteriv = NULL;
				glGetBufferPointerv = NULL;
			}

			entries_t entries(19);
			{
				entries[0] = reinterpret_cast<void**>(&glGenQueries);
				entries[1] = reinterpret_cast<void**>(&glDeleteQueries);
				entries[2] = reinterpret_cast<void**>(&glIsQuery);
				entries[3] = reinterpret_cast<void**>(&glBeginQuery);
				entries[4] = reinterpret_cast<void**>(&glEndQuery);
				entries[5] = reinterpret_cast<void**>(&glGetQueryiv);
				entries[6] = reinterpret_cast<void**>(&glGetQueryObjectiv);
				entries[7] = reinterpret_cast<void**>(&glGetQueryObjectuiv);
				entries[8] = reinterpret_cast<void**>(&glBindBuffer);
				entries[9] = reinterpret_cast<void**>(&glDeleteBuffers);
				entries[10] = reinterpret_cast<void**>(&glGenBuffers);
				entries[11] = reinterpret_cast<void**>(&glIsBuffer);
				entries[12] = reinterpret_cast<void**>(&glBufferData);
				entries[13] = reinterpret_cast<void**>(&glBufferSubData);
				entries[14] = reinterpret_cast<void**>(&glGetBufferSubData);
				entries[15] = reinterpret_cast<void**>(&glMapBuffer);
				entries[16] = reinterpret_cast<void**>(&glUnmapBuffer);
				entries[17] = reinterpret_cast<void**>(&glGetBufferParameteriv);
				entries[18] = reinterpret_cast<void**>(&glGetBufferPointerv);
			}

			funcs_names_t names(19);
			{
				names[0] = "glGenQueries";
				names[1] = "glDeleteQueries";
				names[2] = "glIsQuery";
				names[3] = "glBeginQuery";
				names[4] = "glEndQuery";
				names[5] = "glGetQueryiv";
				names[6] = "glGetQueryObjectiv";
				names[7] = "glGetQueryObjectuiv";
				names[8] = "glBindBuffer";
				names[9] = "glDeleteBuffers";
				names[10] = "glGenBuffers";
				names[11] = "glIsBuffer";
				names[12] = "glBufferData";
				names[13] = "glBufferSubData";
				names[14] = "glGetBufferSubData";
				names[15] = "glMapBuffer";
				names[16] = "glUnmapBuffer";
				names[17] = "glGetBufferParameteriv";
				names[18] = "glGetBufferPointerv";
			}

			load_funcs(entries, names);
		}
		else
		{
			if (glloader_GL_ARB_occlusion_query())
			{
				glGenQueries = glGenQueriesARB;
				glDeleteQueries = glDeleteQueriesARB;
				glIsQuery = glIsQueryARB;
				glBeginQuery = glBeginQueryARB;
				glEndQuery = glEndQueryARB;
				glGetQueryiv = glGetQueryivARB;
				glGetQueryObjectiv = glGetQueryObjectivARB;
				glGetQueryObjectuiv = glGetQueryObjectuivARB;
			}
			if (glloader_GL_ARB_vertex_buffer_object())
			{
				glBindBuffer = glBindBufferARB;
				glDeleteBuffers = glDeleteBuffersARB;
				glGenBuffers = glGenBuffersARB;
				glIsBuffer = glIsBufferARB;
				glBufferData = glBufferDataARB;
				glBufferSubData = glBufferSubDataARB;
				glGetBufferSubData = glGetBufferSubDataARB;
				glMapBuffer = glMapBufferARB;
				glUnmapBuffer = glUnmapBufferARB;
				glGetBufferParameteriv = glGetBufferParameterivARB;
				glGetBufferPointerv = glGetBufferPointervARB;
			}

			if (glloader_GL_ARB_vertex_buffer_object()
				&& glloader_GL_ARB_occlusion_query()
				&& glloader_GL_ARB_depth_texture()
				&& glloader_GL_EXT_shadow_funcs())
			{
				_GL_VERSION_1_5 = true;
				gl_features_extractor::instance().promote("GL_VERSION_1_5");
			}
		}
	}

	char APIENTRY self_init_glloader_GL_VERSION_1_5()
	{
		glloader_GL_VERSION_1_4();

		init_GL_VERSION_1_5();
		return glloader_GL_VERSION_1_5();
	}
}

glloader_GL_VERSION_1_5FUNC glloader_GL_VERSION_1_5 = self_init_glloader_GL_VERSION_1_5;

#ifdef GL_VERSION_1_5

namespace
{
	void APIENTRY self_init_glGenQueries(GLsizei n, GLuint* ids)
	{
		init_GL_VERSION_1_5();
		return glGenQueries(n, ids);
	}
	void APIENTRY self_init_glDeleteQueries(GLsizei n, const GLuint* ids)
	{
		init_GL_VERSION_1_5();
		return glDeleteQueries(n, ids);
	}
	GLboolean APIENTRY self_init_glIsQuery(GLuint id)
	{
		init_GL_VERSION_1_5();
		return glIsQuery(id);
	}
	void APIENTRY self_init_glBeginQuery(GLenum target, GLuint id)
	{
		init_GL_VERSION_1_5();
		return glBeginQuery(target, id);
	}
	void APIENTRY self_init_glEndQuery(GLenum target)
	{
		init_GL_VERSION_1_5();
		return glEndQuery(target);
	}
	void APIENTRY self_init_glGetQueryiv(GLenum target, GLenum pname, GLint* param)
	{
		init_GL_VERSION_1_5();
		return glGetQueryiv(target, pname, param);
	}
	void APIENTRY self_init_glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params)
	{
		init_GL_VERSION_1_5();
		return glGetQueryObjectiv(id, pname, params);
	}
	void APIENTRY self_init_glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
	{
		init_GL_VERSION_1_5();
		return glGetQueryObjectuiv(id, pname, params);
	}
	void APIENTRY self_init_glBindBuffer(GLenum target, GLuint buffer)
	{
		init_GL_VERSION_1_5();
		return glBindBuffer(target, buffer);
	}
	void APIENTRY self_init_glDeleteBuffers(GLsizei n, const GLuint* buffers)
	{
		init_GL_VERSION_1_5();
		return glDeleteBuffers(n, buffers);
	}
	void APIENTRY self_init_glGenBuffers(GLsizei n, GLuint* buffers)
	{
		init_GL_VERSION_1_5();
		return glGenBuffers(n, buffers);
	}
	GLboolean APIENTRY self_init_glIsBuffer(GLuint buffer)
	{
		init_GL_VERSION_1_5();
		return glIsBuffer(buffer);
	}
	void APIENTRY self_init_glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
	{
		init_GL_VERSION_1_5();
		return glBufferData(target, size, data, usage);
	}
	void APIENTRY self_init_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
	{
		init_GL_VERSION_1_5();
		return glBufferSubData(target, offset, size, data);
	}
	void APIENTRY self_init_glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data)
	{
		init_GL_VERSION_1_5();
		return glGetBufferSubData(target, offset, size, data);
	}
	GLvoid* APIENTRY self_init_glMapBuffer(GLenum target, GLenum access)
	{
		init_GL_VERSION_1_5();
		return glMapBuffer(target, access);
	}
	GLboolean APIENTRY self_init_glUnmapBuffer(GLenum target)
	{
		init_GL_VERSION_1_5();
		return glUnmapBuffer(target);
	}
	void APIENTRY self_init_glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
	{
		init_GL_VERSION_1_5();
		return glGetBufferParameteriv(target, pname, params);
	}
	void APIENTRY self_init_glGetBufferPointerv(GLenum target, GLenum pname, GLvoid** params)
	{
		init_GL_VERSION_1_5();
		return glGetBufferPointerv(target, pname, params);
	}
}

glGenQueriesFUNC glGenQueries = self_init_glGenQueries;
glDeleteQueriesFUNC glDeleteQueries = self_init_glDeleteQueries;
glIsQueryFUNC glIsQuery = self_init_glIsQuery;
glBeginQueryFUNC glBeginQuery = self_init_glBeginQuery;
glEndQueryFUNC glEndQuery = self_init_glEndQuery;
glGetQueryivFUNC glGetQueryiv = self_init_glGetQueryiv;
glGetQueryObjectivFUNC glGetQueryObjectiv = self_init_glGetQueryObjectiv;
glGetQueryObjectuivFUNC glGetQueryObjectuiv = self_init_glGetQueryObjectuiv;
glBindBufferFUNC glBindBuffer = self_init_glBindBuffer;
glDeleteBuffersFUNC glDeleteBuffers = self_init_glDeleteBuffers;
glGenBuffersFUNC glGenBuffers = self_init_glGenBuffers;
glIsBufferFUNC glIsBuffer = self_init_glIsBuffer;
glBufferDataFUNC glBufferData = self_init_glBufferData;
glBufferSubDataFUNC glBufferSubData = self_init_glBufferSubData;
glGetBufferSubDataFUNC glGetBufferSubData = self_init_glGetBufferSubData;
glMapBufferFUNC glMapBuffer = self_init_glMapBuffer;
glUnmapBufferFUNC glUnmapBuffer = self_init_glUnmapBuffer;
glGetBufferParameterivFUNC glGetBufferParameteriv = self_init_glGetBufferParameteriv;
glGetBufferPointervFUNC glGetBufferPointerv = self_init_glGetBufferPointerv;

#endif		// GL_VERSION_1_5

#endif			// GLLOADER_GL
