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

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/MapVector.hpp>

#include <vector>

#include <boost/function.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API StaticMesh : public Renderable
	{
	public:
		StaticMesh(RenderModelPtr const & model, std::wstring const & name);
		virtual ~StaticMesh();

		virtual void BuildMeshInfo()
		{
		}

		RenderTechniquePtr const & GetRenderTechnique() const
		{
			return technique_;
		}
		void SetRenderTechnique(RenderTechniquePtr const & tech)
		{
			technique_ = tech;
		}

		RenderLayoutPtr const & GetRenderLayout() const
		{
			return rl_;
		}

		virtual Box const & GetBound() const;
		void SetBound(Box const & box);

		virtual std::wstring const & Name() const;

		void NumVertices(uint32_t n)
		{
			rl_->NumVertices(n);
		}
		uint32_t NumVertices() const
		{
			return rl_->NumVertices();
		}

		void NumTriangles(uint32_t n)
		{
			rl_->NumIndices(n * 3);
		}
		uint32_t NumTriangles() const
		{
			return rl_->NumIndices() / 3;
		}

		void AddVertexStream(void const * buf, uint32_t size, vertex_element const & ve, uint32_t access_hint);
		void AddVertexStream(GraphicsBufferPtr const & buffer, vertex_element const & ve);
		void AddIndexStream(void const * buf, uint32_t size, ElementFormat format, uint32_t access_hint);
		void AddIndexStream(GraphicsBufferPtr const & index_stream, ElementFormat format);

		void StartVertexLocation(uint32_t location)
		{
			rl_->StartVertexLocation(location);
		}
		uint32_t StartVertexLocation() const
		{
			return rl_->StartVertexLocation();
		}

		void StartIndexLocation(uint32_t location)
		{
			rl_->StartIndexLocation(location);
		}
		uint32_t StartIndexLocation() const
		{
			return rl_->StartIndexLocation();
		}

		void BaseVertexLocation(int32_t location)
		{
			rl_->BaseVertexLocation(location);
		}
		int32_t BaseVertexLocation() const
		{
			return rl_->BaseVertexLocation();
		}

		void StartInstanceLocation(uint32_t location)
		{
			rl_->StartInstanceLocation(location);
		}
		uint32_t StartInstanceLocation() const
		{
			return rl_->StartInstanceLocation();
		}

		int32_t MaterialID() const
		{
			return mtl_id_;
		}
		void MaterialID(int32_t mid)
		{
			mtl_id_ = mid;
		}

	protected:
		std::wstring name_;

		RenderLayoutPtr rl_;
		RenderTechniquePtr technique_;

		Box box_;

		int32_t mtl_id_;

		boost::weak_ptr<RenderModel> model_;
	};

	class KLAYGE_CORE_API RenderModel : public Renderable
	{
	public:
		typedef std::vector<std::pair<std::string, std::string> > TextureSlotsType;
		struct Material
		{
			float3 ambient;
			float3 diffuse;
			float3 specular;
			float3 emit;
			float opacity;
			float specular_level;
			float shininess;

			TextureSlotsType texture_slots;
		};

	public:
		explicit RenderModel(std::wstring const & name);
		virtual ~RenderModel()
		{
		}

		virtual void BuildModelInfo()
		{
		}

		virtual bool IsSkinned() const
		{
			return false;
		}

		template <typename ForwardIterator>
		void AssignMeshes(ForwardIterator first, ForwardIterator last)
		{
			meshes_.assign(first, last);

			this->UpdateBoundBox();
		}

		StaticMeshPtr const & Mesh(size_t id) const
		{
			return meshes_[id];
		}
		uint32_t NumMeshes() const
		{
			return static_cast<uint32_t>(meshes_.size());
		}

		RenderTechniquePtr const & GetRenderTechnique() const
		{
			return technique_;
		}
		void SetRenderTechnique(RenderTechniquePtr const & tech)
		{
			technique_ = tech;
		}

		RenderLayoutPtr const & GetRenderLayout() const
		{
			return rl_;
		}

		void OnRenderBegin();
		void OnRenderEnd();

		Box const & GetBound() const
		{
			return box_;
		}
		std::wstring const & Name() const
		{
			return name_;
		}

		size_t NumMaterials() const
		{
			return materials_.size();
		}
		void NumMaterials(size_t i)
		{
			materials_.resize(i);
		}
		RenderModel::Material& GetMaterial(int32_t i)
		{
			return materials_[i];
		}
		RenderModel::Material const & GetMaterial(int32_t i) const
		{
			return materials_[i];
		}

		void AddToRenderQueue();

	protected:
		void UpdateBoundBox();

	protected:
		std::wstring name_;

		RenderLayoutPtr rl_;
		RenderTechniquePtr technique_;

		Box box_;

		std::vector<Material> materials_;

		typedef std::vector<StaticMeshPtr> StaticMeshesPtrType;
		StaticMeshesPtrType meshes_;
	};


	struct Joint
	{
		std::string name;

		float3 bind_pos;
		Quaternion bind_quat;

		float3 inverse_origin_pos;
		Quaternion inverse_origin_quat;

		int16_t parent;
	};

	struct KLAYGE_CORE_API KeyFrames
	{
		std::vector<float3> bind_pos;
		std::vector<Quaternion> bind_quat;

		float3 FramePos(float frame) const;
		Quaternion FrameQuat(float frame) const;
	};
	typedef MapVector<std::string, KeyFrames> KeyFramesType;

	class KLAYGE_CORE_API SkinnedModel : public RenderModel
	{
	public:
		typedef std::vector<Joint> JointsType;
		typedef std::vector<float4> RotationsType;
		typedef std::vector<float4> PositionsType;

	public:
		explicit SkinnedModel(std::wstring const & name);
		virtual ~SkinnedModel()
		{
		}

		virtual bool IsSkinned() const
		{
			return true;
		}

		Joint& GetJoint(uint32_t index)
		{
			return joints_[index];
		}
		Joint const & GetJoint(uint32_t index) const
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
		RotationsType const & GetBindRotations() const
		{
			return bind_rots_;
		}
		PositionsType const & GetBindPositions() const
		{
			return bind_poss_;
		}
		void AttachKeyFrames(boost::shared_ptr<KlayGE::KeyFramesType> const & kf);

		boost::shared_ptr<KlayGE::KeyFramesType> const & GetKeyFrames() const
		{
			return key_frames_;
		}
		uint32_t StartFrame() const
		{
			return start_frame_;
		}
		void StartFrame(uint32_t sf)
		{
			start_frame_ = sf;
		}
		uint32_t EndFrame() const
		{
			return end_frame_;
		}
		void EndFrame(uint32_t ef)
		{
			end_frame_ = ef;
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

	protected:
		void BuildBones(float frame);
		void UpdateBinds();

	protected:
		JointsType joints_;
		RotationsType bind_rots_;
		PositionsType bind_poss_;

		boost::shared_ptr<KeyFramesType> key_frames_;
		float last_frame_;

		uint32_t start_frame_;
		uint32_t end_frame_;
		uint32_t frame_rate_;
	};

	class KLAYGE_CORE_API SkinnedMesh : public StaticMesh
	{
	public:
		SkinnedMesh(RenderModelPtr const & model, std::wstring const & name);
		virtual ~SkinnedMesh()
		{
		}
	};


	template <typename T>
	struct CreateMeshFactory
	{
		StaticMeshPtr operator()(RenderModelPtr const & model, std::wstring const & name)
		{
			return MakeSharedPtr<T>(model, name);
		}
	};

	template <typename T>
	struct CreateModelFactory
	{
		RenderModelPtr operator()(std::wstring const & name)
		{
			return MakeSharedPtr<T>(name);
		}
	};

	KLAYGE_CORE_API void LoadModel(std::string const & meshml_name, std::vector<RenderModel::Material>& mtls,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids, std::vector<std::vector<vertex_element> >& ves,
		std::vector<uint32_t>& max_num_blends, std::vector<std::vector<std::vector<uint8_t> > >& buffs,
		std::vector<char>& is_index_16_bit, std::vector<std::vector<uint8_t> >& indices,
		std::vector<Joint>& joints, boost::shared_ptr<KeyFramesType>& kfs,
		int32_t& start_frame, int32_t& end_frame, int32_t& frame_rate);
	KLAYGE_CORE_API boost::function<RenderModelPtr()> LoadModel(std::string const & meshml_name, uint32_t access_hint,
		boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc = CreateModelFactory<RenderModel>(),
		boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>());

	KLAYGE_CORE_API void SaveModel(std::string const & meshml_name, std::vector<RenderModel::Material> const & mtls,
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<std::vector<vertex_element> > const & ves,
		std::vector<std::vector<std::vector<uint8_t> > > const & buffs,
		std::vector<char> const & is_index_16_bit, std::vector<std::vector<uint8_t> > const & indices,
		std::vector<Joint> const & joints, boost::shared_ptr<KeyFramesType> const & kfs,
		int32_t start_frame, int32_t end_frame, int32_t frame_rate);
	KLAYGE_CORE_API void SaveModel(RenderModelPtr const & model, std::string const & meshml_name);


	class KLAYGE_CORE_API RenderableLightSourceProxy : public StaticMesh
	{
	public:
		RenderableLightSourceProxy(RenderModelPtr const & model, std::wstring const & name);
		virtual void Technique(RenderTechniquePtr const & tech);

		virtual void SetModelMatrix(float4x4 const & mat);

		virtual void Update();

		virtual void OnRenderBegin();

		virtual void AttachLightSrc(LightSourcePtr const & light);

	private:
		float4x4 model_;
		LightSourcePtr light_;

		RenderEffectParameterPtr mvp_param_;
		RenderEffectParameterPtr model_param_;
		RenderEffectParameterPtr light_color_param_;
		RenderEffectParameterPtr light_falloff_param_;
		RenderEffectParameterPtr light_is_projective_param_;
		RenderEffectParameterPtr light_projective_tex_param_;
	};
}

#endif			// _MESH_HPP
