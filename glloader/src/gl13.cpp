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

#include <glloader/glloader.h>
#include <glloader/gl13.h>
#include <glloader/gl12.h>
#include "utils.hpp"

#ifdef GLLOADER_GL

using glloader::load_funcs;
using glloader::gl_features_extractor;

namespace
{
	bool _GL_VERSION_1_3 = false;

	char APIENTRY _glloader_GL_VERSION_1_3()
	{
		return _GL_VERSION_1_3;
	}

	void init_GL_VERSION_1_3()
	{
		glloader_GL_VERSION_1_3 = _glloader_GL_VERSION_1_3;

		{
			glActiveTexture = NULL;
			glClientActiveTexture = NULL;
			glMultiTexCoord1d = NULL;
			glMultiTexCoord1dv = NULL;
			glMultiTexCoord1f = NULL;
			glMultiTexCoord1fv = NULL;
			glMultiTexCoord1i = NULL;
			glMultiTexCoord1iv = NULL;
			glMultiTexCoord1s = NULL;
			glMultiTexCoord1sv = NULL;
			glMultiTexCoord2d = NULL;
			glMultiTexCoord2dv = NULL;
			glMultiTexCoord2f = NULL;
			glMultiTexCoord2fv = NULL;
			glMultiTexCoord2i = NULL;
			glMultiTexCoord2iv = NULL;
			glMultiTexCoord2s = NULL;
			glMultiTexCoord2sv = NULL;
			glMultiTexCoord3d = NULL;
			glMultiTexCoord3dv = NULL;
			glMultiTexCoord3f = NULL;
			glMultiTexCoord3fv = NULL;
			glMultiTexCoord3i = NULL;
			glMultiTexCoord3iv = NULL;
			glMultiTexCoord3s = NULL;
			glMultiTexCoord3sv = NULL;
			glMultiTexCoord4d = NULL;
			glMultiTexCoord4dv = NULL;
			glMultiTexCoord4f = NULL;
			glMultiTexCoord4fv = NULL;
			glMultiTexCoord4i = NULL;
			glMultiTexCoord4iv = NULL;
			glMultiTexCoord4s = NULL;
			glMultiTexCoord4sv = NULL;
			glLoadTransposeMatrixf = NULL;
			glLoadTransposeMatrixd = NULL;
			glMultTransposeMatrixf = NULL;
			glMultTransposeMatrixd = NULL;
			glSampleCoverage = NULL;
			glCompressedTexImage3D = NULL;
			glCompressedTexImage2D = NULL;
			glCompressedTexImage1D = NULL;
			glCompressedTexSubImage3D = NULL;
			glCompressedTexSubImage2D = NULL;
			glCompressedTexSubImage1D = NULL;
			glGetCompressedTexImage = NULL;
		}

		if (glloader_is_supported("GL_VERSION_1_3"))
		{
			_GL_VERSION_1_3 = true;

			entries_t entries(46);
			{
				entries[0] = reinterpret_cast<void**>(&glActiveTexture);
				entries[1] = reinterpret_cast<void**>(&glClientActiveTexture);
				entries[2] = reinterpret_cast<void**>(&glMultiTexCoord1d);
				entries[3] = reinterpret_cast<void**>(&glMultiTexCoord1dv);
				entries[4] = reinterpret_cast<void**>(&glMultiTexCoord1f);
				entries[5] = reinterpret_cast<void**>(&glMultiTexCoord1fv);
				entries[6] = reinterpret_cast<void**>(&glMultiTexCoord1i);
				entries[7] = reinterpret_cast<void**>(&glMultiTexCoord1iv);
				entries[8] = reinterpret_cast<void**>(&glMultiTexCoord1s);
				entries[9] = reinterpret_cast<void**>(&glMultiTexCoord1sv);
				entries[10] = reinterpret_cast<void**>(&glMultiTexCoord2d);
				entries[11] = reinterpret_cast<void**>(&glMultiTexCoord2dv);
				entries[12] = reinterpret_cast<void**>(&glMultiTexCoord2f);
				entries[13] = reinterpret_cast<void**>(&glMultiTexCoord2fv);
				entries[14] = reinterpret_cast<void**>(&glMultiTexCoord2i);
				entries[15] = reinterpret_cast<void**>(&glMultiTexCoord2iv);
				entries[16] = reinterpret_cast<void**>(&glMultiTexCoord2s);
				entries[17] = reinterpret_cast<void**>(&glMultiTexCoord2sv);
				entries[18] = reinterpret_cast<void**>(&glMultiTexCoord3d);
				entries[19] = reinterpret_cast<void**>(&glMultiTexCoord3dv);
				entries[20] = reinterpret_cast<void**>(&glMultiTexCoord3f);
				entries[21] = reinterpret_cast<void**>(&glMultiTexCoord3fv);
				entries[22] = reinterpret_cast<void**>(&glMultiTexCoord3i);
				entries[23] = reinterpret_cast<void**>(&glMultiTexCoord3iv);
				entries[24] = reinterpret_cast<void**>(&glMultiTexCoord3s);
				entries[25] = reinterpret_cast<void**>(&glMultiTexCoord3sv);
				entries[26] = reinterpret_cast<void**>(&glMultiTexCoord4d);
				entries[27] = reinterpret_cast<void**>(&glMultiTexCoord4dv);
				entries[28] = reinterpret_cast<void**>(&glMultiTexCoord4f);
				entries[29] = reinterpret_cast<void**>(&glMultiTexCoord4fv);
				entries[30] = reinterpret_cast<void**>(&glMultiTexCoord4i);
				entries[31] = reinterpret_cast<void**>(&glMultiTexCoord4iv);
				entries[32] = reinterpret_cast<void**>(&glMultiTexCoord4s);
				entries[33] = reinterpret_cast<void**>(&glMultiTexCoord4sv);
				entries[34] = reinterpret_cast<void**>(&glLoadTransposeMatrixf);
				entries[35] = reinterpret_cast<void**>(&glLoadTransposeMatrixd);
				entries[36] = reinterpret_cast<void**>(&glMultTransposeMatrixf);
				entries[37] = reinterpret_cast<void**>(&glMultTransposeMatrixd);
				entries[38] = reinterpret_cast<void**>(&glSampleCoverage);
				entries[39] = reinterpret_cast<void**>(&glCompressedTexImage3D);
				entries[40] = reinterpret_cast<void**>(&glCompressedTexImage2D);
				entries[41] = reinterpret_cast<void**>(&glCompressedTexImage1D);
				entries[42] = reinterpret_cast<void**>(&glCompressedTexSubImage3D);
				entries[43] = reinterpret_cast<void**>(&glCompressedTexSubImage2D);
				entries[44] = reinterpret_cast<void**>(&glCompressedTexSubImage1D);
				entries[45] = reinterpret_cast<void**>(&glGetCompressedTexImage);
			}

			funcs_names_t names(46);
			{
				names[0] = "glActiveTexture";
				names[1] = "glClientActiveTexture";
				names[2] = "glMultiTexCoord1d";
				names[3] = "glMultiTexCoord1dv";
				names[4] = "glMultiTexCoord1f";
				names[5] = "glMultiTexCoord1fv";
				names[6] = "glMultiTexCoord1i";
				names[7] = "glMultiTexCoord1iv";
				names[8] = "glMultiTexCoord1s";
				names[9] = "glMultiTexCoord1sv";
				names[10] = "glMultiTexCoord2d";
				names[11] = "glMultiTexCoord2dv";
				names[12] = "glMultiTexCoord2f";
				names[13] = "glMultiTexCoord2fv";
				names[14] = "glMultiTexCoord2i";
				names[15] = "glMultiTexCoord2iv";
				names[16] = "glMultiTexCoord2s";
				names[17] = "glMultiTexCoord2sv";
				names[18] = "glMultiTexCoord3d";
				names[19] = "glMultiTexCoord3dv";
				names[20] = "glMultiTexCoord3f";
				names[21] = "glMultiTexCoord3fv";
				names[22] = "glMultiTexCoord3i";
				names[23] = "glMultiTexCoord3iv";
				names[24] = "glMultiTexCoord3s";
				names[25] = "glMultiTexCoord3sv";
				names[26] = "glMultiTexCoord4d";
				names[27] = "glMultiTexCoord4dv";
				names[28] = "glMultiTexCoord4f";
				names[29] = "glMultiTexCoord4fv";
				names[30] = "glMultiTexCoord4i";
				names[31] = "glMultiTexCoord4iv";
				names[32] = "glMultiTexCoord4s";
				names[33] = "glMultiTexCoord4sv";
				names[34] = "glLoadTransposeMatrixf";
				names[35] = "glLoadTransposeMatrixd";
				names[36] = "glMultTransposeMatrixf";
				names[37] = "glMultTransposeMatrixd";
				names[38] = "glSampleCoverage";
				names[39] = "glCompressedTexImage3D";
				names[40] = "glCompressedTexImage2D";
				names[41] = "glCompressedTexImage1D";
				names[42] = "glCompressedTexSubImage3D";
				names[43] = "glCompressedTexSubImage2D";
				names[44] = "glCompressedTexSubImage1D";
				names[45] = "glGetCompressedTexImage";
			}

			load_funcs(entries, names);
		}
		else
		{
			if (glloader_GL_ARB_texture_compression()
				&& glloader_GL_ARB_texture_cube_map()
				&& glloader_GL_ARB_multisample()
				&& glloader_GL_ARB_multitexture()
				&& glloader_GL_ARB_texture_env_add()
				&& glloader_GL_ARB_texture_env_combine()
				&& (glloader_GL_ARB_texture_env_dot3() ||  glloader_GL_EXT_texture_env_dot3())
				&& glloader_GL_ARB_texture_border_clamp()
				&& glloader_GL_ARB_transpose_matrix())
			{
				_GL_VERSION_1_3 = true;
				gl_features_extractor::instance().promote("GL_VERSION_1_3");

				glActiveTexture = glActiveTextureARB;
				glClientActiveTexture = glClientActiveTextureARB;
				glMultiTexCoord1d = glMultiTexCoord1dARB;
				glMultiTexCoord1dv = glMultiTexCoord1dvARB;
				glMultiTexCoord1f = glMultiTexCoord1fARB;
				glMultiTexCoord1fv = glMultiTexCoord1fvARB;
				glMultiTexCoord1i = glMultiTexCoord1iARB;
				glMultiTexCoord1iv = glMultiTexCoord1ivARB;
				glMultiTexCoord1s = glMultiTexCoord1sARB;
				glMultiTexCoord1sv = glMultiTexCoord1svARB;
				glMultiTexCoord2d = glMultiTexCoord2dARB;
				glMultiTexCoord2dv = glMultiTexCoord2dvARB;
				glMultiTexCoord2f = glMultiTexCoord2fARB;
				glMultiTexCoord2fv = glMultiTexCoord2fvARB;
				glMultiTexCoord2i = glMultiTexCoord2iARB;
				glMultiTexCoord2iv = glMultiTexCoord2ivARB;
				glMultiTexCoord2s = glMultiTexCoord2sARB;
				glMultiTexCoord2sv = glMultiTexCoord2svARB;
				glMultiTexCoord3d = glMultiTexCoord3dARB;
				glMultiTexCoord3dv = glMultiTexCoord3dvARB;
				glMultiTexCoord3f = glMultiTexCoord3fARB;
				glMultiTexCoord3fv = glMultiTexCoord3fvARB;
				glMultiTexCoord3i = glMultiTexCoord3iARB;
				glMultiTexCoord3iv = glMultiTexCoord3ivARB;
				glMultiTexCoord3s = glMultiTexCoord3sARB;
				glMultiTexCoord3sv = glMultiTexCoord3svARB;
				glMultiTexCoord4d = glMultiTexCoord4dARB;
				glMultiTexCoord4dv = glMultiTexCoord4dvARB;
				glMultiTexCoord4f = glMultiTexCoord4fARB;
				glMultiTexCoord4fv = glMultiTexCoord4fvARB;
				glMultiTexCoord4i = glMultiTexCoord4iARB;
				glMultiTexCoord4iv = glMultiTexCoord4ivARB;
				glMultiTexCoord4s = glMultiTexCoord4sARB;
				glMultiTexCoord4sv = glMultiTexCoord4svARB;
				glLoadTransposeMatrixf = glLoadTransposeMatrixfARB;
				glLoadTransposeMatrixd = glLoadTransposeMatrixdARB;
				glMultTransposeMatrixf = glMultTransposeMatrixfARB;
				glMultTransposeMatrixd = glMultTransposeMatrixdARB;
				glSampleCoverage = glSampleCoverageARB;
				glCompressedTexImage3D = glCompressedTexImage3DARB;
				glCompressedTexImage2D = glCompressedTexImage2DARB;
				glCompressedTexImage1D = glCompressedTexImage1DARB;
				glCompressedTexSubImage3D = glCompressedTexSubImage3DARB;
				glCompressedTexSubImage2D = glCompressedTexSubImage2DARB;
				glCompressedTexSubImage1D = glCompressedTexSubImage1DARB;
				glGetCompressedTexImage = glGetCompressedTexImageARB;
			}
		}
	}

