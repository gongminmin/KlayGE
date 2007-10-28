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
#include <glloader/gl20.h>
#include <glloader/gl15.h>
#include "utils.hpp"

#ifdef GLLOADER_GL

using glloader::load_funcs;
using glloader::gl_features_extractor;

namespace
{
	bool _GL_VERSION_2_0 = false;

	char APIENTRY _glloader_GL_VERSION_2_0()
	{
		return _GL_VERSION_2_0;
	}

	void init_GL_VERSION_2_0()
	{
		glloader_GL_VERSION_2_0 = _glloader_GL_VERSION_2_0;

		if (glloader_is_supported("GL_VERSION_2_0"))
		{
			_GL_VERSION_2_0 = true;

			{
				glBlendEquationSeparate = NULL;
				glDrawBuffers = NULL;
				glStencilOpSeparate = NULL;
				glStencilFuncSeparate = NULL;
				glStencilMaskSeparate = NULL;
				glAttachShader = NULL;
				glBindAttribLocation = NULL;
				glCompileShader = NULL;
				glCreateProgram = NULL;
				glCreateShader = NULL;
				glDeleteProgram = NULL;
				glDeleteShader = NULL;
				glDetachShader = NULL;
				glDisableVertexAttribArray = NULL;
				glEnableVertexAttribArray = NULL;
				glGetActiveAttrib = NULL;
				glGetActiveUniform = NULL;
				glGetAttachedShaders = NULL;
				glGetAttribLocation = NULL;
				glGetProgramiv = NULL;
				glGetProgramInfoLog = NULL;
				glGetShaderiv = NULL;
				glGetShaderInfoLog = NULL;
				glGetShaderSource = NULL;
				glGetUniformLocation = NULL;
				glGetUniformfv = NULL;
				glGetUniformiv = NULL;
				glGetVertexAttribdv = NULL;
				glGetVertexAttribfv = NULL;
				glGetVertexAttribiv = NULL;
				glGetVertexAttribPointerv = NULL;
				glIsProgram = NULL;
				glIsShader = NULL;
				glLinkProgram = NULL;
				glShaderSource = NULL;
				glUseProgram = NULL;
				glUniform1f = NULL;
				glUniform2f = NULL;
				glUniform3f = NULL;
				glUniform4f = NULL;
				glUniform1i = NULL;
				glUniform2i = NULL;
				glUniform3i = NULL;
				glUniform4i = NULL;
				glUniform1fv = NULL;
				glUniform2fv = NULL;
				glUniform3fv = NULL;
				glUniform4fv = NULL;
				glUniform1iv = NULL;
				glUniform2iv = NULL;
				glUniform3iv = NULL;
				glUniform4iv = NULL;
				glUniformMatrix2fv = NULL;
				glUniformMatrix3fv = NULL;
				glUniformMatrix4fv = NULL;
				glValidateProgram = NULL;
				glVertexAttrib1d = NULL;
				glVertexAttrib1dv = NULL;
				glVertexAttrib1f = NULL;
				glVertexAttrib1fv = NULL;
				glVertexAttrib1s = NULL;
				glVertexAttrib1sv = NULL;
				glVertexAttrib2d = NULL;
				glVertexAttrib2dv = NULL;
				glVertexAttrib2f = NULL;
				glVertexAttrib2fv = NULL;
				glVertexAttrib2s = NULL;
				glVertexAttrib2sv = NULL;
				glVertexAttrib3d = NULL;
				glVertexAttrib3dv = NULL;
				glVertexAttrib3f = NULL;
				glVertexAttrib3fv = NULL;
				glVertexAttrib3s = NULL;
				glVertexAttrib3sv = NULL;
				glVertexAttrib4Nbv = NULL;
				glVertexAttrib4Niv = NULL;
				glVertexAttrib4Nsv = NULL;
				glVertexAttrib4Nub = NULL;
				glVertexAttrib4Nubv = NULL;
				glVertexAttrib4Nuiv = NULL;
				glVertexAttrib4Nusv = NULL;
				glVertexAttrib4bv = NULL;
				glVertexAttrib4d = NULL;
				glVertexAttrib4dv = NULL;
				glVertexAttrib4f = NULL;
				glVertexAttrib4fv = NULL;
				glVertexAttrib4iv = NULL;
				glVertexAttrib4s = NULL;
				glVertexAttrib4sv = NULL;
				glVertexAttrib4ubv = NULL;
				glVertexAttrib4uiv = NULL;
				glVertexAttrib4usv = NULL;
				glVertexAttribPointer = NULL;
			}

			entries_t entries(93);
			{
				entries[0] = reinterpret_cast<void**>(&glBlendEquationSeparate);
				entries[1] = reinterpret_cast<void**>(&glDrawBuffers);
				entries[2] = reinterpret_cast<void**>(&glStencilOpSeparate);
				entries[3] = reinterpret_cast<void**>(&glStencilFuncSeparate);
				entries[4] = reinterpret_cast<void**>(&glStencilMaskSeparate);
				entries[5] = reinterpret_cast<void**>(&glAttachShader);
				entries[6] = reinterpret_cast<void**>(&glBindAttribLocation);
				entries[7] = reinterpret_cast<void**>(&glCompileShader);
				entries[8] = reinterpret_cast<void**>(&glCreateProgram);
				entries[9] = reinterpret_cast<void**>(&glCreateShader);
				entries[10] = reinterpret_cast<void**>(&glDeleteProgram);
				entries[11] = reinterpret_cast<void**>(&glDeleteShader);
				entries[12] = reinterpret_cast<void**>(&glDetachShader);
				entries[13] = reinterpret_cast<void**>(&glDisableVertexAttribArray);
				entries[14] = reinterpret_cast<void**>(&glEnableVertexAttribArray);
				entries[15] = reinterpret_cast<void**>(&glGetActiveAttrib);
				entries[16] = reinterpret_cast<void**>(&glGetActiveUniform);
				entries[17] = reinterpret_cast<void**>(&glGetAttachedShaders);
				entries[18] = reinterpret_cast<void**>(&glGetAttribLocation);
				entries[19] = reinterpret_cast<void**>(&glGetProgramiv);
				entries[20] = reinterpret_cast<void**>(&glGetProgramInfoLog);
				entries[21] = reinterpret_cast<void**>(&glGetShaderiv);
				entries[22] = reinterpret_cast<void**>(&glGetShaderInfoLog);
				entries[23] = reinterpret_cast<void**>(&glGetShaderSource);
				entries[24] = reinterpret_cast<void**>(&glGetUniformLocation);
				entries[25] = reinterpret_cast<void**>(&glGetUniformfv);
				entries[26] = reinterpret_cast<void**>(&glGetUniformiv);
				entries[27] = reinterpret_cast<void**>(&glGetVertexAttribdv);
				entries[28] = reinterpret_cast<void**>(&glGetVertexAttribfv);
				entries[29] = reinterpret_cast<void**>(&glGetVertexAttribiv);
				entries[30] = reinterpret_cast<void**>(&glGetVertexAttribPointerv);
				entries[31] = reinterpret_cast<void**>(&glIsProgram);
				entries[32] = reinterpret_cast<void**>(&glIsShader);
				entries[33] = reinterpret_cast<void**>(&glLinkProgram);
				entries[34] = reinterpret_cast<void**>(&glShaderSource);
				entries[35] = reinterpret_cast<void**>(&glUseProgram);
				entries[36] = reinterpret_cast<void**>(&glUniform1f);
				entries[37] = reinterpret_cast<void**>(&glUniform2f);
				entries[38] = reinterpret_cast<void**>(&glUniform3f);
				entries[39] = reinterpret_cast<void**>(&glUniform4f);
				entries[40] = reinterpret_cast<void**>(&glUniform1i);
				entries[41] = reinterpret_cast<void**>(&glUniform2i);
				entries[42] = reinterpret_cast<void**>(&glUniform3i);
				entries[43] = reinterpret_cast<void**>(&glUniform4i);
				entries[44] = reinterpret_cast<void**>(&glUniform1fv);
				entries[45] = reinterpret_cast<void**>(&glUniform2fv);
				entries[46] = reinterpret_cast<void**>(&glUniform3fv);
				entries[47] = reinterpret_cast<void**>(&glUniform4fv);
				entries[48] = reinterpret_cast<void**>(&glUniform1iv);
				entries[49] = reinterpret_cast<void**>(&glUniform2iv);
				entries[50] = reinterpret_cast<void**>(&glUniform3iv);
				entries[51] = reinterpret_cast<void**>(&glUniform4iv);
				entries[52] = reinterpret_cast<void**>(&glUniformMatrix2fv);
				entries[53] = reinterpret_cast<void**>(&glUniformMatrix3fv);
				entries[54] = reinterpret_cast<void**>(&glUniformMatrix4fv);
				entries[55] = reinterpret_cast<void**>(&glValidateProgram);
				entries[56] = reinterpret_cast<void**>(&glVertexAttrib1d);
				entries[57] = reinterpret_cast<void**>(&glVertexAttrib1dv);
				entries[58] = reinterpret_cast<void**>(&glVertexAttrib1f);
				entries[59] = reinterpret_cast<void**>(&glVertexAttrib1fv);
				entries[60] = reinterpret_cast<void**>(&glVertexAttrib1s);
				entries[61] = reinterpret_cast<void**>(&glVertexAttrib1sv);
				entries[62] = reinterpret_cast<void**>(&glVertexAttrib2d);
				entries[63] = reinterpret_cast<void**>(&glVertexAttrib2dv);
				entries[64] = reinterpret_cast<void**>(&glVertexAttrib2f);
				entries[65] = reinterpret_cast<void**>(&glVertexAttrib2fv);
				entries[66] = reinterpret_cast<void**>(&glVertexAttrib2s);
				entries[67] = reinterpret_cast<void**>(&glVertexAttrib2sv);
				entries[68] = reinterpret_cast<void**>(&glVertexAttrib3d);
				entries[69] = reinterpret_cast<void**>(&glVertexAttrib3dv);
				entries[70] = reinterpret_cast<void**>(&glVertexAttrib3f);
				entries[71] = reinterpret_cast<void**>(&glVertexAttrib3fv);
				entries[72] = reinterpret_cast<void**>(&glVertexAttrib3s);
				entries[73] = reinterpret_cast<void**>(&glVertexAttrib3sv);
				entries[74] = reinterpret_cast<void**>(&glVertexAttrib4Nbv);
				entries[75] = reinterpret_cast<void**>(&glVertexAttrib4Niv);
				entries[76] = reinterpret_cast<void**>(&glVertexAttrib4Nsv);
				entries[77] = reinterpret_cast<void**>(&glVertexAttrib4Nub);
				entries[78] = reinterpret_cast<void**>(&glVertexAttrib4Nubv);
				entries[79] = reinterpret_cast<void**>(&glVertexAttrib4Nuiv);
				entries[80] = reinterpret_cast<void**>(&glVertexAttrib4Nusv);
				entries[81] = reinterpret_cast<void**>(&glVertexAttrib4bv);
				entries[82] = reinterpret_cast<void**>(&glVertexAttrib4d);
				entries[83] = reinterpret_cast<void**>(&glVertexAttrib4dv);
				entries[84] = reinterpret_cast<void**>(&glVertexAttrib4f);
				entries[85] = reinterpret_cast<void**>(&glVertexAttrib4fv);
				entries[86] = reinterpret_cast<void**>(&glVertexAttrib4iv);
				entries[87] = reinterpret_cast<void**>(&glVertexAttrib4s);
				entries[88] = reinterpret_cast<void**>(&glVertexAttrib4sv);
				entries[89] = reinterpret_cast<void**>(&glVertexAttrib4ubv);
				entries[90] = reinterpret_cast<void**>(&glVertexAttrib4uiv);
				entries[91] = reinterpret_cast<void**>(&glVertexAttrib4usv);
				entries[92] = reinterpret_cast<void**>(&glVertexAttribPointer);
			}

			funcs_names_t names(93);
			{
				names[0] = "glBlendEquationSeparate";
				names[1] = "glDrawBuffers";
				names[2] = "glStencilOpSeparate";
				names[3] = "glStencilFuncSeparate";
				names[4] = "glStencilMaskSeparate";
				names[5] = "glAttachShader";
				names[6] = "glBindAttribLocation";
				names[7] = "glCompileShader";
				names[8] = "glCreateProgram";
				names[9] = "glCreateShader";
				names[10] = "glDeleteProgram";
				names[11] = "glDeleteShader";
				names[12] = "glDetachShader";
				names[13] = "glDisableVertexAttribArray";
				names[14] = "glEnableVertexAttribArray";
				names[15] = "glGetActiveAttrib";
				names[16] = "glGetActiveUniform";
				names[17] = "glGetAttachedShaders";
				names[18] = "glGetAttribLocation";
				names[19] = "glGetProgramiv";
				names[20] = "glGetProgramInfoLog";
				names[21] = "glGetShaderiv";
				names[22] = "glGetShaderInfoLog";
				names[23] = "glGetShaderSource";
				names[24] = "glGetUniformLocation";
				names[25] = "glGetUniformfv";
				names[26] = "glGetUniformiv";
				names[27] = "glGetVertexAttribdv";
				names[28] = "glGetVertexAttribfv";
				names[29] = "glGetVertexAttribiv";
				names[30] = "glGetVertexAttribPointerv";
				names[31] = "glIsProgram";
				names[32] = "glIsShader";
				names[33] = "glLinkProgram";
				names[34] = "glShaderSource";
				names[35] = "glUseProgram";
				names[36] = "glUniform1f";
				names[37] = "glUniform2f";
				names[38] = "glUniform3f";
				names[39] = "glUniform4f";
				names[40] = "glUniform1i";
				names[41] = "glUniform2i";
				names[42] = "glUniform3i";
				names[43] = "glUniform4i";
				names[44] = "glUniform1fv";
				names[45] = "glUniform2fv";
				names[46] = "glUniform3fv";
				names[47] = "glUniform4fv";
				names[48] = "glUniform1iv";
				names[49] = "glUniform2iv";
				names[50] = "glUniform3iv";
				names[51] = "glUniform4iv";
				names[52] = "glUniformMatrix2fv";
				names[53] = "glUniformMatrix3fv";
				names[54] = "glUniformMatrix4fv";
				names[55] = "glValidateProgram";
				names[56] = "glVertexAttrib1d";
				names[57] = "glVertexAttrib1dv";
				names[58] = "glVertexAttrib1f";
				names[59] = "glVertexAttrib1fv";
				names[60] = "glVertexAttrib1s";
				names[61] = "glVertexAttrib1sv";
				names[62] = "glVertexAttrib2d";
				names[63] = "glVertexAttrib2dv";
				names[64] = "glVertexAttrib2f";
				names[65] = "glVertexAttrib2fv";
				names[66] = "glVertexAttrib2s";
				names[67] = "glVertexAttrib2sv";
				names[68] = "glVertexAttrib3d";
				names[69] = "glVertexAttrib3dv";
				names[70] = "glVertexAttrib3f";
				names[71] = "glVertexAttrib3fv";
				names[72] = "glVertexAttrib3s";
				names[73] = "glVertexAttrib3sv";
				names[74] = "glVertexAttrib4Nbv";
				names[75] = "glVertexAttrib4Niv";
				names[76] = "glVertexAttrib4Nsv";
				names[77] = "glVertexAttrib4Nub";
				names[78] = "glVertexAttrib4Nubv";
				names[79] = "glVertexAttrib4Nuiv";
				names[80] = "glVertexAttrib4Nusv";
				names[81] = "glVertexAttrib4bv";
				names[82] = "glVertexAttrib4d";
				names[83] = "glVertexAttrib4dv";
				names[84] = "glVertexAttrib4f";
				names[85] = "glVertexAttrib4fv";
				names[86] = "glVertexAttrib4iv";
				names[87] = "glVertexAttrib4s";
				names[88] = "glVertexAttrib4sv";
				names[89] = "glVertexAttrib4ubv";
				names[90] = "glVertexAttrib4uiv";
				names[91] = "glVertexAttrib4usv";
				names[92] = "glVertexAttribPointer";
			}

			load_funcs(entries, names);
		}
		/*else
		{
			if (glloader_GL_ARB_shader_objects()
				&& (glloader_GL_ARB_vertex_shader() && glloader_GL_ARB_fragment_shader())
				&& glloader_GL_ARB_shading_language_100()
				&& glloader_GL_ARB_draw_buffers()
				&& glloader_GL_ARB_texture_non_power_of_two()
				&& (glloader_GL_ARB_point_sprite() || glloader_GL_NV_point_sprite())
				&& (glloader_GL_ATI_separate_stencil() || glloader_GL_EXT_stencil_two_side())
				&& glloader_GL_EXT_blend_equation_separate())
			{
				_GL_VERSION_2_0 = true;
				gl_features_extractor::instance().promote("GL_VERSION_2_0");

				glBlendEquationSeparate = glBlendEquationSeparateEXT;
				glDrawBuffers = glDrawBuffersARB;
				glStencilOpSeparate = glStencilOpSeparateATI;
				glStencilFuncSeparate = glStencilFuncSeparateATI;
				glStencilMaskSeparate = glStencilMaskSeparateATI;
				glAttachShader = glAttachObjectARB;
				glBindAttribLocation = glBindAttribLocationARB;
				glCompileShader = glCompileShaderARB;
				glCreateProgram = glCreateProgramObjectARB;
				glCreateShader = glCreateShaderObjectARB;
				glDeleteProgram = glDeleteObjectARB;
				glDeleteShader = glDeleteObjectARB;
				glDetachShader = glDetachObjectARB;
				glDisableVertexAttribArray = glDisableVertexAttribArrayARB;
				glEnableVertexAttribArray = glEnableVertexAttribArrayARB;
				glGetActiveAttrib = glGetActiveAttribARB;
				glGetActiveUniform = glGetActiveUniformARB;
				glGetAttachedShaders = glGetAttachedObjectsARB;
				glGetAttribLocation = glGetAttribLocationARB;
				glGetProgramiv = glGetObjectParameterivARB;
				glGetProgramInfoLog = glGetInfoLogARB;
				glGetShaderiv = glGetObjectParameterivARB;
				glGetShaderInfoLog = glGetInfoLogARB;
				glGetShaderSource = glGetShaderSourceARB;
				glGetUniformLocation = glGetUniformLocationARB;
				glGetUniformfv = glGetUniformfvARB;
				glGetUniformiv = glGetUniformivARB;
				glGetVertexAttribdv = glGetVertexAttribdvARB;
				glGetVertexAttribfv = glGetVertexAttribfvARB;
				glGetVertexAttribiv = glGetVertexAttribivARB;
				glGetVertexAttribPointerv = glGetVertexAttribPointervARB;
				glIsProgram = glIsProgramARB;
				glIsShader = glIsShaderARB;
				glLinkProgram = glLinkProgramARB;
				glShaderSource = glShaderSourceARB;
				glUseProgram = glUseProgramARB;
				glUniform1f = glUniform1fARB;
				glUniform2f = glUniform2fARB;
				glUniform3f = glUniform3fARB;
				glUniform4f = glUniform4fARB;
				glUniform1i = glUniform1iARB;
				glUniform2i = glUniform2iARB;
				glUniform3i = glUniform3iARB;
				glUniform4i = glUniform4iARB;
				glUniform1fv = glUniform1fvARB;
				glUniform2fv = glUniform2fvARB;
				glUniform3fv = glUniform3fvARB;
				glUniform4fv = glUniform4fvARB;
				glUniform1iv = glUniform1ivARB;
				glUniform2iv = glUniform2ivARB;
				glUniform3iv = glUniform3ivARB;
				glUniform4iv = glUniform4ivARB;
				glUniformMatrix2fv = glUniformMatrix2fvARB;
				glUniformMatrix3fv = glUniformMatrix3fvARB;
				glUniformMatrix4fv = glUniformMatrix4fvARB;
				glValidateProgram = glValidateProgramARB;
				glVertexAttrib1d = glVertexAttrib1dARB;
				glVertexAttrib1dv = glVertexAttrib1dvARB;
				glVertexAttrib1f = glVertexAttrib1fARB;
				glVertexAttrib1fv = glVertexAttrib1fvARB;
				glVertexAttrib1s = glVertexAttrib1sARB;
				glVertexAttrib1sv = glVertexAttrib1svARB;
				glVertexAttrib2d = glVertexAttrib2dARB;
				glVertexAttrib2dv = glVertexAttrib2dvARB;
				glVertexAttrib2f = glVertexAttrib2fARB;
				glVertexAttrib2fv = glVertexAttrib2fvARB;
				glVertexAttrib2s = glVertexAttrib2sARB;
				glVertexAttrib2sv = glVertexAttrib2svARB;
				glVertexAttrib3d = glVertexAttrib3dARB;
				glVertexAttrib3dv = glVertexAttrib3dvARB;
				glVertexAttrib3f = glVertexAttrib3fARB;
				glVertexAttrib3fv = glVertexAttrib3fvARB;
				glVertexAttrib3s = glVertexAttrib3sARB;
				glVertexAttrib3sv = glVertexAttrib3svARB;
				glVertexAttrib4Nbv = glVertexAttrib4NbvARB;
				glVertexAttrib4Niv = glVertexAttrib4NivARB;
				glVertexAttrib4Nsv = glVertexAttrib4NsvARB;
				glVertexAttrib4Nub = glVertexAttrib4NubARB;
				glVertexAttrib4Nubv = glVertexAttrib4NubvARB;
				glVertexAttrib4Nuiv = glVertexAttrib4NuivARB;
				glVertexAttrib4Nusv = glVertexAttrib4NusvARB;
				glVertexAttrib4bv = glVertexAttrib4bvARB;
				glVertexAttrib4d = glVertexAttrib4dARB;
				glVertexAttrib4dv = glVertexAttrib4dvARB;
				glVertexAttrib4f = glVertexAttrib4fARB;
				glVertexAttrib4fv = glVertexAttrib4fvARB;
				glVertexAttrib4iv = glVertexAttrib4ivARB;
				glVertexAttrib4s = glVertexAttrib4sARB;
				glVertexAttrib4sv = glVertexAttrib4svARB;
				glVertexAttrib4ubv = glVertexAttrib4ubvARB;
				glVertexAttrib4uiv = glVertexAttrib4uivARB;
				glVertexAttrib4usv = glVertexAttrib4usvARB;
				glVertexAttribPointer = glVertexAttribPointerARB;
			}
		}*/
	}

