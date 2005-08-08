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

#ifndef _GL12_H
#define _GL12_H

#ifdef __cplusplus
extern "C"
{
#endif

/* OpenGL 1.2 */

typedef char (APIENTRY *glloader_GL_VERSION_1_2FUNC)();
extern glloader_GL_VERSION_1_2FUNC glloader_GL_VERSION_1_2;

#ifndef GL_VERSION_1_2
#define GL_VERSION_1_2 1

#define GL_UNSIGNED_BYTE_3_3_2				0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4			0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1			0x8034
#define GL_UNSIGNED_INT_8_8_8_8				0x8035
#define GL_UNSIGNED_INT_10_10_10_2			0x8036
#define GL_RESCALE_NORMAL					0x803A
#define GL_TEXTURE_BINDING_3D				0x806A
#define GL_PACK_SKIP_IMAGES					0x806B
#define GL_PACK_IMAGE_HEIGHT				0x806C
#define GL_UNPACK_SKIP_IMAGES				0x806D
#define GL_UNPACK_IMAGE_HEIGHT				0x806E
#define GL_TEXTURE_3D						0x806F
#define GL_PROXY_TEXTURE_3D					0x8070
#define GL_TEXTURE_DEPTH					0x8071
#define GL_TEXTURE_WRAP_R					0x8072
#define GL_MAX_3D_TEXTURE_SIZE				0x8073
#define GL_UNSIGNED_BYTE_2_3_3_REV			0x8362
#define GL_UNSIGNED_SHORT_5_6_5				0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV			0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV		0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV		0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV			0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV		0x8368
#define GL_BGR								0x80E0
#define GL_BGRA								0x80E1
#define GL_MAX_ELEMENTS_VERTICES			0x80E8
#define GL_MAX_ELEMENTS_INDICES				0x80E9
#define GL_CLAMP_TO_EDGE					0x812F
#define GL_TEXTURE_MIN_LOD					0x813A
#define GL_TEXTURE_MAX_LOD					0x813B
#define GL_TEXTURE_BASE_LEVEL				0x813C
#define GL_TEXTURE_MAX_LEVEL				0x813D
#define GL_LIGHT_MODEL_COLOR_CONTROL		0x81F8
#define GL_SINGLE_COLOR						0x81F9
#define GL_SEPARATE_SPECULAR_COLOR			0x81FA
#define GL_SMOOTH_POINT_SIZE_RANGE			0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY	0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE			0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY	0x0B23
#define GL_ALIASED_POINT_SIZE_RANGE			0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE			0x846E

typedef void (APIENTRY *glBlendColorFUNC)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (APIENTRY *glBlendEquationFUNC)(GLenum mode);
typedef void (APIENTRY *glDrawRangeElementsFUNC)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
typedef void (APIENTRY *glColorTableFUNC)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid* table);
typedef void (APIENTRY *glColorTableParameterfvFUNC)(GLenum target, GLenum pname, const GLfloat* params);
typedef void (APIENTRY *glColorTableParameterivFUNC)(GLenum target, GLenum pname, const GLint* params);
typedef void (APIENTRY *glCopyColorTableFUNC)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY *glGetColorTableFUNC)(GLenum target, GLenum format, GLenum type, GLvoid* table);
typedef void (APIENTRY *glGetColorTableParameterfvFUNC)(GLenum target, GLenum pname, GLfloat* params);
typedef void (APIENTRY *glGetColorTableParameterivFUNC)(GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY *glColorSubTableFUNC)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid* data);
typedef void (APIENTRY *glCopyColorSubTableFUNC)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY *glConvolutionFilter1DFUNC)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid* image);
typedef void (APIENTRY *glConvolutionFilter2DFUNC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* image);
typedef void (APIENTRY *glConvolutionParameterfFUNC)(GLenum target, GLenum pname, GLfloat params);
typedef void (APIENTRY *glConvolutionParameterfvFUNC)(GLenum target, GLenum pname, const GLfloat* params);
typedef void (APIENTRY *glConvolutionParameteriFUNC)(GLenum target, GLenum pname, GLint params);
typedef void (APIENTRY *glConvolutionParameterivFUNC)(GLenum target, GLenum pname, const GLint* params);
typedef void (APIENTRY *glCopyConvolutionFilter1DFUNC)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY *glCopyConvolutionFilter2DFUNC)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY *glGetConvolutionFilterFUNC)(GLenum target, GLenum format, GLenum type, GLvoid* image);
typedef void (APIENTRY *glGetConvolutionParameterfvFUNC)(GLenum target, GLenum pname, GLfloat* params);
typedef void (APIENTRY *glGetConvolutionParameterivFUNC)(GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY *glGetSeparableFilterFUNC)(GLenum target, GLenum format, GLenum type, GLvoid* row, GLvoid* column, GLvoid* span);
typedef void (APIENTRY *glSeparableFilter2DFUNC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* row, const GLvoid* column);
typedef void (APIENTRY *glGetHistogramFUNC)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values);
typedef void (APIENTRY *glGetHistogramParameterfvFUNC)(GLenum target, GLenum pname, GLfloat* params);
typedef void (APIENTRY *glGetHistogramParameterivFUNC)(GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY *glGetMinmaxFUNC)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values);
typedef void (APIENTRY *glGetMinmaxParameterfvFUNC)(GLenum target, GLenum pname, GLfloat* params);
typedef void (APIENTRY *glGetMinmaxParameterivFUNC)(GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY *glHistogramFUNC)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (APIENTRY *glMinmaxFUNC)(GLenum target, GLenum internalformat, GLboolean sink);
typedef void (APIENTRY *glResetHistogramFUNC)(GLenum target);
typedef void (APIENTRY *glResetMinmaxFUNC)(GLenum target);
typedef void (APIENTRY *glTexImage3DFUNC)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (APIENTRY *glTexSubImage3DFUNC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (APIENTRY *glCopyTexSubImage3DFUNC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

extern glBlendColorFUNC glBlendColor;
extern glBlendEquationFUNC glBlendEquation;
extern glDrawRangeElementsFUNC glDrawRangeElements;
extern glColorTableFUNC glColorTable;
extern glColorTableParameterfvFUNC glColorTableParameterfv;
extern glColorTableParameterivFUNC glColorTableParameteriv;
extern glCopyColorTableFUNC glCopyColorTable;
extern glGetColorTableFUNC glGetColorTable;
extern glGetColorTableParameterfvFUNC glGetColorTableParameterfv;
extern glGetColorTableParameterivFUNC glGetColorTableParameteriv;
extern glColorSubTableFUNC glColorSubTable;
extern glCopyColorSubTableFUNC glCopyColorSubTable;
extern glConvolutionFilter1DFUNC glConvolutionFilter1D;
extern glConvolutionFilter2DFUNC glConvolutionFilter2D;
extern glConvolutionParameterfFUNC glConvolutionParameterf;
extern glConvolutionParameterfvFUNC glConvolutionParameterfv;
extern glConvolutionParameteriFUNC glConvolutionParameteri;
extern glConvolutionParameterivFUNC glConvolutionParameteriv;
extern glCopyConvolutionFilter1DFUNC glCopyConvolutionFilter1D;
extern glCopyConvolutionFilter2DFUNC glCopyConvolutionFilter2D;
extern glGetConvolutionFilterFUNC glGetConvolutionFilter;
extern glGetConvolutionParameterfvFUNC glGetConvolutionParameterfv;
extern glGetConvolutionParameterivFUNC glGetConvolutionParameteriv;
extern glGetSeparableFilterFUNC glGetSeparableFilter;
extern glSeparableFilter2DFUNC glSeparableFilter2D;
extern glGetHistogramFUNC glGetHistogram;
extern glGetHistogramParameterfvFUNC glGetHistogramParameterfv;
extern glGetHistogramParameterivFUNC glGetHistogramParameteriv;
extern glGetMinmaxFUNC glGetMinmax;
extern glGetMinmaxParameterfvFUNC glGetMinmaxParameterfv;
extern glGetMinmaxParameterivFUNC glGetMinmaxParameteriv;
extern glHistogramFUNC glHistogram;
extern glMinmaxFUNC glMinmax;
extern glResetHistogramFUNC glResetHistogram;
extern glResetMinmaxFUNC glResetMinmax;
extern glTexImage3DFUNC glTexImage3D;
extern glTexSubImage3DFUNC glTexSubImage3D;
extern glCopyTexSubImage3DFUNC glCopyTexSubImage3D;

#endif		/* GL_VERSION_1_2 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL12_H */
