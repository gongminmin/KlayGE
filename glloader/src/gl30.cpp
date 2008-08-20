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
#include <glloader/gl30.h>
#include <glloader/gl21.h>
#include "utils.hpp"

#ifdef GLLOADER_GL

using glloader::load_funcs;
using glloader::gl_features_extractor;

namespace
{
	bool _GL_VERSION_3_0 = false;

	char APIENTRY _glloader_GL_VERSION_3_0()
	{
		return _GL_VERSION_3_0;
	}

	void init_GL_VERSION_3_0()
	{
		glloader_GL_VERSION_3_0 = _glloader_GL_VERSION_3_0;

		{
			glMapBufferRange = NULL;
			glFlushMappedBufferRange = NULL;
		}

		if (glloader_is_supported("GL_VERSION_3_0"))
		{
			_GL_VERSION_3_0 = true;

			entries_t entries(2);
			{
				entries[0] = reinterpret_cast<void**>(&glMapBufferRange);
				entries[1] = reinterpret_cast<void**>(&glFlushMappedBufferRange);
			}

			funcs_names_t names(6);
			{
				names[0] = "glMapBufferRange";
				names[1] = "glFlushMappedBufferRange";
			}

			load_funcs(entries, names);
		}
		else
		{
			if (glloader_GL_ARB_depth_buffer_float()
				&& glloader_GL_ARB_half_float_vertex()
				&& glloader_GL_ARB_map_buffer_range()
				&& glloader_GL_ARB_texture_compression_rgtc()
				&& glloader_GL_ARB_texture_rg()
				&& glloader_GL_ARB_vertex_array_object())
			{
				_GL_VERSION_3_0 = true;
			}
		}
	}

	char APIENTRY self_init_glloader_GL_VERSION_3_0()
	{
		glloader_GL_VERSION_2_1();

		init_GL_VERSION_3_0();
		return glloader_GL_VERSION_3_0();
	}
}

glloader_GL_VERSION_3_0FUNC glloader_GL_VERSION_3_0 = self_init_glloader_GL_VERSION_3_0;

#ifdef GL_VERSION_3_0

namespace
{
	void APIENTRY self_init_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
	{
		init_GL_VERSION_3_0();
		return glMapBufferRange(target, offset, length, access);
	}
	void APIENTRY self_init_glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
	{
		init_GL_VERSION_3_0();
		return glFlushMappedBufferRange(target, offset, length);
	}
}

glMapBufferRangeFUNC glMapBufferRange = self_init_glMapBufferRange;
glFlushMappedBufferRangeFUNC glFlushMappedBufferRange = self_init_glFlushMappedBufferRange;

#endif		// GL_VERSION_2_1

#endif			// GLLOADER_GL
