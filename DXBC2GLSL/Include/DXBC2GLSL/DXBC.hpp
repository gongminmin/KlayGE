/**************************************************************************
 *
 * Copyright 2013 Shenghua Lin, Minmin Gong
 * Copyright 2010 Luca Barbieri
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

#ifndef _DXBC2GLSL_DXBC_HPP
#define _DXBC2GLSL_DXBC_HPP

#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>
#include <DXBC2GLSL/ShaderDefs.hpp>
#include <DXBC2GLSL/Utils.hpp>

#define FOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24 ))
#define FOURCC_DXBC FOURCC('D', 'X', 'B', 'C')
#define FOURCC_RDEF FOURCC('R', 'D', 'E', 'F') // resource definition
#define FOURCC_ISGN FOURCC('I', 'S', 'G', 'N') // input signature
#define FOURCC_OSGN FOURCC('O', 'S', 'G', 'N') // output signature
#define FOURCC_SHDR FOURCC('S', 'H', 'D', 'R') // ?
#define FOURCC_SHEX FOURCC('S', 'H', 'E', 'X') // shader extension
#define FOURCC_STAT FOURCC('S', 'T', 'A', 'T') // ?
#define FOURCC_PCSG FOURCC('P', 'C', 'S', 'G') // patch signature

uint32_t const MAX_DXBC_STRING_LENGTH = 512;

#pragma pack(push, 1)
// this is always little-endian!
struct DXBCChunkHeader
{
	uint32_t fourcc;
	uint32_t size;
};

// this is always little-endian!
struct DXBCChunkSignature : public DXBCChunkHeader
{
	uint32_t count;
	uint32_t unk;
#pragma warning(push)
#pragma warning(disable: 4200)
	struct
	{
		uint32_t name_offset;
		uint32_t semantic_index;
		uint32_t system_value_type;
		uint32_t component_type;
		uint32_t register_num;
		uint8_t mask;
		uint8_t read_write_mask;//fxc asm 中没有
		uint8_t stream; // TODO: guess!fxc asm中没有
		uint8_t unused;
	} elements[];
#pragma warning(pop)
};
#pragma pack(pop)

// Same layout with D3D11_SHADER_VARIABLE_DESC
struct DXBCShaderVariableDesc
{
	char const* name;
	uint32_t start_offset;
	uint32_t size;
	uint32_t flags;
	void const * default_val;
	uint32_t start_texture;
	uint32_t texture_size;
	uint32_t start_sampler;
	uint32_t sampler_size;
};

// Same layout with D3D11_SHADER_TYPE_DESC
struct DXBCShaderTypeDesc
{
	ShaderVariableClass var_class;
	ShaderVariableType type;
	uint32_t rows;
	uint32_t columns;
	uint32_t elements;
	uint32_t members;
	uint32_t offset;
	char const * name;
};

// Same layout with D3D11_SHADER_BUFFER_DESC
struct DXBCShaderBufferDesc
{
	char const * name;
	ShaderCBufferType type;
	uint32_t variables;
	uint32_t size;
	uint32_t flags;
};

struct DXBCShaderVariable
{
	DXBCShaderVariableDesc var_desc;
	DXBCShaderTypeDesc type_desc;
	bool has_type_desc;
};

struct DXBCConstantBuffer
{
	DXBCShaderBufferDesc desc;
	std::vector<DXBCShaderVariable> vars;
	uint32_t bind_point;//cb register number.this is used to map between cb# and cb member variable name
};

// Same layout with D3D11_SIGNATURE_PARAMETER_DESC
struct DXBCSignatureParamDesc
{
	char const * semantic_name;
	uint32_t semantic_index;
	uint32_t register_index;
	ShaderName system_value_type;
	ShaderRegisterComponentType component_type;
	uint8_t mask;
	uint8_t read_write_mask;
	uint32_t stream;
};

// Same layout with D3D11_SHADER_INPUT_BIND_DESC
struct DXBCInputBindDesc
{
	char const * name;
	ShaderInputType type;
	uint32_t bind_point;
	uint32_t bind_count;
	
	uint32_t flags;
	ShaderResourceReturnType return_type;
	ShaderSRVDimension dimension;
	uint32_t num_samples;
};


struct DXBCContainer
{
	DXBCChunkHeader const * shader_chunk;
	DXBCChunkHeader const * resource_chunk;
	DXBCChunkHeader const * input_signature;
	DXBCChunkHeader const * output_signature;
};

struct DXBCContainerHeader
{
	uint32_t fourcc;
	uint32_t unk[4];
	uint32_t one;
	uint32_t total_size;
	uint32_t chunk_count;
};

enum DXBCFindSignature
{
	DFS_INPUT = 0,
	DFS_OUTPUT,
	DFS_PATCH
};

boost::shared_ptr<DXBCContainer> DXBCParse(void const * data);
DXBCChunkHeader const * DXBCFindChunk(void const * data, uint32_t fourcc);
DXBCChunkHeader const * DXBCFindShaderBytecode(void const * data);
DXBCChunkSignature const * DXBCFindSignature(void const * data, uint32_t kind);

#endif		// _DXBC2GLSL_DXBC_HPP
