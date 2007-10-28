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
#include <glloader/gl14.h>
#include <glloader/gl13.h>
#include "utils.hpp"

#ifdef GLLOADER_GL

using glloader::load_funcs;
using glloader::gl_features_extractor;

namespace
{
	bool _GL_VERSION_1_4 = false;

	char APIENTRY _glloader_GL_VERSION_1_4()
	{
		return _GL_VERSION_1_4;
	}

	void init_GL_VERSION_1_4()
	{
		glloader_GL_VERSION_1_4 = _glloader_GL_VERSION_1_4;

		{
			glBlendFuncSeparate = NULL;
			glFogCoordf = NULL;
			glFogCoordfv = NULL;
			glFogCoordd = NULL;
			glFogCoorddv = NULL;
			glFogCoordPointer = NULL;
			glMultiDrawArrays = NULL;
			glMultiDrawElements = NULL;
			glPointParameterf = NULL;
			glPointParameterfv = NULL;
			glPointParameteri = NULL;
			glPointParameteriv = NULL;
			glSecondaryColor3b = NULL;
			glSecondaryColor3bv = NULL;
			glSecondaryColor3d = NULL;
			glSecondaryColor3dv = NULL;
			glSecondaryColor3f = NULL;
			glSecondaryColor3fv = NULL;
			glSecondaryColor3i = NULL;
			glSecondaryColor3iv = NULL;
			glSecondaryColor3s = NULL;
			glSecondaryColor3sv = NULL;
			glSecondaryColor3ub = NULL;
			glSecondaryColor3ubv = NULL;
			glSecondaryColor3ui = NULL;
			glSecondaryColor3uiv = NULL;
			glSecondaryColor3us = NULL;
			glSecondaryColor3usv = NULL;
			glSecondaryColorPointer = NULL;
			glWindowPos2d = NULL;
			glWindowPos2dv = NULL;
			glWindowPos2f = NULL;
			glWindowPos2fv = NULL;
			glWindowPos2i = NULL;
			glWindowPos2iv = NULL;
			glWindowPos2s = NULL;
			glWindowPos2sv = NULL;
			glWindowPos3d = NULL;
			glWindowPos3dv = NULL;
			glWindowPos3f = NULL;
			glWindowPos3fv = NULL;
			glWindowPos3i = NULL;
			glWindowPos3iv = NULL;
			glWindowPos3s = NULL;
			glWindowPos3sv = NULL;
		}

		if (glloader_is_supported("GL_VERSION_1_4"))
		{
			_GL_VERSION_1_4 = true;

			entries_t entries(45);
			{
				entries[0] = reinterpret_cast<void**>(&glBlendFuncSeparate);
				entries[1] = reinterpret_cast<void**>(&glFogCoordf);
				entries[2] = reinterpret_cast<void**>(&glFogCoordfv);
				entries[3] = reinterpret_cast<void**>(&glFogCoordd);
				entries[4] = reinterpret_cast<void**>(&glFogCoorddv);
				entries[5] = reinterpret_cast<void**>(&glFogCoordPointer);
				entries[6] = reinterpret_cast<void**>(&glMultiDrawArrays);
				entries[7] = reinterpret_cast<void**>(&glMultiDrawElements);
				entries[8] = reinterpret_cast<void**>(&glPointParameterf);
				entries[9] = reinterpret_cast<void**>(&glPointParameterfv);
				entries[10] = reinterpret_cast<void**>(&glPointParameteri);
				entries[11] = reinterpret_cast<void**>(&glPointParameteriv);
				entries[12] = reinterpret_cast<void**>(&glSecondaryColor3b);
				entries[13] = reinterpret_cast<void**>(&glSecondaryColor3bv);
				entries[14] = reinterpret_cast<void**>(&glSecondaryColor3d);
				entries[15] = reinterpret_cast<void**>(&glSecondaryColor3dv);
				entries[16] = reinterpret_cast<void**>(&glSecondaryColor3f);
				entries[17] = reinterpret_cast<void**>(&glSecondaryColor3fv);
				entries[18] = reinterpret_cast<void**>(&glSecondaryColor3i);
				entries[19] = reinterpret_cast<void**>(&glSecondaryColor3iv);
				entries[20] = reinterpret_cast<void**>(&glSecondaryColor3s);
				entries[21] = reinterpret_cast<void**>(&glSecondaryColor3sv);
				entries[22] = reinterpret_cast<void**>(&glSecondaryColor3ub);
				entries[23] = reinterpret_cast<void**>(&glSecondaryColor3ubv);
				entries[24] = reinterpret_cast<void**>(&glSecondaryColor3ui);
				entries[25] = reinterpret_cast<void**>(&glSecondaryColor3uiv);
				entries[26] = reinterpret_cast<void**>(&glSecondaryColor3us);
				entries[27] = reinterpret_cast<void**>(&glSecondaryColor3usv);
				entries[28] = reinterpret_cast<void**>(&glSecondaryColorPointer);
				entries[29] = reinterpret_cast<void**>(&glWindowPos2d);
				entries[30] = reinterpret_cast<void**>(&glWindowPos2dv);
				entries[31] = reinterpret_cast<void**>(&glWindowPos2f);
				entries[32] = reinterpret_cast<void**>(&glWindowPos2fv);
				entries[33] = reinterpret_cast<void**>(&glWindowPos2i);
				entries[34] = reinterpret_cast<void**>(&glWindowPos2iv);
				entries[35] = reinterpret_cast<void**>(&glWindowPos2s);
				entries[36] = reinterpret_cast<void**>(&glWindowPos2sv);
				entries[37] = reinterpret_cast<void**>(&glWindowPos3d);
				entries[38] = reinterpret_cast<void**>(&glWindowPos3dv);
				entries[39] = reinterpret_cast<void**>(&glWindowPos3f);
				entries[40] = reinterpret_cast<void**>(&glWindowPos3fv);
				entries[41] = reinterpret_cast<void**>(&glWindowPos3i);
				entries[42] = reinterpret_cast<void**>(&glWindowPos3iv);
				entries[43] = reinterpret_cast<void**>(&glWindowPos3s);
				entries[44] = reinterpret_cast<void**>(&glWindowPos3sv);
			}

			funcs_names_t names(45);
			{
				names[0] = "glBlendFuncSeparate";
				names[1] = "glFogCoordf";
				names[2] = "glFogCoordfv";
				names[3] = "glFogCoordd";
				names[4] = "glFogCoorddv";
				names[5] = "glFogCoordPointer";
				names[6] = "glMultiDrawArrays";
				names[7] = "glMultiDrawElements";
				names[8] = "glPointParameterf";
				names[9] = "glPointParameterfv";
				names[10] = "glPointParameteri";
				names[11] = "glPointParameteriv";
				names[12] = "glSecondaryColor3b";
				names[13] = "glSecondaryColor3bv";
				names[14] = "glSecondaryColor3d";
				names[15] = "glSecondaryColor3dv";
				names[16] = "glSecondaryColor3f";
				names[17] = "glSecondaryColor3fv";
				names[18] = "glSecondaryColor3i";
				names[19] = "glSecondaryColor3iv";
				names[20] = "glSecondaryColor3s";
				names[21] = "glSecondaryColor3sv";
				names[22] = "glSecondaryColor3ub";
				names[23] = "glSecondaryColor3ubv";
				names[24] = "glSecondaryColor3ui";
				names[25] = "glSecondaryColor3uiv";
				names[26] = "glSecondaryColor3us";
				names[27] = "glSecondaryColor3usv";
				names[28] = "glSecondaryColorPointer";
				names[29] = "glWindowPos2d";
				names[30] = "glWindowPos2dv";
				names[31] = "glWindowPos2f";
				names[32] = "glWindowPos2fv";
				names[33] = "glWindowPos2i";
				names[34] = "glWindowPos2iv";
				names[35] = "glWindowPos2s";
				names[36] = "glWindowPos2sv";
				names[37] = "glWindowPos3d";
				names[38] = "glWindowPos3dv";
				names[39] = "glWindowPos3f";
				names[40] = "glWindowPos3fv";
				names[41] = "glWindowPos3i";
				names[42] = "glWindowPos3iv";
				names[43] = "glWindowPos3s";
				names[44] = "glWindowPos3sv";
			}

			load_funcs(entries, names);
		}
		else
		{
			if (glloader_GL_SGIS_generate_mipmap()
				&& glloader_GL_NV_blend_square()
				&& glloader_GL_ARB_depth_texture()
				&& (glloader_GL_ARB_shadow() || glloader_GL_SGIX_shadow())
				&& glloader_GL_EXT_fog_coord()
				&& glloader_GL_EXT_multi_draw_arrays()
				&& (glloader_GL_ARB_point_parameters() && glloader_GL_NV_point_sprite())
				&& glloader_GL_EXT_secondary_color()
				&& glloader_GL_EXT_blend_func_separate()
				&& glloader_GL_EXT_stencil_wrap()
				&& (glloader_GL_ARB_texture_env_crossbar() || glloader_GL_NV_texture_env_combine4())
				&& glloader_GL_EXT_texture_lod_bias()
				&& glloader_GL_ARB_texture_mirrored_repeat()
				&& (glloader_GL_ARB_window_pos() || glloader_GL_MESA_window_pos()))
			{
				_GL_VERSION_1_4 = true;
				gl_features_extractor::instance().promote("GL_VERSION_1_4");

				glBlendFuncSeparate = glBlendFuncSeparateEXT;
				glFogCoordf = glFogCoordfEXT;
				glFogCoordfv = glFogCoordfvEXT;
				glFogCoordd = glFogCoorddEXT;
				glFogCoorddv = glFogCoorddvEXT;
				glFogCoordPointer = glFogCoordPointerEXT;
				glMultiDrawArrays = glMultiDrawArraysEXT;
				glMultiDrawElements = glMultiDrawElementsEXT;
				glPointParameterf = glPointParameterfARB;
				glPointParameterfv = glPointParameterfvARB;
				glPointParameteri = glPointParameteriNV;
				glPointParameteriv = glPointParameterivNV;
				glSecondaryColor3b = glSecondaryColor3bEXT;
				glSecondaryColor3bv = glSecondaryColor3bvEXT;
				glSecondaryColor3d = glSecondaryColor3dEXT;
				glSecondaryColor3dv = glSecondaryColor3dvEXT;
				glSecondaryColor3f = glSecondaryColor3fEXT;
				glSecondaryColor3fv = glSecondaryColor3fvEXT;
				glSecondaryColor3i = glSecondaryColor3iEXT;
				glSecondaryColor3iv = glSecondaryColor3ivEXT;
				glSecondaryColor3s = glSecondaryColor3sEXT;
				glSecondaryColor3sv = glSecondaryColor3svEXT;
				glSecondaryColor3ub = glSecondaryColor3ubEXT;
				glSecondaryColor3ubv = glSecondaryColor3ubvEXT;
				glSecondaryColor3ui = glSecondaryColor3uiEXT;
				glSecondaryColor3uiv = glSecondaryColor3uivEXT;
				glSecondaryColor3us = glSecondaryColor3usEXT;
				glSecondaryColor3usv = glSecondaryColor3usvEXT;
				glSecondaryColorPointer = glSecondaryColorPointerEXT;
				if (glloader_GL_ARB_window_pos())
				{
					glWindowPos2d = glWindowPos2dARB;
					glWindowPos2dv = glWindowPos2dvARB;
					glWindowPos2f = glWindowPos2fARB;
					glWindowPos2fv = glWindowPos2fvARB;
					glWindowPos2i = glWindowPos2iARB;
					glWindowPos2iv = glWindowPos2ivARB;
					glWindowPos2s = glWindowPos2sARB;
					glWindowPos2sv = glWindowPos2svARB;
					glWindowPos3d = glWindowPos3dARB;
					glWindowPos3dv = glWindowPos3dvARB;
					glWindowPos3f = glWindowPos3fARB;
					glWindowPos3fv = glWindowPos3fvARB;
					glWindowPos3i = glWindowPos3iARB;
					glWindowPos3iv = glWindowPos3ivARB;
					glWindowPos3s = glWindowPos3sARB;
					glWindowPos3sv = glWindowPos3svARB;
				}
				else
				{
					if (glloader_GL_MESA_window_pos())
					{
						glWindowPos2d = glWindowPos2dMESA;
						glWindowPos2dv = glWindowPos2dvMESA;
						glWindowPos2f = glWindowPos2fMESA;
						glWindowPos2fv = glWindowPos2fvMESA;
						glWindowPos2i = glWindowPos2iMESA;
						glWindowPos2iv = glWindowPos2ivMESA;
						glWindowPos2s = glWindowPos2sMESA;
						glWindowPos2sv = glWindowPos2svMESA;
						glWindowPos3d = glWindowPos3dMESA;
						glWindowPos3dv = glWindowPos3dvMESA;
						glWindowPos3f = glWindowPos3fMESA;
						glWindowPos3fv = glWindowPos3fvMESA;
						glWindowPos3i = glWindowPos3iMESA;
						glWindowPos3iv = glWindowPos3ivMESA;
						glWindowPos3s = glWindowPos3sMESA;
						glWindowPos3sv = glWindowPos3svMESA;
					}
				}
			}
		}
	}

