/*
// glloader
// Copyright (C) 2004-2009 Minmin Gong
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

/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2004 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: This software was created using the
** OpenGL(R) version 1.2.1 Sample Implementation published by SGI, but has
** not been independently verified as being compliant with the OpenGL(R)
** version 1.2.1 Specification.
*/

#ifndef _GLLOADER_H
#define _GLLOADER_H

#include <stddef.h>

#if defined(DEBUG) || defined(_DEBUG)
#define GLLOADER_DEBUG
#endif

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
	#define GLLOADER_WGL
#endif
#if defined(__APPLE__) || defined(__APPLE_CC__)
	#define GLLOADER_AGL
#endif
#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__CYGWIN__)
	#define GLLOADER_GLX
#endif

#define GLLOADER_GL

#ifdef GLLOADER_WGL
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

#if defined(__gl_h_) || defined(__GL_H_)
#error GLLoader.h should be included before gl.h
#endif
#if defined(__glext_h_) || defined(__GLEXT_H_)
#error GLLoader.h should be included before glext.h
#endif
#if defined(__glxext_h_) || defined(__GLXEXT_H_)
#error GLLoader.h should be included before glxext.h
#endif
#if defined(__wglext_h_) || defined(__WGLEXT_H_)
#error GLLoader.h should be included before wglext.h
#endif
#if defined(__gl_ATI_h_)
#error GLLoader.h should be included before glATIext.h
#endif

/* GLAPI, part 1 (use WINGDIAPI, if defined) */
#if defined(_WIN32) && defined(WINGDIAPI)
#define GLAPI WINGDIAPI
#endif

#ifndef APIENTRY
#if defined(__CYGWIN__) || defined(__MINGW32__)
#define APIENTRY __stdcall
#elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

#if defined(_MSC_VER)
	#define GLLOADER_HAS_DECLSPEC
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	#if !defined(__GNUC__) && !defined(GLLOADER_HAS_DECLSPEC)
		#define GLLOADER_HAS_DECLSPEC
	#endif

	#if defined(__MINGW32__)
		#define GLLOADER_HAS_DECLSPEC
	#endif
#endif

#ifdef GLLOADER_HAS_DECLSPEC
	#ifdef GLLOADER_SOURCE				// Build dll
		#define GLLOADER_API __declspec(dllexport)
	#else								// Use dll
		#define GLLOADER_API __declspec(dllimport)
	#endif
#else
	#define GLLOADER_API
#endif // GLLOADER_HAS_DECLSPEC

#ifdef GLLOADER_GL
#define __gl_h_
#define __GL_H__
#define __glext_h_
#define __GLEXT_H_
#define __gl_ATI_h_
#endif

#ifdef GLLOADER_GLX
#define __glx_h_
#define __GLX_H__
#define __glxext_h_
#define __GLXEXT_H_
#endif

#ifdef GLLOADER_WGL
#define __wglext_h_
#define __WGLEXT_H_
#endif

typedef void			GLvoid;
typedef unsigned char	GLboolean;
typedef signed char		GLbyte;
typedef unsigned char	GLubyte;
typedef char			GLchar;
typedef short			GLshort;
typedef unsigned short	GLushort;
typedef int				GLint;
typedef unsigned int	GLuint;
typedef int				GLsizei;
typedef unsigned int	GLenum;
typedef ptrdiff_t		GLintptr;
typedef ptrdiff_t		GLsizeiptr;
typedef unsigned int	GLbitfield;
typedef unsigned short	GLhalf;
typedef float			GLfloat;
typedef float			GLclampf;
typedef double			GLdouble;
typedef double			GLclampd;
typedef int				GLfixed;
typedef int				GLclampx;
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
	#include <inttypes.h>
	typedef int64_t		GLint64;
	typedef uint64_t	GLuint64;
#elif defined(__sun__)
	#include <inttypes.h>
	#if defined(__STDC__)
		#if defined(__arch64__)
			typedef long int				GLint64;
			typedef unsigned long int		GLuint64;
		#else
			typedef long long int			GLint64;
			typedef unsigned long long int	GLuint64;
		#endif /* __arch64__ */
	#else
		typedef int64_t		GLint64;
		typedef uint64_t	GLuint64;
	#endif /* __STDC__ */
#elif defined( __VMS )
	#include <inttypes.h>
	typedef int64_t		GLint64;
	typedef uint64_t	GLuint64;
#elif defined(__SCO__) || defined(__USLC__)
	#include <stdint.h>
	typedef int64_t		GLint64;
	typedef uint64_t	GLuint64;
#elif defined(__UNIXOS2__) || defined(__SOL64__)
	typedef long long int			GLint64;
	typedef unsigned long long int	GLuint64;
#elif defined(WIN32) && defined(__GNUC__)
	#include <stdint.h>
	typedef int64_t		GLint64;
	typedef uint64_t	GLuint64;
#elif defined(WIN32)
	typedef __int64				GLint64;
	typedef unsigned __int64	GLuint64;
#else
	#include <inttypes.h>	/* Fallback option */
	typedef int64_t		GLint64;
	typedef uint64_t	GLuint64;
#endif
typedef GLint64			GLtime;

#ifdef __cplusplus
extern "C"
{
#endif

/* Initiate GLLoader */
GLLOADER_API void glloader_init();

/* Check if a feature is supported, including the core and the extensions */
GLLOADER_API int glloader_is_supported(const char* name);

/* Get the address of OpenGL extension functions, given the function name */
GLLOADER_API void* glloader_get_gl_proc_address(const char* name);

/* Get the number of supported features, including the core and the extensions */
GLLOADER_API int glloader_num_features();

/* Get the name of a feature. The parameter 'index' is between [0, num_features - 1] */
GLLOADER_API const char* glloader_get_feature_name(int index);

#ifdef __cplusplus
}
#endif

#ifdef GLLOADER_GL
#include <glloader/glloader_gl.h>
#endif

#ifdef GLLOADER_WGL
#include <glloader/glloader_wgl.h>
#endif

#ifdef GLLOADER_GLX
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef XID GLXDrawable;
typedef XID GLXContextID;

#include <glloader/glloader_glx.h>
#endif

#endif		/* _GLLOADER_H */
