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

#include <KFL/KFL.hpp>
#include <DXBC2GLSL/DXBC.hpp>
#include <memory>

KlayGE::shared_ptr<DXBCContainer> DXBCParse(void const * data)
{
	KlayGE::shared_ptr<DXBCContainer> container = KlayGE::MakeSharedPtr<DXBCContainer>();

	DXBCContainerHeader const * header = reinterpret_cast<DXBCContainerHeader const *>(data);
	uint32_t fourcc = header->fourcc;
	KlayGE::LittleEndianToNative<sizeof(fourcc)>(&fourcc);
	if (fourcc != FOURCC_DXBC)
	{
		return KlayGE::shared_ptr<DXBCContainer>();
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
	uint32_t header_fourcc = header->fourcc;
	KlayGE::LittleEndianToNative<sizeof(header_fourcc)>(&header_fourcc);
	if (header_fourcc != FOURCC_DXBC)
	{
		return nullptr;
	}
	uint32_t num_chunks = header->chunk_count;
	KlayGE::LittleEndianToNative<sizeof(num_chunks)>(&num_chunks);
	for (uint32_t i = 0; i < num_chunks; ++ i)
	{
		uint32_t offset = chunk_offsets[i];
		KlayGE::LittleEndianToNative<sizeof(offset)>(&offset);
		DXBCChunkHeader const * chunk = reinterpret_cast<DXBCChunkHeader const *>(reinterpret_cast<char const *>(data) + offset);
		uint32_t chunk_fourcc = chunk->fourcc;
		KlayGE::LittleEndianToNative<sizeof(chunk_fourcc)>(&chunk_fourcc);
		if (chunk_fourcc == fourcc)
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
