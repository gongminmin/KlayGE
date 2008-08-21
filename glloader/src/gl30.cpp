// glloader
// Copyright (C) 2004-2008 Minmin Gong
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
#include <glloader/gl30.h>
#include <glloader/gl21.h>
#include "utils.hpp"

#ifdef GLLOADER_GL

using glloader::load_funcs;
using glloader::gl_features_extractor;

namespace
{
	bool _GL_VERSION_3_0 = false;

	char APIENTRY _glloader_GL_VERSION_3_0()
	{
		return _GL_VERSION_3_0;
	}

	void init_GL_VERSION_3_0()
	{
		glloader_GL_VERSION_3_0 = _glloader_GL_VERSION_3_0;

		{
			glMapBufferRange = NULL;
			glFlushMappedBufferRange = NULL;
		}

		if (glloader_is_supported("GL_VERSION_3_0"))
		{
			_GL_VERSION_3_0 = true;

			entries_t entries(2);
			{
				entries[0] = reinterpret_cast<void**>(&glMapBufferRange);
				entries[1] = reinterpret_cast<void**>(&glFlushMappedBufferRange);
				entries[2] = reinterpret_cast<void**>(&glVertexAttribI1i);
				entries[3] = reinterpret_cast<void**>(&glVertexAttribI2i);
				entries[4] = reinterpret_cast<void**>(&glVertexAttribI3i);
				entries[5] = reinterpret_cast<void**>(&glVertexAttribI4i);
				entries[6] = reinterpret_cast<void**>(&glVertexAttribI1ui);
				entries[7] = reinterpret_cast<void**>(&glVertexAttribI2ui);
				entries[8] = reinterpret_cast<void**>(&glVertexAttribI3ui);
				entries[9] = reinterpret_cast<void**>(&glVertexAttribI4ui);
				entries[10] = reinterpret_cast<void**>(&glVertexAttribI1iv);
				entries[11] = reinterpret_cast<void**>(&glVertexAttribI2iv);
				entries[12] = reinterpret_cast<void**>(&glVertexAttribI3iv);
				entries[13] = reinterpret_cast<void**>(&glVertexAttribI4iv);
				entries[14] = reinterpret_cast<void**>(&glVertexAttribI1uiv);
				entries[15] = reinterpret_cast<void**>(&glVertexAttribI2uiv);
				entries[16] = reinterpret_cast<void**>(&glVertexAttribI3uiv);
				entries[17] = reinterpret_cast<void**>(&glVertexAttribI4uiv);
				entries[18] = reinterpret_cast<void**>(&glVertexAttribI4bv);
				entries[19] = reinterpret_cast<void**>(&glVertexAttribI4sv);
				entries[20] = reinterpret_cast<void**>(&glVertexAttribI4ubv);
				entries[21] = reinterpret_cast<void**>(&glVertexAttribI4usv);
				entries[22] = reinterpret_cast<void**>(&glVertexAttribIPointer);
				entries[23] = reinterpret_cast<void**>(&glGetVertexAttribIiv);
				entries[24] = reinterpret_cast<void**>(&glGetVertexAttribIuiv);
				entries[25] = reinterpret_cast<void**>(&glUniform1ui);
				entries[26] = reinterpret_cast<void**>(&glUniform2ui);
				entries[27] = reinterpret_cast<void**>(&glUniform3ui);
				entries[28] = reinterpret_cast<void**>(&glUniform4ui);
				entries[29] = reinterpret_cast<void**>(&glUniform1uiv);
				entries[30] = reinterpret_cast<void**>(&glUniform2uiv);
				entries[31] = reinterpret_cast<void**>(&glUniform3uiv);
				entries[32] = reinterpret_cast<void**>(&glUniform4uiv);
				entries[33] = reinterpret_cast<void**>(&glGetUniformuiv);
				entries[34] = reinterpret_cast<void**>(&glBindFragDataLocation);
				entries[35] = reinterpret_cast<void**>(&glGetFragDataLocation);
				entries[36] = reinterpret_cast<void**>(&glBeginConditionalRender);
				entries[37] = reinterpret_cast<void**>(&glEndConditionalRender);
				entries[38] = reinterpret_cast<void**>(&glClampColor);
				entries[39] = reinterpret_cast<void**>(&glRenderbufferStorageMultisample);
				entries[40] = reinterpret_cast<void**>(&glBlitFramebuffer);
				entries[41] = reinterpret_cast<void**>(&glClearColorIi);
				entries[42] = reinterpret_cast<void**>(&glClearColorIui);
				entries[43] = reinterpret_cast<void**>(&glTexParameterIiv);
				entries[44] = reinterpret_cast<void**>(&glTexParameterIuiv);
				entries[45] = reinterpret_cast<void**>(&glGetTexParameterIiv);
				entries[46] = reinterpret_cast<void**>(&glGetTexParameterIuiv);
				entries[47] = reinterpret_cast<void**>(&glFramebufferTextureLayer);
				entries[48] = reinterpret_cast<void**>(&glBindBufferRange);
				entries[49] = reinterpret_cast<void**>(&glBindBufferOffset);
				entries[50] = reinterpret_cast<void**>(&glBindBufferBase);
				entries[51] = reinterpret_cast<void**>(&glBeginTransformFeedback);
				entries[52] = reinterpret_cast<void**>(&glEndTransformFeedback);
				entries[53] = reinterpret_cast<void**>(&glTransformFeedbackVaryings);
				entries[54] = reinterpret_cast<void**>(&glGetTransformFeedbackVarying);
			}

			funcs_names_t names(6);
			{
				names[0] = "glMapBufferRange";
				names[1] = "glFlushMappedBufferRange";
				names[2] = "glVertexAttribI1i";
				names[3] = "glVertexAttribI2i";
				names[4] = "glVertexAttribI3i";
				names[5] = "glVertexAttribI4i";
				names[6] = "glVertexAttribI1ui";
				names[7] = "glVertexAttribI2ui";
				names[8] = "glVertexAttribI3ui";
				names[9] = "glVertexAttribI4ui";
				names[10] = "glVertexAttribI1iv";
				names[11] = "glVertexAttribI2iv";
				names[12] = "glVertexAttribI3iv";
				names[13] = "glVertexAttribI4iv";
				names[14] = "glVertexAttribI1uiv";
				names[15] = "glVertexAttribI2uiv";
				names[16] = "glVertexAttribI3uiv";
				names[17] = "glVertexAttribI4uiv";
				names[18] = "glVertexAttribI4bv";
				names[19] = "glVertexAttribI4sv";
				names[20] = "glVertexAttribI4ubv";
				names[21] = "glVertexAttribI4usv";
				names[22] = "glVertexAttribIPointer";
				names[23] = "glGetVertexAttribIiv";
				names[24] = "glGetVertexAttribIuiv";
				names[25] = "glUniform1ui";
				names[26] = "glUniform2ui";
				names[27] = "glUniform3ui";
				names[28] = "glUniform4ui";
				names[29] = "glUniform1uiv";
				names[30] = "glUniform2uiv";
				names[31] = "glUniform3uiv";
				names[32] = "glUniform4uiv";
				names[33] = "glGetUniformuiv";
				names[34] = "glBindFragDataLocation";
				names[35] = "glGetFragDataLocation";
				names[36] = "glBeginConditionalRender";
				names[37] = "glEndConditionalRender";
				names[38] = "glClampColor";
				names[39] = "glRenderbufferStorageMultisample";
				names[40] = "glBlitFramebuffer";
				names[41] = "glClearColorIi";
				names[42] = "glClearColorIui";
				names[43] = "glTexParameterIiv";
				names[44] = "glTexParameterIuiv";
				names[45] = "glGetTexParameterIiv";
				names[46] = "glGetTexParameterIuiv";
				names[47] = "glFramebufferTextureLayer";
				names[48] = "glBindBufferRange";
				names[49] = "glBindBufferOffset";
				names[50] = "glBindBufferBase";
				names[51] = "glBeginTransformFeedback";
				names[52] = "glEndTransformFeedback";
				names[53] = "glTransformFeedbackVaryings";
				names[54] = "glGetTransformFeedbackVarying";
			}

			load_funcs(entries, names);
		}
		else
		{
			if (glloader_GL_EXT_gpu_shader4())
			{
				glVertexAttribI1i = glVertexAttribI1iEXT;
				glVertexAttribI2i = glVertexAttribI2iEXT;
				glVertexAttribI3i = glVertexAttribI3iEXT;
				glVertexAttribI4i = glVertexAttribI4iEXT;
				glVertexAttribI1ui = glVertexAttribI1uiEXT;
				glVertexAttribI2ui = glVertexAttribI2uiEXT;
				glVertexAttribI3ui = glVertexAttribI3uiEXT;
				glVertexAttribI4ui = glVertexAttribI4uiEXT;
				glVertexAttribI1iv = glVertexAttribI1ivEXT;
				glVertexAttribI2iv = glVertexAttribI2ivEXT;
				glVertexAttribI3iv = glVertexAttribI3ivEXT;
				glVertexAttribI4iv = glVertexAttribI4ivEXT;
				glVertexAttribI1uiv = glVertexAttribI1uivEXT;
				glVertexAttribI2uiv = glVertexAttribI2uivEXT;
				glVertexAttribI3uiv = glVertexAttribI3uivEXT;
				glVertexAttribI4uiv = glVertexAttribI4uivEXT;
				glVertexAttribI4bv = glVertexAttribI4bvEXT;
				glVertexAttribI4sv = glVertexAttribI4svEXT;
				glVertexAttribI4ubv = glVertexAttribI4ubvEXT;
				glVertexAttribI4usv = glVertexAttribI4usvEXT;
				glVertexAttribIPointer = glVertexAttribIPointerEXT;
				glGetVertexAttribIiv = glGetVertexAttribIivEXT;
				glGetVertexAttribIuiv = glGetVertexAttribIuivEXT;
				glUniform1ui = glUniform1uiEXT;
				glUniform2ui = glUniform2uiEXT;
				glUniform3ui = glUniform3uiEXT;
				glUniform4ui = glUniform4uiEXT;
				glUniform1uiv = glUniform1uivEXT;
				glUniform2uiv = glUniform2uivEXT;
				glUniform3uiv = glUniform3uivEXT;
				glUniform4uiv = glUniform4uivEXT;
				glGetUniformuiv = glGetUniformuivEXT;
				glBindFragDataLocation = glBindFragDataLocationEXT;
				glGetFragDataLocation = glGetFragDataLocationEXT;
			}

			if (glloader_GL_NV_conditional_render())
			{
				glBeginConditionalRender = glBeginConditionalRenderNV;
				glEndConditionalRender = glEndConditionalRenderNV;
			}

			if (glloader_GL_ARB_color_buffer_float())
			{
				glClampColor = glClampColorARB;
			}

			if (glloader_GL_EXT_framebuffer_multisample())
			{
				glRenderbufferStorageMultisample = glRenderbufferStorageMultisampleEXT;
			}

			if (glloader_GL_EXT_framebuffer_blit())
			{
				glBlitFramebuffer = glBlitFramebufferEXT;
			}

			if (glloader_GL_EXT_texture_integer())
			{
				glClearColorIi = glClearColorIiEXT;
				glClearColorIui = glClearColorIuiEXT;
				glTexParameterIiv = glTexParameterIivEXT;
				glTexParameterIuiv = glTexParameterIuivEXT;
				glGetTexParameterIiv = glGetTexParameterIivEXT;
				glGetTexParameterIuiv = glGetTexParameterIuivEXT;
			}

			if (glloader_GL_EXT_texture_array())
			{
				glFramebufferTextureLayer = glFramebufferTextureLayerARB;
			}

			if (glloader_GL_EXT_transform_feedback())
			{
				glBindBufferRange = glBindBufferRangeEXT;
				glBindBufferOffset = glBindBufferOffsetEXT;
				glBindBufferBase = glBindBufferBaseEXT;
				glBeginTransformFeedback = glBeginTransformFeedbackEXT;
				glEndTransformFeedback = glEndTransformFeedbackEXT;
				glTransformFeedbackVaryings = glTransformFeedbackVaryingsEXT;
				glGetTransformFeedbackVarying = glGetTransformFeedbackVaryingEXT;
			}

			if (glloader_GL_EXT_gpu_shader4()
				&& glloader_GL_NV_conditional_render()
				&& glloader_GL_APPLE_flush_buffer_range()
				&& glloader_GL_ARB_color_buffer_float()
				&& glloader_GL_ARB_depth_buffer_float()
				&& glloader_GL_ARB_texture_float()
				&& glloader_GL_EXT_packed_float()
				&& glloader_GL_EXT_texture_shared_exponent()
				&& glloader_GL_EXT_framebuffer_multisample()
				&& glloader_GL_EXT_framebuffer_blit()
				&& glloader_GL_ARB_framebuffer_sRGB()
				&& glloader_GL_ARB_half_float_vertex()
				&& glloader_GL_ARB_map_buffer_range()
				&& glloader_GL_ARB_texture_compression_rgtc()
				&& glloader_GL_ARB_texture_rg()
				&& glloader_GL_ARB_vertex_array_object()
				&& glloader_GL_EXT_texture_integer()
				&& glloader_GL_EXT_texture_array()
				&& glloader_GL_EXT_packed_depth_stencil()
				&& glloader_GL_EXT_transform_feedback())
			{
				_GL_VERSION_3_0 = true;
			}
		}
	}

