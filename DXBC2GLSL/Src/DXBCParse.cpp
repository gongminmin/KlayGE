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

#include <DXBC2GLSL/DXBC.hpp>
#include <memory>

boost::shared_ptr<DXBCContainer> DXBCParse(void const * data)
{
	boost::shared_ptr<DXBCContainer> container(new DXBCContainer);

	DXBCContainerHeader const * header = reinterpret_cast<DXBCContainerHeader const *>(data);
	if (LE32ToNative(header->fourcc) != FOURCC_DXBC)
	{
		return boost::shared_ptr<DXBCContainer>();
	}
	container->shader_chunk = DXBCFindShaderBytecode(data);
	container->input_signature = DXBCFindSignature(data, DFS_INPUT);
	container->resource_chunk = DXBCFindChunk(data, FOURCC_RDEF);
	container->output_signature = DXBCFindSignature(data, DFS_OUTPUT);

	return container;
}

DXBCChunkHeader const * DXBCFindChunk(void const * data, uint32_t fourcc)
{
	DXBCContainerHeader const * header = reinterpret_cast<DXBCContainerHeader const *>(data);
	uint32_t const * chunk_offsets = reinterpret_cast<uint32_t const *>(header + 1);
	if (LE32ToNative(header->fourcc) != FOURCC_DXBC)
	{
		return NULL;
	}
	uint32_t num_chunks = LE32ToNative(header->chunk_count);
	for (uint32_t i = 0; i < num_chunks; ++ i)
	{
		uint32_t offset = LE32ToNative(chunk_offsets[i]);
		DXBCChunkHeader const * chunk = reinterpret_cast<DXBCChunkHeader const *>(reinterpret_cast<char const *>(data)+ offset);
		if (LE32ToNative(chunk->fourcc) == fourcc)
		{
			return chunk;
		}
	}
	return NULL;
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

DXBCChunkSignature const * DXBCFindSignature(void const * data, uint32_t kind)
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

	default:
		return NULL;
	}

	return reinterpret_cast<DXBCChunkSignature const *>(DXBCFindChunk(data, fourcc));
}
