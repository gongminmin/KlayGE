// Mesh.hpp
// KlayGE Mesh类 头文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 增加了RenderModel (2005.10.9)
//
// 2.7.0
// 改进了StaticMesh (2005.6.16)
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
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Math.hpp>

#include <vector>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class RenderModel : public Renderable
	{
	public:
		RenderModel(std::wstring const & name);

		template <typename ForwardIterator>
		void AssignMeshes(ForwardIterator first, ForwardIterator last)
		{
			meshes_.assign(first, last);
		}

		StaticMeshPtr Mesh(size_t id)
		{
			return meshes_[id];
		}
		StaticMeshPtr Mesh(size_t id) const
		{
			return meshes_[id];
		}
		size_t NumMesh() const
		{
			return meshes_.size();
		}

		RenderEffectPtr GetRenderEffect() const
		{
			return effect_;
		}
		void SetRenderEffect(RenderEffectPtr const & effect)
		{
			effect_ = effect;
		}

		VertexBufferPtr GetVertexBuffer() const
		{
			return vb_;
		}

		void OnRenderBegin();
		void OnRenderEnd();

		Box GetBound() const
		{
			return box_;
		}
		std::wstring const & Name() const
		{
			return name_;
		}

		void AddToSceneManager();

		void SetModelMatrix(Matrix4 const & mat)
		{
			model_ = mat;
		}

	private:
		std::wstring name_;

		VertexBufferPtr vb_;
		RenderEffectPtr effect_;

		Box box_;

		Matrix4 model_;

		typedef std::vector<StaticMeshPtr> StaticMeshesPtrType;
		StaticMeshesPtrType meshes_;
	};

	class StaticMesh : public Renderable
	{
	public:
		typedef std::vector<Vector3> XYZsType;
		typedef std::vector<Vector2> TexCoordsType;
		typedef std::vector<TexCoordsType> MultiTexCoordsType;
		typedef std::vector<Vector3> NormalsType;
		typedef std::vector<uint16_t> IndicesType;

	public:
		StaticMesh(std::wstring const & name);
		virtual ~StaticMesh();

		RenderEffectPtr GetRenderEffect() const
		{
			return effect_;
		}
		void SetRenderEffect(RenderEffectPtr const & effect)
		{
			effect_ = effect;
		}

		VertexBufferPtr GetVertexBuffer() const
		{
			return vb_;
		}

		virtual Box GetBound() const;

		virtual std::wstring const & Name() const;

		virtual void AddToSceneManager();

		void SetModelMatrix(Matrix4 const & mat);

		void ComputeNormal();

		template <typename ForwardIterator>
		void AssignXYZs(ForwardIterator first, ForwardIterator last)
		{
			xyzs_.assign(first, last);
			beBuilt_ = false;
		}

		template <typename ForwardIterator>
		void AssignMultiTexs(ForwardIterator first, ForwardIterator last)
		{
			multi_tex_coords_.assign(first, last);
			beBuilt_ = false;
		}

		template <typename ForwardIterator>
		void AssignNormals(ForwardIterator first, ForwardIterator last)
		{
			normals_.assign(first, last);
			beBuilt_ = false;
		}

		template <typename ForwardIterator>
		void AssignIndices(ForwardIterator first, ForwardIterator last)
		{
			indices_.assign(first, last);
			beBuilt_ = false;
		}

	protected:
		virtual void BuildRenderable();

	protected:
		std::wstring name_;

		VertexBufferPtr vb_;
		RenderEffectPtr effect_;

		Box box_;

		Matrix4 model_;

		bool beBuilt_;

		XYZsType xyzs_;
		NormalsType normals_;
		MultiTexCoordsType multi_tex_coords_;
		IndicesType indices_;
	};


	struct Bone
	{
		std::string name;

		Vector3 bindpos;
		Matrix4 bindmat;

		Matrix4 originMat;
		Matrix4 inverseOriginMat;

		int16_t parent;

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
		VertexBufferPtr GetVertexBuffer() const
			{ return staticMesh_->GetVertexBuffer(); }

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

		typedef std::vector<KlayGE::uint8_t> BlendIndicesType;
		BlendIndicesType blendIndices_;
	};
}

#endif			// _MESH_HPP
