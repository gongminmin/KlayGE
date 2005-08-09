/*
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
*/

#ifndef _GL15_H
#define _GL15_H

#ifdef __cplusplus
extern "C"
{
#endif

/* OpenGL 1.5 */

typedef char (APIENTRY *glloader_GL_VERSION_1_5FUNC)();
extern glloader_GL_VERSION_1_5FUNC glloader_GL_VERSION_1_5;

#ifndef GL_VERSION_1_5
#define GL_VERSION_1_5 1

#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_BUFFER_ACCESS 0x88BB
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_SAMPLES_PASSED 0x8914

typedef void (APIENTRY *glGenQueriesFUNC)(GLsizei n, GLuint* ids);
typedef void (APIENTRY *glDeleteQueriesFUNC)(GLsizei n, const GLuint* ids);
typedef GLboolean (APIENTRY *glIsQueryFUNC)(GLuint id);
typedef void (APIENTRY *glBeginQueryFUNC)(GLenum target, GLuint id);
typedef void (APIENTRY *glEndQueryFUNC)(GLenum target);
typedef void (APIENTRY *glGetQueryivFUNC)(GLenum target, GLenum pname, GLint* param);
typedef void (APIENTRY *glGetQueryObjectivFUNC)(GLuint id, GLenum pname, GLint* params);
typedef void (APIENTRY *glGetQueryObjectuivFUNC)(GLuint id, GLenum pname, GLuint* params);
typedef void (APIENTRY *glBindBufferFUNC)(GLenum target, GLuint buffer);
typedef void (APIENTRY *glDeleteBuffersFUNC)(GLsizei n, const GLuint* buffers);
typedef void (APIENTRY *glGenBuffersFUNC)(GLsizei n, GLuint* buffers);
typedef GLboolean (APIENTRY *glIsBufferFUNC)(GLuint buffer);
typedef void (APIENTRY *glBufferDataFUNC)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void (APIENTRY *glBufferSubDataFUNC)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef void (APIENTRY *glGetBufferSubDataFUNC)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
typedef GLvoid* (APIENTRY *glMapBufferFUNC)(GLenum target, GLenum access);
typedef GLboolean (APIENTRY *glUnmapBufferFUNC)(GLenum target);
typedef void (APIENTRY *glGetBufferParameterivFUNC)(GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY *glGetBufferPointervFUNC)(GLenum target, GLenum pname, GLvoid** params);

extern glGenQueriesFUNC glGenQueries;
extern glDeleteQueriesFUNC glDeleteQueries;
extern glIsQueryFUNC glIsQuery;
extern glBeginQueryFUNC glBeginQuery;
extern glEndQueryFUNC glEndQuery;
extern glGetQueryivFUNC glGetQueryiv;
extern glGetQueryObjectivFUNC glGetQueryObjectiv;
extern glGetQueryObjectuivFUNC glGetQueryObjectuiv;
extern glBindBufferFUNC glBindBuffer;
extern glDeleteBuffersFUNC glDeleteBuffers;
extern glGenBuffersFUNC glGenBuffers;
extern glIsBufferFUNC glIsBuffer;
extern glBufferDataFUNC glBufferData;
extern glBufferSubDataFUNC glBufferSubData;
extern glGetBufferSubDataFUNC glGetBufferSubData;
extern glMapBufferFUNC glMapBuffer;
extern glUnmapBufferFUNC glUnmapBuffer;
extern glGetBufferParameterivFUNC glGetBufferParameteriv;
extern glGetBufferPointervFUNC glGetBufferPointerv;

#endif		/* GL_VERSION_1_5 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL15_H */
