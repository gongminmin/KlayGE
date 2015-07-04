/**
 * @file DXBCParse.cpp
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

#include <KFL/KFL.hpp>
#include <DXBC2GLSL/DXBC.hpp>
#include <memory>

std::shared_ptr<DXBCContainer> DXBCParse(void const * data)
{
	std::shared_ptr<DXBCContainer> container = KlayGE::MakeSharedPtr<DXBCContainer>();

	DXBCContainerHeader const * header = reinterpret_cast<DXBCContainerHeader const *>(data);
	uint32_t fourcc = KlayGE::LE2Native(header->fourcc);
	if (fourcc != FOURCC_DXBC)
	{
		return std::shared_ptr<DXBCContainer>();
	}
	container->shader_chunk = DXBCFindShaderBytecode(data);
	container->input_signature = DXBCFindSignature(data, DFS_INPUT1);
	if (!container->input_signature)
	{
		container->input_signature = DXBCFindSignature(data, DFS_INPUT);
	}
	container->resource_chunk = DXBCFindChunk(data, FOURCC_RDEF);
	container->output_signature = DXBCFindSignature(data, DFS_OUTPUT1);
	if (!container->output_signature)
	{
		container->output_signature = DXBCFindSignature(data, DFS_OUTPUT5);
		if (!container->output_signature)
		{
			container->output_signature = DXBCFindSignature(data, DFS_OUTPUT);
		}
	}
	container->patch_constant_signature = DXBCFindSignature(data, DFS_PATCH);

	return container;
}

DXBCChunkHeader const * DXBCFindChunk(void const * data, uint32_t fourcc)
{
	DXBCContainerHeader const * header = reinterpret_cast<DXBCContainerHeader const *>(data);
	uint32_t const * chunk_offsets = reinterpret_cast<uint32_t const *>(header + 1);
	uint32_t header_fourcc = KlayGE::LE2Native(header->fourcc);
	if (header_fourcc != FOURCC_DXBC)
	{
		return nullptr;
	}
	uint32_t num_chunks = KlayGE::LE2Native(header->chunk_count);
	for (uint32_t i = 0; i < num_chunks; ++ i)
	{
		uint32_t offset = KlayGE::LE2Native(chunk_offsets[i]);
		DXBCChunkHeader const * chunk = reinterpret_cast<DXBCChunkHeader const *>(reinterpret_cast<char const *>(data) + offset);
		if (KlayGE::LE2Native(chunk->fourcc) == fourcc)
		{
			return chunk;
		}
	}
	return nullptr;
}

DXBCChunkHeader const * DXBCFindShaderBytecode(void const * data)
{
	DXBCChunkHeader const * chunk;
	chunk = DXBCFindChunk(data, FOURCC_SHDR);
	if (!chunk)
	{
		chunk = DXBCFindChunk(data, FOURCC_SHEX);
	}
	return chunk;
}

DXBCChunkSignatureHeader const * DXBCFindSignature(void const * data, uint32_t kind)
{
	uint32_t fourcc;
	switch (kind)
	{
	case DFS_INPUT:
		fourcc = FOURCC_ISGN;
		break;

	case DFS_OUTPUT:
		fourcc = FOURCC_OSGN;
		break;

	case DFS_PATCH:
		fourcc = FOURCC_PCSG;
		break;

	case DFS_INPUT1:
		fourcc = FOURCC_ISG1;
		break;

	case DFS_OUTPUT5:
		fourcc = FOURCC_OSG5;
		break;

	case DFS_OUTPUT1:
		fourcc = FOURCC_OSG1;
		break;

	default:
		return nullptr;
	}

	return reinterpret_cast<DXBCChunkSignatureHeader const *>(DXBCFindChunk(data, fourcc));
}
