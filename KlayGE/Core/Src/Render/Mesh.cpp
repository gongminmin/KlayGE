#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/Mesh.hpp>

namespace KlayGE
{
	Mesh::Mesh()
		: rb_(new RenderBuffer(RenderBuffer::BT_TriangleList))
	{
	}

	const WString& Mesh::Name() const
	{
		static WString name(L"Mesh");
		return name;
	}

	RenderBufferPtr Mesh::GetRenderBuffer() const
	{
		return rb_;
	}

	void Mesh::ComputeNormals()
	{
		if (rb_->UseIndices())
		{
			VertexStreamPtr vstream(rb_->GetVertexStream(VST_Positions));

			IndexStreamPtr istream(rb_->GetIndexStream());
			for (std::vector<U16, alloc<U16> >::iterator iter = indices_.begin(); iter != indices_.end(); iter += 3)
			{
				Vector3* v0(reinterpret_cast<Vector3*>(&vertices_[*(iter + 0) * 3]));
				Vector3* v1(reinterpret_cast<Vector3*>(&vertices_[*(iter + 1) * 3]));
				Vector3* v2(reinterpret_cast<Vector3*>(&vertices_[*(iter + 2) * 3]));

				Vector3 vec;
				MathLib::Cross(vec, *v1 - *v0, *v2 - *v0);

				Vector3* n0(reinterpret_cast<Vector3*>(&normals_[*(iter + 0) * 3]));
				Vector3* n1(reinterpret_cast<Vector3*>(&normals_[*(iter + 1) * 3]));
				Vector3* n2(reinterpret_cast<Vector3*>(&normals_[*(iter + 2) * 3]));

				*n0 += vec;
				*n1 += vec;
				*n2 += vec;
			}

			for (std::vector<float, alloc<float> >::iterator iter = normals_.begin(); iter != normals_.end(); iter += 3)
			{
				Vector3* normal(reinterpret_cast<Vector3*>(&(*iter)));
				MathLib::Normalize(*normal, *normal);
			}

			rb_->GetVertexStream(VST_Normals)->Assign(&normals_[0], normals_.size() / 3);
		}
		else
		{
			VertexStreamPtr stream(rb_->GetVertexStream(VST_Positions));

			for (size_t i = 0; i < vertices_.size(); i += 9)
			{
				Vector3* v0(reinterpret_cast<Vector3*>(&vertices_[i + 0]));
				Vector3* v1(reinterpret_cast<Vector3*>(&vertices_[i + 3]));
				Vector3* v2(reinterpret_cast<Vector3*>(&vertices_[i + 6]));

				Vector3 vec;
				MathLib::Cross(vec, *v1 - *v0, *v2 - *v0);

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
				MathLib::Normalize(*normal, *normal);
			}

			rb_->GetVertexStream(VST_Normals)->Assign(&normals_[0], normals_.size() / 3);
		}
	}
}
