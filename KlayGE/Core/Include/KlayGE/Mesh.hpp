// Mesh.hpp
// KlayGE Mesh类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2004-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 增加了SkinnedModel和SkinnedMesh (2006.4.23)
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
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/MapVector.hpp>

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

			box_ = Box(Vector3(0, 0, 0), Vector3(0, 0, 0));
			for (StaticMeshesPtrType::iterator iter = meshes_.begin();
				iter != meshes_.end(); ++ iter)
			{
				box_ |= (*iter)->GetBound();
			}
		}

		StaticMeshPtr Mesh(size_t id)
		{
			return meshes_[id];
		}
		StaticMeshPtr Mesh(size_t id) const
		{
			return meshes_[id];
		}
		size_t NumMeshes() const
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

		RenderLayoutPtr GetRenderLayout() const
		{
			return rl_;
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

		void AddToRenderQueue();

	protected:
		std::wstring name_;

		RenderLayoutPtr rl_;
		RenderEffectPtr effect_;

		Box box_;

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

		RenderLayoutPtr GetRenderLayout() const
		{
			return rl_;
		}

		virtual Box GetBound() const;

		virtual std::wstring const & Name() const;

		virtual void AddToRenderQueue();

		size_t NumVertices() const
		{
			return xyzs_.size();
		}

		size_t NumTriangles() const
		{
			return indices_.size() / 3;
		}

		template <typename ForwardIterator>
		void AssignXYZs(ForwardIterator first, ForwardIterator last)
		{
			xyzs_.assign(first, last);
			box_ = MathLib::ComputeBoundingBox<float>(xyzs_.begin(), xyzs_.end());
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

		RenderLayoutPtr rl_;
		RenderEffectPtr effect_;

		Box box_;

		bool beBuilt_;

		XYZsType xyzs_;
		NormalsType normals_;
		MultiTexCoordsType multi_tex_coords_;
		IndicesType indices_;
	};


	struct Joint
	{
		std::string name;

		Vector3 bind_pos;
		Quaternion bind_quat;

		Vector3 inverse_origin_pos;
		Quaternion inverse_origin_quat;

		int16_t parent;
	};

	struct KeyFrames
	{
		std::vector<Vector3> bind_pos;
		std::vector<Quaternion> bind_quat;

		Vector3 const & FramePos(int frame) const
		{
			const int lframe(static_cast<int>(frame % bind_pos.size()));
			return bind_pos[lframe];
		}

		Quaternion const & FrameQuat(int frame) const
		{
			const int lframe(static_cast<int>(frame % bind_quat.size()));
			return bind_quat[lframe];
		}
	};
	typedef MapVector<std::string, KeyFrames> KeyFramesType;

	class SkinnedModel : public RenderModel
	{
	public:
		typedef std::vector<Joint> JointsType;
		typedef std::vector<Vector4> RotationsType;
		typedef std::vector<Vector4> PositionsType;

	public:
		SkinnedModel(std::wstring const & name);
		virtual ~SkinnedModel()
		{
		}

		template <typename ForwardIterator>
		void AssignJoints(ForwardIterator first, ForwardIterator last)
		{
			joints_.assign(first, last);
		}
		RotationsType const & GetBindRotations() const
		{
			return bind_rots_;
		}
		PositionsType const & GetBindPositions() const
		{
			return bind_poss_;
		}
		void AttachKeyFrames(boost::shared_ptr<KlayGE::KeyFramesType> const & kf);

		void SetFrame(int frame);

	protected:
		void BuildBones(int frame);
		void UpdateBinds();

	protected:
		JointsType joints_;
		RotationsType bind_rots_;
		PositionsType bind_poss_;

		boost::shared_ptr<KlayGE::KeyFramesType> key_frames_;
		int last_frame_;	
	};

	class SkinnedMesh : public StaticMesh
	{
	public:
		SkinnedMesh(std::wstring const & name);
		virtual ~SkinnedMesh()
		{
		}

		template <typename ForwardIterator>
		void AssignBlendWeights(ForwardIterator first, ForwardIterator last)
		{
			blend_weights_.assign(first, last);
			beBuilt_ = false;
		}

		template <typename ForwardIterator>
		void AssignBlendIndices(ForwardIterator first, ForwardIterator last)
		{
			blend_indices_.assign(first, last);
			beBuilt_ = false;
		}

	protected:
		virtual void BuildRenderable();

	protected:
		typedef std::vector<float> BlendWeightsType;
		BlendWeightsType blend_weights_;

		typedef std::vector<KlayGE::uint8_t> BlendIndicesType;
		BlendIndicesType blend_indices_;
	};
}

#endif			// _MESH_HPP