	char APIENTRY self_init_glloader_GL_VERSION_1_4()
	{
		glloader_GL_VERSION_1_3();

		init_GL_VERSION_1_4();
		return glloader_GL_VERSION_1_4();
	}
}

glloader_GL_VERSION_1_4FUNC glloader_GL_VERSION_1_4 = self_init_glloader_GL_VERSION_1_4;

#ifdef GL_VERSION_1_4

namespace
{
	void APIENTRY self_init_glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
	{
		init_GL_VERSION_1_4();
		return glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
	}
	void APIENTRY self_init_glFogCoordf(GLfloat coord)
	{
		init_GL_VERSION_1_4();
		return glFogCoordf(coord);
	}
	void APIENTRY self_init_glFogCoordfv(const GLfloat* coord)
	{
		init_GL_VERSION_1_4();
		return glFogCoordfv(coord);
	}
	void APIENTRY self_init_glFogCoordd(GLdouble coord)
	{
		init_GL_VERSION_1_4();
		return glFogCoordd(coord);
	}
	void APIENTRY self_init_glFogCoorddv(const GLdouble* coord)
	{
		init_GL_VERSION_1_4();
		return glFogCoorddv(coord);
	}
	void APIENTRY self_init_glFogCoordPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
	{
		init_GL_VERSION_1_4();
		return glFogCoordPointer(type, stride, pointer);
	}
	void APIENTRY self_init_glMultiDrawArrays(GLenum mode, GLint* first, GLsizei* count, GLsizei primcount)
	{
		init_GL_VERSION_1_4();
		return glMultiDrawArrays(mode, first, count, primcount);
	}
	void APIENTRY self_init_glMultiDrawElements(GLenum mode, GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount)
	{
		init_GL_VERSION_1_4();
		return glMultiDrawElements(mode, count, type, indices, primcount);
	}
	void APIENTRY self_init_glPointParameterf(GLenum pname, GLfloat param)
	{
		init_GL_VERSION_1_4();
		return glPointParameterf(pname, param);
	}
	void APIENTRY self_init_glPointParameterfv(GLenum pname, const GLfloat* params)
	{
		init_GL_VERSION_1_4();
		return glPointParameterfv(pname, params);
	}
	void APIENTRY self_init_glPointParameteri(GLenum pname, GLint param)
	{
		init_GL_VERSION_1_4();
		return glPointParameteri(pname, param);
	}
	void APIENTRY self_init_glPointParameteriv(GLenum pname, const GLint* params)
	{
		init_GL_VERSION_1_4();
		return glPointParameteriv(pname, params);
	}
	void APIENTRY self_init_glSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3b(red, green, blue);
	}
	void APIENTRY self_init_glSecondaryColor3bv(const GLbyte* v)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3bv(v);
	}
	void APIENTRY self_init_glSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3d(red, green, blue);
	}
	void APIENTRY self_init_glSecondaryColor3dv(const GLdouble* v)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3dv(v);
	}
	void APIENTRY self_init_glSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3f(red, green, blue);
	}
	void APIENTRY self_init_glSecondaryColor3fv(const GLfloat* v)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3fv(v);
	}
	void APIENTRY self_init_glSecondaryColor3i(GLint red, GLint green, GLint blue)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3i(red, green, blue);
	}
	void APIENTRY self_init_glSecondaryColor3iv(const GLint* v)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3iv(v);
	}
	void APIENTRY self_init_glSecondaryColor3s(GLshort red, GLshort green, GLshort blue)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3s(red, green, blue);
	}
	void APIENTRY self_init_glSecondaryColor3sv(const GLshort* v)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3sv(v);
	}
	void APIENTRY self_init_glSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3ub(red, green, blue);
	}
	void APIENTRY self_init_glSecondaryColor3ubv(const GLubyte* v)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3ubv(v);
	}
	void APIENTRY self_init_glSecondaryColor3ui(GLuint red, GLuint green, GLuint blue)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3ui(red, green, blue);
	}
	void APIENTRY self_init_glSecondaryColor3uiv(const GLuint* v)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3uiv(v);
	}
	void APIENTRY self_init_glSecondaryColor3us(GLushort red, GLushort green, GLushort blue)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3us(red, green, blue);
	}
	void APIENTRY self_init_glSecondaryColor3usv(const GLushort* v)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColor3usv(v);
	}
	void APIENTRY self_init_glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, GLvoid* pointer)
	{
		init_GL_VERSION_1_4();
		return glSecondaryColorPointer(size, type, stride, pointer);
	}
	void APIENTRY self_init_glWindowPos2d(GLdouble x, GLdouble y)
	{
		init_GL_VERSION_1_4();
		return glWindowPos2d(x, y);
	}
	void APIENTRY self_init_glWindowPos2dv(const GLdouble* p)
	{
		init_GL_VERSION_1_4();
		return glWindowPos2dv(p);
	}
	void APIENTRY self_init_glWindowPos2f(GLfloat x, GLfloat y)
	{
		init_GL_VERSION_1_4();
		return glWindowPos2f(x, y);
	}
	void APIENTRY self_init_glWindowPos2fv(const GLfloat* p)
	{
		init_GL_VERSION_1_4();
		return glWindowPos2fv(p);
	}
	void APIENTRY self_init_glWindowPos2i(GLint x, GLint y)
	{
		init_GL_VERSION_1_4();
		return glWindowPos2i(x, y);
	}
	void APIENTRY self_init_glWindowPos2iv(const GLint* p)
	{
		init_GL_VERSION_1_4();
		return glWindowPos2iv(p);
	}
	void APIENTRY self_init_glWindowPos2s(GLshort x, GLshort y)
	{
		init_GL_VERSION_1_4();
		return glWindowPos2s(x, y);
	}
	void APIENTRY self_init_glWindowPos2sv(const GLshort* p)
	{
		init_GL_VERSION_1_4();
		return glWindowPos2sv(p);
	}
	void APIENTRY self_init_glWindowPos3d(GLdouble x, GLdouble y, GLdouble z)
	{
		init_GL_VERSION_1_4();
		return glWindowPos3d(x, y, z);
	}
	void APIENTRY self_init_glWindowPos3dv(const GLdouble* p)
	{
		init_GL_VERSION_1_4();
		return glWindowPos3dv(p);
	}
	void APIENTRY self_init_glWindowPos3f(GLfloat x, GLfloat y, GLfloat z)
	{
		init_GL_VERSION_1_4();
		return glWindowPos3f(x, y, z);
	}
	void APIENTRY self_init_glWindowPos3fv(const GLfloat* p)
	{
		init_GL_VERSION_1_4();
		return glWindowPos3fv(p);
	}
	void APIENTRY self_init_glWindowPos3i(GLint x, GLint y, GLint z)
	{
		init_GL_VERSION_1_4();
		return glWindowPos3i(x, y, z);
	}
	void APIENTRY self_init_glWindowPos3iv(const GLint* p)
	{
		init_GL_VERSION_1_4();
		return glWindowPos3iv(p);
	}
	void APIENTRY self_init_glWindowPos3s(GLshort x, GLshort y, GLshort z)
	{
		init_GL_VERSION_1_4();
		return glWindowPos3s(x, y, z);
	}
	void APIENTRY self_init_glWindowPos3sv(const GLshort* p)
	{
		init_GL_VERSION_1_4();
		return glWindowPos3sv(p);
	}
}

