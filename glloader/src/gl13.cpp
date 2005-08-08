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
#include "utils.hpp"

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

		entries_t entries;
		{
			entries.push_back(reinterpret_cast<void**>(&glActiveTexture));
			entries.push_back(reinterpret_cast<void**>(&glClientActiveTexture));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord1d));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord1dv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord1f));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord1fv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord1i));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord1iv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord1s));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord1sv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord2d));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord2dv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord2f));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord2fv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord2i));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord2iv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord2s));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord2sv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord3d));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord3dv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord3f));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord3fv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord3i));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord3iv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord3s));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord3sv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord4d));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord4dv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord4f));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord4fv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord4i));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord4iv));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord4s));
			entries.push_back(reinterpret_cast<void**>(&glMultiTexCoord4sv));
			entries.push_back(reinterpret_cast<void**>(&glLoadTransposeMatrixf));
			entries.push_back(reinterpret_cast<void**>(&glLoadTransposeMatrixd));
			entries.push_back(reinterpret_cast<void**>(&glMultTransposeMatrixf));
			entries.push_back(reinterpret_cast<void**>(&glMultTransposeMatrixd));
			entries.push_back(reinterpret_cast<void**>(&glSampleCoverage));
			entries.push_back(reinterpret_cast<void**>(&glCompressedTexImage3D));
			entries.push_back(reinterpret_cast<void**>(&glCompressedTexImage2D));
			entries.push_back(reinterpret_cast<void**>(&glCompressedTexImage1D));
			entries.push_back(reinterpret_cast<void**>(&glCompressedTexSubImage3D));
			entries.push_back(reinterpret_cast<void**>(&glCompressedTexSubImage2D));
			entries.push_back(reinterpret_cast<void**>(&glCompressedTexSubImage1D));
			entries.push_back(reinterpret_cast<void**>(&glGetCompressedTexImage));
		}

		if (glloader_is_supported("GL_VERSION_1_3"))
		{
			_GL_VERSION_1_3 = true;

			funcs_names_t names;

			names.push_back("glActiveTexture");
			names.push_back("glClientActiveTexture");
			names.push_back("glMultiTexCoord1d");
			names.push_back("glMultiTexCoord1dv");
			names.push_back("glMultiTexCoord1f");
			names.push_back("glMultiTexCoord1fv");
			names.push_back("glMultiTexCoord1i");
			names.push_back("glMultiTexCoord1iv");
			names.push_back("glMultiTexCoord1s");
			names.push_back("glMultiTexCoord1sv");
			names.push_back("glMultiTexCoord2d");
			names.push_back("glMultiTexCoord2dv");
			names.push_back("glMultiTexCoord2f");
			names.push_back("glMultiTexCoord2fv");
			names.push_back("glMultiTexCoord2i");
			names.push_back("glMultiTexCoord2iv");
			names.push_back("glMultiTexCoord2s");
			names.push_back("glMultiTexCoord2sv");
			names.push_back("glMultiTexCoord3d");
			names.push_back("glMultiTexCoord3dv");
			names.push_back("glMultiTexCoord3f");
			names.push_back("glMultiTexCoord3fv");
			names.push_back("glMultiTexCoord3i");
			names.push_back("glMultiTexCoord3iv");
			names.push_back("glMultiTexCoord3s");
			names.push_back("glMultiTexCoord3sv");
			names.push_back("glMultiTexCoord4d");
			names.push_back("glMultiTexCoord4dv");
			names.push_back("glMultiTexCoord4f");
			names.push_back("glMultiTexCoord4fv");
			names.push_back("glMultiTexCoord4i");
			names.push_back("glMultiTexCoord4iv");
			names.push_back("glMultiTexCoord4s");
			names.push_back("glMultiTexCoord4sv");
			names.push_back("glLoadTransposeMatrixf");
			names.push_back("glLoadTransposeMatrixd");
			names.push_back("glMultTransposeMatrixf");
			names.push_back("glMultTransposeMatrixd");
			names.push_back("glSampleCoverage");
			names.push_back("glCompressedTexImage3D");
			names.push_back("glCompressedTexImage2D");
			names.push_back("glCompressedTexImage1D");
			names.push_back("glCompressedTexSubImage3D");
			names.push_back("glCompressedTexSubImage2D");
			names.push_back("glCompressedTexSubImage1D");
			names.push_back("glGetCompressedTexImage");

			load_funcs(entries, names);

			return;
		}
		else
		{
			if (glloader_is_supported("GL_ARB_texture_compression")
				&& glloader_is_supported("GL_ARB_texture_cube_map")
				&& glloader_is_supported("GL_ARB_multisample")
				&& glloader_is_supported("GL_ARB_multitexture")
				&& (glloader_is_supported("GL_ARB_texture_env_add") || glloader_is_supported("GL_EXT_texture_env_add"))
				&& (glloader_is_supported("GL_ARB_texture_env_combine") ||  glloader_is_supported("GL_EXT_texture_env_combine"))
				&& (glloader_is_supported("GL_ARB_texture_env_dot3") ||  glloader_is_supported("GL_EXT_texture_env_dot3"))
				&& (glloader_is_supported("GL_ARB_texture_border_clamp") ||  glloader_is_supported("GL_SGIS_texture_border_clamp"))
				&& glloader_is_supported("GL_ARB_transpose_matrix"))
			{
				_GL_VERSION_1_3 = true;
				gl_features_extractor::instance().promote("GL_VERSION_1_3");

				funcs_names_t names;

				names.push_back("glActiveTextureARB");
				names.push_back("glClientActiveTextureARB");
				names.push_back("glMultiTexCoord1dARB");
				names.push_back("glMultiTexCoord1dvARB");
				names.push_back("glMultiTexCoord1fARB");
				names.push_back("glMultiTexCoord1fvARB");
				names.push_back("glMultiTexCoord1iARB");
				names.push_back("glMultiTexCoord1ivARB");
				names.push_back("glMultiTexCoord1sARB");
				names.push_back("glMultiTexCoord1svARB");
				names.push_back("glMultiTexCoord2dARB");
				names.push_back("glMultiTexCoord2dvARB");
				names.push_back("glMultiTexCoord2fARB");
				names.push_back("glMultiTexCoord2fvARB");
				names.push_back("glMultiTexCoord2iARB");
				names.push_back("glMultiTexCoord2ivARB");
				names.push_back("glMultiTexCoord2sARB");
				names.push_back("glMultiTexCoord2svARB");
				names.push_back("glMultiTexCoord3dARB");
				names.push_back("glMultiTexCoord3dvARB");
				names.push_back("glMultiTexCoord3fARB");
				names.push_back("glMultiTexCoord3fvARB");
				names.push_back("glMultiTexCoord3iARB");
				names.push_back("glMultiTexCoord3ivARB");
				names.push_back("glMultiTexCoord3sARB");
				names.push_back("glMultiTexCoord3svARB");
				names.push_back("glMultiTexCoord4dARB");
				names.push_back("glMultiTexCoord4dvARB");
				names.push_back("glMultiTexCoord4fARB");
				names.push_back("glMultiTexCoord4fvARB");
				names.push_back("glMultiTexCoord4iARB");
				names.push_back("glMultiTexCoord4ivARB");
				names.push_back("glMultiTexCoord4sARB");
				names.push_back("glMultiTexCoord4svARB");
				names.push_back("glLoadTransposeMatrixfARB");
				names.push_back("glLoadTransposeMatrixdARB");
				names.push_back("glMultTransposeMatrixfARB");
				names.push_back("glMultTransposeMatrixdARB");
				names.push_back("glSampleCoverageARB");
				names.push_back("glCompressedTexImage3DARB");
				names.push_back("glCompressedTexImage2DARB");
				names.push_back("glCompressedTexImage1DARB");
				names.push_back("glCompressedTexSubImage3DARB");
				names.push_back("glCompressedTexSubImage2DARB");
				names.push_back("glCompressedTexSubImage1DARB");
				names.push_back("glGetCompressedTexImageARB");

				load_funcs(entries, names);

				return;
			}
		}
	}

	char APIENTRY self_init_glloader_GL_VERSION_1_3()
	{
		glloader_init();
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
		glloader_init();
		return glActiveTexture(texture);
	}
	void APIENTRY self_init_glClientActiveTexture(GLenum texture)
	{
		glloader_init();
		return glClientActiveTexture(texture);
	}
	void APIENTRY self_init_glMultiTexCoord1d(GLenum target, GLdouble s)
	{
		glloader_init();
		return glMultiTexCoord1d(target, s);
	}
	void APIENTRY self_init_glMultiTexCoord1dv(GLenum target, const GLdouble* v)
	{
		glloader_init();
		return glMultiTexCoord1dv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord1f(GLenum target, GLfloat s)
	{
		glloader_init();
		return glMultiTexCoord1f(target, s);
	}
	void APIENTRY self_init_glMultiTexCoord1fv(GLenum target, const GLfloat* v)
	{
		glloader_init();
		return glMultiTexCoord1fv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord1i(GLenum target, GLint s)
	{
		glloader_init();
		return glMultiTexCoord1i(target, s);
	}
	void APIENTRY self_init_glMultiTexCoord1iv(GLenum target, const GLint* v)
	{
		glloader_init();
		return glMultiTexCoord1iv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord1s(GLenum target, GLshort s)
	{
		glloader_init();
		return glMultiTexCoord1s(target, s);
	}
	void APIENTRY self_init_glMultiTexCoord1sv(GLenum target, const GLshort* v)
	{
		glloader_init();
		return glMultiTexCoord1sv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t)
	{
		glloader_init();
		return glMultiTexCoord2d(target, s, t);
	}
	void APIENTRY self_init_glMultiTexCoord2dv(GLenum target, const GLdouble* v)
	{
		glloader_init();
		return glMultiTexCoord2dv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
	{
		glloader_init();
		return glMultiTexCoord2f(target, s, t);
	}
	void APIENTRY self_init_glMultiTexCoord2fv(GLenum target, const GLfloat* v)
	{
		glloader_init();
		return glMultiTexCoord2fv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord2i(GLenum target, GLint s, GLint t)
	{
		glloader_init();
		return glMultiTexCoord2i(target, s, t);
	}
	void APIENTRY self_init_glMultiTexCoord2iv(GLenum target, const GLint* v)
	{
		glloader_init();
		return glMultiTexCoord2iv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord2s(GLenum target, GLshort s, GLshort t)
	{
		glloader_init();
		return glMultiTexCoord2s(target, s, t);
	}
	void APIENTRY self_init_glMultiTexCoord2sv(GLenum target, const GLshort* v)
	{
		glloader_init();
		return glMultiTexCoord2sv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r)
	{
		glloader_init();
		return glMultiTexCoord3d(target, s, t, r);
	}
	void APIENTRY self_init_glMultiTexCoord3dv(GLenum target, const GLdouble* v)
	{
		glloader_init();
		return glMultiTexCoord3dv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r)
	{
		glloader_init();
		return glMultiTexCoord3f(target, s, t, r);
	}
	void APIENTRY self_init_glMultiTexCoord3fv(GLenum target, const GLfloat* v)
	{
		glloader_init();
		return glMultiTexCoord3fv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r)
	{
		glloader_init();
		return glMultiTexCoord3i(target, s, t, r);
	}
	void APIENTRY self_init_glMultiTexCoord3iv(GLenum target, const GLint* v)
	{
		glloader_init();
		return glMultiTexCoord3iv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r)
	{
		glloader_init();
		return glMultiTexCoord3s(target, s, t, r);
	}
	void APIENTRY self_init_glMultiTexCoord3sv(GLenum target, const GLshort* v)
	{
		glloader_init();
		return glMultiTexCoord3sv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
	{
		glloader_init();
		return glMultiTexCoord4d(target, s, t, r, q);
	}
	void APIENTRY self_init_glMultiTexCoord4dv(GLenum target, const GLdouble* v)
	{
		glloader_init();
		return glMultiTexCoord4dv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
	{
		glloader_init();
		return glMultiTexCoord4f(target, s, t, r, q);
	}
	void APIENTRY self_init_glMultiTexCoord4fv(GLenum target, const GLfloat* v)
	{
		glloader_init();
		return glMultiTexCoord4fv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q)
	{
		glloader_init();
		return glMultiTexCoord4i(target, s, t, r, q);
	}
	void APIENTRY self_init_glMultiTexCoord4iv(GLenum target, const GLint* v)
	{
		glloader_init();
		return glMultiTexCoord4iv(target, v);
	}
	void APIENTRY self_init_glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
	{
		glloader_init();
		return glMultiTexCoord4s(target, s, t, r, q);
	}
	void APIENTRY self_init_glMultiTexCoord4sv(GLenum target, const GLshort* v)
	{
		glloader_init();
		return glMultiTexCoord4sv(target, v);
	}
	void APIENTRY self_init_glLoadTransposeMatrixf(const GLfloat* m)
	{
		glloader_init();
		return glLoadTransposeMatrixf(m);
	}
	void APIENTRY self_init_glLoadTransposeMatrixd(const GLdouble* m)
	{
		glloader_init();
		return glLoadTransposeMatrixd(m);
	}
	void APIENTRY self_init_glMultTransposeMatrixf(const GLfloat* m)
	{
		glloader_init();
		return glMultTransposeMatrixf(m);
	}
	void APIENTRY self_init_glMultTransposeMatrixd(const GLdouble* m)
	{
		glloader_init();
		return glMultTransposeMatrixd(m);
	}
	void APIENTRY self_init_glSampleCoverage(GLclampf value, GLboolean invert)
	{
		glloader_init();
		return glSampleCoverage(value, invert);
	}
	void APIENTRY self_init_glCompressedTexImage3D(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
	{
		glloader_init();
		return glCompressedTexImage3D(target, level, internalFormat, width, height, depth, border, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
	{
		glloader_init();
		return glCompressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data)
	{
		glloader_init();
		return glCompressedTexImage1D(target, level, internalFormat, width, border, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
	{
		glloader_init();
		return glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
	{
		glloader_init();
		return glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
	}
	void APIENTRY self_init_glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data)
	{
		glloader_init();
		return glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
	}
	void APIENTRY self_init_glGetCompressedTexImage(GLenum target, GLint lod, GLvoid* img)
	{
		glloader_init();
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
