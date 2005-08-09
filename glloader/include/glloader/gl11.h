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

#ifndef _GL11_H
#define _GL11_H

#ifdef __cplusplus
extern "C"
{
#endif

/* OpenGL 1.1 */

#ifndef GL_VERSION_1_1
#define GL_VERSION_1_1 1

#define GL_DOUBLE										0x140A
#define GL_VERTEX_ARRAY									0x8074
#define GL_NORMAL_ARRAY									0x8075
#define GL_COLOR_ARRAY									0x8076
#define GL_INDEX_ARRAY									0x8077
#define GL_TEXTURE_COORD_ARRAY							0x8078
#define GL_EDGE_FLAG_ARRAY								0x8079
#define GL_VERTEX_ARRAY_SIZE							0x807A
#define GL_VERTEX_ARRAY_TYPE							0x807B
#define GL_VERTEX_ARRAY_STRIDE							0x807C
#define GL_NORMAL_ARRAY_TYPE							0x807E
#define GL_NORMAL_ARRAY_STRIDE							0x807F
#define GL_COLOR_ARRAY_SIZE								0x8081
#define GL_COLOR_ARRAY_TYPE								0x8082
#define GL_COLOR_ARRAY_STRIDE							0x8083
#define GL_INDEX_ARRAY_TYPE								0x8085
#define GL_INDEX_ARRAY_STRIDE							0x8086
#define GL_TEXTURE_COORD_ARRAY_SIZE						0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE						0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE					0x808A
#define GL_EDGE_FLAG_ARRAY_STRIDE						0x808C
#define GL_VERTEX_ARRAY_POINTER							0x808E
#define GL_NORMAL_ARRAY_POINTER							0x808F
#define GL_COLOR_ARRAY_POINTER							0x8090
#define GL_INDEX_ARRAY_POINTER							0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER					0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER						0x8093
#define GL_V2F											0x2A20
#define GL_V3F											0x2A21
#define GL_C4UB_V2F										0x2A22
#define GL_C4UB_V3F										0x2A23
#define GL_C3F_V3F										0x2A24
#define GL_N3F_V3F										0x2A25
#define GL_C4F_N3F_V3F									0x2A26
#define GL_T2F_V3F										0x2A27
#define GL_T4F_V4F										0x2A28
#define GL_T2F_C4UB_V3F									0x2A29
#define GL_T2F_C3F_V3F									0x2A2A
#define GL_T2F_N3F_V3F									0x2A2B
#define GL_T2F_C4F_N3F_V3F								0x2A2C
#define GL_T4F_C4F_N3F_V4F								0x2A2D
#define GL_POLYGON_OFFSET								0x8037
#define GL_POLYGON_OFFSET_FACTOR						0x8038
#define GL_POLYGON_OFFSET_UNITS							0x2A00
#define GL_POLYGON_OFFSET_POINT							0x2A01
#define GL_POLYGON_OFFSET_LINE							0x2A02
#define GL_POLYGON_OFFSET_FILL							0x8037
#define GL_ALPHA4										0x803B
#define GL_ALPHA8										0x803C
#define GL_ALPHA12										0x803D
#define GL_ALPHA16										0x803E
#define GL_LUMINANCE4									0x803F
#define GL_LUMINANCE8									0x8040
#define GL_LUMINANCE12									0x8041
#define GL_LUMINANCE16									0x8042
#define GL_LUMINANCE4_ALPHA4							0x8043
#define GL_LUMINANCE6_ALPHA2							0x8044
#define GL_LUMINANCE8_ALPHA8							0x8045
#define GL_LUMINANCE12_ALPHA4							0x8046
#define GL_LUMINANCE12_ALPHA12							0x8047
#define GL_LUMINANCE16_ALPHA16							0x8048
#define GL_INTENSITY									0x8049
#define GL_INTENSITY4									0x804A
#define GL_INTENSITY8									0x804B
#define GL_INTENSITY12									0x804C
#define GL_INTENSITY16									0x804D
#define GL_R3_G3_B2										0x2A10
#define GL_RGB4											0x804F
#define GL_RGB5											0x8050
#define GL_RGB8											0x8051
#define GL_RGB10										0x8052
#define GL_RGB12										0x8053
#define GL_RGB16										0x8054
#define GL_RGBA2										0x8055
#define GL_RGBA4										0x8056
#define GL_RGB5_A1										0x8057
#define GL_RGBA8										0x8058
#define GL_RGB10_A2										0x8059
#define GL_RGBA12										0x805A
#define GL_RGBA16										0x805B
#define GL_TEXTURE_RED_SIZE								0x805C
#define GL_TEXTURE_GREEN_SIZE							0x805D
#define GL_TEXTURE_BLUE_SIZE							0x805E
#define GL_TEXTURE_ALPHA_SIZE							0x805F
#define GL_TEXTURE_LUMINANCE_SIZE						0x8060
#define GL_TEXTURE_INTENSITY_SIZE						0x8061
#define GL_PROXY_TEXTURE_1D								0x8063
#define GL_PROXY_TEXTURE_2D								0x8064
#define GL_TEXTURE_TOO_LARGE							0x8065
#define GL_TEXTURE_PRIORITY								0x8066
#define GL_TEXTURE_RESIDENT								0x8067
#define GL_TEXTURE_BINDING_1D							0x8068
#define GL_TEXTURE_BINDING_2D							0x8069
#define GL_CLIENT_PIXEL_STORE_BIT						0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT						0x00000002
#define GL_CLIENT_ALL_ATTRIB_BITS						0xFFFFFFFF


void APIENTRY glArrayElement(GLint i);
void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count);
void APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void APIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer);
void APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void APIENTRY glIndexPointer(GLenum type, GLsizei stride, const GLvoid* pointer);
void APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void APIENTRY glEdgeFlagPointer(GLsizei stride, const GLboolean* pointer);
void APIENTRY glGetPointerv(GLenum pname, GLvoid** params);
void APIENTRY glPolygonOffset(GLfloat factor, GLfloat units);
void APIENTRY glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels);
void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
void APIENTRY glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
void APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void APIENTRY glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void APIENTRY glGenTextures(GLsizei n, GLuint* textures);
void APIENTRY glDeleteTextures(GLsizei n, const GLuint* textures);
void APIENTRY glBindTexture(GLenum target, GLuint texture);
void APIENTRY glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLclampf* priorities);
GLboolean APIENTRY glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences);
GLboolean APIENTRY glIsTexture(GLuint texture);
void APIENTRY glEnableClientState(GLenum array);
void APIENTRY glDisableClientState(GLenum array);
void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
void APIENTRY glIndexub(GLubyte c);
void APIENTRY glIndexubv(const GLubyte* c);
void APIENTRY glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid* pointer);
void APIENTRY glPushClientAttrib(GLbitfield mask);
void APIENTRY glPopClientAttrib();

#endif		/* GL_VERSION_1_1 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL11_H */