glBlendFuncSeparateFUNC glBlendFuncSeparate = self_init_glBlendFuncSeparate;
glFogCoordfFUNC glFogCoordf = self_init_glFogCoordf;
glFogCoordfvFUNC glFogCoordfv = self_init_glFogCoordfv;
glFogCoorddFUNC glFogCoordd = self_init_glFogCoordd;
glFogCoorddvFUNC glFogCoorddv = self_init_glFogCoorddv;
glFogCoordPointerFUNC glFogCoordPointer = self_init_glFogCoordPointer;
glMultiDrawArraysFUNC glMultiDrawArrays = self_init_glMultiDrawArrays;
glMultiDrawElementsFUNC glMultiDrawElements = self_init_glMultiDrawElements;
glPointParameterfFUNC glPointParameterf = self_init_glPointParameterf;
glPointParameterfvFUNC glPointParameterfv = self_init_glPointParameterfv;
glPointParameteriFUNC glPointParameteri = self_init_glPointParameteri;
glPointParameterivFUNC glPointParameteriv = self_init_glPointParameteriv;
glSecondaryColor3bFUNC glSecondaryColor3b = self_init_glSecondaryColor3b;
glSecondaryColor3bvFUNC glSecondaryColor3bv = self_init_glSecondaryColor3bv;
glSecondaryColor3dFUNC glSecondaryColor3d = self_init_glSecondaryColor3d;
glSecondaryColor3dvFUNC glSecondaryColor3dv = self_init_glSecondaryColor3dv;
glSecondaryColor3fFUNC glSecondaryColor3f = self_init_glSecondaryColor3f;
glSecondaryColor3fvFUNC glSecondaryColor3fv = self_init_glSecondaryColor3fv;
glSecondaryColor3iFUNC glSecondaryColor3i = self_init_glSecondaryColor3i;
glSecondaryColor3ivFUNC glSecondaryColor3iv = self_init_glSecondaryColor3iv;
glSecondaryColor3sFUNC glSecondaryColor3s = self_init_glSecondaryColor3s;
glSecondaryColor3svFUNC glSecondaryColor3sv = self_init_glSecondaryColor3sv;
glSecondaryColor3ubFUNC glSecondaryColor3ub = self_init_glSecondaryColor3ub;
glSecondaryColor3ubvFUNC glSecondaryColor3ubv = self_init_glSecondaryColor3ubv;
glSecondaryColor3uiFUNC glSecondaryColor3ui = self_init_glSecondaryColor3ui;
glSecondaryColor3uivFUNC glSecondaryColor3uiv = self_init_glSecondaryColor3uiv;
glSecondaryColor3usFUNC glSecondaryColor3us = self_init_glSecondaryColor3us;
glSecondaryColor3usvFUNC glSecondaryColor3usv = self_init_glSecondaryColor3usv;
glSecondaryColorPointerFUNC glSecondaryColorPointer = self_init_glSecondaryColorPointer;
glWindowPos2dFUNC glWindowPos2d = self_init_glWindowPos2d;
glWindowPos2dvFUNC glWindowPos2dv = self_init_glWindowPos2dv;
glWindowPos2fFUNC glWindowPos2f = self_init_glWindowPos2f;
glWindowPos2fvFUNC glWindowPos2fv = self_init_glWindowPos2fv;
glWindowPos2iFUNC glWindowPos2i = self_init_glWindowPos2i;
glWindowPos2ivFUNC glWindowPos2iv = self_init_glWindowPos2iv;
glWindowPos2sFUNC glWindowPos2s = self_init_glWindowPos2s;
glWindowPos2svFUNC glWindowPos2sv = self_init_glWindowPos2sv;
glWindowPos3dFUNC glWindowPos3d = self_init_glWindowPos3d;
glWindowPos3dvFUNC glWindowPos3dv = self_init_glWindowPos3dv;
glWindowPos3fFUNC glWindowPos3f = self_init_glWindowPos3f;
glWindowPos3fvFUNC glWindowPos3fv = self_init_glWindowPos3fv;
glWindowPos3iFUNC glWindowPos3i = self_init_glWindowPos3i;
glWindowPos3ivFUNC glWindowPos3iv = self_init_glWindowPos3iv;
glWindowPos3sFUNC glWindowPos3s = self_init_glWindowPos3s;
glWindowPos3svFUNC glWindowPos3sv = self_init_glWindowPos3sv;

#endif		// GL_VERSION_1_4

#endif			// GLLOADER_GL
