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

#ifndef _GLX11_H
#define _GLX11_H

#ifdef __cplusplus
extern "C"
{
#endif

/* GLX 1.0 */

#ifndef GLX_VERSION_1_0
#define GLX_VERSION_1_0 1

typedef XID GLXDrawable;
typedef XID GLXPixmap;
typedef struct __GLXcontextRec* GLXContext;

#define GLX_USE_GL				1
#define GLX_BUFFER_SIZE			2
#define GLX_LEVEL				3
#define GLX_RGBA				4
#define GLX_DOUBLEBUFFER		5
#define GLX_STEREO				6
#define GLX_AUX_BUFFERS			7
#define GLX_RED_SIZE			8
#define GLX_GREEN_SIZE			9
#define GLX_BLUE_SIZE			10
#define GLX_ALPHA_SIZE			11
#define GLX_DEPTH_SIZE			12
#define GLX_STENCIL_SIZE		13
#define GLX_ACCUM_RED_SIZE		14
#define GLX_ACCUM_GREEN_SIZE	15
#define GLX_ACCUM_BLUE_SIZE		16
#define GLX_ACCUM_ALPHA_SIZE	17
#define GLX_BAD_SCREEN			1
#define GLX_BAD_ATTRIBUTE		2
#define GLX_NO_EXTENSION		3
#define GLX_BAD_VISUAL			4
#define GLX_BAD_CONTEXT			5
#define GLX_BAD_VALUE			6
#define GLX_BAD_ENUM			7

Bool glXQueryExtension(Display* dpy, int* errorBase, int* eventBase);
Bool glXQueryVersion(Display* dpy, int* major, int* minor);
int glXGetConfig(Display* dpy, XVisualInfo* vis, int attrib, int* value);
XVisualInfo* glXChooseVisual(Display* dpy, int screen, int* attribList);
GLXPixmap glXCreateGLXPixmap(Display* dpy, XVisualInfo* vis, Pixmap pixmap);
void glXDestroyGLXPixmap(Display* dpy, GLXPixmap pix);
GLXContext glXCreateContext(Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct);
void glXDestroyContext(Display* dpy, GLXContext ctx);
Bool glXIsDirect(Display* dpy, GLXContext ctx);
void glXCopyContext(Display* dpy, GLXContext src, GLXContext dst, GLuint mask);
Bool glXMakeCurrent(Display* dpy, GLXDrawable drawable, GLXContext ctx);
GLXContext glXGetCurrentContext(void);
GLXDrawable glXGetCurrentDrawable(void);
void glXWaitGL(void);
void glXWaitX(void);
void glXSwapBuffers(Display* dpy, GLXDrawable drawable);
void glXUseXFont(Font font, int first, int count, int listBase);

#endif		/* GLX_VERSION_1_0 */


/* GLX 1.1 */

#ifndef GLX_VERSION_1_1
#define GLX_VERSION_1_1 1

#define GLX_VENDOR			0x1
#define GLX_VERSION			0x2
#define GLX_EXTENSIONS		0x3

const char* glXQueryExtensionsString(Display* dpy, int screen);
const char* glXGetClientString(Display* dpy, int name);
const char* glXQueryServerString(Display* dpy, int screen, int name);

#endif		/* GLX_VERSION_1_1 */

#ifdef __cplusplus
}
#endif

#endif			/* _GLX11_H */
