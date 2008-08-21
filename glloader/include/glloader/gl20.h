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

#ifndef _GL20_H
#define _GL20_H

#ifdef __cplusplus
extern "C"
{
#endif

/* OpenGL 2.0 */

typedef char (APIENTRY *glloader_GL_VERSION_2_0FUNC)();
extern glloader_GL_VERSION_2_0FUNC glloader_GL_VERSION_2_0;

#ifndef GL_VERSION_2_0
#define GL_VERSION_2_0 1

#define GL_BLEND_EQUATION_RGB				GL_BLEND_EQUATION
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED		0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE			0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE		0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE			0x8625
#define GL_CURRENT_VERTEX_ATTRIB			0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE		0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE			0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER		0x8645
#define GL_STENCIL_BACK_FUNC				0x8800
#define GL_STENCIL_BACK_FAIL				0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL		0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS		0x8803
#define GL_MAX_DRAW_BUFFERS					0x8824
#define GL_DRAW_BUFFER0 					0x8825
#define GL_DRAW_BUFFER1 					0x8826
#define GL_DRAW_BUFFER2 					0x8827
#define GL_DRAW_BUFFER3 					0x8828
#define GL_DRAW_BUFFER4 					0x8829
#define GL_DRAW_BUFFER5 					0x882A
#define GL_DRAW_BUFFER6 					0x882B
#define GL_DRAW_BUFFER7 					0x882C
#define GL_DRAW_BUFFER8 					0x882D
#define GL_DRAW_BUFFER9 					0x882E
#define GL_DRAW_BUFFER10 					0x882F
#define GL_DRAW_BUFFER11 					0x8830
#define GL_DRAW_BUFFER12 					0x8831
#define GL_DRAW_BUFFER13 					0x8832
#define GL_DRAW_BUFFER14 					0x8833
#define GL_DRAW_BUFFER15 					0x8834
#define GL_BLEND_EQUATION_ALPHA				0x883D
#define GL_POINT_SPRITE						0x8861
#define GL_COORD_REPLACE					0x8862
#define GL_MAX_VERTEX_ATTRIBS				0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED	0x886A
#define GL_MAX_TEXTURE_COORDS				0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS			0x8872
#define GL_FRAGMENT_SHADER					0x8B30
#define GL_VERTEX_SHADER					0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS	0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS	0x8B4A
#define GL_MAX_VARYING_COMPONENTS			0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS	0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS	0x8B4D
#define GL_SHADER_TYPE						0x8B4F
#define GL_FLOAT_VEC2						0x8B50
#define GL_FLOAT_VEC3						0x8B51
#define GL_FLOAT_VEC4						0x8B52
#define GL_INT_VEC2							0x8B53
#define GL_INT_VEC3							0x8B54
#define GL_INT_VEC4							0x8B55
#define GL_BOOL								0x8B56
#define GL_BOOL_VEC2						0x8B57
#define GL_BOOL_VEC3						0x8B58
#define GL_BOOL_VEC4						0x8B59
#define GL_FLOAT_MAT2						0x8B5A
#define GL_FLOAT_MAT3						0x8B5B
#define GL_FLOAT_MAT4 						0x8B5C
#define GL_SAMPLER_1D 						0x8B5D
#define GL_SAMPLER_2D 						0x8B5E
#define GL_SAMPLER_3D 						0x8B5F
#define GL_SAMPLER_CUBE						0x8B60
#define GL_SAMPLER_1D_SHADOW				0x8B61
#define GL_SAMPLER_2D_SHADOW				0x8B62
#define GL_DELETE_STATUS					0x8B80
#define GL_COMPILE_STATUS					0x8B81
#define GL_LINK_STATUS						0x8B82
#define GL_VALIDATE_STATUS					0x8B83
#define GL_INFO_LOG_LENGTH					0x8B84
#define GL_ATTACHED_SHADERS					0x8B85
#define GL_ACTIVE_UNIFORMS					0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH		0x8B87
#define GL_SHADER_SOURCE_LENGTH				0x8B88
#define GL_ACTIVE_ATTRIBUTES				0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH		0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT	0x8B8B
#define GL_SHADING_LANGUAGE_VERSION			0x8B8C
#define GL_CURRENT_PROGRAM					0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN		0x8CA0
#define GL_LOWER_LEFT 						0x8CA1
#define GL_UPPER_LEFT 						0x8CA2
#define GL_STENCIL_BACK_REF					0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK			0x8CA4
#define GL_STENCIL_BACK_WRITEMASK			0x8CA5

typedef void (APIENTRY *glBlendEquationSeparateFUNC)(GLenum modeRGB, GLenum modeAlpha);
typedef void (APIENTRY *glDrawBuffersFUNC)(GLsizei n, const GLenum* bufs);
typedef void (APIENTRY *glStencilOpSeparateFUNC)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (APIENTRY *glStencilFuncSeparateFUNC)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
typedef void (APIENTRY *glStencilMaskSeparateFUNC)(GLenum face, GLuint mask);
typedef void (APIENTRY *glAttachShaderFUNC)(GLuint program, GLuint shader);
typedef void (APIENTRY *glBindAttribLocationFUNC)(GLuint program, GLuint index, const GLchar* name);
typedef void (APIENTRY *glCompileShaderFUNC)(GLuint shader);
typedef GLuint (APIENTRY *glCreateProgramFUNC)();
typedef GLuint (APIENTRY *glCreateShaderFUNC)(GLenum type);
typedef void (APIENTRY *glDeleteProgramFUNC)(GLuint program);
typedef void (APIENTRY *glDeleteShaderFUNC)(GLuint shader);
typedef void (APIENTRY *glDetachShaderFUNC)(GLuint program, GLuint shader);
typedef void (APIENTRY *glDisableVertexAttribArrayFUNC)(GLuint index);
typedef void (APIENTRY *glEnableVertexAttribArrayFUNC)(GLuint index);
typedef void (APIENTRY *glGetActiveAttribFUNC)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void (APIENTRY *glGetActiveUniformFUNC)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void (APIENTRY *glGetAttachedShadersFUNC)(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shader);
typedef GLint (APIENTRY *glGetAttribLocationFUNC)(GLuint program, const GLchar* name);
typedef void (APIENTRY *glGetProgramivFUNC)(GLuint program, GLenum pname, const GLint* params);
typedef void (APIENTRY *glGetProgramInfoLogFUNC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRY *glGetShaderivFUNC)(GLuint shader, GLenum pname, const GLint* params);
typedef void (APIENTRY *glGetShaderInfoLogFUNC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRY *glGetShaderSourceFUNC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source);
typedef GLint (APIENTRY *glGetUniformLocationFUNC)(GLuint program, const GLchar* name);
typedef void (APIENTRY *glGetUniformfvFUNC)(GLuint program, GLint location, GLfloat* params);
typedef void (APIENTRY *glGetUniformivFUNC)(GLuint program, GLint location, GLint* params);
typedef void (APIENTRY *glGetVertexAttribdvFUNC)(GLuint index, GLenum pname, GLdouble* params);
typedef void (APIENTRY *glGetVertexAttribfvFUNC)(GLuint index, GLenum pname, GLfloat* params);
typedef void (APIENTRY *glGetVertexAttribivFUNC)(GLuint index, GLenum pname, GLint* params);
typedef void (APIENTRY *glGetVertexAttribPointervFUNC)(GLuint index, GLenum pname, GLvoid** pointer);
typedef GLboolean (APIENTRY *glIsProgramFUNC)(GLuint program);
typedef GLboolean (APIENTRY *glIsShaderFUNC)(GLuint shader);
typedef void (APIENTRY *glLinkProgramFUNC)(GLuint program);
typedef void (APIENTRY *glShaderSourceFUNC)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
typedef void (APIENTRY *glUseProgramFUNC)(GLuint program);
typedef void (APIENTRY *glUniform1fFUNC)(GLint location, GLfloat v0);
typedef void (APIENTRY *glUniform2fFUNC)(GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRY *glUniform3fFUNC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY *glUniform4fFUNC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY *glUniform1iFUNC)(GLint location, GLint v0);
typedef void (APIENTRY *glUniform2iFUNC)(GLint location, GLint v0, GLint v1);
typedef void (APIENTRY *glUniform3iFUNC)(GLint location, GLint v0, GLint v1, GLint v2);
typedef void (APIENTRY *glUniform4iFUNC)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (APIENTRY *glUniform1fvFUNC)(GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRY *glUniform2fvFUNC)(GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRY *glUniform3fvFUNC)(GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRY *glUniform4fvFUNC)(GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRY *glUniform1ivFUNC)(GLint location, GLsizei count, const GLint* value);
typedef void (APIENTRY *glUniform2ivFUNC)(GLint location, GLsizei count, const GLint* value);
typedef void (APIENTRY *glUniform3ivFUNC)(GLint location, GLsizei count, const GLint* value);
typedef void (APIENTRY *glUniform4ivFUNC)(GLint location, GLsizei count, const GLint* value);
typedef void (APIENTRY *glUniformMatrix2fvFUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY *glUniformMatrix3fvFUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY *glUniformMatrix4fvFUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY *glValidateProgramFUNC)(GLuint program);
typedef void (APIENTRY *glVertexAttrib1dFUNC)(GLuint index, GLdouble x);
typedef void (APIENTRY *glVertexAttrib1dvFUNC)(GLuint index, const GLdouble* v);
typedef void (APIENTRY *glVertexAttrib1fFUNC)(GLuint index, GLfloat x);
typedef void (APIENTRY *glVertexAttrib1fvFUNC)(GLuint index, const GLfloat* v);
typedef void (APIENTRY *glVertexAttrib1sFUNC)(GLuint index, GLshort x);
typedef void (APIENTRY *glVertexAttrib1svFUNC)(GLuint index, const GLshort* v);
typedef void (APIENTRY *glVertexAttrib2dFUNC)(GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRY *glVertexAttrib2dvFUNC)(GLuint index, const GLdouble* v);
typedef void (APIENTRY *glVertexAttrib2fFUNC)(GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRY *glVertexAttrib2fvFUNC)(GLuint index, const GLfloat* v);
typedef void (APIENTRY *glVertexAttrib2sFUNC)(GLuint index, GLshort x, GLshort y);
typedef void (APIENTRY *glVertexAttrib2svFUNC)(GLuint index, const GLshort* v);
typedef void (APIENTRY *glVertexAttrib3dFUNC)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY *glVertexAttrib3dvFUNC)(GLuint index, const GLdouble* v);
typedef void (APIENTRY *glVertexAttrib3fFUNC)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY *glVertexAttrib3fvFUNC)(GLuint index, const GLfloat* v);
typedef void (APIENTRY *glVertexAttrib3sFUNC)(GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY *glVertexAttrib3svFUNC)(GLuint index, const GLshort* v);
typedef void (APIENTRY *glVertexAttrib4NbvFUNC)(GLuint index, const GLbyte* v);
typedef void (APIENTRY *glVertexAttrib4NivFUNC)(GLuint index, const GLint* v);
typedef void (APIENTRY *glVertexAttrib4NsvFUNC)(GLuint index, const GLshort* v);
typedef void (APIENTRY *glVertexAttrib4NubFUNC)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (APIENTRY *glVertexAttrib4NubvFUNC)(GLuint index, const GLubyte* v);
typedef void (APIENTRY *glVertexAttrib4NuivFUNC)(GLuint index, const GLuint* v);
typedef void (APIENTRY *glVertexAttrib4NusvFUNC)(GLuint index, const GLushort* v);
typedef void (APIENTRY *glVertexAttrib4bvFUNC)(GLuint index, const GLbyte* v);
typedef void (APIENTRY *glVertexAttrib4dFUNC)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY *glVertexAttrib4dvFUNC)(GLuint index, const GLdouble* v);
typedef void (APIENTRY *glVertexAttrib4fFUNC)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY *glVertexAttrib4fvFUNC)(GLuint index, const GLfloat* v);
typedef void (APIENTRY *glVertexAttrib4ivFUNC)(GLuint index, const GLint* v);
typedef void (APIENTRY *glVertexAttrib4sFUNC)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY *glVertexAttrib4svFUNC)(GLuint index, const GLshort* v);
typedef void (APIENTRY *glVertexAttrib4ubvFUNC)(GLuint index, const GLubyte* v);
typedef void (APIENTRY *glVertexAttrib4uivFUNC)(GLuint index, const GLuint* v);
typedef void (APIENTRY *glVertexAttrib4usvFUNC)(GLuint index, const GLushort* v);
typedef void (APIENTRY *glVertexAttribPointerFUNC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);