	char APIENTRY self_init_glloader_GL_VERSION_2_0()
	{
		glloader_GL_VERSION_1_5();

		init_GL_VERSION_2_0();
		return glloader_GL_VERSION_2_0();
	}
}

glloader_GL_VERSION_2_0FUNC glloader_GL_VERSION_2_0 = self_init_glloader_GL_VERSION_2_0;

#ifdef GL_VERSION_2_0

namespace
{
	void APIENTRY self_init_glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
	{
		init_GL_VERSION_2_0();
		return glBlendEquationSeparate(modeRGB, modeAlpha);
	}
	void APIENTRY self_init_glDrawBuffers(GLsizei n, const GLenum* bufs)
	{
		init_GL_VERSION_2_0();
		return glDrawBuffers(n, bufs);
	}
	void APIENTRY self_init_glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
	{
		init_GL_VERSION_2_0();
		return glStencilOpSeparate(face, sfail, dpfail, dppass);
	}
	void APIENTRY self_init_glStencilFuncSeparate(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
	{
		init_GL_VERSION_2_0();
		return glStencilFuncSeparate(frontfunc, backfunc, ref, mask);
	}
	void APIENTRY self_init_glStencilMaskSeparate(GLenum face, GLuint mask)
	{
		init_GL_VERSION_2_0();
		return glStencilMaskSeparate(face, mask);
	}
	void APIENTRY self_init_glAttachShader(GLuint program, GLuint shader)
	{
		init_GL_VERSION_2_0();
		return glAttachShader(program, shader);
	}
	void APIENTRY self_init_glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
	{
		init_GL_VERSION_2_0();
		return glBindAttribLocation(program, index, name);
	}
	void APIENTRY self_init_glCompileShader(GLuint shader)
	{
		init_GL_VERSION_2_0();
		return glCompileShader(shader);
	}
	GLuint APIENTRY self_init_glCreateProgram()
	{
		init_GL_VERSION_2_0();
		return glCreateProgram();
	}
	GLuint APIENTRY self_init_glCreateShader(GLenum type)
	{
		init_GL_VERSION_2_0();
		return glCreateShader(type);
	}
	void APIENTRY self_init_glDeleteProgram(GLuint program)
	{
		init_GL_VERSION_2_0();
		return glDeleteProgram(program);
	}
	void APIENTRY self_init_glDeleteShader(GLuint shader)
	{
		init_GL_VERSION_2_0();
		return glDeleteShader(shader);
	}
	void APIENTRY self_init_glDetachShader(GLuint program, GLuint shader)
	{
		init_GL_VERSION_2_0();
		return glDetachShader(program, shader);
	}
	void APIENTRY self_init_glDisableVertexAttribArray(GLuint index)
	{
		init_GL_VERSION_2_0();
		return glDisableVertexAttribArray(index);
	}
	void APIENTRY self_init_glEnableVertexAttribArray(GLuint index)
	{
		init_GL_VERSION_2_0();
		return glEnableVertexAttribArray(index);
	}
	void APIENTRY self_init_glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
	{
		init_GL_VERSION_2_0();
		return glGetActiveAttrib(program, index, bufSize, length, size, type, name);
	}
	void APIENTRY self_init_glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
	{
		init_GL_VERSION_2_0();
		return glGetActiveUniform(program, index, bufSize, length, size, type, name);
	}
	void APIENTRY self_init_glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shader)
	{
		init_GL_VERSION_2_0();
		return glGetAttachedShaders(program, maxCount, count, shader);
	}
	GLint APIENTRY self_init_glGetAttribLocation(GLuint program, const GLchar* name)
	{
		init_GL_VERSION_2_0();
		return glGetAttribLocation(program, name);
	}
	void APIENTRY self_init_glGetProgramiv(GLuint program, GLenum pname, const GLint* params)
	{
		init_GL_VERSION_2_0();
		return glGetProgramiv(program, pname, params);
	}
	void APIENTRY self_init_glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
	{
		init_GL_VERSION_2_0();
		return glGetProgramInfoLog(program, bufSize, length, infoLog);
	}
	void APIENTRY self_init_glGetShaderiv(GLuint shader, GLenum pname, const GLint* params)
	{
		init_GL_VERSION_2_0();
		return glGetShaderiv(shader, pname, params);
	}
	void APIENTRY self_init_glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
	{
		init_GL_VERSION_2_0();
		return glGetShaderInfoLog(shader, bufSize, length, infoLog);
	}
	void APIENTRY self_init_glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source)
	{
		init_GL_VERSION_2_0();
		return glGetShaderSource(shader, bufSize, length, source);
	}
	GLint APIENTRY self_init_glGetUniformLocation(GLuint program, const GLchar* name)
	{
		init_GL_VERSION_2_0();
		return glGetUniformLocation(program, name);
	}
	void APIENTRY self_init_glGetUniformfv(GLuint program, GLint location, GLfloat* params)
	{
		init_GL_VERSION_2_0();
		return glGetUniformfv(program, location, params);
	}
	void APIENTRY self_init_glGetUniformiv(GLuint program, GLint location, GLint* params)
	{
		init_GL_VERSION_2_0();
		return glGetUniformiv(program, location, params);
	}
	void APIENTRY self_init_glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble* params)
	{
		init_GL_VERSION_2_0();
		return glGetVertexAttribdv(index, pname, params);
	}
	void APIENTRY self_init_glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
	{
		init_GL_VERSION_2_0();
		return glGetVertexAttribfv(index, pname, params);
	}
	void APIENTRY self_init_glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
	{
		init_GL_VERSION_2_0();
		return glGetVertexAttribiv(index, pname, params);
	}
	void APIENTRY self_init_glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer)
	{
		init_GL_VERSION_2_0();
		return glGetVertexAttribPointerv(index, pname, pointer);
	}
	GLboolean APIENTRY self_init_glIsProgram(GLuint program)
	{
		init_GL_VERSION_2_0();
		return glIsProgram(program);
	}
	GLboolean APIENTRY self_init_glIsShader(GLuint shader)
	{
		init_GL_VERSION_2_0();
		return glIsShader(shader);
	}
	void APIENTRY self_init_glLinkProgram(GLuint program)
	{
		init_GL_VERSION_2_0();
		return glLinkProgram(program);
	}
	void APIENTRY self_init_glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
	{
		init_GL_VERSION_2_0();
		return glShaderSource(shader, count, string, length);
	}
	void APIENTRY self_init_glUseProgram(GLuint program)
	{
		init_GL_VERSION_2_0();
		return glUseProgram(program);
	}
	void APIENTRY self_init_glUniform1f(GLint location, GLfloat v0)
	{
		init_GL_VERSION_2_0();
		return glUniform1f(location, v0);
	}
	void APIENTRY self_init_glUniform2f(GLint location, GLfloat v0, GLfloat v1)
	{
		init_GL_VERSION_2_0();
		return glUniform2f(location, v0, v1);
	}
	void APIENTRY self_init_glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
	{
		init_GL_VERSION_2_0();
		return glUniform3f(location, v0, v1, v2);
	}
	void APIENTRY self_init_glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
	{
		init_GL_VERSION_2_0();
		return glUniform4f(location, v0, v1, v2, v3);
	}
	void APIENTRY self_init_glUniform1i(GLint location, GLint v0)
	{
		init_GL_VERSION_2_0();
		return glUniform1i(location, v0);
	}
	void APIENTRY self_init_glUniform2i(GLint location, GLint v0, GLint v1)
	{
		init_GL_VERSION_2_0();
		return glUniform2i(location, v0, v1);
	}
	void APIENTRY self_init_glUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
	{
		init_GL_VERSION_2_0();
		return glUniform3i(location, v0, v1, v2);
	}
	void APIENTRY self_init_glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
	{
		init_GL_VERSION_2_0();
		return glUniform4i(location, v0, v1, v2, v3);
	}
	void APIENTRY self_init_glUniform1fv(GLint location, GLsizei count, const GLfloat* value)
	{
		init_GL_VERSION_2_0();
		return glUniform1fv(location, count, value);
	}
	void APIENTRY self_init_glUniform2fv(GLint location, GLsizei count, const GLfloat* value)
	{
		init_GL_VERSION_2_0();
		return glUniform2fv(location, count, value);
	}
	void APIENTRY self_init_glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
	{
		init_GL_VERSION_2_0();
		return glUniform3fv(location, count, value);
	}
	void APIENTRY self_init_glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
	{
		init_GL_VERSION_2_0();
		return glUniform4fv(location, count, value);
	}
	void APIENTRY self_init_glUniform1iv(GLint location, GLsizei count, const GLint* value)
	{
		init_GL_VERSION_2_0();
		return glUniform1iv(location, count, value);
	}
	void APIENTRY self_init_glUniform2iv(GLint location, GLsizei count, const GLint* value)
	{
		init_GL_VERSION_2_0();
		return glUniform2iv(location, count, value);
	}
	void APIENTRY self_init_glUniform3iv(GLint location, GLsizei count, const GLint* value)
	{
		init_GL_VERSION_2_0();
		return glUniform3iv(location, count, value);
	}
	void APIENTRY self_init_glUniform4iv(GLint location, GLsizei count, const GLint* value)
	{
		init_GL_VERSION_2_0();
		return glUniform4iv(location, count, value);
	}
	void APIENTRY self_init_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
	{
		init_GL_VERSION_2_0();
		return glUniformMatrix2fv(location, count, transpose, value);
	}
	void APIENTRY self_init_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
	{
		init_GL_VERSION_2_0();
		return glUniformMatrix3fv(location, count, transpose, value);
	}
	void APIENTRY self_init_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
	{
		init_GL_VERSION_2_0();
		return glUniformMatrix4fv(location, count, transpose, value);
	}
	void APIENTRY self_init_glValidateProgram(GLuint program)
	{
		init_GL_VERSION_2_0();
		return glValidateProgram(program);
	}
	void APIENTRY self_init_glVertexAttrib1d(GLuint index, GLdouble x)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib1d(index, x);
	}
	void APIENTRY self_init_glVertexAttrib1dv(GLuint index, const GLdouble* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib1dv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib1f(GLuint index, GLfloat x)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib1f(index, x);
	}
	void APIENTRY self_init_glVertexAttrib1fv(GLuint index, const GLfloat* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib1fv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib1s(GLuint index, GLshort x)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib1s(index, x);
	}
	void APIENTRY self_init_glVertexAttrib1sv(GLuint index, const GLshort* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib1sv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib2d(index, x, y);
	}
	void APIENTRY self_init_glVertexAttrib2dv(GLuint index, const GLdouble* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib2dv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib2f(index, x, y);
	}
	void APIENTRY self_init_glVertexAttrib2fv(GLuint index, const GLfloat* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib2fv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib2s(GLuint index, GLshort x, GLshort y)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib2s(index, x, y);
	}
	void APIENTRY self_init_glVertexAttrib2sv(GLuint index, const GLshort* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib2sv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib3d(index, x, y, z);
	}
	void APIENTRY self_init_glVertexAttrib3dv(GLuint index, const GLdouble* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib3dv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib3f(index, x, y, z);
	}
	void APIENTRY self_init_glVertexAttrib3fv(GLuint index, const GLfloat* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib3fv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib3s(index, x, y, z);
	}
	void APIENTRY self_init_glVertexAttrib3sv(GLuint index, const GLshort* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib3sv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4Nbv(GLuint index, const GLbyte* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4Nbv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4Niv(GLuint index, const GLint* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4Niv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4Nsv(GLuint index, const GLshort* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4Nsv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4Nub(index, x, y, z, w);
	}
	void APIENTRY self_init_glVertexAttrib4Nubv(GLuint index, const GLubyte* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4Nubv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4Nuiv(GLuint index, const GLuint* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4Nuiv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4Nusv(GLuint index, const GLushort* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4Nusv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4bv(GLuint index, const GLbyte* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4bv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4d(index, x, y, z, w);
	}
	void APIENTRY self_init_glVertexAttrib4dv(GLuint index, const GLdouble* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4dv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4f(index, x, y, z, w);
	}
	void APIENTRY self_init_glVertexAttrib4fv(GLuint index, const GLfloat* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4fv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4iv(GLuint index, const GLint* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4iv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4s(index, x, y, z, w);
	}
	void APIENTRY self_init_glVertexAttrib4sv(GLuint index, const GLshort* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4sv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4ubv(GLuint index, const GLubyte* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4ubv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4uiv(GLuint index, const GLuint* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4uiv(index, v);
	}
	void APIENTRY self_init_glVertexAttrib4usv(GLuint index, const GLushort* v)
	{
		init_GL_VERSION_2_0();
		return glVertexAttrib4usv(index, v);
	}
	void APIENTRY self_init_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
	{
		init_GL_VERSION_2_0();
		return glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	}
}

glBlendEquationSeparateFUNC glBlendEquationSeparate = self_init_glBlendEquationSeparate;
glDrawBuffersFUNC glDrawBuffers = self_init_glDrawBuffers;
glStencilOpSeparateFUNC glStencilOpSeparate = self_init_glStencilOpSeparate;
glStencilFuncSeparateFUNC glStencilFuncSeparate = self_init_glStencilFuncSeparate;
glStencilMaskSeparateFUNC glStencilMaskSeparate = self_init_glStencilMaskSeparate;
glAttachShaderFUNC glAttachShader = self_init_glAttachShader;
glBindAttribLocationFUNC glBindAttribLocation = self_init_glBindAttribLocation;
glCompileShaderFUNC glCompileShader = self_init_glCompileShader;
glCreateProgramFUNC glCreateProgram = self_init_glCreateProgram;
glCreateShaderFUNC glCreateShader = self_init_glCreateShader;
glDeleteProgramFUNC glDeleteProgram = self_init_glDeleteProgram;
glDeleteShaderFUNC glDeleteShader = self_init_glDeleteShader;
glDetachShaderFUNC glDetachShader = self_init_glDetachShader;
glDisableVertexAttribArrayFUNC glDisableVertexAttribArray = self_init_glDisableVertexAttribArray;
glEnableVertexAttribArrayFUNC glEnableVertexAttribArray = self_init_glEnableVertexAttribArray;
glGetActiveAttribFUNC glGetActiveAttrib = self_init_glGetActiveAttrib;
glGetActiveUniformFUNC glGetActiveUniform = self_init_glGetActiveUniform;
glGetAttachedShadersFUNC glGetAttachedShaders = self_init_glGetAttachedShaders;
glGetAttribLocationFUNC glGetAttribLocation = self_init_glGetAttribLocation;
glGetProgramivFUNC glGetProgramiv = self_init_glGetProgramiv;
glGetProgramInfoLogFUNC glGetProgramInfoLog = self_init_glGetProgramInfoLog;
glGetShaderivFUNC glGetShaderiv = self_init_glGetShaderiv;
glGetShaderInfoLogFUNC glGetShaderInfoLog = self_init_glGetShaderInfoLog;
glGetShaderSourceFUNC glGetShaderSource = self_init_glGetShaderSource;
glGetUniformLocationFUNC glGetUniformLocation = self_init_glGetUniformLocation;
glGetUniformfvFUNC glGetUniformfv = self_init_glGetUniformfv;
glGetUniformivFUNC glGetUniformiv = self_init_glGetUniformiv;
glGetVertexAttribdvFUNC glGetVertexAttribdv = self_init_glGetVertexAttribdv;
glGetVertexAttribfvFUNC glGetVertexAttribfv = self_init_glGetVertexAttribfv;
glGetVertexAttribivFUNC glGetVertexAttribiv = self_init_glGetVertexAttribiv;
glGetVertexAttribPointervFUNC glGetVertexAttribPointerv = self_init_glGetVertexAttribPointerv;
glIsProgramFUNC glIsProgram = self_init_glIsProgram;
glIsShaderFUNC glIsShader = self_init_glIsShader;
glLinkProgramFUNC glLinkProgram = self_init_glLinkProgram;
glShaderSourceFUNC glShaderSource = self_init_glShaderSource;
glUseProgramFUNC glUseProgram = self_init_glUseProgram;
glUniform1fFUNC glUniform1f = self_init_glUniform1f;
glUniform2fFUNC glUniform2f = self_init_glUniform2f;
glUniform3fFUNC glUniform3f = self_init_glUniform3f;
glUniform4fFUNC glUniform4f = self_init_glUniform4f;
glUniform1iFUNC glUniform1i = self_init_glUniform1i;
glUniform2iFUNC glUniform2i = self_init_glUniform2i;
glUniform3iFUNC glUniform3i = self_init_glUniform3i;
glUniform4iFUNC glUniform4i = self_init_glUniform4i;
glUniform1fvFUNC glUniform1fv = self_init_glUniform1fv;
glUniform2fvFUNC glUniform2fv = self_init_glUniform2fv;
glUniform3fvFUNC glUniform3fv = self_init_glUniform3fv;
glUniform4fvFUNC glUniform4fv = self_init_glUniform4fv;
glUniform1ivFUNC glUniform1iv = self_init_glUniform1iv;
glUniform2ivFUNC glUniform2iv = self_init_glUniform2iv;
glUniform3ivFUNC glUniform3iv = self_init_glUniform3iv;
glUniform4ivFUNC glUniform4iv = self_init_glUniform4iv;
glUniformMatrix2fvFUNC glUniformMatrix2fv = self_init_glUniformMatrix2fv;
glUniformMatrix3fvFUNC glUniformMatrix3fv = self_init_glUniformMatrix3fv;
glUniformMatrix4fvFUNC glUniformMatrix4fv = self_init_glUniformMatrix4fv;
glValidateProgramFUNC glValidateProgram = self_init_glValidateProgram;
glVertexAttrib1dFUNC glVertexAttrib1d = self_init_glVertexAttrib1d;
glVertexAttrib1dvFUNC glVertexAttrib1dv = self_init_glVertexAttrib1dv;
glVertexAttrib1fFUNC glVertexAttrib1f = self_init_glVertexAttrib1f;
glVertexAttrib1fvFUNC glVertexAttrib1fv = self_init_glVertexAttrib1fv;
glVertexAttrib1sFUNC glVertexAttrib1s = self_init_glVertexAttrib1s;
glVertexAttrib1svFUNC glVertexAttrib1sv = self_init_glVertexAttrib1sv;
glVertexAttrib2dFUNC glVertexAttrib2d = self_init_glVertexAttrib2d;
glVertexAttrib2dvFUNC glVertexAttrib2dv = self_init_glVertexAttrib2dv;
glVertexAttrib2fFUNC glVertexAttrib2f = self_init_glVertexAttrib2f;
glVertexAttrib2fvFUNC glVertexAttrib2fv = self_init_glVertexAttrib2fv;
glVertexAttrib2sFUNC glVertexAttrib2s = self_init_glVertexAttrib2s;
glVertexAttrib2svFUNC glVertexAttrib2sv = self_init_glVertexAttrib2sv;
glVertexAttrib3dFUNC glVertexAttrib3d = self_init_glVertexAttrib3d;
glVertexAttrib3dvFUNC glVertexAttrib3dv = self_init_glVertexAttrib3dv;
glVertexAttrib3fFUNC glVertexAttrib3f = self_init_glVertexAttrib3f;
glVertexAttrib3fvFUNC glVertexAttrib3fv = self_init_glVertexAttrib3fv;
glVertexAttrib3sFUNC glVertexAttrib3s = self_init_glVertexAttrib3s;
glVertexAttrib3svFUNC glVertexAttrib3sv = self_init_glVertexAttrib3sv;
glVertexAttrib4NbvFUNC glVertexAttrib4Nbv = self_init_glVertexAttrib4Nbv;
glVertexAttrib4NivFUNC glVertexAttrib4Niv = self_init_glVertexAttrib4Niv;
glVertexAttrib4NsvFUNC glVertexAttrib4Nsv = self_init_glVertexAttrib4Nsv;
glVertexAttrib4NubFUNC glVertexAttrib4Nub = self_init_glVertexAttrib4Nub;
glVertexAttrib4NubvFUNC glVertexAttrib4Nubv = self_init_glVertexAttrib4Nubv;
glVertexAttrib4NuivFUNC glVertexAttrib4Nuiv = self_init_glVertexAttrib4Nuiv;
glVertexAttrib4NusvFUNC glVertexAttrib4Nusv = self_init_glVertexAttrib4Nusv;
glVertexAttrib4bvFUNC glVertexAttrib4bv = self_init_glVertexAttrib4bv;
glVertexAttrib4dFUNC glVertexAttrib4d = self_init_glVertexAttrib4d;
glVertexAttrib4dvFUNC glVertexAttrib4dv = self_init_glVertexAttrib4dv;
glVertexAttrib4fFUNC glVertexAttrib4f = self_init_glVertexAttrib4f;
glVertexAttrib4fvFUNC glVertexAttrib4fv = self_init_glVertexAttrib4fv;
glVertexAttrib4ivFUNC glVertexAttrib4iv = self_init_glVertexAttrib4iv;
glVertexAttrib4sFUNC glVertexAttrib4s = self_init_glVertexAttrib4s;
glVertexAttrib4svFUNC glVertexAttrib4sv = self_init_glVertexAttrib4sv;
glVertexAttrib4ubvFUNC glVertexAttrib4ubv = self_init_glVertexAttrib4ubv;
glVertexAttrib4uivFUNC glVertexAttrib4uiv = self_init_glVertexAttrib4uiv;
glVertexAttrib4usvFUNC glVertexAttrib4usv = self_init_glVertexAttrib4usv;
glVertexAttribPointerFUNC glVertexAttribPointer = self_init_glVertexAttribPointer;

#endif		// GL_VERSION_2_0

#endif			// GLLOADER_GL
