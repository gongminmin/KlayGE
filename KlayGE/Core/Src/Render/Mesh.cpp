#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/Mesh.hpp>

namespace KlayGE
{
	Mesh::Mesh()
		: vb_(new VertexBuffer)
	{
		vb_->type					= VertexBuffer::BT_TriangleList;
		vb_->vertexOptions			= VertexBuffer::VO_Normals | VertexBuffer::VO_TextureCoords;
		vb_->numTextureCoordSets	= 1;
		vb_->texCoordSets[0].first	= 2;
	}

	const WString& Mesh::Name() const
	{
		static WString name(L"Mesh");
		return name;
	}

	VertexBufferPtr Mesh::GetVertexBuffer()
	{
		return vb_;
	}

	void Mesh::ComputeNormals()
	{
		MathLib& math(Engine::MathInstance());

		if (vb_->indices.empty())
		{
			for (VertexBuffer::IndicesType::iterator iter = vb_->indices.begin(); iter != vb_->indices.end(); iter += 3)
			{
				Vector3* v0(reinterpret_cast<Vector3*>(&vb_->vertices[*(iter + 0) * 3]));
				Vector3* v1(reinterpret_cast<Vector3*>(&vb_->vertices[*(iter + 1) * 3]));
				Vector3* v2(reinterpret_cast<Vector3*>(&vb_->vertices[*(iter + 2) * 3]));

				Vector3 vec;
				math.Cross(vec, *v1 - *v0, *v2 - *v0);

				Vector3* n0(reinterpret_cast<Vector3*>(&vb_->normals[*(iter + 0) * 3]));
				Vector3* n1(reinterpret_cast<Vector3*>(&vb_->normals[*(iter + 1) * 3]));
				Vector3* n2(reinterpret_cast<Vector3*>(&vb_->normals[*(iter + 2) * 3]));

				*n0 += vec;
				*n1 += vec;
				*n2 += vec;
			}

			for (VertexBuffer::NormalsType::iterator iter = vb_->normals.begin(); iter != vb_->normals.end(); iter += 3)
			{
				Vector3* normal(reinterpret_cast<Vector3*>(&(*iter)));
				math.Normalize(*normal, *normal);
			}
		}
		else
		{
			for (size_t i = 0; i < vb_->vertices.size(); i += 9)
			{
				Vector3* v0(reinterpret_cast<Vector3*>(&vb_->vertices[i + 0]));
				Vector3* v1(reinterpret_cast<Vector3*>(&vb_->vertices[i + 3]));
				Vector3* v2(reinterpret_cast<Vector3*>(&vb_->vertices[i + 6]));

				Vector3 vec;
				math.Cross(vec, *v1 - *v0, *v2 - *v0);

				Vector3* n0(reinterpret_cast<Vector3*>(&vb_->normals[i + 0]));
				Vector3* n1(reinterpret_cast<Vector3*>(&vb_->normals[i + 3]));
				Vector3* n2(reinterpret_cast<Vector3*>(&vb_->normals[i + 6]));

				*n0 += vec;
				*n1 += vec;
				*n2 += vec;
			}

			for (size_t i = 0; i < vb_->normals.size(); i += 3)
			{
				Vector3* normal(reinterpret_cast<Vector3*>(&vb_->normals[i]));
				math.Normalize(*normal, *normal);
			}
		}
	}
}