	char APIENTRY self_init_glloader_GL_VERSION_3_0()
	{
		glloader_GL_VERSION_2_1();

		init_GL_VERSION_3_0();
		return glloader_GL_VERSION_3_0();
	}
}

glloader_GL_VERSION_3_0FUNC glloader_GL_VERSION_3_0 = self_init_glloader_GL_VERSION_3_0;

#ifdef GL_VERSION_3_0

namespace
{
	void APIENTRY self_init_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
	{
		init_GL_VERSION_3_0();
		return glMapBufferRange(target, offset, length, access);
	}
	void APIENTRY self_init_glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
	{
		init_GL_VERSION_3_0();
		return glFlushMappedBufferRange(target, offset, length);
	}
	void APIENTRY self_init_glVertexAttribI1i(GLuint index, GLint x)
	{
		glloader_init();
		return glVertexAttribI1i(index, x);
	}
	void APIENTRY self_init_glVertexAttribI2i(GLuint index, GLint x, GLint y)
	{
		glloader_init();
		return glVertexAttribI2i(index, x, y);
	}
	void APIENTRY self_init_glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z)
	{
		glloader_init();
		return glVertexAttribI3i(index, x, y, z);
	}
	void APIENTRY self_init_glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
	{
		glloader_init();
		return glVertexAttribI4i(index, x, y, z, w);
	}
	void APIENTRY self_init_glVertexAttribI1ui(GLuint index, GLuint x)
	{
		glloader_init();
		return glVertexAttribI1ui(index, x);
	}
	void APIENTRY self_init_glVertexAttribI2ui(GLuint index, GLuint x, GLuint y)
	{
		glloader_init();
		return glVertexAttribI2ui(index, x, y);
	}
	void APIENTRY self_init_glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z)
	{
		glloader_init();
		return glVertexAttribI3ui(index, x, y, z);
	}
	void APIENTRY self_init_glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
	{
		glloader_init();
		return glVertexAttribI4ui(index, x, y, z, w);
	}
	void APIENTRY self_init_glVertexAttribI1iv(GLuint index, const GLint* v)
	{
		glloader_init();
		return glVertexAttribI1iv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI2iv(GLuint index, const GLint* v)
	{
		glloader_init();
		return glVertexAttribI2iv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI3iv(GLuint index, const GLint* v)
	{
		glloader_init();
		return glVertexAttribI3iv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI4iv(GLuint index, const GLint* v)
	{
		glloader_init();
		return glVertexAttribI4iv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI1uiv(GLuint index, const GLuint* v)
	{
		glloader_init();
		return glVertexAttribI1uiv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI2uiv(GLuint index, const GLuint* v)
	{
		glloader_init();
		return glVertexAttribI2uiv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI3uiv(GLuint index, const GLuint* v)
	{
		glloader_init();
		return glVertexAttribI3uiv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI4uiv(GLuint index, const GLuint* v)
	{
		glloader_init();
		return glVertexAttribI4uiv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI4bv(GLuint index, const GLbyte* v)
	{
		glloader_init();
		return glVertexAttribI4bv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI4sv(GLuint index, const GLshort* v)
	{
		glloader_init();
		return glVertexAttribI4sv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI4ubv(GLuint index, const GLubyte* v)
	{
		glloader_init();
		return glVertexAttribI4ubv(index, v);
	}
	void APIENTRY self_init_glVertexAttribI4usv(GLuint index, const GLushort* v)
	{
		glloader_init();
		return glVertexAttribI4usv(index, v);
	}
	void APIENTRY self_init_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
	{
		glloader_init();
		return glVertexAttribIPointer(index, size, type, stride, pointer);
	}
	void APIENTRY self_init_glGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
	{
		glloader_init();
		return glGetVertexAttribIiv(index, pname, params);
	}
	void APIENTRY self_init_glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
	{
		glloader_init();
		return glGetVertexAttribIuiv(index, pname, params);
	}
	void APIENTRY self_init_glUniform1ui(GLint location, GLuint v0)
	{
		glloader_init();
		return glUniform1ui(location, v0);
	}
	void APIENTRY self_init_glUniform2ui(GLint location, GLuint v0, GLuint v1)
	{
		glloader_init();
		return glUniform2ui(location, v0, v1);
	}
	void APIENTRY self_init_glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
	{
		glloader_init();
		return glUniform3ui(location, v0, v1, v2);
	}
	void APIENTRY self_init_glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
	{
		glloader_init();
		return glUniform4ui(location, v0, v1, v2, v3);
	}
	void APIENTRY self_init_glUniform1uiv(GLint location, GLsizei count, const GLuint* value)
	{
		glloader_init();
		return glUniform1uiv(location, count, value);
	}
	void APIENTRY self_init_glUniform2uiv(GLint location, GLsizei count, const GLuint* value)
	{
		glloader_init();
		return glUniform2uiv(location, count, value);
	}
	void APIENTRY self_init_glUniform3uiv(GLint location, GLsizei count, const GLuint* value)
	{
		glloader_init();
		return glUniform3uiv(location, count, value);
	}
	void APIENTRY self_init_glUniform4uiv(GLint location, GLsizei count, const GLuint* value)
	{
		glloader_init();
		return glUniform4uiv(location, count, value);
	}
	void APIENTRY self_init_glGetUniformuiv(GLuint program, GLint location, GLuint* params)
	{
		glloader_init();
		return glGetUniformuiv(program, location, params);
	}
	void APIENTRY self_init_glBindFragDataLocation(GLuint program, GLuint colorNumber, const GLchar* name)
	{
		glloader_init();
		return glBindFragDataLocation(program, colorNumber, name);
	}
	GLint APIENTRY self_init_glGetFragDataLocation(GLuint program, const GLchar* name)
	{
		glloader_init();
		return glGetFragDataLocation(program, name);
	}
	void APIENTRY self_init_glBeginConditionalRender(GLuint id, GLenum mode)
	{
		glloader_init();
		return glBeginConditionalRender(id, mode);
	}
	void APIENTRY self_init_glEndConditionalRender()
	{
		glloader_init();
		return glEndConditionalRender();
	}
	void APIENTRY self_init_glClampColor(GLenum target, GLenum clamp)
	{
		glloader_init();
		return glClampColor(target, clamp);
	}
	void APIENTRY self_init_glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
	{
		glloader_init();
		return glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
	}
	void APIENTRY self_init_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
	{
		glloader_init();
		return glBlitFramebufferEXT(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
	}
	void APIENTRY self_init_glClearColorIi(GLint r, GLint g, GLint b, GLint a)
	{
		glloader_init();
		return glClearColorIi(r, g, b, a);
	}
	void APIENTRY self_init_glClearColorIui(GLuint r, GLuint g, GLuint b, GLuint a)
	{
		glloader_init();
		return glClearColorIui(r, g, b, a);
	}
	void APIENTRY self_init_glTexParameterIiv(GLenum target, GLenum pname, GLint* params)
	{
		glloader_init();
		return glTexParameterIiv(target, pname, params);
	}
	void APIENTRY self_init_glTexParameterIuiv(GLenum target, GLenum pname, GLuint* params)
	{
		glloader_init();
		return glTexParameterIuiv(target, pname, params);
	}
	void APIENTRY self_init_glGetTexParameterIiv(GLenum target, GLenum pname, GLint* params)
	{
		glloader_init();
		return glGetTexParameterIiv(target, pname, params);
	}
	void APIENTRY self_init_glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint* params)
	{
		glloader_init();
		return glGetTexParameterIuiv(target, pname, params);
	}
	void APIENTRY self_init_glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
	{
		glloader_init();
		return glBindBufferRange(target, index, buffer, offset, size);
	}
	void APIENTRY self_init_glBindBufferOffset(GLenum target, GLuint index, GLuint buffer, GLintptr offset)
	{
		glloader_init();
		return glBindBufferOffset(target, index, buffer, offset);
	}
	void APIENTRY self_init_glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
	{
		glloader_init();
		return glBindBufferBase(target, index, buffer);
	}
	void APIENTRY self_init_glBeginTransformFeedback(GLenum primitiveMode)
	{
		glloader_init();
		return glBeginTransformFeedback(primitiveMode);
	}
	void APIENTRY self_init_glEndTransformFeedback()
	{
		glloader_init();
		return glEndTransformFeedback();
	}
	void APIENTRY self_init_glTransformFeedbackVaryings(GLuint program, GLsizei count, const char** varyings, GLenum bufferMode)
	{
		glloader_init();
		return glTransformFeedbackVaryings(program, count, varyings, bufferMode);
	}
	void APIENTRY self_init_glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
	{
		glloader_init();
		return glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
	}
}

glMapBufferRangeFUNC glMapBufferRange = self_init_glMapBufferRange;
glFlushMappedBufferRangeFUNC glFlushMappedBufferRange = self_init_glFlushMappedBufferRange;
glVertexAttribI1iFUNC glVertexAttribI1i = self_init_glVertexAttribI1i;
glVertexAttribI2iFUNC glVertexAttribI2i = self_init_glVertexAttribI2i;
glVertexAttribI3iFUNC glVertexAttribI3i = self_init_glVertexAttribI3i;
glVertexAttribI4iFUNC glVertexAttribI4i = self_init_glVertexAttribI4i;
glVertexAttribI1uiFUNC glVertexAttribI1ui = self_init_glVertexAttribI1ui;
glVertexAttribI2uiFUNC glVertexAttribI2ui = self_init_glVertexAttribI2ui;
glVertexAttribI3uiFUNC glVertexAttribI3ui = self_init_glVertexAttribI3ui;
glVertexAttribI4uiFUNC glVertexAttribI4ui = self_init_glVertexAttribI4ui;
glVertexAttribI1ivFUNC glVertexAttribI1iv = self_init_glVertexAttribI1iv;
glVertexAttribI2ivFUNC glVertexAttribI2iv = self_init_glVertexAttribI2iv;
glVertexAttribI3ivFUNC glVertexAttribI3iv = self_init_glVertexAttribI3iv;
glVertexAttribI4ivFUNC glVertexAttribI4iv = self_init_glVertexAttribI4iv;
glVertexAttribI1uivFUNC glVertexAttribI1uiv = self_init_glVertexAttribI1uiv;
glVertexAttribI2uivFUNC glVertexAttribI2uiv = self_init_glVertexAttribI2uiv;
glVertexAttribI3uivFUNC glVertexAttribI3uiv = self_init_glVertexAttribI3uiv;
glVertexAttribI4uivFUNC glVertexAttribI4uiv = self_init_glVertexAttribI4uiv;
glVertexAttribI4bvFUNC glVertexAttribI4bv = self_init_glVertexAttribI4bv;
glVertexAttribI4svFUNC glVertexAttribI4sv = self_init_glVertexAttribI4sv;
glVertexAttribI4ubvFUNC glVertexAttribI4ubv = self_init_glVertexAttribI4ubv;
glVertexAttribI4usvFUNC glVertexAttribI4usv = self_init_glVertexAttribI4usv;
glVertexAttribIPointerFUNC glVertexAttribIPointer = self_init_glVertexAttribIPointer;
glGetVertexAttribIivFUNC glGetVertexAttribIiv = self_init_glGetVertexAttribIiv;
glGetVertexAttribIuivFUNC glGetVertexAttribIuiv = self_init_glGetVertexAttribIuiv;
glUniform1uiFUNC glUniform1ui = self_init_glUniform1ui;
glUniform2uiFUNC glUniform2ui = self_init_glUniform2ui;
glUniform3uiFUNC glUniform3ui = self_init_glUniform3ui;
glUniform4uiFUNC glUniform4ui = self_init_glUniform4ui;
glUniform1uivFUNC glUniform1uiv = self_init_glUniform1uiv;
glUniform2uivFUNC glUniform2uiv = self_init_glUniform2uiv;
glUniform3uivFUNC glUniform3uiv = self_init_glUniform3uiv;
glUniform4uivFUNC glUniform4uiv = self_init_glUniform4uiv;
glGetUniformuivFUNC glGetUniformuiv = self_init_glGetUniformuiv;
glBindFragDataLocationFUNC glBindFragDataLocation = self_init_glBindFragDataLocation;
glGetFragDataLocationFUNC glGetFragDataLocation = self_init_glGetFragDataLocation;
glBeginConditionalRenderFUNC glBeginConditionalRender = self_init_glBeginConditionalRender;
glEndConditionalRenderFUNC glEndConditionalRender = self_init_glEndConditionalRender;
glClampColorFUNC glClampColor = self_init_glClampColor;
glRenderbufferStorageMultisampleFUNC glRenderbufferStorageMultisample = self_init_glRenderbufferStorageMultisample;
glBlitFramebufferFUNC glBlitFramebuffer = self_init_glBlitFramebuffer;
glClearColorIiFUNC glClearColorIi = self_init_glClearColorIi;
glClearColorIuiFUNC glClearColorIui = self_init_glClearColorIui;
glTexParameterIivFUNC glTexParameterIiv = self_init_glTexParameterIiv;
glTexParameterIuivFUNC glTexParameterIuiv = self_init_glTexParameterIuiv;
glGetTexParameterIivFUNC glGetTexParameterIiv = self_init_glGetTexParameterIiv;
glGetTexParameterIuivFUNC glGetTexParameterIuiv = self_init_glGetTexParameterIuiv;
glBindBufferRangeFUNC glBindBufferRange = self_init_glBindBufferRange;
glBindBufferOffsetFUNC glBindBufferOffset = self_init_glBindBufferOffset;
glBindBufferBaseFUNC glBindBufferBase = self_init_glBindBufferBase;
glBeginTransformFeedbackFUNC glBeginTransformFeedback = self_init_glBeginTransformFeedback;
glEndTransformFeedbackFUNC glEndTransformFeedback = self_init_glEndTransformFeedback;
glTransformFeedbackVaryingsFUNC glTransformFeedbackVaryings = self_init_glTransformFeedbackVaryings;
glGetTransformFeedbackVaryingFUNC glGetTransformFeedbackVarying = self_init_glGetTransformFeedbackVarying;

#endif		// GL_VERSION_3_0

#endif			// GLLOADER_GL
