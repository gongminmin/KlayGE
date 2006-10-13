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

#ifndef _GL21_H
#define _GL21_H

#ifdef __cplusplus
extern "C"
{
#endif

/* OpenGL 2.1 */

typedef char (APIENTRY *glloader_GL_VERSION_2_1FUNC)();
extern glloader_GL_VERSION_2_1FUNC glloader_GL_VERSION_2_1;

#ifndef GL_VERSION_2_1
#define GL_VERSION_2_1 1

#define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#define GL_FLOAT_MAT2x3 0x8B65
#define GL_FLOAT_MAT2x4 0x8B66
#define GL_FLOAT_MAT3x2 0x8B67
#define GL_FLOAT_MAT3x4 0x8B68
#define GL_FLOAT_MAT4x2 0x8B69
#define GL_FLOAT_MAT4x3 0x8B6A
#define GL_SRGB 0x8C40
#define GL_SRGB8 0x8C41
#define GL_SRGB_ALPHA 0x8C42
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_SLUMINANCE_ALPHA 0x8C44
#define GL_SLUMINANCE8_ALPHA8 0x8C45
#define GL_SLUMINANCE 0x8C46
#define GL_SLUMINANCE8 0x8C47
#define GL_COMPRESSED_SRGB 0x8C48
#define GL_COMPRESSED_SRGB_ALPHA 0x8C49
#define GL_COMPRESSED_SLUMINANCE 0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA 0x8C4B

typedef void (APIENTRY *glUniformMatrix2x3fvFUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY *glUniformMatrix3x2fvFUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY *glUniformMatrix2x4fvFUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY *glUniformMatrix4x2fvFUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY *glUniformMatrix3x4fvFUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY *glUniformMatrix4x3fvFUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

extern glUniformMatrix2x3fvFUNC glUniformMatrix2x3fv;
extern glUniformMatrix3x2fvFUNC glUniformMatrix3x2fv;
extern glUniformMatrix2x4fvFUNC glUniformMatrix2x4fv;
extern glUniformMatrix4x2fvFUNC glUniformMatrix4x2fv;
extern glUniformMatrix3x4fvFUNC glUniformMatrix3x4fv;
extern glUniformMatrix4x3fvFUNC glUniformMatrix4x3fv;

#endif		/* GL_VERSION_2_1 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL21_H */