extern glBlendEquationSeparateFUNC glBlendEquationSeparate;
extern glDrawBuffersFUNC glDrawBuffers;
extern glStencilOpSeparateFUNC glStencilOpSeparate;
extern glStencilFuncSeparateFUNC glStencilFuncSeparate;
extern glStencilMaskSeparateFUNC glStencilMaskSeparate;
extern glAttachShaderFUNC glAttachShader;
extern glBindAttribLocationFUNC glBindAttribLocation;
extern glCompileShaderFUNC glCompileShader;
extern glCreateProgramFUNC glCreateProgram;
extern glCreateShaderFUNC glCreateShader;
extern glDeleteProgramFUNC glDeleteProgram;
extern glDeleteShaderFUNC glDeleteShader;
extern glDetachShaderFUNC glDetachShader;
extern glDisableVertexAttribArrayFUNC glDisableVertexAttribArray;
extern glEnableVertexAttribArrayFUNC glEnableVertexAttribArray;
extern glGetActiveAttribFUNC glGetActiveAttrib;
extern glGetActiveUniformFUNC glGetActiveUniform;
extern glGetAttachedShadersFUNC glGetAttachedShaders;
extern glGetAttribLocationFUNC glGetAttribLocation;
extern glGetProgramivFUNC glGetProgramiv;
extern glGetProgramInfoLogFUNC glGetProgramInfoLog;
extern glGetShaderivFUNC glGetShaderiv;
extern glGetShaderInfoLogFUNC glGetShaderInfoLog;
extern glGetShaderSourceFUNC glGetShaderSource;
extern glGetUniformLocationFUNC glGetUniformLocation;
extern glGetUniformfvFUNC glGetUniformfv;
extern glGetUniformivFUNC glGetUniformiv;
extern glGetVertexAttribdvFUNC glGetVertexAttribdv;
extern glGetVertexAttribfvFUNC glGetVertexAttribfv;
extern glGetVertexAttribivFUNC glGetVertexAttribiv;
extern glGetVertexAttribPointervFUNC glGetVertexAttribPointerv;
extern glIsProgramFUNC glIsProgram;
extern glIsShaderFUNC glIsShader;
extern glLinkProgramFUNC glLinkProgram;
extern glShaderSourceFUNC glShaderSource;
extern glUseProgramFUNC glUseProgram;
extern glUniform1fFUNC glUniform1f;
extern glUniform2fFUNC glUniform2f;
extern glUniform3fFUNC glUniform3f;
extern glUniform4fFUNC glUniform4f;
extern glUniform1iFUNC glUniform1i;
extern glUniform2iFUNC glUniform2i;
extern glUniform3iFUNC glUniform3i;
extern glUniform4iFUNC glUniform4i;
extern glUniform1fvFUNC glUniform1fv;
extern glUniform2fvFUNC glUniform2fv;
extern glUniform3fvFUNC glUniform3fv;
extern glUniform4fvFUNC glUniform4fv;
extern glUniform1ivFUNC glUniform1iv;
extern glUniform2ivFUNC glUniform2iv;
extern glUniform3ivFUNC glUniform3iv;
extern glUniform4ivFUNC glUniform4iv;
extern glUniformMatrix2fvFUNC glUniformMatrix2fv;
extern glUniformMatrix3fvFUNC glUniformMatrix3fv;
extern glUniformMatrix4fvFUNC glUniformMatrix4fv;
extern glValidateProgramFUNC glValidateProgram;
extern glVertexAttrib1dFUNC glVertexAttrib1d;
extern glVertexAttrib1dvFUNC glVertexAttrib1dv;
extern glVertexAttrib1fFUNC glVertexAttrib1f;
extern glVertexAttrib1fvFUNC glVertexAttrib1fv;
extern glVertexAttrib1sFUNC glVertexAttrib1s;
extern glVertexAttrib1svFUNC glVertexAttrib1sv;
extern glVertexAttrib2dFUNC glVertexAttrib2d;
extern glVertexAttrib2dvFUNC glVertexAttrib2dv;
extern glVertexAttrib2fFUNC glVertexAttrib2f;
extern glVertexAttrib2fvFUNC glVertexAttrib2fv;
extern glVertexAttrib2sFUNC glVertexAttrib2s;
extern glVertexAttrib2svFUNC glVertexAttrib2sv;
extern glVertexAttrib3dFUNC glVertexAttrib3d;
extern glVertexAttrib3dvFUNC glVertexAttrib3dv;
extern glVertexAttrib3fFUNC glVertexAttrib3f;
extern glVertexAttrib3fvFUNC glVertexAttrib3fv;
extern glVertexAttrib3sFUNC glVertexAttrib3s;
extern glVertexAttrib3svFUNC glVertexAttrib3sv;
extern glVertexAttrib4NbvFUNC glVertexAttrib4Nbv;
extern glVertexAttrib4NivFUNC glVertexAttrib4Niv;
extern glVertexAttrib4NsvFUNC glVertexAttrib4Nsv;
extern glVertexAttrib4NubFUNC glVertexAttrib4Nub;
extern glVertexAttrib4NubvFUNC glVertexAttrib4Nubv;
extern glVertexAttrib4NuivFUNC glVertexAttrib4Nuiv;
extern glVertexAttrib4NusvFUNC glVertexAttrib4Nusv;
extern glVertexAttrib4bvFUNC glVertexAttrib4bv;
extern glVertexAttrib4dFUNC glVertexAttrib4d;
extern glVertexAttrib4dvFUNC glVertexAttrib4dv;
extern glVertexAttrib4fFUNC glVertexAttrib4f;
extern glVertexAttrib4fvFUNC glVertexAttrib4fv;
extern glVertexAttrib4ivFUNC glVertexAttrib4iv;
extern glVertexAttrib4sFUNC glVertexAttrib4s;
extern glVertexAttrib4svFUNC glVertexAttrib4sv;
extern glVertexAttrib4ubvFUNC glVertexAttrib4ubv;
extern glVertexAttrib4uivFUNC glVertexAttrib4uiv;
extern glVertexAttrib4usvFUNC glVertexAttrib4usv;
extern glVertexAttribPointerFUNC glVertexAttribPointer;

#endif		/* GL_VERSION_2_0 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL20_H */
