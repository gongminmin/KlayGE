/*
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
*/

#ifndef _GL30_H
#define _GL30_H

#ifdef __cplusplus
extern "C"
{
#endif

/* OpenGL 3.0 */

typedef char (APIENTRY *glloader_GL_VERSION_3_0FUNC)();
extern glloader_GL_VERSION_3_0FUNC glloader_GL_VERSION_3_0;

#ifndef GL_VERSION_3_0
#define GL_VERSION_3_0 1

#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C
#define GL_NUM_EXTENSIONS 0x821D
#define GL_CONTEXT_FLAGS 0x821E
#define GL_DEPTH_BUFFER 0x8223
#define GL_STENCIL_BUFFER 0x8224
#define GL_COMPRESSED_RED 0x8225
#define GL_COMPRESSED_RG 0x8226
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER 0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET 0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET 0x8905
#define GL_CLAMP_VERTEX_COLOR 0x891A
#define GL_CLAMP_FRAGMENT_COLOR 0x891B
#define GL_CLAMP_READ_COLOR 0x891C
#define GL_FIXED_ONLY 0x891D
#define GL_TEXTURE_RED_TYPE 0x8C10
#define GL_TEXTURE_GREEN_TYPE 0x8C11
#define GL_TEXTURE_BLUE_TYPE 0x8C12
#define GL_TEXTURE_ALPHA_TYPE 0x8C13
#define GL_TEXTURE_LUMINANCE_TYPE 0x8C14
#define GL_TEXTURE_INTENSITY_TYPE 0x8C15
#define GL_TEXTURE_DEPTH_TYPE 0x8C16
#define GL_UNSIGNED_NORMALIZED 0x8C17
#define GL_TEXTURE_1D_ARRAY 0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY 0x8C19
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY 0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY 0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY 0x8C1D
#define GL_R11F_G11F_B10F 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
#define GL_RGB9_E5 0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
#define GL_TEXTURE_SHARED_SIZE 0x8C3F
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS 0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#define GL_PRIMITIVES_GENERATED 0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#define GL_RASTERIZER_DISCARD 0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#define GL_INTERLEAVED_ATTRIBS 0x8C8C
#define GL_SEPARATE_ATTRIBS 0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#define GL_RGBA32UI 0x8D70
#define GL_RGB32UI 0x8D71
#define GL_RGBA16UI 0x8D76
#define GL_RGB16UI 0x8D77
#define GL_RGBA8UI 0x8D7C
#define GL_RGB8UI 0x8D7D
#define GL_RGBA32I 0x8D82
#define GL_RGB32I 0x8D83
#define GL_RGBA16I 0x8D88
#define GL_RGB16I 0x8D89
#define GL_RGBA8I 0x8D8E
#define GL_RGB8I 0x8D8F
#define GL_RED_INTEGER 0x8D94
#define GL_GREEN_INTEGER 0x8D95
#define GL_BLUE_INTEGER 0x8D96
#define GL_ALPHA_INTEGER 0x8D97
#define GL_RGB_INTEGER 0x8D98
#define GL_RGBA_INTEGER 0x8D99
#define GL_BGR_INTEGER 0x8D9A
#define GL_BGRA_INTEGER 0x8D9B
#define GL_SAMPLER_1D_ARRAY 0x8DC0
#define GL_SAMPLER_2D_ARRAY 0x8DC1
#define GL_SAMPLER_1D_ARRAY_SHADOW 0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW 0x8DC4
#define GL_SAMPLER_CUBE_SHADOW 0x8DC5
#define GL_UNSIGNED_INT_VEC2 0x8DC6
#define GL_UNSIGNED_INT_VEC3 0x8DC7
#define GL_UNSIGNED_INT_VEC4 0x8DC8
#define GL_INT_SAMPLER_1D 0x8DC9
#define GL_INT_SAMPLER_2D 0x8DCA
#define GL_INT_SAMPLER_3D 0x8DCB
#define GL_INT_SAMPLER_CUBE 0x8DCC
#define GL_INT_SAMPLER_1D_ARRAY 0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY 0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_1D 0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D 0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D 0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE 0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 0x8DD7
#define GL_QUERY_WAIT 0x8E13
#define GL_QUERY_NO_WAIT 0x8E14
#define GL_QUERY_BY_REGION_WAIT 0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT 0x8E16
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#define GL_FRAMEBUFFER_DEFAULT 0x8218
#define GL_FRAMEBUFFER_UNDEFINED 0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_INDEX 0x8222
#define GL_MAX_RENDERBUFFER_SIZE 0x84E8
#define GL_DEPTH_STENCIL 0x84F9
#define GL_UNSIGNED_INT_24_8 0x84FA
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_TEXTURE_STENCIL_SIZE 0x88F1
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_RENDERBUFFER_BINDING 0x8CA7
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define GL_RENDERBUFFER_SAMPLES 0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_COLOR_ATTACHMENT5 0x8CE5
#define GL_COLOR_ATTACHMENT6 0x8CE6
#define GL_COLOR_ATTACHMENT7 0x8CE7
#define GL_COLOR_ATTACHMENT8 0x8CE8
#define GL_COLOR_ATTACHMENT9 0x8CE9
#define GL_COLOR_ATTACHMENT10 0x8CEA
#define GL_COLOR_ATTACHMENT11 0x8CEB
#define GL_COLOR_ATTACHMENT12 0x8CEC
#define GL_COLOR_ATTACHMENT13 0x8CED
#define GL_COLOR_ATTACHMENT14 0x8CEE
#define GL_COLOR_ATTACHMENT15 0x8CEF
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_RENDERBUFFER_WIDTH 0x8D42
#define GL_RENDERBUFFER_HEIGHT 0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT 0x8D44
#define GL_STENCIL_INDEX1 0x8D46
#define GL_STENCIL_INDEX4 0x8D47
#define GL_STENCIL_INDEX8 0x8D48
#define GL_STENCIL_INDEX16 0x8D49
#define GL_RENDERBUFFER_RED_SIZE 0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE 0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE 0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE 0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE 0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE 0x8D55
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_MAX_SAMPLES 0x8D57
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_DEPTH32F_STENCIL8 0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_HALF_FLOAT 0x140B
#define GL_MAP_READ_BIT 0x0001
#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT 0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT 0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT 0x0020
#define GL_COMPRESSED_RED_RGTC1 0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
#define GL_COMPRESSED_RED_GREEN_RGTC2 0x8DBD
#define GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2 0x8DBE
#define GL_RG 0x8227
#define GL_RG_INTEGER 0x8228
#define GL_R8 0x8229
#define GL_R16 0x822A
#define GL_RG8 0x822B
#define GL_RG16 0x822C
#define GL_R16F 0x822D
#define GL_R32F 0x822E
#define GL_RG16F 0x822F
#define GL_RG32F 0x8230
#define GL_R8I 0x8231
#define GL_R8UI 0x8232
#define GL_R16I 0x8233
#define GL_R16UI 0x8234
#define GL_R32I 0x8235
#define GL_R32UI 0x8236
#define GL_RG8I 0x8237
#define GL_RG8UI 0x8238
#define GL_RG16I 0x8239
#define GL_RG16UI 0x823A
#define GL_RG32I 0x823B
#define GL_RG32UI 0x823C
#define GL_FRAMEBUFFER_SRGB 0x8DB9

typedef void (APIENTRY *glMapBufferRangeFUNC)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (APIENTRY *glFlushMappedBufferRangeFUNC)(GLenum target, GLintptr offset, GLsizeiptr length);
typedef void (APIENTRY *glVertexAttribI1iFUNC)(GLuint index, GLint x);
typedef void (APIENTRY *glVertexAttribI2iFUNC)(GLuint index, GLint x, GLint y);
typedef void (APIENTRY *glVertexAttribI3iFUNC)(GLuint index, GLint x, GLint y, GLint z);
typedef void (APIENTRY *glVertexAttribI4iFUNC)(GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void (APIENTRY *glVertexAttribI1uiFUNC)(GLuint index, GLuint x);
typedef void (APIENTRY *glVertexAttribI2uiFUNC)(GLuint index, GLuint x, GLuint y);
typedef void (APIENTRY *glVertexAttribI3uiFUNC)(GLuint index, GLuint x, GLuint y, GLuint z);
typedef void (APIENTRY *glVertexAttribI4uiFUNC)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void (APIENTRY *glVertexAttribI1ivFUNC)(GLuint index, const GLint* v);
typedef void (APIENTRY *glVertexAttribI2ivFUNC)(GLuint index, const GLint* v);
typedef void (APIENTRY *glVertexAttribI3ivFUNC)(GLuint index, const GLint* v);
typedef void (APIENTRY *glVertexAttribI4ivFUNC)(GLuint index, const GLint* v);
typedef void (APIENTRY *glVertexAttribI1uivFUNC)(GLuint index, const GLuint* v);
typedef void (APIENTRY *glVertexAttribI2uivFUNC)(GLuint index, const GLuint* v);
typedef void (APIENTRY *glVertexAttribI3uivFUNC)(GLuint index, const GLuint* v);
typedef void (APIENTRY *glVertexAttribI4uivFUNC)(GLuint index, const GLuint* v);
typedef void (APIENTRY *glVertexAttribI4bvFUNC)(GLuint index, const GLbyte* v);
typedef void (APIENTRY *glVertexAttribI4svFUNC)(GLuint index, const GLshort* v);
typedef void (APIENTRY *glVertexAttribI4ubvFUNC)(GLuint index, const GLubyte* v);
typedef void (APIENTRY *glVertexAttribI4usvFUNC)(GLuint index, const GLushort* v);
typedef void (APIENTRY *glVertexAttribIPointerFUNC)(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
typedef void (APIENTRY *glGetVertexAttribIivFUNC)(GLuint index, GLenum pname, GLint* params);
typedef void (APIENTRY *glGetVertexAttribIuivFUNC)(GLuint index, GLenum pname, GLuint* params);
typedef void (APIENTRY *glUniform1uiFUNC)(GLint location, GLuint v0);
typedef void (APIENTRY *glUniform2uiFUNC)(GLint location, GLuint v0, GLuint v1);
typedef void (APIENTRY *glUniform3uiFUNC)(GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (APIENTRY *glUniform4uiFUNC)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (APIENTRY *glUniform1uivFUNC)(GLint location, GLsizei count, const GLuint* value);
typedef void (APIENTRY *glUniform2uivFUNC)(GLint location, GLsizei count, const GLuint* value);
typedef void (APIENTRY *glUniform3uivFUNC)(GLint location, GLsizei count, const GLuint* value);
typedef void (APIENTRY *glUniform4uivFUNC)(GLint location, GLsizei count, const GLuint* value);
typedef void (APIENTRY *glGetUniformuivFUNC)(GLuint program, GLint location, GLuint* params);
typedef void (APIENTRY *glBindFragDataLocationFUNC)(GLuint program, GLuint colorNumber, const GLchar* name);
typedef GLint (APIENTRY *glGetFragDataLocationFUNC)(GLuint program, const GLchar* name);
typedef void (APIENTRY *glBeginConditionalRenderFUNC)(GLuint id, GLenum mode);
typedef void (APIENTRY *glEndConditionalRenderFUNC)();
typedef void (APIENTRY *glClampColorFUNC)(GLenum target, GLenum clamp);
typedef void (APIENTRY *glRenderbufferStorageMultisampleFUNC)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY *glBlitFramebufferFUNC)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void (APIENTRY *glClearColorIiFUNC)(GLint r, GLint g, GLint b, GLint a);
typedef void (APIENTRY *glClearColorIuiFUNC)(GLuint r, GLuint g, GLuint b, GLuint a);
typedef void (APIENTRY *glTexParameterIivFUNC)(GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY *glTexParameterIuivFUNC)(GLenum target, GLenum pname, GLuint* params);
typedef void (APIENTRY *glGetTexParameterIivFUNC)(GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY *glGetTexParameterIuivFUNC)(GLenum target, GLenum pname, GLuint* params);
typedef void (APIENTRY *glFramebufferTextureLayerFUNC)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void (APIENTRY *glBindBufferRangeFUNC)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRY *glBindBufferOffsetFUNC)(GLenum target, GLuint index, GLuint buffer, GLintptr offset);
typedef void (APIENTRY *glBindBufferBaseFUNC)(GLenum target, GLuint index, GLuint buffer);
typedef void (APIENTRY *glBeginTransformFeedbackFUNC)(GLenum primitiveMode);
typedef void (APIENTRY *glEndTransformFeedbackFUNC)();
typedef void (APIENTRY *glTransformFeedbackVaryingsFUNC)(GLuint program, GLsizei count, const char** varyings, GLenum bufferMode);
typedef void (APIENTRY *glGetTransformFeedbackVaryingFUNC)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
typedef void (APIENTRY *glClearBufferivFUNC)(GLenum buffer, GLint const * value);
typedef void (APIENTRY *glClearBufferfvFUNC)(GLenum buffer, GLfloat const * value);
typedef void (APIENTRY *glClearBufferuivFUNC)(GLenum buffer, GLuint const * value);
typedef void (APIENTRY *glClearBufferfiFUNC)(GLenum buffer, GLfloat depth, GLint stencil);

extern glMapBufferRangeFUNC glMapBufferRange;
extern glFlushMappedBufferRangeFUNC glFlushMappedBufferRange;
extern glVertexAttribI1iFUNC glVertexAttribI1i;
extern glVertexAttribI2iFUNC glVertexAttribI2i;
extern glVertexAttribI3iFUNC glVertexAttribI3i;
extern glVertexAttribI4iFUNC glVertexAttribI4i;
extern glVertexAttribI1uiFUNC glVertexAttribI1ui;
extern glVertexAttribI2uiFUNC glVertexAttribI2ui;
extern glVertexAttribI3uiFUNC glVertexAttribI3ui;
extern glVertexAttribI4uiFUNC glVertexAttribI4ui;
extern glVertexAttribI1ivFUNC glVertexAttribI1iv;
extern glVertexAttribI2ivFUNC glVertexAttribI2iv;
extern glVertexAttribI3ivFUNC glVertexAttribI3iv;
extern glVertexAttribI4ivFUNC glVertexAttribI4iv;
extern glVertexAttribI1uivFUNC glVertexAttribI1uiv;
extern glVertexAttribI2uivFUNC glVertexAttribI2uiv;
extern glVertexAttribI3uivFUNC glVertexAttribI3uiv;
extern glVertexAttribI4uivFUNC glVertexAttribI4uiv;
extern glVertexAttribI4bvFUNC glVertexAttribI4bv;
extern glVertexAttribI4svFUNC glVertexAttribI4sv;
extern glVertexAttribI4ubvFUNC glVertexAttribI4ubv;
extern glVertexAttribI4usvFUNC glVertexAttribI4usv;
extern glVertexAttribIPointerFUNC glVertexAttribIPointer;
extern glGetVertexAttribIivFUNC glGetVertexAttribIiv;
extern glGetVertexAttribIuivFUNC glGetVertexAttribIuiv;
extern glUniform1uiFUNC glUniform1ui;
extern glUniform2uiFUNC glUniform2ui;
extern glUniform3uiFUNC glUniform3ui;
extern glUniform4uiFUNC glUniform4ui;
extern glUniform1uivFUNC glUniform1uiv;
extern glUniform2uivFUNC glUniform2uiv;
extern glUniform3uivFUNC glUniform3uiv;
extern glUniform4uivFUNC glUniform4uiv;
extern glGetUniformuivFUNC glGetUniformuiv;
extern glBindFragDataLocationFUNC glBindFragDataLocation;
extern glGetFragDataLocationFUNC glGetFragDataLocation;
extern glBeginConditionalRenderFUNC glBeginConditionalRender;
extern glEndConditionalRenderFUNC glEndConditionalRender;
extern glClampColorFUNC glClampColor;
extern glRenderbufferStorageMultisampleFUNC glRenderbufferStorageMultisample;
extern glBlitFramebufferFUNC glBlitFramebuffer;
extern glClearColorIiFUNC glClearColorIi;
extern glClearColorIuiFUNC glClearColorIui;
extern glTexParameterIivFUNC glTexParameterIiv;
extern glTexParameterIuivFUNC glTexParameterIuiv;
extern glGetTexParameterIivFUNC glGetTexParameterIiv;
extern glGetTexParameterIuivFUNC glGetTexParameterIuiv;
extern glFramebufferTextureLayerFUNC glFramebufferTextureLayer;
extern glBindBufferRangeFUNC glBindBufferRange;
extern glBindBufferOffsetFUNC glBindBufferOffset;
extern glBindBufferBaseFUNC glBindBufferBase;
extern glBeginTransformFeedbackFUNC glBeginTransformFeedback;
extern glEndTransformFeedbackFUNC glEndTransformFeedback;
extern glTransformFeedbackVaryingsFUNC glTransformFeedbackVaryings;
extern glGetTransformFeedbackVaryingFUNC glGetTransformFeedbackVarying;
extern glClearBufferivFUNC glClearBufferiv;
extern glClearBufferfvFUNC glClearBufferfv;
extern glClearBufferuivFUNC glClearBufferuiv;
extern glClearBufferfiFUNC glClearBufferfi;

#endif		/* GL_VERSION_3_0 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL30_H */
