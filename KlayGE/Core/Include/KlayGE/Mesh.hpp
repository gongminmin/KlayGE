#ifndef _MESH_HPP
#define _MESH_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/MathTypes.hpp>
#include <KlayGE/VertexBuffer.hpp>

#include <vector>

namespace KlayGE
{
	class Mesh : public Renderable
	{
	public:
		Mesh();

		RenderEffectPtr GetRenderEffect()
			{ return effect_; }
		void SetRenderEffect(const RenderEffectPtr& effect)
			{ effect_ = effect; }

		VertexBufferPtr GetVertexBuffer();

		const WString& Name() const;

		template <typename ForwardIterator>
		void AssignXYZs(ForwardIterator first, ForwardIterator last)
			{ xyzs_.assign(first, last); }

		template <typename ForwardIterator>
		void AssignNormals(ForwardIterator first, ForwardIterator last)
			{ normals_.assign(first, last); }

		template <typename ForwardIterator>
		void AssignTextures(ForwardIterator first, ForwardIterator last)
			{ textures_.assign(first, last); }

		template <typename ForwardIterator>
		void AssignIndices(ForwardIterator first, ForwardIterator last)
			{ indices_.assign(first, last); }

		void ComputeNormals();

	private:
		VertexBufferPtr vb_;
		RenderEffectPtr effect_;

		std::vector<float> xyzs_;
		std::vector<float> normals_;
		std::vector<float> textures_;
		std::vector<U16> indices_;
	};
}

#endif			// _MESH_HPP
