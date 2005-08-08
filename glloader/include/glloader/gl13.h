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

#ifndef _GL13_H
#define _GL13_H

#ifdef __cplusplus
extern "C"
{
#endif

/* OpenGL 1.3 */

typedef char (APIENTRY *glloader_GL_VERSION_1_3FUNC)();
extern glloader_GL_VERSION_1_3FUNC glloader_GL_VERSION_1_3;

#ifndef GL_VERSION_1_3
#define GL_VERSION_1_3 1

#define GL_TEXTURE0 						0x84C0
#define GL_TEXTURE1 						0x84C1
#define GL_TEXTURE2 						0x84C2
#define GL_TEXTURE3 						0x84C3
#define GL_TEXTURE4 						0x84C4
#define GL_TEXTURE5 						0x84C5
#define GL_TEXTURE6 						0x84C6
#define GL_TEXTURE7 						0x84C7
#define GL_TEXTURE8 						0x84C8
#define GL_TEXTURE9 						0x84C9
#define GL_TEXTURE10 						0x84CA
#define GL_TEXTURE11 						0x84CB
#define GL_TEXTURE12 						0x84CC
#define GL_TEXTURE13 						0x84CD
#define GL_TEXTURE14 						0x84CE
#define GL_TEXTURE15 						0x84CF
#define GL_TEXTURE16 						0x84D0
#define GL_TEXTURE17 						0x84D1
#define GL_TEXTURE18 						0x84D2
#define GL_TEXTURE19 						0x84D3
#define GL_TEXTURE20 						0x84D4
#define GL_TEXTURE21 						0x84D5
#define GL_TEXTURE22 						0x84D6
#define GL_TEXTURE23 						0x84D7
#define GL_TEXTURE24 						0x84D8
#define GL_TEXTURE25 						0x84D9
#define GL_TEXTURE26 						0x84DA
#define GL_TEXTURE27 						0x84DB
#define GL_TEXTURE28 						0x84DC
#define GL_TEXTURE29 						0x84DD
#define GL_TEXTURE30 						0x84DE
#define GL_TEXTURE31 						0x84DF
#define GL_ACTIVE_TEXTURE					0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE			0x84E1
#define GL_MAX_TEXTURE_UNITS				0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX		0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX		0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX			0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX			0x84E6
#define GL_MULTISAMPLE						0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE			0x809E
#define GL_SAMPLE_ALPHA_TO_ONE				0x809F
#define GL_SAMPLE_COVERAGE					0x80A0
#define GL_SAMPLE_BUFFERS					0x80A8
#define GL_SAMPLES							0x80A9
#define GL_SAMPLE_COVERAGE_VALUE			0x80AA
#define GL_SAMPLE_COVERAGE_INVERT			0x80AB
#define GL_MULTISAMPLE_BIT					0x20000000
#define GL_NORMAL_MAP						0x8511
#define GL_REFLECTION_MAP					0x8512
#define GL_TEXTURE_CUBE_MAP					0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP			0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X		0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X		0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y		0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y		0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z		0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z		0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP			0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE		0x851C
#define GL_COMPRESSED_ALPHA					0x84E9
#define GL_COMPRESSED_LUMINANCE				0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA		0x84EB
#define GL_COMPRESSED_INTENSITY				0x84EC
#define GL_COMPRESSED_RGB					0x84ED
#define GL_COMPRESSED_RGBA					0x84EE
#define GL_TEXTURE_COMPRESSION_HINT			0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE	0x86A0
#define GL_TEXTURE_COMPRESSED				0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS	0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS		0x86A3
#define GL_CLAMP_TO_BORDER					0x812D
#define GL_COMBINE							0x8570
#define GL_COMBINE_RGB						0x8571
#define GL_COMBINE_ALPHA					0x8572
#define GL_SRC0_RGB							0x8580
#define GL_SRC1_RGB							0x8581
#define GL_SRC2_RGB							0x8582
#define GL_SRC0_ALPHA						0x8588
#define GL_SRC1_ALPHA						0x8589
#define GL_SRC2_ALPHA						0x858A
#define GL_OPERAND0_RGB						0x8590
#define GL_OPERAND1_RGB						0x8591
#define GL_OPERAND2_RGB						0x8592
#define GL_OPERAND0_ALPHA					0x8598
#define GL_OPERAND1_ALPHA					0x8599
#define GL_OPERAND2_ALPHA					0x859A
#define GL_RGB_SCALE						0x8573
#define GL_ADD_SIGNED						0x8574
#define GL_INTERPOLATE						0x8575
#define GL_SUBTRACT							0x84E7
#define GL_CONSTANT							0x8576
#define GL_PRIMARY_COLOR					0x8577
#define GL_PREVIOUS							0x8578
#define GL_DOT3_RGB							0x86AE
#define GL_DOT3_RGBA						0x86AF

typedef void (APIENTRY *glActiveTextureFUNC)(GLenum texture);
typedef void (APIENTRY *glClientActiveTextureFUNC)(GLenum texture);
typedef void (APIENTRY *glMultiTexCoord1dFUNC)(GLenum target, GLdouble s);
typedef void (APIENTRY *glMultiTexCoord1dvFUNC)(GLenum target, const GLdouble* v);
typedef void (APIENTRY *glMultiTexCoord1fFUNC)(GLenum target, GLfloat s);
typedef void (APIENTRY *glMultiTexCoord1fvFUNC)(GLenum target, const GLfloat* v);
typedef void (APIENTRY *glMultiTexCoord1iFUNC)(GLenum target, GLint s);
typedef void (APIENTRY *glMultiTexCoord1ivFUNC)(GLenum target, const GLint* v);
typedef void (APIENTRY *glMultiTexCoord1sFUNC)(GLenum target, GLshort s);
typedef void (APIENTRY *glMultiTexCoord1svFUNC)(GLenum target, const GLshort* v);
typedef void (APIENTRY *glMultiTexCoord2dFUNC)(GLenum target, GLdouble s, GLdouble t);
typedef void (APIENTRY *glMultiTexCoord2dvFUNC)(GLenum target, const GLdouble* v);
typedef void (APIENTRY *glMultiTexCoord2fFUNC)(GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY *glMultiTexCoord2fvFUNC)(GLenum target, const GLfloat* v);
typedef void (APIENTRY *glMultiTexCoord2iFUNC)(GLenum target, GLint s, GLint t);
typedef void (APIENTRY *glMultiTexCoord2ivFUNC)(GLenum target, const GLint* v);
typedef void (APIENTRY *glMultiTexCoord2sFUNC)(GLenum target, GLshort s, GLshort t);
typedef void (APIENTRY *glMultiTexCoord2svFUNC)(GLenum target, const GLshort* v);
typedef void (APIENTRY *glMultiTexCoord3dFUNC)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (APIENTRY *glMultiTexCoord3dvFUNC)(GLenum target, const GLdouble* v);
typedef void (APIENTRY *glMultiTexCoord3fFUNC)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (APIENTRY *glMultiTexCoord3fvFUNC)(GLenum target, const GLfloat* v);
typedef void (APIENTRY *glMultiTexCoord3iFUNC)(GLenum target, GLint s, GLint t, GLint r);
typedef void (APIENTRY *glMultiTexCoord3ivFUNC)(GLenum target, const GLint* v);
typedef void (APIENTRY *glMultiTexCoord3sFUNC)(GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (APIENTRY *glMultiTexCoord3svFUNC)(GLenum target, const GLshort* v);
typedef void (APIENTRY *glMultiTexCoord4dFUNC)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (APIENTRY *glMultiTexCoord4dvFUNC)(GLenum target, const GLdouble* v);
typedef void (APIENTRY *glMultiTexCoord4fFUNC)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (APIENTRY *glMultiTexCoord4fvFUNC)(GLenum target, const GLfloat* v);
typedef void (APIENTRY *glMultiTexCoord4iFUNC)(GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (APIENTRY *glMultiTexCoord4ivFUNC)(GLenum target, const GLint* v);
typedef void (APIENTRY *glMultiTexCoord4sFUNC)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (APIENTRY *glMultiTexCoord4svFUNC)(GLenum target, const GLshort* v);
typedef void (APIENTRY *glLoadTransposeMatrixfFUNC)(const GLfloat* m);
typedef void (APIENTRY *glLoadTransposeMatrixdFUNC)(const GLdouble* m);
typedef void (APIENTRY *glMultTransposeMatrixfFUNC)(const GLfloat* m);
typedef void (APIENTRY *glMultTransposeMatrixdFUNC)(const GLdouble* m);
typedef void (APIENTRY *glSampleCoverageFUNC)(GLclampf value, GLboolean invert);
typedef void (APIENTRY *glCompressedTexImage3DFUNC)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void (APIENTRY *glCompressedTexImage2DFUNC)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void (APIENTRY *glCompressedTexImage1DFUNC)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void (APIENTRY *glCompressedTexSubImage3DFUNC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void (APIENTRY *glCompressedTexSubImage2DFUNC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void (APIENTRY *glCompressedTexSubImage1DFUNC)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void (APIENTRY *glGetCompressedTexImageFUNC)(GLenum target, GLint lod, GLvoid* img);

extern glActiveTextureFUNC glActiveTexture;
extern glClientActiveTextureFUNC glClientActiveTexture;
extern glMultiTexCoord1dFUNC glMultiTexCoord1d;
extern glMultiTexCoord1dvFUNC glMultiTexCoord1dv;
extern glMultiTexCoord1fFUNC glMultiTexCoord1f;
extern glMultiTexCoord1fvFUNC glMultiTexCoord1fv;
extern glMultiTexCoord1iFUNC glMultiTexCoord1i;
extern glMultiTexCoord1ivFUNC glMultiTexCoord1iv;
extern glMultiTexCoord1sFUNC glMultiTexCoord1s;
extern glMultiTexCoord1svFUNC glMultiTexCoord1sv;
extern glMultiTexCoord2dFUNC glMultiTexCoord2d;
extern glMultiTexCoord2dvFUNC glMultiTexCoord2dv;
extern glMultiTexCoord2fFUNC glMultiTexCoord2f;
extern glMultiTexCoord2fvFUNC glMultiTexCoord2fv;
extern glMultiTexCoord2iFUNC glMultiTexCoord2i;
extern glMultiTexCoord2ivFUNC glMultiTexCoord2iv;
extern glMultiTexCoord2sFUNC glMultiTexCoord2s;
extern glMultiTexCoord2svFUNC glMultiTexCoord2sv;
extern glMultiTexCoord3dFUNC glMultiTexCoord3d;
extern glMultiTexCoord3dvFUNC glMultiTexCoord3dv;
extern glMultiTexCoord3fFUNC glMultiTexCoord3f;
extern glMultiTexCoord3fvFUNC glMultiTexCoord3fv;
extern glMultiTexCoord3iFUNC glMultiTexCoord3i;
extern glMultiTexCoord3ivFUNC glMultiTexCoord3iv;
extern glMultiTexCoord3sFUNC glMultiTexCoord3s;
extern glMultiTexCoord3svFUNC glMultiTexCoord3sv;
extern glMultiTexCoord4dFUNC glMultiTexCoord4d;
extern glMultiTexCoord4dvFUNC glMultiTexCoord4dv;
extern glMultiTexCoord4fFUNC glMultiTexCoord4f;
extern glMultiTexCoord4fvFUNC glMultiTexCoord4fv;
extern glMultiTexCoord4iFUNC glMultiTexCoord4i;
extern glMultiTexCoord4ivFUNC glMultiTexCoord4iv;
extern glMultiTexCoord4sFUNC glMultiTexCoord4s;
extern glMultiTexCoord4svFUNC glMultiTexCoord4sv;
extern glLoadTransposeMatrixfFUNC glLoadTransposeMatrixf;
extern glLoadTransposeMatrixdFUNC glLoadTransposeMatrixd;
extern glMultTransposeMatrixfFUNC glMultTransposeMatrixf;
extern glMultTransposeMatrixdFUNC glMultTransposeMatrixd;
extern glSampleCoverageFUNC glSampleCoverage;
extern glCompressedTexImage3DFUNC glCompressedTexImage3D;
extern glCompressedTexImage2DFUNC glCompressedTexImage2D;
extern glCompressedTexImage1DFUNC glCompressedTexImage1D;
extern glCompressedTexSubImage3DFUNC glCompressedTexSubImage3D;
extern glCompressedTexSubImage2DFUNC glCompressedTexSubImage2D;
extern glCompressedTexSubImage1DFUNC glCompressedTexSubImage1D;
extern glGetCompressedTexImageFUNC glGetCompressedTexImage;

#endif		/* GL_VERSION_1_3 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL13_H */
