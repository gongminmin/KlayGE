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

#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_DEPTH32F_STENCIL8 0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_HALF_FLOAT 0x140B
#define MAP_READ_BIT 0x0001
#define MAP_WRITE_BIT 0x0002
#define MAP_INVALIDATE_RANGE_BIT 0x0004
#define MAP_INVALIDATE_BUFFER_BIT 0x0008
#define MAP_FLUSH_EXPLICIT_BIT 0x0010
#define MAP_UNSYNCHRONIZED_BIT 0x0020
#define GL_COMPRESSED_RED_RGTC1 0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
#define GL_COMPRESSED_RED_GREEN_RGTC2 0x8DBD
#define GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2 0x8DBE
#define R8 0x8229
#define R16 0x822A
#define RG8 0x822B
#define RG16 0x822C
#define R16F 0x822D
#define R32F 0x822E
#define RG16F 0x822F
#define RG32F 0x8230
#define R8I 0x8231
#define R8UI 0x8232
#define R16I 0x8233
#define R16UI 0x8234
#define R32I 0x8235
#define R32UI 0x8236
#define RG8I 0x8237
#define RG8UI 0x8238
#define RG16I 0x8239
#define RG16UI 0x823A
#define RG32I 0x823B
#define RG32UI 0x823C


typedef void (APIENTRY *glMapBufferRangeFUNC)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (APIENTRY *glFlushMappedBufferRangeFUNC)(GLenum target, GLintptr offset, GLsizeiptr length);

extern glMapBufferRangeFUNC glMapBufferRange;
extern glFlushMappedBufferRangeFUNC glFlushMappedBufferRange;

#endif		/* GL_VERSION_3_0 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL30_H */
