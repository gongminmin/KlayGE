// Mesh.hpp
// KlayGE Mesh类 头文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2004-2011
// Homepage: http://www.klayge.org
//
// 3.4.0
// 增加了AddVertexStream和AddIndexStream (2006.8.21)
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

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SceneComponent.hpp>

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace KlayGE
{
	template <typename T>
	inline StaticMeshPtr CreateMeshFactory(std::wstring_view name)
	{
		return MakeSharedPtr<T>(name);
	}

	template <typename T>
	inline RenderModelPtr CreateModelFactory(std::wstring_view name, uint32_t node_attrib)
	{
		return MakeSharedPtr<T>(name, node_attrib);
	}

	KLAYGE_CORE_API void AddToSceneHelper(SceneNode& node, RenderModel& model);
	KLAYGE_CORE_API void AddToSceneRootHelper(RenderModel& model);


	class KLAYGE_CORE_API StaticMesh : public Renderable
	{
	public:
		explicit StaticMesh(std::wstring_view name);

		void BuildMeshInfo(RenderModel const & model);

		virtual void Technique(RenderEffectPtr const & effect, RenderTechnique* tech)
		{
			effect_ = effect;
			technique_ = tech;
		}

		void NumLods(uint32_t lods) override;
		using Renderable::NumLods;

		virtual void PosBound(AABBox const & aabb);
		using Renderable::PosBound;
		virtual void TexcoordBound(AABBox const & aabb);
		using Renderable::TexcoordBound;

		void NumVertices(uint32_t lod, uint32_t n)
		{
			rls_[lod]->NumVertices(n);
		}
		uint32_t NumVertices(uint32_t lod) const
		{
			return rls_[lod]->NumVertices();
		}

		void NumIndices(uint32_t lod, uint32_t n)
		{
			rls_[lod]->NumIndices(n);
		}
		uint32_t NumIndices(uint32_t lod) const
		{
			return rls_[lod]->NumIndices();
		}

		void AddVertexStream(uint32_t lod, void const * buf, uint32_t size, VertexElement const & ve, uint32_t access_hint);
		void AddVertexStream(uint32_t lod, GraphicsBufferPtr const & buffer, VertexElement const & ve);
		void AddIndexStream(uint32_t lod, void const * buf, uint32_t size, ElementFormat format, uint32_t access_hint);
		void AddIndexStream(uint32_t lod, GraphicsBufferPtr const & index_stream, ElementFormat format);

		void StartVertexLocation(uint32_t lod, uint32_t location)
		{
			rls_[lod]->StartVertexLocation(location);
		}
		uint32_t StartVertexLocation(uint32_t lod) const
		{
			return rls_[lod]->StartVertexLocation();
		}

		void StartIndexLocation(uint32_t lod, uint32_t location)
		{
			rls_[lod]->StartIndexLocation(location);
		}
		uint32_t StartIndexLocation(uint32_t lod) const
		{
			return rls_[lod]->StartIndexLocation();
		}

		void StartInstanceLocation(uint32_t lod, uint32_t location)
		{
			rls_[lod]->StartInstanceLocation(location);
		}
		uint32_t StartInstanceLocation(uint32_t lod) const
		{
			return rls_[lod]->StartInstanceLocation();
		}

		int32_t MaterialID() const
		{
			return mtl_id_;
		}
		void MaterialID(int32_t mid)
		{
			mtl_id_ = mid;
		}

		bool HWResourceReady() const override
		{
			return hw_res_ready_;
		}

	protected:
		virtual void DoBuildMeshInfo(RenderModel const & model);

	protected:
		int32_t mtl_id_;

		bool hw_res_ready_;
	};

	class KLAYGE_CORE_API RenderModel
	{
	public:
		explicit RenderModel(SceneNodePtr const & root_node);
		RenderModel(std::wstring_view name, uint32_t node_attrib);
		virtual ~RenderModel() noexcept;

		SceneNodePtr const & RootNode() const
		{
			return root_node_;
		}

		void BuildModelInfo();

		virtual bool IsSkinned() const
		{
			return false;
		}

		uint32_t NumLods() const;

		void ActiveLod(int32_t lod);

		size_t NumMaterials() const
		{
			return materials_.size();
		}
		void NumMaterials(size_t i)
		{
			materials_.resize(i);
		}
		RenderMaterialPtr& GetMaterial(int32_t i)
		{
			return materials_[i];
		}
		RenderMaterialPtr const & GetMaterial(int32_t i) const
		{
			return materials_[i];
		}

		template <typename ForwardIterator>
		void AssignMeshes(ForwardIterator first, ForwardIterator last)
		{
			meshes_.assign(first, last);
		}
		RenderablePtr const & Mesh(size_t id) const
		{
			return meshes_[id];
		}
		uint32_t NumMeshes() const
		{
			return static_cast<uint32_t>(meshes_.size());
		}

		void ForEachMesh(std::function<void(Renderable&)> const & callback) const;

		bool HWResourceReady() const;

		RenderModelPtr Clone(
			std::function<RenderModelPtr(std::wstring_view, uint32_t)> const & CreateModelFactoryFunc = CreateModelFactory<RenderModel>,
			std::function<StaticMeshPtr(std::wstring_view)> const & CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>);
		virtual void CloneDataFrom(RenderModel const & source,
			std::function<StaticMeshPtr(std::wstring_view)> const & CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>);

	protected:
		virtual void DoBuildModelInfo()
		{
		}

	protected:
		SceneNodePtr root_node_;
		std::vector<RenderablePtr> meshes_;

		std::vector<RenderMaterialPtr> materials_;

		bool hw_res_ready_;
	};


	class KLAYGE_CORE_API JointComponent : public SceneComponent
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS((SceneComponent))
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		SceneComponentPtr Clone() const override;

		void BindParams(Quaternion const& real, Quaternion const& dual, float scale);

		Quaternion const& BindReal() const
		{
			return bind_real_;
		}
		Quaternion const& BindDual() const
		{
			return bind_dual_;
		}
		float BindScale() const
		{
			return bind_scale_;
		}

		void InverseOriginParams(Quaternion const& real, Quaternion const& dual, float scale);

		Quaternion const& InverseOriginReal() const
		{
			return inverse_origin_real_;
		}
		Quaternion const& InverseOriginDual() const
		{
			return inverse_origin_dual_;
		}
		float InverseOriginScale() const
		{
			return inverse_origin_scale_;
		}

		void InitInverseOriginParams();

	private:
		Quaternion bind_real_;
		Quaternion bind_dual_;
		float bind_scale_;

		Quaternion inverse_origin_real_;
		Quaternion inverse_origin_dual_;
		float inverse_origin_scale_;
	};

	struct KLAYGE_CORE_API KeyFrameSet
	{
		std::vector<uint32_t> frame_id;
		std::vector<Quaternion> bind_real;
		std::vector<Quaternion> bind_dual;
		std::vector<float> bind_scale;

		std::tuple<Quaternion, Quaternion, float> Frame(float frame) const;
	};

	struct KLAYGE_CORE_API AABBKeyFrameSet
	{
		std::vector<uint32_t> frame_id;
		std::vector<AABBox> bb;

		AABBox Frame(float frame) const;
	};

	struct KLAYGE_CORE_API Animation
	{
		std::string name;
		uint32_t start_frame;
		uint32_t end_frame;
	};

	class KLAYGE_CORE_API SkinnedModel : public RenderModel
	{
	public:
		explicit SkinnedModel(SceneNodePtr const & root_node);
		SkinnedModel(std::wstring_view name, uint32_t node_attrib);

		bool IsSkinned() const override
		{
			return true;
		}

		void CloneDataFrom(RenderModel const & source,
			std::function<StaticMeshPtr(std::wstring_view)> const & CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>) override;

		JointComponentPtr& GetJoint(uint32_t index)
		{
			return joints_[index];
		}
		JointComponentPtr const& GetJoint(uint32_t index) const
		{
			return joints_[index];
		}
		uint32_t NumJoints() const
		{
			return static_cast<uint32_t>(joints_.size());
		}

		template <typename ForwardIterator>
		void AssignJoints(ForwardIterator first, ForwardIterator last)
		{
			joints_.assign(first, last);
			this->UpdateBinds();
		}
		void AttachKeyFrameSets(std::shared_ptr<std::vector<KeyFrameSet>> const & kf)
		{
			key_frame_sets_ = kf;
		}
		std::shared_ptr<std::vector<KeyFrameSet>> const & GetKeyFrameSets() const
		{
			return key_frame_sets_;
		}
		uint32_t NumFrames() const
		{
			return num_frames_;
		}
		void NumFrames(uint32_t nf)
		{
			num_frames_ = nf;
		}
		uint32_t FrameRate() const
		{
			return frame_rate_;
		}
		void FrameRate(uint32_t fr)
		{
			frame_rate_ = fr;
		}

		float GetFrame() const;
		void SetFrame(float frame);

		void RebindJoints();
		void UnbindJoints();

		AABBox FramePosBound(uint32_t frame) const;

		void AttachAnimations(std::shared_ptr<std::vector<Animation>> const & animations);
		std::shared_ptr<std::vector<Animation>> const & GetAnimations() const
		{
			return animations_;
		}
		uint32_t NumAnimations() const;
		void GetAnimation(uint32_t index, std::string& name, uint32_t& start_frame, uint32_t& end_frame);

	protected:
		void BuildBones(float frame);
		void UpdateBinds();
		void SetToEffect();

	protected:
		std::vector<JointComponentPtr> joints_;
		std::vector<float4> bind_reals_;
		std::vector<float4> bind_duals_;

		std::shared_ptr<std::vector<KeyFrameSet>> key_frame_sets_;
		float last_frame_;

		uint32_t num_frames_;
		uint32_t frame_rate_;

		std::shared_ptr<std::vector<Animation>> animations_;
	};

	class KLAYGE_CORE_API SkinnedMesh : public StaticMesh
	{
	public:
		explicit SkinnedMesh(std::wstring_view name);

		AABBox FramePosBound(uint32_t frame) const;
		void AttachFramePosBounds(std::shared_ptr<AABBKeyFrameSet> const & frame_pos_aabbs);
		std::shared_ptr<AABBKeyFrameSet> const & GetFramePosBounds() const
		{
			return frame_pos_aabbs_;
		}

	private:
		std::shared_ptr<AABBKeyFrameSet> frame_pos_aabbs_;
	};


	KLAYGE_CORE_API RenderModelPtr SyncLoadModel(std::string_view model_name, uint32_t access_hint,
		uint32_t node_attrib,
		std::function<void(RenderModel&)> OnFinishLoading = nullptr,
		std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc = CreateModelFactory<RenderModel>,
		std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>);
	KLAYGE_CORE_API RenderModelPtr ASyncLoadModel(std::string_view model_name, uint32_t access_hint,
		uint32_t node_attrib,
		std::function<void(RenderModel&)> OnFinishLoading = nullptr,
		std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc = CreateModelFactory<RenderModel>,
		std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>);
	KLAYGE_CORE_API RenderModelPtr LoadSoftwareModel(std::string_view model_name);

	KLAYGE_CORE_API void SaveModel(RenderModel const & model, std::string_view model_name);


	class KLAYGE_CORE_API RenderableLightSourceProxy : public StaticMesh
	{
	public:
		explicit RenderableLightSourceProxy(std::wstring_view name);

		void Technique(RenderEffectPtr const & effect, RenderTechnique* tech) override;

		void AttachLightSrc(LightSourcePtr const & light);

		void OnRenderBegin() override;

	protected:
		void DoBuildMeshInfo(RenderModel const & model) override
		{
			KFL_UNUSED(model);
		}

	private:
		LightSourcePtr light_;

		RenderEffectParameter* light_is_projective_param_;
		RenderEffectParameter* projective_map_2d_tex_param_;
		RenderEffectParameter* projective_map_cube_tex_param_;
	};

	class KLAYGE_CORE_API RenderableCameraProxy : public StaticMesh
	{
	public:
		explicit RenderableCameraProxy(std::wstring_view name);

		void Technique(RenderEffectPtr const & effect, RenderTechnique* tech) override;

		void AttachCamera(CameraPtr const & camera);

	protected:
		void DoBuildMeshInfo(RenderModel const & model) override
		{
			KFL_UNUSED(model);
		}

	private:
		CameraPtr camera_;
	};

	KLAYGE_CORE_API RenderModelPtr LoadLightSourceProxyModel(LightSourcePtr const& light);
	KLAYGE_CORE_API RenderModelPtr LoadCameraProxyModel(CameraPtr const& camera);
}

#endif			// _MESH_HPP
