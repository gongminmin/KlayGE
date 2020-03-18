/**
 * @file DXBC.hpp
 * @author Shenghua Lin, Minmin Gong, Luca Barbieri
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
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
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _DXBC2GLSL_DXBC_HPP
#define _DXBC2GLSL_DXBC_HPP

#pragma once

#include <KFL/KFL.hpp>
#include <vector>
#include <DXBC2GLSL/ShaderDefs.hpp>
#include <DXBC2GLSL/Utils.hpp>

#define FOURCC_DXBC (KlayGE::MakeFourCC<'D', 'X', 'B', 'C'>::value) // DirectX byte code
#define FOURCC_RDEF (KlayGE::MakeFourCC<'R', 'D', 'E', 'F'>::value) // Resource definition
#define FOURCC_ISGN (KlayGE::MakeFourCC<'I', 'S', 'G', 'N'>::value) // Input signature
#define FOURCC_OSGN (KlayGE::MakeFourCC<'O', 'S', 'G', 'N'>::value) // Output signature
#define FOURCC_SHDR (KlayGE::MakeFourCC<'S', 'H', 'D', 'R'>::value) // Shader model 4 code
#define FOURCC_SHEX (KlayGE::MakeFourCC<'S', 'H', 'E', 'X'>::value) // Shader model 5 code
#define FOURCC_PCSG (KlayGE::MakeFourCC<'P', 'C', 'S', 'G'>::value) // Patch signature
#define FOURCC_IFCH (KlayGE::MakeFourCC<'I', 'F', 'C', 'E'>::value) // Interface (for dynamic linking)
#define FOURCC_OSG5 (KlayGE::MakeFourCC<'O', 'S', 'G', '5'>::value) // Input signature in shader model 5
#define FOURCC_ISG1 (KlayGE::MakeFourCC<'I', 'S', 'G', '1'>::value) // Input signature with Stream and MinPrecision in D3D 11.1
#define FOURCC_OSG1 (KlayGE::MakeFourCC<'O', 'S', 'G', '1'>::value) // Output signature with Stream and MinPrecision in D3D 11.1
#define FOURCC_PSG1 (KlayGE::MakeFourCC<'P', 'S', 'G', '1'>::value) // Patch signature in D3D 11.1

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
// this is always little-endian!
struct DXBCChunkHeader
{
	uint32_t fourcc;
	uint32_t size;
};
KLAYGE_STATIC_ASSERT(sizeof(DXBCChunkHeader) == 8);

// this is always little-endian!
struct DXBCChunkSignatureHeader : public DXBCChunkHeader
{
	uint32_t count;
	uint32_t offset;
};
KLAYGE_STATIC_ASSERT(sizeof(DXBCChunkSignatureHeader) == 16);
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

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

struct DXBCShaderMemberDesc;

// Similar to D3D11_SHADER_TYPE_DESC, except member_desc
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

	std::vector<DXBCShaderMemberDesc> member_desc;
};

struct DXBCShaderMemberDesc
{
	char const* name;
	uint32_t start_offset;
	DXBCShaderTypeDesc type;
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
	uint32_t min_precision;
};

enum DXBCShaderInputFlags
{
	DSIF_UserPacked = 0x1,
	DSIF_ComparisonSampler = 0x2,
	DSIF_TextureComponent0 = 0x4,
	DSIF_TextureComponent1 = 0x8,
	DSIF_TextureComponents = 0xC,
	DSIF_Unused = 0x10,
	DSIF_ForceDWORD = 0x7FFFFFFF
};

// Same layout with D3D11_SHADER_INPUT_BIND_DESC
struct DXBCInputBindDesc
{
	char const * name;
	ShaderInputType type;
	uint32_t bind_point;
	uint32_t bind_count;
	
	uint32_t flags; // Combination of DXBCShaderInputFlags
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
	DXBCChunkHeader const * patch_constant_signature;
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
	DFS_PATCH,
	DFS_INPUT1,
	DFS_OUTPUT5,
	DFS_OUTPUT1
};

std::shared_ptr<DXBCContainer> DXBCParse(void const * data);
DXBCChunkHeader const * DXBCFindChunk(void const * data, uint32_t fourcc);
DXBCChunkHeader const * DXBCFindShaderBytecode(void const * data);
DXBCChunkSignatureHeader const * DXBCFindSignature(void const * data, uint32_t kind);

#endif		// _DXBC2GLSL_DXBC_HPP
