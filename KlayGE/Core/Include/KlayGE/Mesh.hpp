#ifndef _MESH_HPP
#define _MESH_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/MathTypes.hpp>
#include <KlayGE/RenderBuffer.hpp>

#include <vector>

namespace KlayGE
{
	class Mesh : public Renderable
	{
	public:
		Mesh();

		RenderEffectPtr GetRenderEffect() const
			{ return effect_; }
		void SetRenderEffect(const RenderEffectPtr& effect)
			{ effect_ = effect; }

		RenderBufferPtr GetRenderBuffer() const;

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
		RenderBufferPtr rb_;
		RenderEffectPtr effect_;

		std::vector<float, alloc<float> > vertices_;
		std::vector<float, alloc<float> > normals_;

		std::vector<U16, alloc<U16> > indices_;
	};
}

#endif			// _MESH_HPP
