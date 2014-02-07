/**************************************************************************
 *
 * Copyright 2013 Shenghua Lin, Minmin Gong
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef _DXBC2GLSL_HPP
#define _DXBC2GLSL_HPP

#pragma once

#ifdef _DEBUG
	#define DEBUG_SUFFIX "_d"
#else
	#define DEBUG_SUFFIX ""
#endif

#define LIB_FILE_NAME "DXBC2GLSLLib_vc_x86" DEBUG_SUFFIX ".lib"

#pragma comment(lib, LIB_FILE_NAME)
//#pragma message("Linking to lib file: " LIB_FILE_NAME)
#undef LIB_FILE_NAME
#undef DEBUG_SUFFIX

#include <DXBC2GLSL/DXBC.hpp>
#include <DXBC2GLSL/Shader.hpp>
#include <DXBC2GLSL/GLSLGen.hpp>

namespace DXBC2GLSL
{
	class DXBC2GLSL
	{
	public:
		void FeedDXBC(void const * dxbc_data, GLSLVersion version);
		void FeedDXBC(void const * dxbc_data, GLSLVersion version, uint32_t glsl_rules);

		std::string const & GLSLString() const;

		uint32_t NumInputParams() const;
		DXBCSignatureParamDesc const & InputParam(uint32_t index) const;

		uint32_t NumOutputParams() const;
		DXBCSignatureParamDesc const & OutputParam(uint32_t index) const;

		uint32_t NumCBuffers() const;
		uint32_t NumVariables(uint32_t cb_index) const;
		char const * VariableName(uint32_t cb_index, uint32_t var_index) const;
		bool VariableUsed(uint32_t cb_index, uint32_t var_index) const;

		uint32_t NumResources() const;
		char const * ResourceName(uint32_t index) const;
		uint32_t ResourceBindPoint(uint32_t index) const;
		ShaderInputType ResourceType(uint32_t index) const;
		bool ResourceUsed(uint32_t index) const;

	private:
		boost::shared_ptr<DXBCContainer> dxbc_;
		boost::shared_ptr<ShaderProgram> shader_;
		std::string glsl_;
	};
}

#endif		// _DXBC2GLSL_HPP
