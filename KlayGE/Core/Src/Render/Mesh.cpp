#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/Mesh.hpp>

namespace KlayGE
{
	Mesh::Mesh()
		: vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
	{
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

		if (vb_->UseIndices())
		{
			VertexStreamPtr vstream(vb_->GetVertexStream(VST_Positions));

			std::vector<float, alloc<float> > vertices(vstream->VertexNum() * 3);
			vstream->CopyTo(&vertices[0], vstream->VertexNum());

			std::vector<float, alloc<float> > normals(vertices.size(), 0);

			IndexStreamPtr istream(vb_->GetIndexStream());
			std::vector<U16, alloc<U16> > indices(istream->IndexNum());
			istream->CopyTo(&indices[0], istream->IndexNum());

			for (std::vector<U16, alloc<U16> >::iterator iter = indices.begin(); iter != indices.end(); iter += 3)
			{
				Vector3* v0(reinterpret_cast<Vector3*>(&vertices[*(iter + 0) * 3]));
				Vector3* v1(reinterpret_cast<Vector3*>(&vertices[*(iter + 1) * 3]));
				Vector3* v2(reinterpret_cast<Vector3*>(&vertices[*(iter + 2) * 3]));

				Vector3 vec;
				math.Cross(vec, *v1 - *v0, *v2 - *v0);

				Vector3* n0(reinterpret_cast<Vector3*>(&normals[*(iter + 0) * 3]));
				Vector3* n1(reinterpret_cast<Vector3*>(&normals[*(iter + 1) * 3]));
				Vector3* n2(reinterpret_cast<Vector3*>(&normals[*(iter + 2) * 3]));

				*n0 += vec;
				*n1 += vec;
				*n2 += vec;
			}

			for (std::vector<float, alloc<float> >::iterator iter = normals.begin(); iter != normals.end(); iter += 3)
			{
				Vector3* normal(reinterpret_cast<Vector3*>(&(*iter)));
				math.Normalize(*normal, *normal);
			}

			vb_->GetVertexStream(VST_Normals)->Assign(&normals[0], normals.size() / 3);
		}
		else
		{
			VertexStreamPtr stream(vb_->GetVertexStream(VST_Positions));

			std::vector<float, alloc<float> > vertices(stream->VertexNum() * 3);
			stream->CopyTo(&vertices[0], stream->VertexNum());

			std::vector<float, alloc<float> > normals(vertices.size(), 0);

			for (size_t i = 0; i < vertices.size(); i += 9)
			{
				Vector3* v0(reinterpret_cast<Vector3*>(&vertices[i + 0]));
				Vector3* v1(reinterpret_cast<Vector3*>(&vertices[i + 3]));
				Vector3* v2(reinterpret_cast<Vector3*>(&vertices[i + 6]));

				Vector3 vec;
				math.Cross(vec, *v1 - *v0, *v2 - *v0);

				Vector3* n0(reinterpret_cast<Vector3*>(&normals[i + 0]));
				Vector3* n1(reinterpret_cast<Vector3*>(&normals[i + 3]));
				Vector3* n2(reinterpret_cast<Vector3*>(&normals[i + 6]));

				*n0 += vec;
				*n1 += vec;
				*n2 += vec;
			}

			for (size_t i = 0; i < normals.size(); i += 3)
			{
				Vector3* normal(reinterpret_cast<Vector3*>(&normals[i]));
				math.Normalize(*normal, *normal);
			}

			vb_->GetVertexStream(VST_Normals)->Assign(&normals[0], normals.size() / 3);
		}
	}
}
