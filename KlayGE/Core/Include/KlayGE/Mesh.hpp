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

		size_t NumSubs() const
			{ return 1; }

		RenderEffectPtr GetRenderEffect(size_t /*index*/)
			{ return effect_; }
		void SetRenderEffect(const RenderEffectPtr& effect)
			{ effect_ = effect; }

		VertexBufferPtr GetVertexBuffer(size_t /*index*/);

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
	};
}

#endif			// _MESH_HPP
