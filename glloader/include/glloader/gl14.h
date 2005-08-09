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

#ifndef _GL14_H
#define _GL14_H

#ifdef __cplusplus
extern "C"
{
#endif

/* OpenGL 1.4 */

typedef char (APIENTRY *glloader_GL_VERSION_1_4FUNC)();
extern glloader_GL_VERSION_1_4FUNC glloader_GL_VERSION_1_4;

#ifndef GL_VERSION_1_4
#define GL_VERSION_1_4 1

#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_POINT_SIZE_MIN 0x8126
#define GL_POINT_SIZE_MAX 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#define GL_GENERATE_MIPMAP 0x8191
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_MIRRORED_REPEAT 0x8370
#define GL_FOG_COORD_SRC 0x8450
#define GL_FOG_COORD 0x8451
#define GL_FRAGMENT_DEPTH 0x8452
#define GL_CURRENT_FOG_COORD 0x8453
#define GL_FOG_COORD_ARRAY_TYPE 0x8454
#define GL_FOG_COORD_ARRAY_STRIDE 0x8455
#define GL_FOG_COORD_ARRAY_POINTER 0x8456
#define GL_FOG_COORD_ARRAY 0x8457
#define GL_COLOR_SUM 0x8458
#define GL_CURRENT_SECONDARY_COLOR 0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#define GL_SECONDARY_COLOR_ARRAY 0x845E
#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#define GL_TEXTURE_FILTER_CONTROL 0x8500
#define GL_TEXTURE_LOD_BIAS 0x8501
#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_DEPTH_TEXTURE_MODE 0x884B
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_COMPARE_R_TO_TEXTURE 0x884E

typedef void (APIENTRY *glBlendFuncSeparateFUNC)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (APIENTRY *glFogCoordfFUNC)(GLfloat coord);
typedef void (APIENTRY *glFogCoordfvFUNC)(const GLfloat* coord);
typedef void (APIENTRY *glFogCoorddFUNC)(GLdouble coord);
typedef void (APIENTRY *glFogCoorddvFUNC)(const GLdouble* coord);
typedef void (APIENTRY *glFogCoordPointerFUNC)(GLenum type, GLsizei stride, const GLvoid* pointer);
typedef void (APIENTRY *glMultiDrawArraysFUNC)(GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);
typedef void (APIENTRY *glMultiDrawElementsFUNC)(GLenum mode, GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount);
typedef void (APIENTRY *glPointParameterfFUNC)(GLenum pname, GLfloat param);
typedef void (APIENTRY *glPointParameterfvFUNC)(GLenum pname, const GLfloat* params);
typedef void (APIENTRY *glPointParameteriFUNC)(GLenum pname, GLint param);
typedef void (APIENTRY *glPointParameterivFUNC)(GLenum pname, const GLint* params);
typedef void (APIENTRY *glSecondaryColor3bFUNC)(GLbyte red, GLbyte green, GLbyte blue);
typedef void (APIENTRY *glSecondaryColor3bvFUNC)(const GLbyte* v);
typedef void (APIENTRY *glSecondaryColor3dFUNC)(GLdouble red, GLdouble green, GLdouble blue);
typedef void (APIENTRY *glSecondaryColor3dvFUNC)(const GLdouble* v);
typedef void (APIENTRY *glSecondaryColor3fFUNC)(GLfloat red, GLfloat green, GLfloat blue);
typedef void (APIENTRY *glSecondaryColor3fvFUNC)(const GLfloat* v);
typedef void (APIENTRY *glSecondaryColor3iFUNC)(GLint red, GLint green, GLint blue);
typedef void (APIENTRY *glSecondaryColor3ivFUNC)(const GLint* v);
typedef void (APIENTRY *glSecondaryColor3sFUNC)(GLshort red, GLshort green, GLshort blue);
typedef void (APIENTRY *glSecondaryColor3svFUNC)(const GLshort* v);
typedef void (APIENTRY *glSecondaryColor3ubFUNC)(GLubyte red, GLubyte green, GLubyte blue);
typedef void (APIENTRY *glSecondaryColor3ubvFUNC)(const GLubyte* v);
typedef void (APIENTRY *glSecondaryColor3uiFUNC)(GLuint red, GLuint green, GLuint blue);
typedef void (APIENTRY *glSecondaryColor3uivFUNC)(const GLuint* v);
typedef void (APIENTRY *glSecondaryColor3usFUNC)(GLushort red, GLushort green, GLushort blue);
typedef void (APIENTRY *glSecondaryColor3usvFUNC)(const GLushort* v);
typedef void (APIENTRY *glSecondaryColorPointerFUNC)(GLint size, GLenum type, GLsizei stride, GLvoid* pointer);
typedef void (APIENTRY *glWindowPos2dFUNC)(GLdouble x, GLdouble y);
typedef void (APIENTRY *glWindowPos2dvFUNC)(const GLdouble* p);
typedef void (APIENTRY *glWindowPos2fFUNC)(GLfloat x, GLfloat y);
typedef void (APIENTRY *glWindowPos2fvFUNC)(const GLfloat* p);
typedef void (APIENTRY *glWindowPos2iFUNC)(GLint x, GLint y);
typedef void (APIENTRY *glWindowPos2ivFUNC)(const GLint* p);
typedef void (APIENTRY *glWindowPos2sFUNC)(GLshort x, GLshort y);
typedef void (APIENTRY *glWindowPos2svFUNC)(const GLshort* p);
typedef void (APIENTRY *glWindowPos3dFUNC)(GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY *glWindowPos3dvFUNC)(const GLdouble* p);
typedef void (APIENTRY *glWindowPos3fFUNC)(GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY *glWindowPos3fvFUNC)(const GLfloat* p);
typedef void (APIENTRY *glWindowPos3iFUNC)(GLint x, GLint y, GLint z);
typedef void (APIENTRY *glWindowPos3ivFUNC)(const GLint* p);
typedef void (APIENTRY *glWindowPos3sFUNC)(GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY *glWindowPos3svFUNC)(const GLshort* p);

extern glBlendFuncSeparateFUNC glBlendFuncSeparate;
extern glFogCoordfFUNC glFogCoordf;
extern glFogCoordfvFUNC glFogCoordfv;
extern glFogCoorddFUNC glFogCoordd;
extern glFogCoorddvFUNC glFogCoorddv;
extern glFogCoordPointerFUNC glFogCoordPointer;
extern glMultiDrawArraysFUNC glMultiDrawArrays;
extern glMultiDrawElementsFUNC glMultiDrawElements;
extern glPointParameterfFUNC glPointParameterf;
extern glPointParameterfvFUNC glPointParameterfv;
extern glPointParameteriFUNC glPointParameteri;
extern glPointParameterivFUNC glPointParameteriv;
extern glSecondaryColor3bFUNC glSecondaryColor3b;
extern glSecondaryColor3bvFUNC glSecondaryColor3bv;
extern glSecondaryColor3dFUNC glSecondaryColor3d;
extern glSecondaryColor3dvFUNC glSecondaryColor3dv;
extern glSecondaryColor3fFUNC glSecondaryColor3f;
extern glSecondaryColor3fvFUNC glSecondaryColor3fv;
extern glSecondaryColor3iFUNC glSecondaryColor3i;
extern glSecondaryColor3ivFUNC glSecondaryColor3iv;
extern glSecondaryColor3sFUNC glSecondaryColor3s;
extern glSecondaryColor3svFUNC glSecondaryColor3sv;
extern glSecondaryColor3ubFUNC glSecondaryColor3ub;
extern glSecondaryColor3ubvFUNC glSecondaryColor3ubv;
extern glSecondaryColor3uiFUNC glSecondaryColor3ui;
extern glSecondaryColor3uivFUNC glSecondaryColor3uiv;
extern glSecondaryColor3usFUNC glSecondaryColor3us;
extern glSecondaryColor3usvFUNC glSecondaryColor3usv;
extern glSecondaryColorPointerFUNC glSecondaryColorPointer;
extern glWindowPos2dFUNC glWindowPos2d;
extern glWindowPos2dvFUNC glWindowPos2dv;
extern glWindowPos2fFUNC glWindowPos2f;
extern glWindowPos2fvFUNC glWindowPos2fv;
extern glWindowPos2iFUNC glWindowPos2i;
extern glWindowPos2ivFUNC glWindowPos2iv;
extern glWindowPos2sFUNC glWindowPos2s;
extern glWindowPos2svFUNC glWindowPos2sv;
extern glWindowPos3dFUNC glWindowPos3d;
extern glWindowPos3dvFUNC glWindowPos3dv;
extern glWindowPos3fFUNC glWindowPos3f;
extern glWindowPos3fvFUNC glWindowPos3fv;
extern glWindowPos3iFUNC glWindowPos3i;
extern glWindowPos3ivFUNC glWindowPos3iv;
extern glWindowPos3sFUNC glWindowPos3s;
extern glWindowPos3svFUNC glWindowPos3sv;

#endif		/* GL_VERSION_1_4 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL14_H */
