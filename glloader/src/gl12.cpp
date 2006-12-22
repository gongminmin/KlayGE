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
#include <glloader/gl12.h>
#include <glloader/gl11.h>
#include "utils.hpp"

#ifdef GLLOADER_GL

using glloader::load_funcs;
using glloader::gl_features_extractor;

namespace
{
	bool _GL_VERSION_1_2 = false;

	char APIENTRY _glloader_GL_VERSION_1_2()
	{
		return _GL_VERSION_1_2;
	}

	void init_GL_VERSION_1_2()
	{
		glloader_GL_VERSION_1_2 = _glloader_GL_VERSION_1_2;

		{
			glBlendColor = NULL;
			glBlendEquation = NULL;
			glDrawRangeElements = NULL;
			glColorTable = NULL;
			glColorTableParameterfv = NULL;
			glColorTableParameteriv = NULL;
			glCopyColorTable = NULL;
			glGetColorTable = NULL;
			glGetColorTableParameterfv = NULL;
			glGetColorTableParameteriv = NULL;
			glColorSubTable = NULL;
			glCopyColorSubTable = NULL;
			glConvolutionFilter1D = NULL;
			glConvolutionFilter2D = NULL;
			glConvolutionParameterf = NULL;
			glConvolutionParameterfv = NULL;
			glConvolutionParameteri = NULL;
			glConvolutionParameteriv = NULL;
			glCopyConvolutionFilter1D = NULL;
			glCopyConvolutionFilter2D = NULL;
			glGetConvolutionFilter = NULL;
			glGetConvolutionParameterfv = NULL;
			glGetConvolutionParameteriv = NULL;
			glGetSeparableFilter = NULL;
			glSeparableFilter2D = NULL;
			glGetHistogram = NULL;
			glGetHistogramParameterfv = NULL;
			glGetHistogramParameteriv = NULL;
			glGetMinmax = NULL;
			glGetMinmaxParameterfv = NULL;
			glGetMinmaxParameteriv = NULL;
			glHistogram = NULL;
			glMinmax = NULL;
			glResetHistogram = NULL;
			glResetMinmax = NULL;
			glTexImage3D = NULL;
			glTexSubImage3D = NULL;
			glCopyTexSubImage3D = NULL;
		}

		if (glloader_is_supported("GL_VERSION_1_2"))
		{
			_GL_VERSION_1_2 = true;

			entries_t entries;
			{
				entries.push_back(reinterpret_cast<void**>(&glBlendColor));
				entries.push_back(reinterpret_cast<void**>(&glBlendEquation));
				entries.push_back(reinterpret_cast<void**>(&glDrawRangeElements));
				entries.push_back(reinterpret_cast<void**>(&glColorTable));
				entries.push_back(reinterpret_cast<void**>(&glColorTableParameterfv));
				entries.push_back(reinterpret_cast<void**>(&glColorTableParameteriv));
				entries.push_back(reinterpret_cast<void**>(&glCopyColorTable));
				entries.push_back(reinterpret_cast<void**>(&glGetColorTable));
				entries.push_back(reinterpret_cast<void**>(&glGetColorTableParameterfv));
				entries.push_back(reinterpret_cast<void**>(&glGetColorTableParameteriv));
				entries.push_back(reinterpret_cast<void**>(&glColorSubTable));
				entries.push_back(reinterpret_cast<void**>(&glCopyColorSubTable));
				entries.push_back(reinterpret_cast<void**>(&glConvolutionFilter1D));
				entries.push_back(reinterpret_cast<void**>(&glConvolutionFilter2D));
				entries.push_back(reinterpret_cast<void**>(&glConvolutionParameterf));
				entries.push_back(reinterpret_cast<void**>(&glConvolutionParameterfv));
				entries.push_back(reinterpret_cast<void**>(&glConvolutionParameteri));
				entries.push_back(reinterpret_cast<void**>(&glConvolutionParameteriv));
				entries.push_back(reinterpret_cast<void**>(&glCopyConvolutionFilter1D));
				entries.push_back(reinterpret_cast<void**>(&glCopyConvolutionFilter2D));
				entries.push_back(reinterpret_cast<void**>(&glGetConvolutionFilter));
				entries.push_back(reinterpret_cast<void**>(&glGetConvolutionParameterfv));
				entries.push_back(reinterpret_cast<void**>(&glGetConvolutionParameteriv));
				entries.push_back(reinterpret_cast<void**>(&glGetSeparableFilter));
				entries.push_back(reinterpret_cast<void**>(&glSeparableFilter2D));
				entries.push_back(reinterpret_cast<void**>(&glGetHistogram));
				entries.push_back(reinterpret_cast<void**>(&glGetHistogramParameterfv));
				entries.push_back(reinterpret_cast<void**>(&glGetHistogramParameteriv));
				entries.push_back(reinterpret_cast<void**>(&glGetMinmax));
				entries.push_back(reinterpret_cast<void**>(&glGetMinmaxParameterfv));
				entries.push_back(reinterpret_cast<void**>(&glGetMinmaxParameteriv));
				entries.push_back(reinterpret_cast<void**>(&glHistogram));
				entries.push_back(reinterpret_cast<void**>(&glMinmax));
				entries.push_back(reinterpret_cast<void**>(&glResetHistogram));
				entries.push_back(reinterpret_cast<void**>(&glResetMinmax));
				entries.push_back(reinterpret_cast<void**>(&glTexImage3D));
				entries.push_back(reinterpret_cast<void**>(&glTexSubImage3D));
				entries.push_back(reinterpret_cast<void**>(&glCopyTexSubImage3D));
			}

			funcs_names_t names;
			{
				names.push_back("glBlendColor");
				names.push_back("glBlendEquation");
				names.push_back("glDrawRangeElements");
				names.push_back("glColorTable");
				names.push_back("glColorTableParameterfv");
				names.push_back("glColorTableParameteriv");
				names.push_back("glCopyColorTable");
				names.push_back("glGetColorTable");
				names.push_back("glGetColorTableParameterfv");
				names.push_back("glGetColorTableParameteriv");
				names.push_back("glColorSubTable");
				names.push_back("glCopyColorSubTable");
				names.push_back("glConvolutionFilter1D");
				names.push_back("glConvolutionFilter2D");
				names.push_back("glConvolutionParameterf");
				names.push_back("glConvolutionParameterfv");
				names.push_back("glConvolutionParameteri");
				names.push_back("glConvolutionParameteriv");
				names.push_back("glCopyConvolutionFilter1D");
				names.push_back("glCopyConvolutionFilter2D");
				names.push_back("glGetConvolutionFilter");
				names.push_back("glGetConvolutionParameterfv");
				names.push_back("glGetConvolutionParameteriv");
				names.push_back("glGetSeparableFilter");
				names.push_back("glSeparableFilter2D");
				names.push_back("glGetHistogram");
				names.push_back("glGetHistogramParameterfv");
				names.push_back("glGetHistogramParameteriv");
				names.push_back("glGetMinmax");
				names.push_back("glGetMinmaxParameterfv");
				names.push_back("glGetMinmaxParameteriv");
				names.push_back("glHistogram");
				names.push_back("glMinmax");
				names.push_back("glResetHistogram");
				names.push_back("glResetMinmax");
				names.push_back("glTexImage3D");
				names.push_back("glTexSubImage3D");
				names.push_back("glCopyTexSubImage3D");
			}

			load_funcs(entries, names);
		}
		else
		{
			if (glloader_GL_EXT_texture3D()
				&& glloader_GL_EXT_bgra()
				&& glloader_GL_EXT_packed_pixels()
				&& glloader_GL_EXT_rescale_normal()
				&& glloader_GL_EXT_separate_specular_color()
				&& glloader_GL_SGIS_texture_edge_clamp()
				&& glloader_GL_SGIS_texture_lod()
				&& glloader_GL_EXT_draw_range_elements()
				&& glloader_GL_ARB_imaging()
				&& (glloader_GL_SGI_color_table() && glloader_GL_EXT_color_subtable())
				&& (glloader_GL_EXT_convolution() &&  glloader_GL_HP_convolution_border_modes())
				&& glloader_GL_SGI_color_matrix()
				&& glloader_GL_EXT_histogram()
				&& glloader_GL_EXT_blend_color()
				&& (glloader_GL_EXT_blend_minmax() && glloader_GL_EXT_blend_subtract()))
			{
				_GL_VERSION_1_2 = true;
				gl_features_extractor::instance().promote("GL_VERSION_1_2");

				glBlendColor = glBlendColorEXT;
				glBlendEquation = glBlendEquationEXT;
				glDrawRangeElements = glDrawRangeElementsEXT;
				glColorTable = glColorTableSGI;
				glColorTableParameterfv = glColorTableParameterfvSGI;
				glColorTableParameteriv = glColorTableParameterivSGI;
				glCopyColorTable = glCopyColorTableSGI;
				glGetColorTable = glGetColorTableSGI;
				glGetColorTableParameterfv = glGetColorTableParameterfvSGI;
				glGetColorTableParameteriv = glGetColorTableParameterivSGI;
				glColorSubTable = glColorSubTableEXT;
				glCopyColorSubTable = glCopyColorSubTableEXT;
				glConvolutionFilter1D = glConvolutionFilter1DEXT;
				glConvolutionFilter2D = glConvolutionFilter2DEXT;
				glConvolutionParameterf = glConvolutionParameterfEXT;
				glConvolutionParameterfv = glConvolutionParameterfvEXT;
				glConvolutionParameteri = glConvolutionParameteriEXT;
				glConvolutionParameteriv = glConvolutionParameterivEXT;
				glCopyConvolutionFilter1D = glCopyConvolutionFilter1DEXT;
				glCopyConvolutionFilter2D = glCopyConvolutionFilter2DEXT;
				glGetConvolutionFilter = glGetConvolutionFilterEXT;
				glGetConvolutionParameterfv = glGetConvolutionParameterfvEXT;
				glGetConvolutionParameteriv = glGetConvolutionParameterivEXT;
				glGetSeparableFilter = glGetSeparableFilterEXT;
				glSeparableFilter2D = glSeparableFilter2DEXT;
				glGetHistogram = glGetHistogramEXT;
				glGetHistogramParameterfv = glGetHistogramParameterfvEXT;
				glGetHistogramParameteriv = glGetHistogramParameterivEXT;
				glGetMinmax = glGetMinmaxEXT;
				glGetMinmaxParameterfv = glGetMinmaxParameterfvEXT;
				glGetMinmaxParameteriv = glGetMinmaxParameterivEXT;
				glHistogram = glHistogramEXT;
				glMinmax = glMinmaxEXT;
				glResetHistogram = glResetHistogramEXT;
				glResetMinmax = glResetMinmaxEXT;
				glTexImage3D = glTexImage3DEXT;
				glTexSubImage3D = glTexSubImage3DEXT;
				glCopyTexSubImage3D = glCopyTexSubImage3DEXT;
			}
		}
	}

