/**
 * @file glloader.h
 * @author  Minmin Gong
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * @section DESCRIPTION
 * The glloader, a subproject of Klay Game Engine, is an OpenGL extension
 * loading library. It supports OpenGL core 1.0 to 4.2, OpenGL ES core 1.0
 * to 2.0, as well as WGL, GLX, and other GL extensions. There is a automatic
 * code generater. All the things you want to do is to write a xml script
 * if you have to support new extensions. 
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

#ifdef GLLOADER_GLES_SUPPORT
#include <KHR/khrplatform.h>
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define GLLOADER_DEBUG
#endif

#ifndef GLLOADER_GLES_SUPPORT
#define GLLOADER_GL

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
	#define GLLOADER_WGL
#endif
#if defined(__APPLE__) || defined(__APPLE_CC__)
	#define GLLOADER_AGL
#endif
#if defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__CYGWIN__)
	#define GLLOADER_GLX
#endif

#else
#define GLLOADER_GLES
#define GLLOADER_EGL
#endif

#ifndef GLLOADER_GLES
#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#elif (defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__))
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef XID GLXDrawable;
typedef XID GLXContextID;
#endif
#else

#if defined(_WIN32) || defined(__VC32__) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
typedef HDC     EGLNativeDisplayType;
typedef HBITMAP EGLNativePixmapType;
typedef HWND    EGLNativeWindowType;

#elif defined(__WINSCW__) || defined(__SYMBIAN32__)

typedef int   EGLNativeDisplayType;
typedef void* EGLNativeWindowType;
typedef void* EGLNativePixmapType;

#elif defined(__ANDROID__) || defined(ANDROID)
#include <android/native_window.h>
struct egl_native_pixmap_t;

typedef struct ANativeWindow*           EGLNativeWindowType;
typedef struct egl_native_pixmap_t*     EGLNativePixmapType;
typedef void*                           EGLNativeDisplayType;

#elif (defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__))

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef XID GLXDrawable;
typedef XID GLXContextID;

typedef Display* EGLNativeDisplayType;
typedef Pixmap   EGLNativePixmapType;
typedef Window   EGLNativeWindowType;
#endif

#endif

#ifdef GLLOADER_EGL
typedef EGLNativeDisplayType NativeDisplayType;
typedef EGLNativePixmapType  NativePixmapType;
typedef EGLNativeWindowType  NativeWindowType;
#endif

#if defined(__gl_h_) || defined(__GL_H_)
#error glloader.h should be included before gl.h
#endif
#if defined(__gl2_h_) || defined(__GL2_H_)
#error glloader.h should be included before gl2.h
#endif
#if defined(__gl3_h_) || defined(__GL3_H_)
#error glloader.h should be included before gl3.h
#endif
#if defined(__gl31_h_) || defined(__GL31_H_)
#error glloader.h should be included before gl31.h
#endif
#if defined(__glcorearb_h_)
#error glloader.h should be included before glcorearb.h
#endif
#if defined(__glext_h_) || defined(__GLEXT_H_)
#error glloader.h should be included before glext.h
#endif
#if defined(__glxext_h_) || defined(__GLXEXT_H_)
#error glloader.h should be included before glxext.h
#endif
#if defined(__wglext_h_) || defined(__WGLEXT_H_)
#error glloader.h should be included before wglext.h
#endif
#if defined(__gl2ext_h_) || defined(__GL2EXT_H_)
#error glloader.h should be included before gl2ext.h
#endif
#if defined(__gl_ATI_h_)
#error glloader.h should be included before glATIext.h
#endif
#if defined(__gl2atiext_h_)
#error glloader.h should be included before gl2amdext.h
#endif
#if defined(__egl_h_) || defined(__EGL_H_)
#error glloader.h should be included before egl.h
#endif

/* GLAPI, part 1 (use WINGDIAPI, if defined) */
#if defined(_WIN32) && defined(WINGDIAPI)
#define GLAPI WINGDIAPI
#endif

#ifndef GLLOADER_APIENTRY
#ifdef KHRONOS_APIENTRY
#define GLLOADER_APIENTRY KHRONOS_APIENTRY
#else
#if defined(__CYGWIN__) || defined(__MINGW32__)
#define GLLOADER_APIENTRY __stdcall
#elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define GLLOADER_APIENTRY __stdcall
#else
#define GLLOADER_APIENTRY
#endif
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

#if defined(GLLOADER_GL) || defined(GLLOADER_GLES)
#define __gl_h_
#define __GL_H__
#define __gl2_h_
#define __GL2_H_
#define __gl3_h_
#define __GL3_H_
#define __gl31_h_
#define __GL31_H_
#define __glcorearb_h_
#define __glext_h_
#define __GLEXT_H_
#define __gl2ext_h_
#define __GL2EXT_h_
#define __gl_ATI_h_
#define __gl2atiext_h_
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

#ifdef GLLOADER_EGL
#define __egl_h_
#define __EGL_H_
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
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
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

/**
 * Initiate GLLoader
 */
GLLOADER_API void glloader_init();

/**
 * Find out if a particular feature is available on your platform, including the core and the extensions.
 *
 * @param name The name string of a feature.
 *       Note: You can use a name string "GL_VERSION_x_y" to determine if the x.y core version is supported. 
 *
 * @return Non-zero if a feature is supported.
 */
GLLOADER_API int glloader_is_supported(const char* name);

/**
 * Load an OpenGL function.
 *
 * @param name The function name.
 *
 * @return The address of the extension function. When the function fails, there is NOT guarantee that the return value is NULL.
 */
GLLOADER_API void* glloader_get_gl_proc_address(const char* name);

/**
 * Get the number of supported features.
 *
 * @return The number of supported features, including the core and the extensions.
 */
GLLOADER_API int glloader_num_features();

/**
 * Get the name of a feature.
 *
 * @param index The index of a feature. It's between [0, glloader_num_features() - 1].
 *
 * @return The name of a feature.
 */
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
#include <glloader/glloader_glx.h>
#endif

#ifdef GLLOADER_GLES
#include <glloader/glloader_gles.h>
#endif

#ifdef GLLOADER_EGL
#include <glloader/glloader_egl.h>
#endif

#endif		/* _GLLOADER_H */
