// VertexBuffer.hpp
// KlayGE VertexBuffer类 头文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 去掉了VO_2D (2004.3.1)
//
// 2.0.0
// 初次建立 (2003.8.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _VERTEXBUFFER_HPP
#define _VERTEXBUFFER_HPP

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	class VertexBuffer
	{
	public:
		enum BufferType
		{
			BT_PointList,
			BT_LineList,
			BT_LineStrip,
			BT_TriangleList,
			BT_TriangleStrip,
			BT_TriangleFan
		};
		
		enum VertexOptions
		{
			// vertex normals included (for lighting)
			VO_Normals			= 1 << 0,
			// Vertex colors - diffuse
			VO_Diffuses			= 1 << 1,
			// Vertex colors - specular
			VO_Speculars		= 1 << 2,
			// at least one set of texture coords (exact number specified in class)
			VO_TextureCoords	= 1 << 3,
			// Vertex blend weights
			VO_BlendWeights		= 1 << 4,
			// Vertex blend indices
			VO_BlendIndices		= 1 << 5,
		};

		// true to use pIndexes to reference individual lines/triangles rather than embed. Allows vertex reuse.
		bool useIndices;

		// Number of vertices (applies to all components)
		U32 numVertices;

		// No memory allocation here,
		// assumed that all pointers are pointing
		// elsewhere e.g. model class data

		// Pointer to list of vertices (float {x, y, z} * numVertices).
		// @remarks
		// If useIndexes is false each group of 3 vertices describes a face (anticlockwise winding) in
		// trianglelist mode.
		float* pVertices;
		// The 'Stride' between sets of vertex data. 0 indicates data is packed with no gaps.
		U16 vertexStride;


		// Optional vertex normals for vertices (float {x, y, z} * numVertices).
		float* pNormals;
		// The 'Stride' between sets of normal data. 0 indicates data is packed with no gaps.
		U16 normalStride;


		// Optional pointer to a list of diffuse vertex colors (32-bit RGBA * numVertices).
		U32* pDiffuses;
		// The 'Stride' between sets of diffuse color data. 0 indicates data is packed with no gaps.
		U16 diffuseStride;


		// Optional pointer to a list of specular vertex colors (32-bit RGBA * numVertices)
		U32* pSpeculars;
		// The 'Stride' between sets of specular color data. 0 indicates data is packed with no gaps.
		U16 specularStride;


		/// Number of groups of u,[v],[w].
		U8 numTextureCoordSets;

		// Number of dimensions in each corresponding texture coordinate set.
		// @note
		// There should be 1-4 dimensions on each set.
		U8 numTextureDimensions[8];

		// Optional texture coordinates for vertices (float {u, [v], [w]} * numVertices).
		// @remarks
		// There can be up to 8 sets of texture coordinates, and the number of components per
		// vertex depends on the number of texture dimensions (2 is most common).
		float* pTexCoords[8];
		// The 'Stride' between each set of texture data. 0 indicates data is packed with no gaps.
		U16 texCoordStride[8];


		float* pBlendWeights;
		U16 blendWeightsStride;


		U8* pBlendIndices;
		U16 blendIndicesStride;


		// Pointer to a list of vertex indexes describing faces (only used if useIndexes is true).
		// @note
		// Each group of 3 describes a face (anticlockwise winding order).
		U16* pIndices;

		// The number of vertex indexes (must be a multiple of 3).
		U32 numIndices;

		// The type of rendering operation.
		BufferType type;
		// Flags indicating vertex types
		int vertexOptions;

		VertexBuffer()
		{
			useIndices = false;

			// Initialise all things
			vertexStride = normalStride = diffuseStride = specularStride = blendWeightsStride = blendIndicesStride = 0;
			vertexOptions = 0;

			pVertices = NULL;
			pNormals = NULL;
			pDiffuses = NULL;
			pSpeculars = NULL;
			pBlendWeights = NULL;
			pBlendIndices = NULL;

			pIndices = NULL;

			numTextureCoordSets = 1;
			for (size_t i = 0; i < 8; ++ i)
			{
				pTexCoords[i] = NULL;
				texCoordStride[i] = 0;
				numTextureDimensions[i] = 2;
			}
		}
	};
}

#endif		// _VERTEXBUFFER_HPP
