#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/Mesh.hpp>

namespace KlayGE
{
	Mesh::Mesh()
		: vb_(new VertexBuffer)
	{
	}

	const WString& Mesh::Name() const
	{
		static WString name(L"Mesh");
		return name;
	}

	VertexBufferPtr Mesh::GetVertexBuffer()
	{
		vb_->type					= VertexBuffer::BT_TriangleList;
		vb_->vertexOptions			= VertexBuffer::VO_Normals | VertexBuffer::VO_TextureCoords;
		vb_->numVertices			= static_cast<U32>(xyzs_.size() / 3);
		vb_->pVertices				= &xyzs_[0];
		vb_->pNormals				= &normals_[0];
		vb_->pIndices				= &indices_[0];
		vb_->pTexCoords[0]			= &textures_[0];
		vb_->numTextureCoordSets	= 1;
		vb_->numTextureDimensions[0] = 2;
		vb_->numIndices				= static_cast<U32>(indices_.size());
		vb_->useIndices				= !indices_.empty();

		return vb_;
	}

	void Mesh::ComputeNormals()
	{
		MathLib& math(Engine::MathInstance());

		if (indices_.empty())
		{
			for (std::vector<U16>::iterator iter = indices_.begin(); iter != indices_.end(); iter += 3)
			{
				Vector3* v0(reinterpret_cast<Vector3*>(&xyzs_[*(iter + 0) * 3]));
				Vector3* v1(reinterpret_cast<Vector3*>(&xyzs_[*(iter + 1) * 3]));
				Vector3* v2(reinterpret_cast<Vector3*>(&xyzs_[*(iter + 2) * 3]));

				Vector3 vec;
				math.Cross(vec, *v1 - *v0, *v2 - *v0);

				Vector3* n0(reinterpret_cast<Vector3*>(&normals_[*(iter + 0) * 3]));
				Vector3* n1(reinterpret_cast<Vector3*>(&normals_[*(iter + 1) * 3]));
				Vector3* n2(reinterpret_cast<Vector3*>(&normals_[*(iter + 2) * 3]));

				*n0 += vec;
				*n1 += vec;
				*n2 += vec;
			}

			for (std::vector<float>::iterator iter = normals_.begin(); iter != normals_.end(); iter += 3)
			{
				Vector3* normal(reinterpret_cast<Vector3*>(&(*iter)));
				math.Normalize(*normal, *normal);
			}
		}
		else
		{
			for (size_t i = 0; i < xyzs_.size(); i += 9)
			{
				Vector3* v0(reinterpret_cast<Vector3*>(&xyzs_[i + 0]));
				Vector3* v1(reinterpret_cast<Vector3*>(&xyzs_[i + 3]));
				Vector3* v2(reinterpret_cast<Vector3*>(&xyzs_[i + 6]));

				Vector3 vec;
				math.Cross(vec, *v1 - *v0, *v2 - *v0);

				Vector3* n0(reinterpret_cast<Vector3*>(&normals_[i + 0]));
				Vector3* n1(reinterpret_cast<Vector3*>(&normals_[i + 3]));
				Vector3* n2(reinterpret_cast<Vector3*>(&normals_[i + 6]));

				*n0 += vec;
				*n1 += vec;
				*n2 += vec;
			}

			for (size_t i = 0; i < normals_.size(); i += 3)
			{
				Vector3* normal(reinterpret_cast<Vector3*>(&normals_[i]));
				math.Normalize(*normal, *normal);
			}
		}
	}
}