	char APIENTRY self_init_glloader_GL_VERSION_1_2()
	{
		glloader_init();

		init_GL_VERSION_1_2();
		return glloader_GL_VERSION_1_2();
	}
}

glloader_GL_VERSION_1_2FUNC glloader_GL_VERSION_1_2 = self_init_glloader_GL_VERSION_1_2;

#ifdef GL_VERSION_1_2

namespace
{
	void APIENTRY self_init_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
	{
		init_GL_VERSION_1_2();
		return glBlendColor(red, green, blue, alpha);
	}
	void APIENTRY self_init_glBlendEquation(GLenum mode)
	{
		init_GL_VERSION_1_2();
		return glBlendEquation(mode);
	}
	void APIENTRY self_init_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
	{
		init_GL_VERSION_1_2();
		return glDrawRangeElements(mode, start, end, count, type, indices);
	}
	void APIENTRY self_init_glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid* table)
	{
		init_GL_VERSION_1_2();
		return glColorTable(target, internalformat, width, format, type, table);
	}
	void APIENTRY self_init_glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat* params)
	{
		init_GL_VERSION_1_2();
		return glColorTableParameterfv(target, pname, params);
	}
	void APIENTRY self_init_glColorTableParameteriv(GLenum target, GLenum pname, const GLint* params)
	{
		init_GL_VERSION_1_2();
		return glColorTableParameteriv(target, pname, params);
	}
	void APIENTRY self_init_glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
	{
		init_GL_VERSION_1_2();
		return glCopyColorTable(target, internalformat, x, y, width);
	}
	void APIENTRY self_init_glGetColorTable(GLenum target, GLenum format, GLenum type, GLvoid* table)
	{
		init_GL_VERSION_1_2();
		return glGetColorTable(target, format, type, table);
	}
	void APIENTRY self_init_glGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat* params)
	{
		init_GL_VERSION_1_2();
		return glGetColorTableParameterfv(target, pname, params);
	}
	void APIENTRY self_init_glGetColorTableParameteriv(GLenum target, GLenum pname, GLint* params)
	{
		init_GL_VERSION_1_2();
		return glGetColorTableParameteriv(target, pname, params);
	}
	void APIENTRY self_init_glColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid* data)
	{
		init_GL_VERSION_1_2();
		return glColorSubTable(target, start, count, format, type, data);
	}
	void APIENTRY self_init_glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
	{
		init_GL_VERSION_1_2();
		return glCopyColorSubTable(target, start, x, y, width);
	}
	void APIENTRY self_init_glConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid* image)
	{
		init_GL_VERSION_1_2();
		return glConvolutionFilter1D(target, internalformat, width, format, type, image);
	}
	void APIENTRY self_init_glConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* image)
	{
		init_GL_VERSION_1_2();
		return glConvolutionFilter2D(target, internalformat, width, height, format, type, image);
	}
	void APIENTRY self_init_glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params)
	{
		init_GL_VERSION_1_2();
		return glConvolutionParameterf(target, pname, params);
	}
	void APIENTRY self_init_glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat* params)
	{
		init_GL_VERSION_1_2();
		return glConvolutionParameterfv(target, pname, params);
	}
	void APIENTRY self_init_glConvolutionParameteri(GLenum target, GLenum pname, GLint params)
	{
		init_GL_VERSION_1_2();
		return glConvolutionParameteri(target, pname, params);
	}
	void APIENTRY self_init_glConvolutionParameteriv(GLenum target, GLenum pname, const GLint* params)
	{
		init_GL_VERSION_1_2();
		return glConvolutionParameteriv(target, pname, params);
	}
	void APIENTRY self_init_glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
	{
		init_GL_VERSION_1_2();
		return glCopyConvolutionFilter1D(target, internalformat, x, y, width);
	}
	void APIENTRY self_init_glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
	{
		init_GL_VERSION_1_2();
		return glCopyConvolutionFilter2D(target, internalformat, x, y, width, height);
	}
	void APIENTRY self_init_glGetConvolutionFilter(GLenum target, GLenum format, GLenum type, GLvoid* image)
	{
		init_GL_VERSION_1_2();
		return glGetConvolutionFilter(target, format, type, image);
	}
	void APIENTRY self_init_glGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat* params)
	{
		init_GL_VERSION_1_2();
		return glGetConvolutionParameterfv(target, pname, params);
	}
	void APIENTRY self_init_glGetConvolutionParameteriv(GLenum target, GLenum pname, GLint* params)
	{
		init_GL_VERSION_1_2();
		return glGetConvolutionParameteriv(target, pname, params);
	}
	void APIENTRY self_init_glGetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid* row, GLvoid* column, GLvoid* span)
	{
		init_GL_VERSION_1_2();
		return glGetSeparableFilter(target, format, type, row, column, span);
	}
	void APIENTRY self_init_glSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* row, const GLvoid* column)
	{
		init_GL_VERSION_1_2();
		return glSeparableFilter2D(target, internalformat, width, height, format, type, row, column);
	}
	void APIENTRY self_init_glGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values)
	{
		init_GL_VERSION_1_2();
		return glGetHistogram(target, reset, format, type, values);
	}
	void APIENTRY self_init_glGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat* params)
	{
		init_GL_VERSION_1_2();
		return glGetHistogramParameterfv(target, pname, params);
	}
	void APIENTRY self_init_glGetHistogramParameteriv(GLenum target, GLenum pname, GLint* params)
	{
		init_GL_VERSION_1_2();
		return glGetHistogramParameteriv(target, pname, params);
	}
	void APIENTRY self_init_glGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values)
	{
		init_GL_VERSION_1_2();
		return glGetMinmax(target, reset, format, type, values);
	}
	void APIENTRY self_init_glGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat* params)
	{
		init_GL_VERSION_1_2();
		return glGetMinmaxParameterfv(target, pname, params);
	}
	void APIENTRY self_init_glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint* params)
	{
		init_GL_VERSION_1_2();
		return glGetMinmaxParameteriv(target, pname, params);
	}
	void APIENTRY self_init_glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
	{
		init_GL_VERSION_1_2();
		return glHistogram(target, width, internalformat, sink);
	}
	void APIENTRY self_init_glMinmax(GLenum target, GLenum internalformat, GLboolean sink)
	{
		init_GL_VERSION_1_2();
		return glMinmax(target, internalformat, sink);
	}
	void APIENTRY self_init_glResetHistogram(GLenum target)
	{
		init_GL_VERSION_1_2();
		return glResetHistogram(target);
	}
	void APIENTRY self_init_glResetMinmax(GLenum target)
	{
		init_GL_VERSION_1_2();
		return glResetMinmax(target);
	}
	void APIENTRY self_init_glTexImage3D(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
	{
		init_GL_VERSION_1_2();
		return glTexImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels);
	}
	void APIENTRY self_init_glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
	{
		init_GL_VERSION_1_2();
		return glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
	}
	void APIENTRY self_init_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
	{
		init_GL_VERSION_1_2();
		return glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
	}
}

