// VertexBuffer.hpp
// KlayGE VertexBuffer类 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 修改了纹理坐标 (2004.3.16)
//
// 2.0.3
// 去掉了VO_2D (2004.3.1)
// 改用vector存放数据 (2004.3.13)
//
// 2.0.0
// 初次建立 (2003.8.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _VERTEXBUFFER_HPP
#define _VERTEXBUFFER_HPP

#pragma comment(lib, "KlayGE_Core.lib")

#include <vector>
#include <KlayGE/array.hpp>

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

		bool UseIndices() const
			{ return !indices.empty(); }

		// Number of vertices (applies to all components)
		size_t NumVertices() const
			{ return vertices.size() / 3; }

		// No memory allocation here,
		// assumed that all pointers are pointing
		// elsewhere e.g. model class data

		// Pointer to list of vertices (float {x, y, z} * numVertices).
		// @remarks
		// If useIndexes is false each group of 3 vertices describes a face (anticlockwise winding) in
		// trianglelist mode.
		typedef std::vector<float, alloc<float> > VerticesType;
		VerticesType vertices;


		// Optional vertex normals for vertices (float {x, y, z} * numVertices).
		typedef std::vector<float, alloc<float> > NormalsType;
		NormalsType normals;


		// Optional pointer to a list of diffuse vertex colors (float {r, g, b, a} * numVertices).
		typedef std::vector<float, alloc<float> > DiffusesType;
		DiffusesType diffuses;


		// Optional pointer to a list of specular vertex colors (float {r, g, b, a} RGBA * numVertices)
		typedef std::vector<float, alloc<float> > SpecularsType;
		SpecularsType speculars;


		/// Number of groups of u,[v],[w].
		U8 numTextureCoordSets;

		// Number of dimensions in each corresponding texture coordinate set.
		// @note
		// There should be 1-4 dimensions on each set.

		// Optional texture coordinates for vertices (float {u, [v], [w]} * numVertices).
		// @remarks
		// There can be up to 8 sets of texture coordinates, and the number of components per
		// vertex depends on the number of texture dimensions (2 is most common).
		typedef std::vector<float, alloc<float> > TexCoordsType;
		typedef array<std::pair<U8, TexCoordsType>, 8> TexCoordSetsType;
		TexCoordSetsType texCoordSets;


		typedef std::vector<float, alloc<float> > BlendWeightsType;
		BlendWeightsType blendWeights;


		typedef std::vector<float, alloc<float> > BlendIndicesType;
		BlendIndicesType blendIndices;


		// Pointer to a list of vertex indexes describing faces (only used if useIndexes is true).
		// @note
		// Each group of 3 describes a face (anticlockwise winding order).
		typedef std::vector<U16, alloc<U16> > IndicesType;
		IndicesType indices;

		// The number of vertex indexes
		size_t NumIndices() const
			{ return indices.size(); }

		// The type of rendering operation.
		BufferType type;
		// Flags indicating vertex types
		int vertexOptions;

		VertexBuffer()
			: vertexOptions(0),
				numTextureCoordSets(1),
				texCoordSets(TexCoordSetsType::value_type(2, TexCoordsType()))
			{ }
	};
}

#endif		// _VERTEXBUFFER_HPP