	char APIENTRY self_init_glloader_GL_VERSION_1_3()
	{
		glloader_GL_VERSION_1_2();

		init_GL_VERSION_1_3();
		return glloader_GL_VERSION_1_3();
	}
}

glloader_GL_VERSION_1_3FUNC glloader_GL_VERSION_1_3 = self_init_glloader_GL_VERSION_1_3;

#ifdef GL_VERSION_1_3

namespace
{
	void APIENTRY self_init_glActiveTexture(GLenum texture)
	{
		init_GL_VERSION_1_3();
		return glActiveTexture(texture);
	}
	void APIENTRY self_init_glClientActiveTexture(GLenum texture)
	{
		init_GL_VERSION_1_3();
		return glClientActiveTexture(texture);
	}
	void APIENTRY self_init_glMultiTexCoord1d(GLenum target, GLdouble s)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord1d(target, s);
	}
	void APIENTRY self_init_glMultiTexCoord1dv(GLenum target, const GLdouble* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord1dv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord1f(GLenum target, GLfloat s)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord1f(target, s);
	}
	void APIENTRY self_init_glMultiTexCoord1fv(GLenum target, const GLfloat* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord1fv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord1i(GLenum target, GLint s)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord1i(target, s);
	}
	void APIENTRY self_init_glMultiTexCoord1iv(GLenum target, const GLint* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord1iv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord1s(GLenum target, GLshort s)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord1s(target, s);
	}
	void APIENTRY self_init_glMultiTexCoord1sv(GLenum target, const GLshort* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord1sv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord2d(target, s, t);
	}
	void APIENTRY self_init_glMultiTexCoord2dv(GLenum target, const GLdouble* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord2dv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord2f(target, s, t);
	}
	void APIENTRY self_init_glMultiTexCoord2fv(GLenum target, const GLfloat* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord2fv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord2i(GLenum target, GLint s, GLint t)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord2i(target, s, t);
	}
	void APIENTRY self_init_glMultiTexCoord2iv(GLenum target, const GLint* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord2iv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord2s(GLenum target, GLshort s, GLshort t)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord2s(target, s, t);
	}
	void APIENTRY self_init_glMultiTexCoord2sv(GLenum target, const GLshort* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord2sv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord3d(target, s, t, r);
	}
	void APIENTRY self_init_glMultiTexCoord3dv(GLenum target, const GLdouble* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord3dv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord3f(target, s, t, r);
	}
	void APIENTRY self_init_glMultiTexCoord3fv(GLenum target, const GLfloat* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord3fv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord3i(target, s, t, r);
	}
	void APIENTRY self_init_glMultiTexCoord3iv(GLenum target, const GLint* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord3iv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord3s(target, s, t, r);
	}
	void APIENTRY self_init_glMultiTexCoord3sv(GLenum target, const GLshort* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord3sv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord4d(target, s, t, r, q);
	}
	void APIENTRY self_init_glMultiTexCoord4dv(GLenum target, const GLdouble* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord4dv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord4f(target, s, t, r, q);
	}
	void APIENTRY self_init_glMultiTexCoord4fv(GLenum target, const GLfloat* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord4fv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord4i(target, s, t, r, q);
	}
	void APIENTRY self_init_glMultiTexCoord4iv(GLenum target, const GLint* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord4iv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord4s(target, s, t, r, q);
	}
	void APIENTRY self_init_glMultiTexCoord4sv(GLenum target, const GLshort* v)
	{
		init_GL_VERSION_1_3();
		return glMultiTexCoord4sv(target, v);
	}
	void APIENTRY self_init_glLoadTransposeMatrixf(const GLfloat* m)
	{
		init_GL_VERSION_1_3();
		return glLoadTransposeMatrixf(m);
	}
	void APIENTRY self_init_glLoadTransposeMatrixd(const GLdouble* m)
	{
		init_GL_VERSION_1_3();
		return glLoadTransposeMatrixd(m);
	}
	void APIENTRY self_init_glMultTransposeMatrixf(const GLfloat* m)
	{
		init_GL_VERSION_1_3();
		return glMultTransposeMatrixf(m);
	}
	void APIENTRY self_init_glMultTransposeMatrixd(const GLdouble* m)
	{
		init_GL_VERSION_1_3();
		return glMultTransposeMatrixd(m);
	}
	void APIENTRY self_init_glSampleCoverage(GLclampf value, GLboolean invert)
	{
		init_GL_VERSION_1_3();
		return glSampleCoverage(value, invert);
	}
	void APIENTRY self_init_glCompressedTexImage3D(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
	{
		init_GL_VERSION_1_3();
		return glCompressedTexImage3D(target, level, internalFormat, width, height, depth, border, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
	{
		init_GL_VERSION_1_3();
		return glCompressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data)
	{
		init_GL_VERSION_1_3();
		return glCompressedTexImage1D(target, level, internalFormat, width, border, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
	{
		init_GL_VERSION_1_3();
		return glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
	{
		init_GL_VERSION_1_3();
		return glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data)
	{
		init_GL_VERSION_1_3();
		return glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
	}
	void APIENTRY self_init_glGetCompressedTexImage(GLenum target, GLint lod, GLvoid* img)
	{
		init_GL_VERSION_1_3();
		return glGetCompressedTexImage(target, lod, img);
	}
}

glActiveTextureFUNC glActiveTexture = self_init_glActiveTexture;
glClientActiveTextureFUNC glClientActiveTexture = self_init_glClientActiveTexture;
glMultiTexCoord1dFUNC glMultiTexCoord1d = self_init_glMultiTexCoord1d;
glMultiTexCoord1dvFUNC glMultiTexCoord1dv = self_init_glMultiTexCoord1dv;
glMultiTexCoord1fFUNC glMultiTexCoord1f = self_init_glMultiTexCoord1f;
glMultiTexCoord1fvFUNC glMultiTexCoord1fv = self_init_glMultiTexCoord1fv;
glMultiTexCoord1iFUNC glMultiTexCoord1i = self_init_glMultiTexCoord1i;
glMultiTexCoord1ivFUNC glMultiTexCoord1iv = self_init_glMultiTexCoord1iv;
glMultiTexCoord1sFUNC glMultiTexCoord1s = self_init_glMultiTexCoord1s;
glMultiTexCoord1svFUNC glMultiTexCoord1sv = self_init_glMultiTexCoord1sv;
glMultiTexCoord2dFUNC glMultiTexCoord2d = self_init_glMultiTexCoord2d;
glMultiTexCoord2dvFUNC glMultiTexCoord2dv = self_init_glMultiTexCoord2dv;
glMultiTexCoord2fFUNC glMultiTexCoord2f = self_init_glMultiTexCoord2f;
glMultiTexCoord2fvFUNC glMultiTexCoord2fv = self_init_glMultiTexCoord2fv;
glMultiTexCoord2iFUNC glMultiTexCoord2i = self_init_glMultiTexCoord2i;
glMultiTexCoord2ivFUNC glMultiTexCoord2iv = self_init_glMultiTexCoord2iv;
glMultiTexCoord2sFUNC glMultiTexCoord2s = self_init_glMultiTexCoord2s;
glMultiTexCoord2svFUNC glMultiTexCoord2sv = self_init_glMultiTexCoord2sv;
glMultiTexCoord3dFUNC glMultiTexCoord3d = self_init_glMultiTexCoord3d;
glMultiTexCoord3dvFUNC glMultiTexCoord3dv = self_init_glMultiTexCoord3dv;
glMultiTexCoord3fFUNC glMultiTexCoord3f = self_init_glMultiTexCoord3f;
glMultiTexCoord3fvFUNC glMultiTexCoord3fv = self_init_glMultiTexCoord3fv;
glMultiTexCoord3iFUNC glMultiTexCoord3i = self_init_glMultiTexCoord3i;
glMultiTexCoord3ivFUNC glMultiTexCoord3iv = self_init_glMultiTexCoord3iv;
glMultiTexCoord3sFUNC glMultiTexCoord3s = self_init_glMultiTexCoord3s;
glMultiTexCoord3svFUNC glMultiTexCoord3sv = self_init_glMultiTexCoord3sv;
glMultiTexCoord4dFUNC glMultiTexCoord4d = self_init_glMultiTexCoord4d;
glMultiTexCoord4dvFUNC glMultiTexCoord4dv = self_init_glMultiTexCoord4dv;
glMultiTexCoord4fFUNC glMultiTexCoord4f = self_init_glMultiTexCoord4f;
glMultiTexCoord4fvFUNC glMultiTexCoord4fv = self_init_glMultiTexCoord4fv;
glMultiTexCoord4iFUNC glMultiTexCoord4i = self_init_glMultiTexCoord4i;
glMultiTexCoord4ivFUNC glMultiTexCoord4iv = self_init_glMultiTexCoord4iv;
glMultiTexCoord4sFUNC glMultiTexCoord4s = self_init_glMultiTexCoord4s;
glMultiTexCoord4svFUNC glMultiTexCoord4sv = self_init_glMultiTexCoord4sv;
glLoadTransposeMatrixfFUNC glLoadTransposeMatrixf = self_init_glLoadTransposeMatrixf;
glLoadTransposeMatrixdFUNC glLoadTransposeMatrixd = self_init_glLoadTransposeMatrixd;
glMultTransposeMatrixfFUNC glMultTransposeMatrixf = self_init_glMultTransposeMatrixf;
glMultTransposeMatrixdFUNC glMultTransposeMatrixd = self_init_glMultTransposeMatrixd;
glSampleCoverageFUNC glSampleCoverage = self_init_glSampleCoverage;
glCompressedTexImage3DFUNC glCompressedTexImage3D = self_init_glCompressedTexImage3D;
glCompressedTexImage2DFUNC glCompressedTexImage2D = self_init_glCompressedTexImage2D;
glCompressedTexImage1DFUNC glCompressedTexImage1D = self_init_glCompressedTexImage1D;
glCompressedTexSubImage3DFUNC glCompressedTexSubImage3D = self_init_glCompressedTexSubImage3D;
glCompressedTexSubImage2DFUNC glCompressedTexSubImage2D = self_init_glCompressedTexSubImage2D;
glCompressedTexSubImage1DFUNC glCompressedTexSubImage1D = self_init_glCompressedTexSubImage1D;
glGetCompressedTexImageFUNC glGetCompressedTexImage = self_init_glGetCompressedTexImage;

#endif		// GL_VERSION_1_3

#endif			// GLLOADER_GL
