// Mesh.hpp
// KlayGE Mesh类 头文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://klayge.sourceforge.net
//
// 2.1.2
// 初次建立 (2004.5.26)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _MESH_HPP
#define _MESH_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderBuffer.hpp>

#include <vector>

namespace KlayGE
{
	class StaticMesh : public Renderable
	{
	public:
		StaticMesh();
		virtual ~StaticMesh();

		RenderEffectPtr GetRenderEffect() const
			{ return effect_; }
		void SetRenderEffect(RenderEffectPtr const & effect)
			{ effect_ = effect; }

		RenderBufferPtr GetRenderBuffer() const
			{ return rb_; }

		std::wstring const & Name() const;

		template <typename ForwardIterator>
		void AssignXYZs(ForwardIterator first, ForwardIterator last)
			{ xyzs_.assign(first, last); }

		template <typename ForwardIterator>
		void AssignTexs(ForwardIterator first, ForwardIterator last)
			{ texs_.assign(first, last); }

		template <typename ForwardIterator>
		void AssignIndices(ForwardIterator first, ForwardIterator last)
			{ indices_.assign(first, last); }

	protected:
		RenderBufferPtr rb_;
		RenderEffectPtr effect_;

		typedef std::vector<Vector3> XYZsType;
		XYZsType xyzs_;

		typedef std::vector<Vector2> TexsType;
		TexsType texs_;

		typedef std::vector<uint16> IndicesType;
		IndicesType indices_;
	};


	struct Bone
	{
		std::string name;

		Vector3 bindpos;
		Matrix4 bindmat;

		Matrix4 originMat;
		Matrix4 inverseOriginMat;

		int16 parent;

		boost::array<float, 6> attribute;
	};

	class BoneMesh : public Renderable
	{
	public:
		typedef boost::shared_ptr<std::vector<Bone> > BonesType;

	public:
		virtual ~BoneMesh();

		void SetBones(BonesType& bones)
			{ bones_ = bones; }

		RenderEffectPtr GetRenderEffect() const
			{ return staticMesh_->GetRenderEffect(); }
		RenderBufferPtr GetRenderBuffer() const
			{ return staticMesh_->GetRenderBuffer(); }

		std::wstring const & Name() const;

		template <typename ForwardIterator>
		void AssignBlendWeights(ForwardIterator first, ForwardIterator last)
			{ blendWeights_.assign(first, last); }

		template <typename ForwardIterator>
		void AssignBlendIndices(ForwardIterator first, ForwardIterator last)
			{ blendIndices_.assign(first, last); }

	protected:
		StaticMeshPtr staticMesh_;

		BonesType bones_;

		typedef std::vector<float> BlendWeightsType;
		BlendWeightsType blendWeights_;

		typedef std::vector<KlayGE::uint8> BlendIndicesType;
		BlendIndicesType blendIndices_;
	};
}

#endif			// _MESH_HPP