glBlendColorFUNC glBlendColor = self_init_glBlendColor;
glBlendEquationFUNC glBlendEquation = self_init_glBlendEquation;
glDrawRangeElementsFUNC glDrawRangeElements = self_init_glDrawRangeElements;
glColorTableFUNC glColorTable = self_init_glColorTable;
glColorTableParameterfvFUNC glColorTableParameterfv = self_init_glColorTableParameterfv;
glColorTableParameterivFUNC glColorTableParameteriv = self_init_glColorTableParameteriv;
glCopyColorTableFUNC glCopyColorTable = self_init_glCopyColorTable;
glGetColorTableFUNC glGetColorTable = self_init_glGetColorTable;
glGetColorTableParameterfvFUNC glGetColorTableParameterfv = self_init_glGetColorTableParameterfv;
glGetColorTableParameterivFUNC glGetColorTableParameteriv = self_init_glGetColorTableParameteriv;
glColorSubTableFUNC glColorSubTable = self_init_glColorSubTable;
glCopyColorSubTableFUNC glCopyColorSubTable = self_init_glCopyColorSubTable;
glConvolutionFilter1DFUNC glConvolutionFilter1D = self_init_glConvolutionFilter1D;
glConvolutionFilter2DFUNC glConvolutionFilter2D = self_init_glConvolutionFilter2D;
glConvolutionParameterfFUNC glConvolutionParameterf = self_init_glConvolutionParameterf;
glConvolutionParameterfvFUNC glConvolutionParameterfv = self_init_glConvolutionParameterfv;
glConvolutionParameteriFUNC glConvolutionParameteri = self_init_glConvolutionParameteri;
glConvolutionParameterivFUNC glConvolutionParameteriv = self_init_glConvolutionParameteriv;
glCopyConvolutionFilter1DFUNC glCopyConvolutionFilter1D = self_init_glCopyConvolutionFilter1D;
glCopyConvolutionFilter2DFUNC glCopyConvolutionFilter2D = self_init_glCopyConvolutionFilter2D;
glGetConvolutionFilterFUNC glGetConvolutionFilter = self_init_glGetConvolutionFilter;
glGetConvolutionParameterfvFUNC glGetConvolutionParameterfv = self_init_glGetConvolutionParameterfv;
glGetConvolutionParameterivFUNC glGetConvolutionParameteriv = self_init_glGetConvolutionParameteriv;
glGetSeparableFilterFUNC glGetSeparableFilter = self_init_glGetSeparableFilter;
glSeparableFilter2DFUNC glSeparableFilter2D = self_init_glSeparableFilter2D;
glGetHistogramFUNC glGetHistogram = self_init_glGetHistogram;
glGetHistogramParameterfvFUNC glGetHistogramParameterfv = self_init_glGetHistogramParameterfv;
glGetHistogramParameterivFUNC glGetHistogramParameteriv = self_init_glGetHistogramParameteriv;
glGetMinmaxFUNC glGetMinmax = self_init_glGetMinmax;
glGetMinmaxParameterfvFUNC glGetMinmaxParameterfv = self_init_glGetMinmaxParameterfv;
glGetMinmaxParameterivFUNC glGetMinmaxParameteriv = self_init_glGetMinmaxParameteriv;
glHistogramFUNC glHistogram = self_init_glHistogram;
glMinmaxFUNC glMinmax = self_init_glMinmax;
glResetHistogramFUNC glResetHistogram = self_init_glResetHistogram;
glResetMinmaxFUNC glResetMinmax = self_init_glResetMinmax;
glTexImage3DFUNC glTexImage3D = self_init_glTexImage3D;
glTexSubImage3DFUNC glTexSubImage3D = self_init_glTexSubImage3D;
glCopyTexSubImage3DFUNC glCopyTexSubImage3D = self_init_glCopyTexSubImage3D;

#endif		// GL_VERSION_1_2


#endif			// GLLOADER_GL
