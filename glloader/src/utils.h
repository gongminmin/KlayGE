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

#ifndef _UTILS_H
#define _UTILS_H

#define LOAD_FUNC1(f) f = (f##FUNC)(glloader_get_gl_proc_address(#f));
#define LOAD_FUNC2(f, name) f = (f##FUNC)(glloader_get_gl_proc_address(#name));

#ifdef __cplusplus
extern "C"
{
#endif

void promote_low_high(char const * low_name, char const * high_name);
void promote_high(char const * high_name);

void gl_init();
void wgl_init();
void glx_init();
void gles_init();
void egl_init();

#ifdef __cplusplus
}
#endif

#endif			// _UTILS_H
