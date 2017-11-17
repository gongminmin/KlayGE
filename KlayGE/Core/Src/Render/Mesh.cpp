// Mesh.cpp
// KlayGE Mesh类 实现文件
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
// 2.7.0
// 改进了StaticMesh (2005.6.16)
//
// 2.1.2
// 初次建立 (2004.5.26)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/LZMACodec.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KFL/Hash.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>

#include <MeshMLLib/MeshMLLib.hpp>

#include <KlayGE/Mesh.hpp>

namespace
{
	using namespace KlayGE;

	uint32_t const MODEL_BIN_VERSION = 15;

	class RenderModelLoadingDesc : public ResLoadingDesc
	{
	private:
		struct ModelDesc
		{
			std::string res_name;
			uint32_t access_hint;
			std::function<RenderModelPtr(std::wstring const &)> CreateModelFactoryFunc;
			std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc;

			struct ModelData
			{
				struct MeshData
				{
					std::string name;
					int32_t mtl_id;
					uint32_t lods;
					AABBox pos_bb;
					AABBox tc_bb;
					std::vector<uint32_t> num_vertices;
					std::vector<uint32_t> base_vertices;
					std::vector<uint32_t> num_indices;
					std::vector<uint32_t> start_indices;
				};

				std::vector<RenderMaterialPtr> mtls;

				std::vector<VertexElement> merged_ves;
				char all_is_index_16_bit;
				std::vector<std::vector<uint8_t>> merged_buff;
				std::vector<uint8_t> merged_indices;
				std::vector<GraphicsBufferPtr> merged_vbs;
				GraphicsBufferPtr merged_ib;

				std::vector<MeshData> meshes;

				std::vector<Joint> joints;
				std::shared_ptr<AnimationActionsType> actions;
				std::shared_ptr<KeyFramesType> kfs;
				uint32_t num_frames;
				uint32_t frame_rate;
				std::vector<std::shared_ptr<AABBKeyFrames>> frame_pos_bbs;
			};
			std::shared_ptr<ModelData> model_data;

			std::shared_ptr<RenderModelPtr> model;
		};

	public:
		RenderModelLoadingDesc(std::string const & res_name, uint32_t access_hint,
			std::function<RenderModelPtr(std::wstring const &)> CreateModelFactoryFunc,
			std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
		{
			model_desc_.res_name = res_name;
			model_desc_.access_hint = access_hint;
			model_desc_.CreateModelFactoryFunc = CreateModelFactoryFunc;
			model_desc_.CreateMeshFactoryFunc = CreateMeshFactoryFunc;
			model_desc_.model_data = MakeSharedPtr<ModelDesc::ModelData>();
			model_desc_.model = MakeSharedPtr<RenderModelPtr>();
		}

		uint64_t Type() const override
		{
			static uint64_t const type = CT_HASH("RenderModelLoadingDesc");
			return type;
		}

		bool StateLess() const override
		{
			return false;
		}

		std::shared_ptr<void> CreateResource() override
		{
			RenderModelPtr model = model_desc_.CreateModelFactoryFunc(L"Model");
			*model_desc_.model = model;
			return model;
		}

		void SubThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);

			RenderModelPtr const & model = *model_desc_.model;
			if (model && model->HWResourceReady())
			{
				return;
			}

			std::vector<std::string> mesh_names;
			std::vector<int32_t> mtl_ids;
			std::vector<uint32_t> mesh_lods;
			std::vector<AABBox> pos_bbs;
			std::vector<AABBox> tc_bbs;
			std::vector<uint32_t> mesh_num_vertices;
			std::vector<uint32_t> mesh_base_vertices;
			std::vector<uint32_t> mesh_num_indices;
			std::vector<uint32_t> mesh_start_indices;
			LoadModel(model_desc_.res_name, model_desc_.model_data->mtls, model_desc_.model_data->merged_ves,
				model_desc_.model_data->all_is_index_16_bit,
				model_desc_.model_data->merged_buff, model_desc_.model_data->merged_indices,
				mesh_names, mtl_ids, mesh_lods,
				pos_bbs, tc_bbs,
				mesh_num_vertices, mesh_base_vertices,
				mesh_num_indices, mesh_start_indices,
				model_desc_.model_data->joints, model_desc_.model_data->actions, model_desc_.model_data->kfs,
				model_desc_.model_data->num_frames, model_desc_.model_data->frame_rate,
				model_desc_.model_data->frame_pos_bbs);

			model_desc_.model_data->meshes.resize(mesh_names.size());
			uint32_t mesh_lod_index = 0;
			for (size_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
			{
				model_desc_.model_data->meshes[mesh_index].name = mesh_names[mesh_index];
				model_desc_.model_data->meshes[mesh_index].mtl_id = mtl_ids[mesh_index];
				model_desc_.model_data->meshes[mesh_index].lods = mesh_lods[mesh_index];
				model_desc_.model_data->meshes[mesh_index].pos_bb = pos_bbs[mesh_index];
				model_desc_.model_data->meshes[mesh_index].tc_bb = tc_bbs[mesh_index];

				uint32_t const lods = model_desc_.model_data->meshes[mesh_index].lods;
				model_desc_.model_data->meshes[mesh_index].num_vertices.resize(lods);
				model_desc_.model_data->meshes[mesh_index].base_vertices.resize(lods);
				model_desc_.model_data->meshes[mesh_index].num_indices.resize(lods);
				model_desc_.model_data->meshes[mesh_index].start_indices.resize(lods);
				memcpy(&model_desc_.model_data->meshes[mesh_index].num_vertices[0], &mesh_num_vertices[mesh_lod_index], lods * sizeof(uint32_t));
				memcpy(&model_desc_.model_data->meshes[mesh_index].base_vertices[0], &mesh_base_vertices[mesh_lod_index], lods * sizeof(uint32_t));
				memcpy(&model_desc_.model_data->meshes[mesh_index].num_indices[0], &mesh_num_indices[mesh_lod_index], lods * sizeof(uint32_t));
				memcpy(&model_desc_.model_data->meshes[mesh_index].start_indices[0], &mesh_start_indices[mesh_lod_index], lods * sizeof(uint32_t));
				mesh_lod_index += lods;
			}

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if (caps.multithread_res_creating_support)
			{
				this->MainThreadStageNoLock();
			}
		}

		void MainThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
			this->MainThreadStageNoLock();
		}

		bool HasSubThreadStage() const override
		{
			return true;
		}

		bool Match(ResLoadingDesc const & rhs) const override
		{
			if (this->Type() == rhs.Type())
			{
				RenderModelLoadingDesc const & rmld = static_cast<RenderModelLoadingDesc const &>(rhs);
				return (model_desc_.res_name == rmld.model_desc_.res_name)
					&& (model_desc_.access_hint == rmld.model_desc_.access_hint);
			}
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs) override
		{
			BOOST_ASSERT(this->Type() == rhs.Type());

			RenderModelLoadingDesc const & rmld = static_cast<RenderModelLoadingDesc const &>(rhs);
			model_desc_.res_name = rmld.model_desc_.res_name;
			model_desc_.access_hint = rmld.model_desc_.access_hint;
			model_desc_.model_data = rmld.model_desc_.model_data;
			model_desc_.model = rmld.model_desc_.model;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
		{
			RenderModelPtr rhs_model = std::static_pointer_cast<RenderModel>(resource);
			RenderModelPtr model = model_desc_.CreateModelFactoryFunc(rhs_model->Name());

			model->NumMaterials(rhs_model->NumMaterials());
			for (uint32_t mtl_index = 0; mtl_index < model->NumMaterials(); ++ mtl_index)
			{
				RenderMaterialPtr mtl = MakeSharedPtr<RenderMaterial>();
				*mtl = *rhs_model->GetMaterial(mtl_index);
				model->GetMaterial(mtl_index) = mtl;
			}

			if (rhs_model->NumSubrenderables() > 0)
			{
				std::vector<StaticMeshPtr> meshes(rhs_model->NumSubrenderables());
				for (uint32_t mesh_index = 0; mesh_index < rhs_model->NumSubrenderables(); ++ mesh_index)
				{
					StaticMeshPtr rhs_mesh = checked_pointer_cast<StaticMesh>(rhs_model->Subrenderable(mesh_index));

					meshes[mesh_index] = model_desc_.CreateMeshFactoryFunc(model, rhs_mesh->Name());
					StaticMeshPtr& mesh = meshes[mesh_index];

					mesh->MaterialID(rhs_mesh->MaterialID());
					mesh->NumLods(rhs_mesh->NumLods());
					mesh->PosBound(rhs_mesh->PosBound());
					mesh->TexcoordBound(rhs_mesh->TexcoordBound());

					for (uint32_t lod = 0; lod < rhs_mesh->NumLods(); ++ lod)
					{
						RenderLayout const & rhs_rl = rhs_mesh->GetRenderLayout(lod);

						for (uint32_t ve_index = 0; ve_index < rhs_rl.NumVertexStreams(); ++ ve_index)
						{
							mesh->AddVertexStream(lod, rhs_rl.GetVertexStream(ve_index),
								rhs_rl.VertexStreamFormat(ve_index)[0]);
						}
						mesh->AddIndexStream(lod, rhs_rl.GetIndexStream(), rhs_rl.IndexStreamFormat());

						mesh->NumVertices(lod, rhs_mesh->NumVertices(lod));
						mesh->NumIndices(lod, rhs_mesh->NumIndices(lod));
						mesh->StartVertexLocation(lod, rhs_mesh->StartVertexLocation(lod));
						mesh->StartIndexLocation(lod, rhs_mesh->StartIndexLocation(lod));
					}
				}

				BOOST_ASSERT(model->IsSkinned() == rhs_model->IsSkinned());

				if (rhs_model->IsSkinned())
				{
					SkinnedModelPtr rhs_skinned_model = checked_pointer_cast<SkinnedModel>(rhs_model);
					SkinnedModelPtr skinned_model = checked_pointer_cast<SkinnedModel>(model);

					std::vector<Joint> joints(rhs_skinned_model->NumJoints());
					for (uint32_t i = 0; i < rhs_skinned_model->NumJoints(); ++ i)
					{
						joints[i] = rhs_skinned_model->GetJoint(i);
					}
					skinned_model->AssignJoints(joints.begin(), joints.end());
					skinned_model->AttachKeyFrames(rhs_skinned_model->GetKeyFrames());

					skinned_model->NumFrames(rhs_skinned_model->NumFrames());
					skinned_model->FrameRate(rhs_skinned_model->FrameRate());

					for (size_t mesh_index = 0; mesh_index < meshes.size(); ++ mesh_index)
					{
						SkinnedMeshPtr rhs_skinned_mesh = checked_pointer_cast<SkinnedMesh>(rhs_skinned_model->Subrenderable(mesh_index));
						SkinnedMeshPtr skinned_mesh = checked_pointer_cast<SkinnedMesh>(meshes[mesh_index]);
						skinned_mesh->AttachFramePosBounds(rhs_skinned_mesh->GetFramePosBounds());
					}
				}

				model->AssignSubrenderables(meshes.begin(), meshes.end());
			}

			this->AddsSubPath();

			model->BuildModelInfo();
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<StaticMesh>(model->Subrenderable(i))->BuildMeshInfo();
			}

			return std::static_pointer_cast<void>(model);
		}

		std::shared_ptr<void> Resource() const override
		{
			return *model_desc_.model;
		}

	private:
		void FillModel()
		{
			RenderModelPtr const & model = *model_desc_.model;

			model->NumMaterials(model_desc_.model_data->mtls.size());
			for (uint32_t mtl_index = 0; mtl_index < model_desc_.model_data->mtls.size(); ++ mtl_index)
			{
				model->GetMaterial(mtl_index) = model_desc_.model_data->mtls[mtl_index];
			}

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			model_desc_.model_data->merged_vbs.resize(model_desc_.model_data->merged_buff.size());
			for (size_t i = 0; i < model_desc_.model_data->merged_buff.size(); ++ i)
			{
				model_desc_.model_data->merged_vbs[i] = rf.MakeDelayCreationVertexBuffer(BU_Static, model_desc_.access_hint,
					static_cast<uint32_t>(model_desc_.model_data->merged_buff[i].size()));
			}
			model_desc_.model_data->merged_ib = rf.MakeDelayCreationIndexBuffer(BU_Static, model_desc_.access_hint,
				static_cast<uint32_t>(model_desc_.model_data->merged_indices.size()));

			std::vector<StaticMeshPtr> meshes(model_desc_.model_data->meshes.size());
			for (uint32_t mesh_index = 0; mesh_index < model_desc_.model_data->meshes.size(); ++ mesh_index)
			{
				std::wstring wname;
				Convert(wname, model_desc_.model_data->meshes[mesh_index].name);

				meshes[mesh_index] = model_desc_.CreateMeshFactoryFunc(model, wname);
				StaticMeshPtr& mesh = meshes[mesh_index];

				mesh->MaterialID(model_desc_.model_data->meshes[mesh_index].mtl_id);
				mesh->PosBound(model_desc_.model_data->meshes[mesh_index].pos_bb);
				mesh->TexcoordBound(model_desc_.model_data->meshes[mesh_index].tc_bb);

				uint32_t const lods = model_desc_.model_data->meshes[mesh_index].lods;
				mesh->NumLods(lods);
				for (uint32_t lod = 0; lod < lods; ++ lod)
				{
					for (uint32_t ve_index = 0; ve_index < model_desc_.model_data->merged_buff.size(); ++ ve_index)
					{
						mesh->AddVertexStream(lod, model_desc_.model_data->merged_vbs[ve_index], model_desc_.model_data->merged_ves[ve_index]);
					}
					mesh->AddIndexStream(lod, model_desc_.model_data->merged_ib, model_desc_.model_data->all_is_index_16_bit ? EF_R16UI : EF_R32UI);

					mesh->NumVertices(lod, model_desc_.model_data->meshes[mesh_index].num_vertices[lod]);
					mesh->NumIndices(lod, model_desc_.model_data->meshes[mesh_index].num_indices[lod]);
					mesh->StartVertexLocation(lod, model_desc_.model_data->meshes[mesh_index].base_vertices[lod]);
					mesh->StartIndexLocation(lod, model_desc_.model_data->meshes[mesh_index].start_indices[lod]);
				}
			}

			if (model_desc_.model_data->kfs && !model_desc_.model_data->kfs->empty())
			{
				if (!model_desc_.model_data->joints.empty())
				{
					SkinnedModelPtr skinned_model = checked_pointer_cast<SkinnedModel>(model);

					skinned_model->AssignJoints(model_desc_.model_data->joints.begin(), model_desc_.model_data->joints.end());
					skinned_model->AttachKeyFrames(model_desc_.model_data->kfs);

					skinned_model->NumFrames(model_desc_.model_data->num_frames);
					skinned_model->FrameRate(model_desc_.model_data->frame_rate);

					for (size_t mesh_index = 0; mesh_index < meshes.size(); ++ mesh_index)
					{
						SkinnedMeshPtr skinned_mesh = checked_pointer_cast<SkinnedMesh>(meshes[mesh_index]);
						skinned_mesh->AttachFramePosBounds(model_desc_.model_data->frame_pos_bbs[mesh_index]);
					}
				}
			}

			model->AssignSubrenderables(meshes.begin(), meshes.end());
		}

		void AddsSubPath()
		{
			std::string sub_path;
			auto sub_path_loc = model_desc_.res_name.find_last_of('/');
			if (sub_path_loc != std::string::npos)
			{
				sub_path = ResLoader::Instance().Locate(model_desc_.res_name.substr(0, sub_path_loc));
				if (!sub_path.empty())
				{
					ResLoader::Instance().AddPath(sub_path);
				}
			}
		}

		void MainThreadStageNoLock()
		{
			RenderModelPtr const & model = *model_desc_.model;
			if (!model || !model->HWResourceReady())
			{
				this->FillModel();

				for (size_t i = 0; i < model_desc_.model_data->merged_buff.size(); ++ i)
				{
					model_desc_.model_data->merged_vbs[i]->CreateHWResource(&model_desc_.model_data->merged_buff[i][0]);
				}
				model_desc_.model_data->merged_ib->CreateHWResource(&model_desc_.model_data->merged_indices[0]);

				this->AddsSubPath();

				model->BuildModelInfo();
				for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
				{
					checked_pointer_cast<StaticMesh>(model->Subrenderable(i))->BuildMeshInfo();
				}

				model_desc_.model_data.reset();
			}
		}

	private:
		ModelDesc model_desc_;
		std::mutex main_thread_stage_mutex_;
	};
}

namespace KlayGE
{
	RenderModel::RenderModel(std::wstring const & name)
		: name_(name),
			hw_res_ready_(false)
	{
	}

	void RenderModel::NumLods(uint32_t lods)
	{
		for (auto const & mesh : subrenderables_)
		{
			mesh->NumLods(lods);
		}
	}

	uint32_t RenderModel::NumLods() const
	{
		uint32_t max_lod = 0;
		for (auto const & mesh : subrenderables_)
		{
			max_lod = std::max(max_lod, mesh->NumLods());
		}
		return max_lod;
	}

	void RenderModel::AddToRenderQueue()
	{
		for (auto const & mesh : subrenderables_)
		{
			mesh->AddToRenderQueue();
		}
	}

	void RenderModel::OnRenderBegin()
	{
		for (auto const & mesh : subrenderables_)
		{
			mesh->OnRenderBegin();
		}
	}

	void RenderModel::OnRenderEnd()
	{
		for (auto const & mesh : subrenderables_)
		{
			mesh->OnRenderEnd();
		}
	}

	AABBox const & RenderModel::PosBound() const
	{
		return pos_aabb_;
	}

	AABBox const & RenderModel::TexcoordBound() const
	{
		return tc_aabb_;
	}

	void RenderModel::UpdateBoundBox()
	{
		pos_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		for (auto const & mesh : subrenderables_)
		{
			pos_aabb_ |= mesh->PosBound();
			tc_aabb_ |= mesh->TexcoordBound();
		}
	}

	void RenderModel::Pass(PassType type)
	{
		Renderable::Pass(type);
		for (auto const & mesh : subrenderables_)
		{
			mesh->Pass(type);
		}
	}

	bool RenderModel::SpecialShading() const
	{
		bool ss = false;
		for (auto const & mesh : subrenderables_)
		{
			ss |= mesh->SpecialShading();
		}
		return ss;
	}
	
	bool RenderModel::TransparencyBackFace() const
	{
		bool ab = false;
		for (auto const & mesh : subrenderables_)
		{
			ab |= mesh->TransparencyBackFace();
		}
		return ab;
	}

	bool RenderModel::TransparencyFrontFace() const
	{
		bool ab = false;
		for (auto const & mesh : subrenderables_)
		{
			ab |= mesh->TransparencyFrontFace();
		}
		return ab;
	}

	bool RenderModel::Reflection() const
	{
		bool ab = false;
		for (auto const & mesh : subrenderables_)
		{
			ab |= mesh->Reflection();
		}
		return ab;
	}

	bool RenderModel::SimpleForward() const
	{
		bool ab = false;
		for (auto const & mesh : subrenderables_)
		{
			ab |= mesh->SimpleForward();
		}
		return ab;
	}

	bool RenderModel::HWResourceReady() const
	{
		bool ready = hw_res_ready_;
		if (ready)
		{
			for (uint32_t i = 0; i < this->NumSubrenderables(); ++ i)
			{
				ready &= checked_pointer_cast<StaticMesh>(this->Subrenderable(i))->HWResourceReady();
				if (!ready)
				{
					break;
				}
			}
		}
		return ready;
	}


	StaticMesh::StaticMesh(RenderModelPtr const & model, std::wstring const & name)
		: name_(name), model_(model),
			hw_res_ready_(false)
	{
	}

	StaticMesh::~StaticMesh()
	{
	}

	void StaticMesh::NumLods(uint32_t lods)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		rls_.resize(lods);
		for (auto& rl : rls_)
		{
			rl = rf.MakeRenderLayout();
			rl->TopologyType(RenderLayout::TT_TriangleList);
		}
	}

	void StaticMesh::DoBuildMeshInfo()
	{
		RenderModelPtr model = model_.lock();

		mtl_ = model->GetMaterial(this->MaterialID());

		for (size_t i = 0; i < RenderMaterial::TS_NumTextureSlots; ++ i)
		{
			if (!mtl_->tex_names[i].empty())
			{
				if (!ResLoader::Instance().Locate(mtl_->tex_names[i]).empty())
				{
					textures_[i] = ASyncLoadTexture(mtl_->tex_names[i], EAH_GPU_Read | EAH_Immutable);
				}
			}
		}

		if (mtl_->transparent)
		{
			effect_attrs_ |= EA_TransparencyBack;
			effect_attrs_ |= EA_TransparencyFront;
		}
		if (mtl_->alpha_test > 0)
		{
			effect_attrs_ |= EA_AlphaTest;
		}
		if (mtl_->sss)
		{
			effect_attrs_ |= EA_SSS;
		}

		if ((mtl_->emissive.x() > 0) || (mtl_->emissive.y() > 0) || (mtl_->emissive.z() > 0) || textures_[RenderMaterial::TS_Emissive]
			|| (effect_attrs_ & EA_TransparencyBack) || (effect_attrs_ & EA_TransparencyFront)
			|| (effect_attrs_ & EA_Reflection))
		{
			effect_attrs_ |= EA_SpecialShading;
		}

		auto drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			this->UpdateTechniques();
		}
	}

	std::wstring const & StaticMesh::Name() const
	{
		return name_;
	}

	AABBox const & StaticMesh::PosBound() const
	{
		return pos_aabb_;
	}

	void StaticMesh::PosBound(AABBox const & aabb)
	{
		pos_aabb_ = aabb;
	}

	AABBox const & StaticMesh::TexcoordBound() const
	{
		return tc_aabb_;
	}

	void StaticMesh::TexcoordBound(AABBox const & aabb)
	{
		tc_aabb_ = aabb;
	}

	void StaticMesh::AddVertexStream(uint32_t lod, void const * buf, uint32_t size, VertexElement const & ve, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, access_hint, size, buf);
		this->AddVertexStream(lod, vb, ve);
	}

	void StaticMesh::AddVertexStream(uint32_t lod, GraphicsBufferPtr const & buffer, VertexElement const & ve)
	{
		rls_[lod]->BindVertexStream(buffer, ve);
	}

	void StaticMesh::AddIndexStream(uint32_t lod, void const * buf, uint32_t size, ElementFormat format, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, access_hint, size, buf);
		this->AddIndexStream(lod, ib, format);
	}

	void StaticMesh::AddIndexStream(uint32_t lod, GraphicsBufferPtr const & index_stream, ElementFormat format)
	{
		rls_[lod]->BindIndexStream(index_stream, format);
	}


	std::pair<std::pair<Quaternion, Quaternion>, float> KeyFrames::Frame(float frame) const
	{
		std::pair<std::pair<Quaternion, Quaternion>, float> ret;
		if (frame_id.size() == 1)
		{
			ret.first.first = bind_real[0];
			ret.first.second = bind_dual[0];
			ret.second = bind_scale[0];
		}
		else
		{
			frame = std::fmod(frame, static_cast<float>(frame_id.back() + 1));

			auto iter = std::upper_bound(frame_id.begin(), frame_id.end(), frame);
			int index = static_cast<int>(iter - frame_id.begin());

			int index0 = index - 1;
			int index1 = index % frame_id.size();
			int frame0 = frame_id[index0];
			int frame1 = frame_id[index1];
			float factor = (frame - frame0) / (frame1 - frame0);
			ret.first = MathLib::sclerp(bind_real[index0], bind_dual[index0], bind_real[index1], bind_dual[index1], factor);
			ret.second = MathLib::lerp(bind_scale[index0], bind_scale[index1], factor);
		}
		return ret;
	}

	AABBox AABBKeyFrames::Frame(float frame) const
	{
		if (frame_id.size() == 1)
		{
			return bb[0];
		}
		else
		{
			frame = std::fmod(frame, static_cast<float>(frame_id.back() + 1));

			auto iter = std::upper_bound(frame_id.begin(), frame_id.end(), frame);
			int index = static_cast<int>(iter - frame_id.begin());

			int index0 = index - 1;
			int index1 = index % frame_id.size();
			int frame0 = frame_id[index0];
			int frame1 = frame_id[index1];
			float factor = (frame - frame0) / (frame1 - frame0);
			return AABBox(MathLib::lerp(bb[index0].Min(), bb[index1].Min(), factor),
				MathLib::lerp(bb[index0].Max(), bb[index1].Max(), factor));
		}
	}


	SkinnedModel::SkinnedModel(std::wstring const & name)
		: RenderModel(name),
			last_frame_(-1),
			num_frames_(0), frame_rate_(0)
	{
	}
	
	void SkinnedModel::BuildBones(float frame)
	{
		for (size_t i = 0; i < joints_.size(); ++ i)
		{
			Joint& joint = joints_[i];
			KeyFrames const & kf = (*key_frames_)[i];

			std::pair<std::pair<Quaternion, Quaternion>, float> key_dq = kf.Frame(frame);

			if (joint.parent != -1)
			{
				Joint const & parent(joints_[joint.parent]);

				if (MathLib::dot(key_dq.first.first, parent.bind_real) < 0)
				{
					key_dq.first.first = -key_dq.first.first;
					key_dq.first.second = -key_dq.first.second;
				}

				if ((MathLib::SignBit(key_dq.second) > 0) && (MathLib::SignBit(parent.bind_scale) > 0))
				{
					joint.bind_real = MathLib::mul_real(key_dq.first.first, parent.bind_real);
					joint.bind_dual = MathLib::mul_dual(key_dq.first.first, key_dq.first.second * parent.bind_scale, parent.bind_real, parent.bind_dual);
					joint.bind_scale = key_dq.second * parent.bind_scale;
				}
				else
				{
					float4x4 tmp_mat = MathLib::scaling(MathLib::abs(key_dq.second), MathLib::abs(key_dq.second), key_dq.second)
						* MathLib::to_matrix(key_dq.first.first)
						* MathLib::translation(MathLib::udq_to_trans(key_dq.first.first, key_dq.first.second))
						* MathLib::scaling(MathLib::abs(parent.bind_scale), MathLib::abs(parent.bind_scale), parent.bind_scale)
						* MathLib::to_matrix(parent.bind_real)
						* MathLib::translation(MathLib::udq_to_trans(parent.bind_real, parent.bind_dual));

					float flip = 1;
					if (MathLib::dot(MathLib::cross(float3(tmp_mat(0, 0), tmp_mat(0, 1), tmp_mat(0, 2)),
						float3(tmp_mat(1, 0), tmp_mat(1, 1), tmp_mat(1, 2))),
						float3(tmp_mat(2, 0), tmp_mat(2, 1), tmp_mat(2, 2))) < 0)
					{
						tmp_mat(2, 0) = -tmp_mat(2, 0);
						tmp_mat(2, 1) = -tmp_mat(2, 1);
						tmp_mat(2, 2) = -tmp_mat(2, 2);

						flip = -1;
					}

					float3 scale;
					Quaternion rot;
					float3 trans;
					MathLib::decompose(scale, rot, trans, tmp_mat);

					joint.bind_real = rot;
					joint.bind_dual = MathLib::quat_trans_to_udq(rot, trans);
					joint.bind_scale = flip * scale.x();
				}
			}
			else
			{
				joint.bind_real = key_dq.first.first;
				joint.bind_dual = key_dq.first.second;
				joint.bind_scale = key_dq.second;
			}
		}

		this->UpdateBinds();
	}

	void SkinnedModel::UpdateBinds()
	{
		bind_reals_.resize(joints_.size());
		bind_duals_.resize(joints_.size());
		for (size_t i = 0; i < joints_.size(); ++ i)
		{
			Joint const & joint = joints_[i];

			Quaternion bind_real, bind_dual;
			float bind_scale;
			if ((MathLib::SignBit(joint.inverse_origin_scale) > 0) && (MathLib::SignBit(joint.bind_scale) > 0))
			{
				bind_real = MathLib::mul_real(joint.inverse_origin_real, joint.bind_real);
				bind_dual = MathLib::mul_dual(joint.inverse_origin_real, joint.inverse_origin_dual,
					joint.bind_real, joint.bind_dual);
				bind_scale = joint.inverse_origin_scale * joint.bind_scale;

				if (MathLib::SignBit(bind_real.w()) < 0)
				{
					bind_real = -bind_real;
					bind_dual = -bind_dual;
				}
			}
			else
			{
				float4x4 tmp_mat = MathLib::scaling(MathLib::abs(joint.inverse_origin_scale), MathLib::abs(joint.inverse_origin_scale), joint.inverse_origin_scale)
					* MathLib::to_matrix(joint.inverse_origin_real)
					* MathLib::translation(MathLib::udq_to_trans(joint.inverse_origin_real, joint.inverse_origin_dual))
					* MathLib::scaling(MathLib::abs(joint.bind_scale), MathLib::abs(joint.bind_scale), joint.bind_scale)
					* MathLib::to_matrix(joint.bind_real)
					* MathLib::translation(MathLib::udq_to_trans(joint.bind_real, joint.bind_dual));

				float flip = 1;
				if (MathLib::dot(MathLib::cross(float3(tmp_mat(0, 0), tmp_mat(0, 1), tmp_mat(0, 2)),
					float3(tmp_mat(1, 0), tmp_mat(1, 1), tmp_mat(1, 2))),
					float3(tmp_mat(2, 0), tmp_mat(2, 1), tmp_mat(2, 2))) < 0)
				{
					tmp_mat(2, 0) = -tmp_mat(2, 0);
					tmp_mat(2, 1) = -tmp_mat(2, 1);
					tmp_mat(2, 2) = -tmp_mat(2, 2);

					flip = -1;
				}

				float3 scale;
				Quaternion rot;
				float3 trans;
				MathLib::decompose(scale, rot, trans, tmp_mat);

				bind_real = rot;
				bind_dual = MathLib::quat_trans_to_udq(rot, trans);
				bind_scale = scale.x();

				if (flip * MathLib::SignBit(bind_real.w()) < 0)
				{
					bind_real = -bind_real;
					bind_dual = -bind_dual;
				}
			}

			bind_reals_[i] = float4(bind_real.x(), bind_real.y(), bind_real.z(), bind_real.w()) * bind_scale;
			bind_duals_[i] = float4(bind_dual.x(), bind_dual.y(), bind_dual.z(), bind_dual.w());
		}
	}

	float SkinnedModel::GetFrame() const
	{
		return last_frame_;
	}

	void SkinnedModel::SetFrame(float frame)
	{
		if (last_frame_ != frame)
		{
			last_frame_ = frame;

			this->BuildBones(frame);
		}
	}

	void SkinnedModel::RebindJoints()
	{
		this->BuildBones(last_frame_);
	}

	void SkinnedModel::UnbindJoints()
	{
		for (size_t i = 0; i < bind_reals_.size(); ++ i)
		{
			bind_reals_[i] = float4(0, 0, 0, 1);
			bind_duals_[i] = float4(0, 0, 0, 0);
		}
	}

	AABBox SkinnedModel::FramePosBound(uint32_t frame) const
	{
		AABBox pos_aabb(float3(0, 0, 0), float3(0, 0, 0));
		for (auto const & mesh : subrenderables_)
		{
			pos_aabb |= checked_pointer_cast<SkinnedMesh>(mesh)->FramePosBound(frame);
		}

		return pos_aabb;
	}

	void SkinnedModel::AttachActions(std::shared_ptr<AnimationActionsType> const & actions)
	{
		actions_ = actions;
	}
	
	uint32_t SkinnedModel::NumActions() const
	{
		return actions_ ? static_cast<uint32_t>(actions_->size()) : 1;
	}

	void SkinnedModel::GetAction(uint32_t index, std::string& name, uint32_t& start_frame, uint32_t& end_frame)
	{
		if (actions_)
		{
			BOOST_ASSERT(index < actions_->size());

			name = (*actions_)[index].name;
			start_frame = (*actions_)[index].start_frame;
			end_frame = (*actions_)[index].end_frame;
		}
		else
		{
			BOOST_ASSERT(0 == index);

			name = "root";
			start_frame = 0;
			end_frame = num_frames_;
		}
	}


	SkinnedMesh::SkinnedMesh(RenderModelPtr const & model, std::wstring const & name)
		: StaticMesh(model, name)
	{
	}

	AABBox SkinnedMesh::FramePosBound(uint32_t frame) const
	{
		BOOST_ASSERT(frame_pos_aabbs_);
		return frame_pos_aabbs_->Frame(static_cast<float>(frame));
	}
	
	void SkinnedMesh::AttachFramePosBounds(std::shared_ptr<AABBKeyFrames> const & frame_pos_aabbs)
	{
		frame_pos_aabbs_ = frame_pos_aabbs;
	}


	std::string const jit_ext_name = ".model_bin";

	void ModelJIT(std::string const & meshml_name)
	{
		std::string::size_type const pkt_offset(meshml_name.find("//"));
		std::string folder_name;
		std::string path_name;
		if (pkt_offset != std::string::npos)
		{
			std::string pkt_name = meshml_name.substr(0, pkt_offset);
			std::string::size_type const password_offset = pkt_name.find("|");
			if (password_offset != std::string::npos)
			{
				pkt_name = pkt_name.substr(0, password_offset - 1);
			}

			std::string::size_type offset = pkt_name.rfind("/");
			if (offset != std::string::npos)
			{
				folder_name = pkt_name.substr(0, offset + 1);
			}

			std::string const file_name = meshml_name.substr(pkt_offset + 2);
			path_name = folder_name + file_name;
		}
		else
		{
			path_name = meshml_name;
		}

		bool jit = false;
		if (ResLoader::Instance().Locate(path_name + jit_ext_name).empty())
		{
			jit = true;
		}
		else
		{
			ResIdentifierPtr lzma_file = ResLoader::Instance().Open(path_name + jit_ext_name);
			uint32_t fourcc;
			lzma_file->read(&fourcc, sizeof(fourcc));
			fourcc = LE2Native(fourcc);
			uint32_t ver;
			lzma_file->read(&ver, sizeof(ver));
			ver = LE2Native(ver);
			if ((fourcc != MakeFourCC<'K', 'L', 'M', ' '>::value) || (ver != MODEL_BIN_VERSION))
			{
				jit = true;
			}
			else
			{
				ResIdentifierPtr file = ResLoader::Instance().Open(meshml_name);
				if (file)
				{
					if (lzma_file->Timestamp() < file->Timestamp())
					{
						jit = true;
					}
				}
			}
		}

#if KLAYGE_IS_DEV_PLATFORM
		if (jit)
		{
			std::string meshmljit_name = "MeshMLJIT" KLAYGE_DBG_SUFFIX;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			meshmljit_name += ".exe";
#endif
			meshmljit_name = ResLoader::Instance().Locate(meshmljit_name);
			bool failed = false;
			if (meshmljit_name.empty())
			{
				failed = true;
			}
			else
			{
#ifndef KLAYGE_PLATFORM_WINDOWS
				if (std::string::npos == meshmljit_name.find("/"))
				{
					meshmljit_name = "./" + meshmljit_name;
				}
#endif
				if (system((meshmljit_name + " -I \"" + meshml_name + "\" -T \"" + folder_name + "\" -q").c_str()) != 0)
				{
					failed = true;
				}
			}

			if (failed)
			{
				LogError("MeshMLJIT failed. Forgot to build Tools?");
			}
		}
#else
		BOOST_ASSERT(!jit);
		KFL_UNUSED(jit);
#endif
	}

	void LoadModel(std::string const & meshml_name, std::vector<RenderMaterialPtr>& mtls,
		std::vector<VertexElement>& merged_ves, char& all_is_index_16_bit,
		std::vector<std::vector<uint8_t>>& merged_buff, std::vector<uint8_t>& merged_indices,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids, std::vector<uint32_t>& mesh_lods,
		std::vector<AABBox>& pos_bbs, std::vector<AABBox>& tc_bbs,
		std::vector<uint32_t>& mesh_num_vertices, std::vector<uint32_t>& mesh_base_vertices,
		std::vector<uint32_t>& mesh_num_indices, std::vector<uint32_t>& mesh_base_indices,
		std::vector<Joint>& joints, std::shared_ptr<AnimationActionsType>& actions,
		std::shared_ptr<KeyFramesType>& kfs, uint32_t& num_frames, uint32_t& frame_rate,
		std::vector<std::shared_ptr<AABBKeyFrames>>& frame_pos_bbs)
	{
		ResIdentifierPtr lzma_file;
		if (meshml_name.rfind(jit_ext_name) + jit_ext_name.size() == meshml_name.size())
		{
			lzma_file = ResLoader::Instance().Open(meshml_name);
		}
		else
		{
			std::string full_meshml_name = ResLoader::Instance().Locate(meshml_name);
			if (full_meshml_name.empty())
			{
				full_meshml_name = meshml_name;
			}
			std::replace(full_meshml_name.begin(), full_meshml_name.end(), '\\', '/');
			ModelJIT(full_meshml_name);

			std::string no_packing_name;
			size_t offset = full_meshml_name.rfind("//");
			if (offset != std::string::npos)
			{
				no_packing_name = full_meshml_name.substr(offset + 2);
			}
			else
			{
				no_packing_name = full_meshml_name;
			}
			lzma_file = ResLoader::Instance().Open(no_packing_name + jit_ext_name);
		}
		uint32_t fourcc;
		lzma_file->read(&fourcc, sizeof(fourcc));
		fourcc = LE2Native(fourcc);
		BOOST_ASSERT((fourcc == MakeFourCC<'K', 'L', 'M', ' '>::value));

		uint32_t ver;
		lzma_file->read(&ver, sizeof(ver));
		ver = LE2Native(ver);
		BOOST_ASSERT(MODEL_BIN_VERSION == ver);

		std::shared_ptr<std::stringstream> ss = MakeSharedPtr<std::stringstream>();

		uint64_t original_len, len;
		lzma_file->read(&original_len, sizeof(original_len));
		original_len = LE2Native(original_len);
		lzma_file->read(&len, sizeof(len));
		len = LE2Native(len);

		LZMACodec lzma;
		lzma.Decode(*ss, lzma_file, len, original_len);

		ResIdentifierPtr decoded = MakeSharedPtr<ResIdentifier>(lzma_file->ResName(), lzma_file->Timestamp(), ss);

		uint32_t num_mtls;
		decoded->read(&num_mtls, sizeof(num_mtls));
		num_mtls = LE2Native(num_mtls);
		uint32_t num_meshes;
		decoded->read(&num_meshes, sizeof(num_meshes));
		num_meshes = LE2Native(num_meshes);
		uint32_t num_joints;
		decoded->read(&num_joints, sizeof(num_joints));
		num_joints = LE2Native(num_joints);
		uint32_t num_kfs;
		decoded->read(&num_kfs, sizeof(num_kfs));
		num_kfs = LE2Native(num_kfs);
		uint32_t num_actions;
		decoded->read(&num_actions, sizeof(num_actions));
		num_actions = LE2Native(num_actions);

		mtls.resize(num_mtls);
		for (uint32_t mtl_index = 0; mtl_index < num_mtls; ++ mtl_index)
		{
			RenderMaterialPtr mtl = MakeSharedPtr<RenderMaterial>();
			mtls[mtl_index] = mtl;

			mtl->name = ReadShortString(decoded);

			decoded->read(&mtl->albedo, sizeof(mtl->albedo));
			mtl->albedo.x() = LE2Native(mtl->albedo.x());
			mtl->albedo.y() = LE2Native(mtl->albedo.y());
			mtl->albedo.z() = LE2Native(mtl->albedo.z());
			mtl->albedo.w() = LE2Native(mtl->albedo.w());

			decoded->read(&mtl->metalness, sizeof(float));
			mtl->metalness = LE2Native(mtl->metalness);

			decoded->read(&mtl->glossiness, sizeof(float));
			mtl->glossiness = LE2Native(mtl->glossiness);

			decoded->read(&mtl->emissive, sizeof(mtl->emissive));
			mtl->emissive.x() = LE2Native(mtl->emissive.x());
			mtl->emissive.y() = LE2Native(mtl->emissive.y());
			mtl->emissive.z() = LE2Native(mtl->emissive.z());

			uint8_t transparent;
			decoded->read(&transparent, sizeof(transparent));
			mtl->transparent = transparent ? true : false;

			uint8_t alpha_test;
			decoded->read(&alpha_test, sizeof(uint8_t));
			mtl->alpha_test = alpha_test / 255.0f;

			uint8_t sss;
			decoded->read(&sss, sizeof(sss));
			mtl->sss = sss ? true : false;

			uint8_t two_sided;
			decoded->read(&two_sided, sizeof(two_sided));
			mtl->two_sided = two_sided ? true : false;

			for (size_t i = 0; i < RenderMaterial::TS_NumTextureSlots; ++ i)
			{
				mtl->tex_names[i] = ReadShortString(decoded);
			}
			if (!mtl->tex_names[RenderMaterial::TS_Height].empty())
			{
				float height_offset;
				decoded->read(&height_offset, sizeof(height_offset));
				mtl->height_offset_scale.x() = LE2Native(height_offset);
				float height_scale;
				decoded->read(&height_scale, sizeof(height_scale));
				mtl->height_offset_scale.y() = LE2Native(height_scale);
			}

			uint8_t detail_mode;
			decoded->read(&detail_mode, sizeof(detail_mode));
			mtl->detail_mode = static_cast<RenderMaterial::SurfaceDetailMode>(detail_mode);
			if (mtl->detail_mode != RenderMaterial::SDM_Parallax)
			{
				float tess_factor;
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->tess_factors.x() = LE2Native(tess_factor);
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->tess_factors.y() = LE2Native(tess_factor);
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->tess_factors.z() = LE2Native(tess_factor);
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->tess_factors.w() = LE2Native(tess_factor);
			}
			else
			{
				mtl->tess_factors = float4(5, 5, 1, 9);
			}
		}

		uint32_t num_merged_ves;
		decoded->read(&num_merged_ves, sizeof(num_merged_ves));
		num_merged_ves = LE2Native(num_merged_ves);
		merged_ves.resize(num_merged_ves);
		for (size_t i = 0; i < merged_ves.size(); ++ i)
		{
			decoded->read(&merged_ves[i], sizeof(merged_ves[i]));

			merged_ves[i].usage = LE2Native(merged_ves[i].usage);
			merged_ves[i].format = LE2Native(merged_ves[i].format);
		}

		uint32_t all_num_vertices;
		uint32_t all_num_indices;
		decoded->read(&all_num_vertices, sizeof(all_num_vertices));
		all_num_vertices = LE2Native(all_num_vertices);
		decoded->read(&all_num_indices, sizeof(all_num_indices));
		all_num_indices = LE2Native(all_num_indices);
		decoded->read(&all_is_index_16_bit, sizeof(all_is_index_16_bit));

		int const index_elem_size = all_is_index_16_bit ? 2 : 4;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		merged_buff.resize(merged_ves.size());
		for (size_t i = 0; i < merged_buff.size(); ++ i)
		{
			merged_buff[i].resize(all_num_vertices * merged_ves[i].element_size());
			decoded->read(&merged_buff[i][0], merged_buff[i].size() * sizeof(merged_buff[i][0]));

			if ((EF_A2BGR10 == merged_ves[i].format) && !rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_A2BGR10))
			{
				merged_ves[i].format = EF_ARGB8;

				uint32_t* p = reinterpret_cast<uint32_t*>(&merged_buff[i][0]);
				for (uint32_t j = 0; j < all_num_vertices; ++ j)
				{
					float x = ((p[j] >>  0) & 0x3FF) / 1023.0f;
					float y = ((p[j] >> 10) & 0x3FF) / 1023.0f;
					float z = ((p[j] >> 20) & 0x3FF) / 1023.0f;
					float w = ((p[j] >> 30) & 0x3) / 3.0f;

					p[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(x * 255), 0, 255) << 16)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(y * 255), 0, 255) << 8)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(z * 255), 0, 255) << 0)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(w * 255), 0, 255) << 24);
				}
			}
			if ((EF_ARGB8 == merged_ves[i].format) && !rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ARGB8))
			{
				BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ABGR8));

				merged_ves[i].format = EF_ABGR8;

				uint32_t* p = reinterpret_cast<uint32_t*>(&merged_buff[i][0]);
				for (uint32_t j = 0; j < all_num_vertices; ++ j)
				{
					float x = ((p[j] >> 16) & 0xFF) / 255.0f;
					float y = ((p[j] >>  8) & 0xFF) / 255.0f;
					float z = ((p[j] >>  0) & 0xFF) / 255.0f;
					float w = ((p[j] >> 24) & 0xFF) / 255.0f;

					p[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(x * 255), 0, 255) << 0)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(y * 255), 0, 255) << 8)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(z * 255), 0, 255) << 16)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(w * 255), 0, 255) << 24);
				}
			}
		}
		merged_indices.resize(all_num_indices * index_elem_size);
		decoded->read(&merged_indices[0], merged_indices.size() * sizeof(merged_indices[0]));

		mesh_names.resize(num_meshes);
		mtl_ids.resize(num_meshes);
		mesh_lods.resize(num_meshes);
		pos_bbs.resize(num_meshes);
		tc_bbs.resize(num_meshes);
		mesh_num_vertices.clear();
		mesh_base_vertices.clear();
		mesh_num_indices.clear();
		mesh_base_indices.clear();
		for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
		{
			mesh_names[mesh_index] = ReadShortString(decoded);

			decoded->read(&mtl_ids[mesh_index], sizeof(mtl_ids[mesh_index]));
			mtl_ids[mesh_index] = LE2Native(mtl_ids[mesh_index]);

			decoded->read(&mesh_lods[mesh_index], sizeof(mesh_lods[mesh_index]));
			mesh_lods[mesh_index] = LE2Native(mesh_lods[mesh_index]);

			float3 min_bb, max_bb;
			decoded->read(&min_bb, sizeof(min_bb));
			min_bb.x() = LE2Native(min_bb.x());
			min_bb.y() = LE2Native(min_bb.y());
			min_bb.z() = LE2Native(min_bb.z());
			decoded->read(&max_bb, sizeof(max_bb));
			max_bb.x() = LE2Native(max_bb.x());
			max_bb.y() = LE2Native(max_bb.y());
			max_bb.z() = LE2Native(max_bb.z());
			pos_bbs[mesh_index] = AABBox(min_bb, max_bb);

			decoded->read(&min_bb[0], sizeof(min_bb[0]));
			decoded->read(&min_bb[1], sizeof(min_bb[1]));
			min_bb.x() = LE2Native(min_bb.x());
			min_bb.y() = LE2Native(min_bb.y());
			min_bb.z() = 0;
			decoded->read(&max_bb[0], sizeof(max_bb[0]));
			decoded->read(&max_bb[1], sizeof(max_bb[1]));
			max_bb.x() = LE2Native(max_bb.x());
			max_bb.y() = LE2Native(max_bb.y());
			max_bb.z() = 0;
			tc_bbs[mesh_index] = AABBox(min_bb, max_bb);

			for (uint32_t lod = 0; lod < mesh_lods[mesh_index]; ++ lod)
			{
				uint32_t tmp;
				decoded->read(&tmp, sizeof(tmp));
				mesh_num_vertices.push_back(LE2Native(tmp));
				decoded->read(&tmp, sizeof(tmp));
				mesh_base_vertices.push_back(LE2Native(tmp));
				decoded->read(&tmp, sizeof(tmp));
				mesh_num_indices.push_back(LE2Native(tmp));
				decoded->read(&tmp, sizeof(tmp));
				mesh_base_indices.push_back(LE2Native(tmp));
			}
		}

		joints.resize(num_joints);
		for (uint32_t joint_index = 0; joint_index < num_joints; ++ joint_index)
		{
			Joint& joint = joints[joint_index];

			joint.name = ReadShortString(decoded);
			decoded->read(&joint.parent, sizeof(joint.parent));
			joint.parent = LE2Native(joint.parent);

			decoded->read(&joint.bind_real, sizeof(joint.bind_real));
			joint.bind_real[0] = LE2Native(joint.bind_real[0]);
			joint.bind_real[1] = LE2Native(joint.bind_real[1]);
			joint.bind_real[2] = LE2Native(joint.bind_real[2]);
			joint.bind_real[3] = LE2Native(joint.bind_real[3]);
			decoded->read(&joint.bind_dual, sizeof(joint.bind_dual));
			joint.bind_dual[0] = LE2Native(joint.bind_dual[0]);
			joint.bind_dual[1] = LE2Native(joint.bind_dual[1]);
			joint.bind_dual[2] = LE2Native(joint.bind_dual[2]);
			joint.bind_dual[3] = LE2Native(joint.bind_dual[3]);

			float flip = MathLib::SignBit(joint.bind_real.w());

			joint.bind_scale = MathLib::length(joint.bind_real);
			joint.inverse_origin_scale = 1 / joint.bind_scale;
			joint.bind_real *= joint.inverse_origin_scale;

			if (flip > 0)
			{
				std::pair<Quaternion, Quaternion> inv = MathLib::inverse(joint.bind_real, joint.bind_dual);
				joint.inverse_origin_real = inv.first;
				joint.inverse_origin_dual = inv.second;
			}
			else
			{
				float4x4 tmp_mat = MathLib::scaling(joint.bind_scale, joint.bind_scale, flip * joint.bind_scale)
					* MathLib::to_matrix(joint.bind_real)
					* MathLib::translation(MathLib::udq_to_trans(joint.bind_real, joint.bind_dual));
				tmp_mat = MathLib::inverse(tmp_mat);
				tmp_mat(2, 0) = -tmp_mat(2, 0);
				tmp_mat(2, 1) = -tmp_mat(2, 1);
				tmp_mat(2, 2) = -tmp_mat(2, 2);

				float3 scale;
				Quaternion rot;
				float3 trans;
				MathLib::decompose(scale, rot, trans, tmp_mat);

				joint.inverse_origin_real = rot;
				joint.inverse_origin_dual = MathLib::quat_trans_to_udq(rot, trans);
				joint.inverse_origin_scale = -scale.x();
			}
			
			joint.bind_scale *= flip;
		}

		if (num_kfs > 0)
		{
			decoded->read(&num_frames, sizeof(num_frames));
			num_frames = LE2Native(num_frames);
			decoded->read(&frame_rate, sizeof(frame_rate));
			frame_rate = LE2Native(frame_rate);

			kfs = MakeSharedPtr<KeyFramesType>(joints.size());
			for (uint32_t kf_index = 0; kf_index < num_kfs; ++ kf_index)
			{
				uint32_t joint_index = kf_index;

				uint32_t num_kf;
				decoded->read(&num_kf, sizeof(num_kf));
				num_kf = LE2Native(num_kf);

				KeyFrames kf;
				kf.frame_id.resize(num_kf);
				kf.bind_real.resize(num_kf);
				kf.bind_dual.resize(num_kf);
				kf.bind_scale.resize(num_kf);
				for (uint32_t k_index = 0; k_index < num_kf; ++ k_index)
				{
					decoded->read(&kf.frame_id[k_index], sizeof(kf.frame_id[k_index]));
					kf.frame_id[k_index] = LE2Native(kf.frame_id[k_index]);
					decoded->read(&kf.bind_real[k_index], sizeof(kf.bind_real[k_index]));
					kf.bind_real[k_index][0] = LE2Native(kf.bind_real[k_index][0]);
					kf.bind_real[k_index][1] = LE2Native(kf.bind_real[k_index][1]);
					kf.bind_real[k_index][2] = LE2Native(kf.bind_real[k_index][2]);
					kf.bind_real[k_index][3] = LE2Native(kf.bind_real[k_index][3]);
					decoded->read(&kf.bind_dual[k_index], sizeof(kf.bind_dual[k_index]));
					kf.bind_dual[k_index][0] = LE2Native(kf.bind_dual[k_index][0]);
					kf.bind_dual[k_index][1] = LE2Native(kf.bind_dual[k_index][1]);
					kf.bind_dual[k_index][2] = LE2Native(kf.bind_dual[k_index][2]);
					kf.bind_dual[k_index][3] = LE2Native(kf.bind_dual[k_index][3]);

					float flip = MathLib::SignBit(kf.bind_real[k_index].w());

					kf.bind_scale[k_index] = MathLib::length(kf.bind_real[k_index]);
					kf.bind_real[k_index] /= kf.bind_scale[k_index];

					kf.bind_scale[k_index] *= flip;
				}

				if (joint_index < num_joints)
				{
					(*kfs)[joint_index] = kf;
				}
			}

			frame_pos_bbs.resize(num_meshes);
			for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
			{
				uint32_t num_bb_kf;
				decoded->read(&num_bb_kf, sizeof(num_bb_kf));
				num_bb_kf = LE2Native(num_bb_kf);

				frame_pos_bbs[mesh_index] = MakeSharedPtr<AABBKeyFrames>();
				frame_pos_bbs[mesh_index]->frame_id.resize(num_bb_kf);
				frame_pos_bbs[mesh_index]->bb.resize(num_bb_kf);

				for (uint32_t bb_k_index = 0; bb_k_index < num_bb_kf; ++ bb_k_index)
				{
					decoded->read(&frame_pos_bbs[mesh_index]->frame_id[bb_k_index], sizeof(frame_pos_bbs[mesh_index]->frame_id[bb_k_index]));
					frame_pos_bbs[mesh_index]->frame_id[bb_k_index] = LE2Native(frame_pos_bbs[mesh_index]->frame_id[bb_k_index]);

					float3 bb_min, bb_max;
					decoded->read(&bb_min, sizeof(bb_min));
					bb_min[0] = LE2Native(bb_min[0]);
					bb_min[1] = LE2Native(bb_min[1]);
					bb_min[2] = LE2Native(bb_min[2]);
					decoded->read(&bb_max, sizeof(bb_max));
					bb_max[0] = LE2Native(bb_max[0]);
					bb_max[1] = LE2Native(bb_max[1]);
					bb_max[2] = LE2Native(bb_max[2]);
					frame_pos_bbs[mesh_index]->bb[bb_k_index] = AABBox(bb_min, bb_max);
				}
			}
			
			if (num_actions > 0)
			{
				actions = MakeSharedPtr<AnimationActionsType>(num_actions);
				for (uint32_t action_index = 0; action_index < num_actions; ++ action_index)
				{
					AnimationAction action;
					action.name = ReadShortString(decoded);
					decoded->read(&action.start_frame, sizeof(action.start_frame));
					action.start_frame = LE2Native(action.start_frame);
					decoded->read(&action.end_frame, sizeof(action.end_frame));
					action.end_frame = LE2Native(action.end_frame);
					actions->push_back(action);
				}
			}
		}
	}

	RenderModelPtr SyncLoadModel(std::string const & meshml_name, uint32_t access_hint,
		std::function<RenderModelPtr(std::wstring const &)> CreateModelFactoryFunc,
		std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		return ResLoader::Instance().SyncQueryT<RenderModel>(MakeSharedPtr<RenderModelLoadingDesc>(meshml_name,
			access_hint, CreateModelFactoryFunc, CreateMeshFactoryFunc));
	}

	RenderModelPtr ASyncLoadModel(std::string const & meshml_name, uint32_t access_hint,
		std::function<RenderModelPtr(std::wstring const &)> CreateModelFactoryFunc,
		std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		return ResLoader::Instance().ASyncQueryT<RenderModel>(MakeSharedPtr<RenderModelLoadingDesc>(meshml_name,
			access_hint, CreateModelFactoryFunc, CreateMeshFactoryFunc));
	}

	void SaveModelToMeshML(std::string const & meshml_name, std::vector<RenderMaterialPtr> const & mtls,
		std::vector<VertexElement> const & merged_ves, char all_is_index_16_bit, 
		std::vector<std::vector<uint8_t>> const & merged_buffs, std::vector<uint8_t> const & merged_indices,
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<uint32_t> const & mesh_lods,
		std::vector<AABBox> const & pos_bbs, std::vector<AABBox> const & tc_bbs,
		std::vector<uint32_t> const & mesh_num_vertices, std::vector<uint32_t> const & mesh_base_vertices,
		std::vector<uint32_t> const & mesh_num_indices, std::vector<uint32_t> const & mesh_base_indices,
		std::vector<Joint> const & joints, std::shared_ptr<AnimationActionsType> const & actions,
		std::shared_ptr<KeyFramesType> const & kfs, uint32_t num_frames, uint32_t frame_rate)
	{
		MeshMLObj obj(1);
		obj.NumFrames(num_frames);
		obj.FrameRate(frame_rate);

		std::map<size_t, int> joint_map;
		for (size_t i = 0; i < joints.size(); ++ i)
		{
			int joint_id = obj.AllocJoint();
			joint_map.emplace(i, joint_id);

			int parent_id = -1;
			if (joints[i].parent != -1)
			{
				parent_id = joint_map[joints[i].parent];
			}

			obj.SetJoint(joint_id, joints[i].name, parent_id, joints[i].bind_real * joints[i].bind_scale, joints[i].bind_dual);
		}

		std::map<size_t, int> mtl_map;
		for (size_t i = 0; i < mtls.size(); ++ i)
		{
			int mtl_id = obj.AllocMaterial();
			mtl_map.emplace(i, mtl_id);

			obj.SetMaterial(mtl_id, mtls[i]->name, mtls[i]->albedo, mtls[i]->metalness, mtls[i]->glossiness,
				mtls[i]->emissive, mtls[i]->transparent, mtls[i]->alpha_test, mtls[i]->sss, mtls[i]->two_sided);

			KLAYGE_STATIC_ASSERT(static_cast<int>(MeshMLObj::Material::TS_Albedo) == RenderMaterial::TS_Albedo);
			KLAYGE_STATIC_ASSERT(static_cast<int>(MeshMLObj::Material::TS_Metalness) == RenderMaterial::TS_Metalness);
			KLAYGE_STATIC_ASSERT(static_cast<int>(MeshMLObj::Material::TS_Glossiness) == RenderMaterial::TS_Glossiness);
			KLAYGE_STATIC_ASSERT(static_cast<int>(MeshMLObj::Material::TS_Emissive) == RenderMaterial::TS_Emissive);
			KLAYGE_STATIC_ASSERT(static_cast<int>(MeshMLObj::Material::TS_Normal) == RenderMaterial::TS_Normal);
			KLAYGE_STATIC_ASSERT(static_cast<int>(MeshMLObj::Material::TS_Height) == RenderMaterial::TS_Height);
			for (size_t j = 0; j < RenderMaterial::TS_NumTextureSlots; ++ j)
			{
				obj.SetTextureSlot(mtl_id, static_cast<MeshMLObj::Material::TextureSlot>(j), mtls[i]->tex_names[j]);
			}

			obj.SetDetailMaterial(mtl_id, static_cast<MeshMLObj::Material::SurfaceDetailMode>(mtls[i]->detail_mode),
				mtls[i]->height_offset_scale.x(), mtls[i]->height_offset_scale.y(),
				mtls[i]->tess_factors.x(), mtls[i]->tess_factors.y(), mtls[i]->tess_factors.z(), mtls[i]->tess_factors.w());
		}

		uint32_t mesh_lod_index = 0;
		for (size_t i = 0; i < mesh_names.size(); ++ i)
		{
			int mesh_id = obj.AllocMesh();
			
			obj.SetMesh(mesh_id, mtl_map[mtl_ids[i]], mesh_names[i], mesh_lods[i]);

			AABBox const & pos_bb = pos_bbs[i];
			AABBox const & tc_bb = tc_bbs[i];
			float3 const pos_center = pos_bb.Center();
			float3 const pos_extent = pos_bb.HalfSize();
			float3 const tc_center = tc_bb.Center();
			float3 const tc_extent = tc_bb.HalfSize();

			for (uint32_t lod = 0; lod < mesh_lods[i]; ++ lod, ++ mesh_lod_index)
			{
				for (size_t v = 0; v < mesh_num_vertices[mesh_lod_index]; ++ v)
				{
					int vert_id = obj.AllocVertex(mesh_id, lod);

					float3 pos;
					float3 normal;
					Quaternion tangent_quat;
					std::vector<float3> texcoords;
					std::vector<std::pair<int, float>> bindings;
					bool has_normal = false;
					for (size_t ve = 0; ve < merged_ves.size(); ++ ve)
					{
						uint8_t const * src = &merged_buffs[ve][(mesh_base_vertices[i] + v) * merged_ves[ve].element_size()];

						switch (merged_ves[ve].usage)
						{
						case VEU_Position:
							pos = float3(0, 0, 0);
							switch (merged_ves[ve].format)
							{
							case EF_ABGR32F:
							case EF_BGR32F:
							case EF_GR32F:
							case EF_R32F:
								std::memcpy(&pos, src, std::min<int>(merged_ves[ve].element_size(), sizeof(pos)));
								break;

							default:
								{
									BOOST_ASSERT(EF_SIGNED_ABGR16 == merged_ves[ve].format);

									int16_t const * p = reinterpret_cast<int16_t const *>(src);
									pos.x() = (((p[0] + 32768) / 65536.0f) * 2 - 1) * pos_extent.x() + pos_center.x();
									pos.y() = (((p[1] + 32768) / 65536.0f) * 2 - 1) * pos_extent.y() + pos_center.y();
									pos.z() = (((p[2] + 32768) / 65536.0f) * 2 - 1) * pos_extent.z() + pos_center.z();
								}
								break;
							}
							break;

						case VEU_TextureCoord:
							texcoords.push_back(float3(0, 0, 0));
							switch (merged_ves[ve].format)
							{
							case EF_ABGR32F:
							case EF_BGR32F:
							case EF_GR32F:
							case EF_R32F:
								std::memcpy(&texcoords.back(), src, std::min<int>(merged_ves[ve].element_size(), sizeof(texcoords.back())));
								break;

							default:
								{
									BOOST_ASSERT(EF_SIGNED_GR16 == merged_ves[ve].format);

									int16_t const * p = reinterpret_cast<int16_t const *>(src);
									texcoords.back().x() = (((p[0] + 32768) / 65536.0f) * 2 - 1) * tc_extent.x() + tc_center.x();
									texcoords.back().y() = (((p[1] + 32768) / 65536.0f) * 2 - 1) * tc_extent.y() + tc_center.y();
								}
								break;
							}
							break;

						case VEU_Normal:
							has_normal = true;
							switch (merged_ves[ve].format)
							{
							case EF_ABGR32F:
							case EF_BGR32F:
								std::memcpy(&normal, src, std::min<int>(merged_ves[ve].element_size(), sizeof(normal)));
								break;

							case EF_A2BGR10:
								{
									uint32_t const p = *reinterpret_cast<uint32_t const *>(src);
									normal.x() = ((p >>  0) & 0x3FF) / 1023.0f * 2 - 1;
									normal.y() = ((p >> 10) & 0x3FF) / 1023.0f * 2 - 1;
									normal.z() = ((p >> 20) & 0x3FF) / 1023.0f * 2 - 1;
								}
								break;

							case EF_ABGR8:
								{
									uint32_t const p = *reinterpret_cast<uint32_t const *>(src);
									normal.x() = ((p >>  0) & 0xFF) / 255.0f * 2 - 1;
									normal.y() = ((p >>  8) & 0xFF) / 255.0f * 2 - 1;
									normal.z() = ((p >> 16) & 0xFF) / 255.0f * 2 - 1;
								}
								break;

							default:
								{
									BOOST_ASSERT(EF_ARGB8 == merged_ves[ve].format);

									uint32_t const p = *reinterpret_cast<uint32_t const *>(src);
									normal.x() = ((p >> 16) & 0xFF) / 255.0f * 2 - 1;
									normal.y() = ((p >>  8) & 0xFF) / 255.0f * 2 - 1;
									normal.z() = ((p >>  0) & 0xFF) / 255.0f * 2 - 1;
								}
								break;
							}
							break;

						case VEU_Tangent:
							has_normal = false;
							switch (merged_ves[ve].format)
							{
							case EF_ABGR32F:
								std::memcpy(&tangent_quat, src, std::min<int>(merged_ves[ve].element_size(), sizeof(tangent_quat)));
								break;

							case EF_ABGR8:
								{
									uint32_t const p = *reinterpret_cast<uint32_t const *>(src);
									tangent_quat.x() = ((p >>  0) & 0xFF) / 255.0f * 2 - 1;
									tangent_quat.y() = ((p >>  8) & 0xFF) / 255.0f * 2 - 1;
									tangent_quat.z() = ((p >> 16) & 0xFF) / 255.0f * 2 - 1;
									tangent_quat.w() = ((p >> 24) & 0xFF) / 255.0f * 2 - 1;
								}
								break;

							default:
								{
									BOOST_ASSERT(EF_ARGB8 == merged_ves[ve].format);

									uint32_t const p = *reinterpret_cast<uint32_t const *>(src);
									tangent_quat.x() = ((p >> 16) & 0xFF) / 255.0f * 2 - 1;
									tangent_quat.y() = ((p >>  8) & 0xFF) / 255.0f * 2 - 1;
									tangent_quat.z() = ((p >>  0) & 0xFF) / 255.0f * 2 - 1;
									tangent_quat.w() = ((p >> 24) & 0xFF) / 255.0f * 2 - 1;
								}
								break;
							}
							break;

						case VEU_BlendIndex:
							bindings.resize(4);
							for (int b = 0; b < 4; ++ b)
							{
								bindings[b].first = src[b];
							}
							break;

						case VEU_BlendWeight:
							bindings.resize(4);
							switch (merged_ves[ve].format)
							{
							case EF_ABGR32F:
								{
									float const * p = reinterpret_cast<float const *>(src);
									for (int b = 0; b < 4; ++ b)
									{
										bindings[b].second = p[b];
									}
								}
								break;

							case EF_ABGR8:
								for (int b = 0; b < 4; ++ b)
								{
									bindings[b].second = src[b] / 255.0f;
								}
								break;

							default:
								BOOST_ASSERT(EF_ARGB8 == merged_ves[ve].format);
								
								for (int b = 0; b < 4; ++ b)
								{
									bindings[b].second = src[b] / 255.0f;
								}
								std::swap(bindings[0].second, bindings[2].second);
								break;
							}
							break;

						default:
							break;
						}
					}
					if (has_normal)
					{
						obj.SetVertex(mesh_id, lod, vert_id, pos, normal, 2, texcoords);
					}
					else
					{
						obj.SetVertex(mesh_id, lod, vert_id, pos, tangent_quat, 2, texcoords);
					}

					for (size_t b = 0; b < bindings.size(); ++ b)
					{
						if (bindings[b].second > 0)
						{
							int binding_id = obj.AllocJointBinding(mesh_id, lod, vert_id);
							obj.SetJointBinding(mesh_id, lod, vert_id, binding_id, joint_map[bindings[b].first], bindings[b].second);
						}
					}
				}

				for (size_t t = 0; t < mesh_num_indices[mesh_lod_index]; t += 3)
				{
					int tri_id = obj.AllocTriangle(mesh_id, lod);
					int index[3];
					if (all_is_index_16_bit)
					{
						uint16_t const * src = reinterpret_cast<uint16_t const *>(&merged_indices[(mesh_base_indices[mesh_lod_index] + t) * sizeof(uint16_t)]);
						index[0] = src[0];
						index[1] = src[1];
						index[2] = src[2];
					}
					else
					{
						uint32_t const * src = reinterpret_cast<uint32_t const *>(&merged_indices[(mesh_base_indices[mesh_lod_index] + t)* sizeof(uint32_t)]);
						index[0] = src[0];
						index[1] = src[1];
						index[2] = src[2];
					}
					obj.SetTriangle(mesh_id, lod, tri_id, index[0], index[1], index[2]);
				}
			}
		}

		if (kfs)
		{
			for (size_t i = 0; i < kfs->size(); ++ i)
			{
				int kfs_id = obj.AllocKeyframes();
				obj.SetKeyframes(kfs_id, joint_map[i]);

				for (size_t k = 0; k < (*kfs)[i].frame_id.size(); ++ k)
				{
					int kf_id = obj.AllocKeyframe(kfs_id);
					obj.SetKeyframe(kfs_id, kf_id, (*kfs)[i].frame_id[k], (*kfs)[i].bind_real[k] * (*kfs)[i].bind_scale[k], (*kfs)[i].bind_dual[k]);
				}
			}

			if (actions)
			{
				for (size_t i = 0; i < actions->size(); ++ i)
				{
					int action_id = obj.AllocAction();
					obj.SetAction(action_id, (*actions)[i].name, (*actions)[i].start_frame, (*actions)[i].end_frame);
				}
			}
			else
			{
				int action_id = obj.AllocAction();
				obj.SetAction(action_id, "root", 0, num_frames);
			}
		}

		std::ofstream ofs(meshml_name.c_str());
		if (!ofs)
		{
			ofs.open((ResLoader::Instance().LocalFolder() + meshml_name).c_str());
		}
		obj.WriteMeshML(ofs);
	}

	void WriteMaterialsChunk(std::vector<RenderMaterialPtr> const & mtls, std::ostream& os)
	{
		for (size_t i = 0; i < mtls.size(); ++ i)
		{
			auto& mtl = *mtls[i];

			WriteShortString(os, mtl.name);

			for (uint32_t j = 0; j < 4; ++ j)
			{
				float const value = Native2LE(mtl.albedo[j]);
				os.write(reinterpret_cast<char const *>(&value), sizeof(value));
			}

			float metalness = Native2LE(mtl.metalness);
			os.write(reinterpret_cast<char*>(&metalness), sizeof(metalness));

			float glossiness = Native2LE(mtl.glossiness);
			os.write(reinterpret_cast<char*>(&glossiness), sizeof(glossiness));

			for (uint32_t j = 0; j < 3; ++ j)
			{
				float const value = Native2LE(mtl.emissive[j]);
				os.write(reinterpret_cast<char const *>(&value), sizeof(value));
			}

			uint8_t transparent = mtl.transparent;
			os.write(reinterpret_cast<char*>(&transparent), sizeof(transparent));

			uint8_t alpha_test = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(mtl.alpha_test * 255.0f + 0.5f), 0, 255));
			os.write(reinterpret_cast<char*>(&alpha_test), sizeof(alpha_test));

			uint8_t sss = mtl.sss;
			os.write(reinterpret_cast<char*>(&sss), sizeof(sss));

			uint8_t two_sided = mtl.two_sided;
			os.write(reinterpret_cast<char*>(&two_sided), sizeof(two_sided));

			for (size_t j = 0; j < RenderMaterial::TS_NumTextureSlots; ++ j)
			{
				WriteShortString(os, mtl.tex_names[j]);
			}
			if (!mtl.tex_names[RenderMaterial::TS_Height].empty())
			{
				float height_offset = Native2LE(mtl.height_offset_scale.x());
				os.write(reinterpret_cast<char*>(&height_offset), sizeof(height_offset));
				float height_scale = Native2LE(mtl.height_offset_scale.y());
				os.write(reinterpret_cast<char*>(&height_scale), sizeof(height_scale));
			}

			uint8_t detail_mode = static_cast<uint8_t>(mtl.detail_mode);
			os.write(reinterpret_cast<char*>(&detail_mode), sizeof(detail_mode));
			if (mtl.detail_mode != RenderMaterial::SDM_Parallax)
			{
				float tess_factor = Native2LE(mtl.tess_factors.x());
				os.write(reinterpret_cast<char*>(&tess_factor), sizeof(tess_factor));
				tess_factor = Native2LE(mtl.tess_factors.y());
				os.write(reinterpret_cast<char*>(&tess_factor), sizeof(tess_factor));
				tess_factor = Native2LE(mtl.tess_factors.z());
				os.write(reinterpret_cast<char*>(&tess_factor), sizeof(tess_factor));
				tess_factor = Native2LE(mtl.tess_factors.w());
				os.write(reinterpret_cast<char*>(&tess_factor), sizeof(tess_factor));
			}
		}
	}

	void WriteMeshesChunk(std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<uint32_t> const & mesh_lods,
		std::vector<AABBox> const & pos_bbs, std::vector<AABBox> const & tc_bbs,
		std::vector<uint32_t> const & mesh_num_vertices, std::vector<uint32_t> const & mesh_base_vertices,
		std::vector<uint32_t> const & mesh_num_indices, std::vector<uint32_t> const & mesh_start_indices,
		std::vector<VertexElement> const & merged_ves,
		std::vector<std::vector<uint8_t>> const & merged_vertices, std::vector<uint8_t> const & merged_indices,
		char is_index_16_bit, std::ostream& os)
	{
		uint32_t num_merged_ves = Native2LE(static_cast<uint32_t>(merged_ves.size()));
		os.write(reinterpret_cast<char*>(&num_merged_ves), sizeof(num_merged_ves));
		for (size_t i = 0; i < merged_ves.size(); ++ i)
		{
			VertexElement ve = merged_ves[i];
			ve.usage = Native2LE(ve.usage);
			ve.format = Native2LE(ve.format);
			os.write(reinterpret_cast<char*>(&ve), sizeof(ve));
		}

		uint32_t num_vertices = Native2LE(mesh_base_vertices.back());
		os.write(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));
		uint32_t num_indices = Native2LE(mesh_start_indices.back());
		os.write(reinterpret_cast<char*>(&num_indices), sizeof(num_indices));
		os.write(&is_index_16_bit, sizeof(is_index_16_bit));

		for (size_t i = 0; i < merged_vertices.size(); ++ i)
		{
			os.write(reinterpret_cast<char const *>(&merged_vertices[i][0]), merged_vertices[i].size() * sizeof(merged_vertices[i][0]));
		}
		os.write(reinterpret_cast<char const *>(&merged_indices[0]), merged_indices.size() * sizeof(merged_indices[0]));

		uint32_t mesh_lod_index = 0;
		for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
		{
			WriteShortString(os, mesh_names[mesh_index]);

			int32_t mtl_id = Native2LE(mtl_ids[mesh_index]);
			os.write(reinterpret_cast<char*>(&mtl_id), sizeof(mtl_id));

			uint32_t lods = Native2LE(mesh_lods[mesh_index]);
			os.write(reinterpret_cast<char*>(&lods), sizeof(lods));

			float3 min_bb;
			min_bb.x() = Native2LE(pos_bbs[mesh_index].Min().x());
			min_bb.y() = Native2LE(pos_bbs[mesh_index].Min().y());
			min_bb.z() = Native2LE(pos_bbs[mesh_index].Min().z());
			os.write(reinterpret_cast<char*>(&min_bb), sizeof(min_bb));
			float3 max_bb;
			max_bb.x() = Native2LE(pos_bbs[mesh_index].Max().x());
			max_bb.y() = Native2LE(pos_bbs[mesh_index].Max().y());
			max_bb.z() = Native2LE(pos_bbs[mesh_index].Max().z());
			os.write(reinterpret_cast<char*>(&max_bb), sizeof(max_bb));

			min_bb.x() = Native2LE(tc_bbs[mesh_index].Min().x());
			min_bb.y() = Native2LE(tc_bbs[mesh_index].Min().y());
			os.write(reinterpret_cast<char*>(&min_bb[0]), sizeof(min_bb[0]));
			os.write(reinterpret_cast<char*>(&min_bb[1]), sizeof(min_bb[1]));
			max_bb.x() = Native2LE(tc_bbs[mesh_index].Max().x());
			max_bb.y() = Native2LE(tc_bbs[mesh_index].Max().y());
			os.write(reinterpret_cast<char*>(&max_bb[0]), sizeof(max_bb[0]));
			os.write(reinterpret_cast<char*>(&max_bb[1]), sizeof(max_bb[1]));

			for (uint32_t lod = 0; lod < lods; ++ lod, ++ mesh_lod_index)
			{
				uint32_t nv = Native2LE(mesh_num_vertices[mesh_lod_index]);
				os.write(reinterpret_cast<char*>(&nv), sizeof(nv));
				uint32_t bv = Native2LE(mesh_base_vertices[mesh_lod_index]);
				os.write(reinterpret_cast<char*>(&bv), sizeof(bv));
				uint32_t ni = Native2LE(mesh_num_indices[mesh_lod_index]);
				os.write(reinterpret_cast<char*>(&ni), sizeof(ni));
				uint32_t si = Native2LE(mesh_start_indices[mesh_lod_index]);
				os.write(reinterpret_cast<char*>(&si), sizeof(si));
			}
		}
	}

	void WriteBonesChunk(std::vector<Joint> const & joints, std::ostream& os)
	{
		for (size_t i = 0; i < joints.size(); ++ i)
		{
			WriteShortString(os, joints[i].name);

			int16_t joint_parent = Native2LE(joints[i].parent);
			os.write(reinterpret_cast<char*>(&joint_parent), sizeof(joint_parent));

			Quaternion bind_real;
			bind_real.x() = Native2LE(joints[i].bind_real.x());
			bind_real.y() = Native2LE(joints[i].bind_real.y());
			bind_real.z() = Native2LE(joints[i].bind_real.z());
			bind_real.w() = Native2LE(joints[i].bind_real.w());
			os.write(reinterpret_cast<char*>(&bind_real), sizeof(bind_real));
			Quaternion bind_dual;
			bind_dual.x() = Native2LE(joints[i].bind_dual.x());
			bind_dual.y() = Native2LE(joints[i].bind_dual.y());
			bind_dual.z() = Native2LE(joints[i].bind_dual.z());
			bind_dual.w() = Native2LE(joints[i].bind_dual.w());
			os.write(reinterpret_cast<char*>(&bind_dual), sizeof(bind_dual));
		}
	}

	void WriteKeyFramesChunk(uint32_t num_frames, uint32_t frame_rate, std::vector<KeyFrames>& kfs,
		std::ostream& os)
	{
		num_frames = Native2LE(num_frames);
		os.write(reinterpret_cast<char*>(&num_frames), sizeof(num_frames));
		frame_rate = Native2LE(frame_rate);
		os.write(reinterpret_cast<char*>(&frame_rate), sizeof(frame_rate));

		for (size_t i = 0; i < kfs.size(); ++ i)
		{
			uint32_t num_kf = Native2LE(static_cast<uint32_t>(kfs[i].frame_id.size()));
			os.write(reinterpret_cast<char*>(&num_kf), sizeof(num_kf));

			for (size_t j = 0; j < kfs[i].frame_id.size(); ++ j)
			{
				Quaternion bind_real = kfs[i].bind_real[j];
				Quaternion bind_dual = kfs[i].bind_dual[j];
				float bind_scale = kfs[i].bind_scale[j];

				uint32_t frame_id = Native2LE(kfs[i].frame_id[j]);
				os.write(reinterpret_cast<char*>(&frame_id), sizeof(frame_id));
				bind_real *= bind_scale;
				bind_real.x() = Native2LE(bind_real.x());
				bind_real.y() = Native2LE(bind_real.y());
				bind_real.z() = Native2LE(bind_real.z());
				bind_real.w() = Native2LE(bind_real.w());
				os.write(reinterpret_cast<char*>(&bind_real), sizeof(bind_real));
				bind_dual.x() = Native2LE(bind_dual.x());
				bind_dual.y() = Native2LE(bind_dual.y());
				bind_dual.z() = Native2LE(bind_dual.z());
				bind_dual.w() = Native2LE(bind_dual.w());
				os.write(reinterpret_cast<char*>(&bind_dual), sizeof(bind_dual));
			}
		}
	}

	void WriteBBKeyFramesChunk(std::vector<AABBKeyFrames> const & bb_kfs, std::ostream& os)
	{
		for (size_t i = 0; i < bb_kfs.size(); ++ i)
		{
			uint32_t num_bb_kf = Native2LE(static_cast<uint32_t>(bb_kfs[i].frame_id.size()));
			os.write(reinterpret_cast<char*>(&num_bb_kf), sizeof(num_bb_kf));

			for (uint32_t j = 0; j < bb_kfs[i].frame_id.size(); ++ j)
			{
				uint32_t frame_id = Native2LE(bb_kfs[i].frame_id[j]);
				os.write(reinterpret_cast<char*>(&frame_id), sizeof(frame_id));
				float3 bb_min;
				bb_min.x() = Native2LE(bb_kfs[i].bb[j].Min().x());
				bb_min.y() = Native2LE(bb_kfs[i].bb[j].Min().y());
				bb_min.z() = Native2LE(bb_kfs[i].bb[j].Min().z());
				os.write(reinterpret_cast<char*>(&bb_min), sizeof(bb_min));
				float3 bb_max = bb_kfs[i].bb[j].Max();
				bb_max.x() = Native2LE(bb_kfs[i].bb[j].Max().x());
				bb_max.y() = Native2LE(bb_kfs[i].bb[j].Max().y());
				bb_max.z() = Native2LE(bb_kfs[i].bb[j].Max().z());
				os.write(reinterpret_cast<char*>(&bb_max), sizeof(bb_max));
			}
		}
	}

	void WriteActionsChunk(std::vector<AnimationAction> const & actions, std::ostream& os)
	{
		for (size_t i = 0; i < actions.size(); ++ i)
		{
			WriteShortString(os, actions[i].name);

			uint32_t sf = Native2LE(actions[i].start_frame);
			os.write(reinterpret_cast<char*>(&sf), sizeof(sf));

			uint32_t ef = Native2LE(actions[i].end_frame);
			os.write(reinterpret_cast<char*>(&ef), sizeof(ef));
		}
	}

	void SaveModelToJIT(std::string const & jit_name, std::vector<RenderMaterialPtr> const & mtls,
		std::vector<VertexElement> const & merged_ves, char all_is_index_16_bit,
		std::vector<std::vector<uint8_t>> const & merged_buffs, std::vector<uint8_t> const & merged_indices,
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<uint32_t> const & mesh_lods,
		std::vector<AABBox> const & pos_bbs, std::vector<AABBox> const & tc_bbs,
		std::vector<uint32_t> const & mesh_num_vertices, std::vector<uint32_t> const & mesh_base_vertices,
		std::vector<uint32_t> const & mesh_num_indices, std::vector<uint32_t> const & mesh_base_indices,
		std::vector<Joint> const & joints, std::shared_ptr<AnimationActionsType> const & actions,
		std::shared_ptr<KeyFramesType> const & kfs, uint32_t num_frames, uint32_t frame_rate)
	{
		std::ostringstream ss;

		{
			uint32_t num_mtls = Native2LE(static_cast<uint32_t>(mtls.size()));
			ss.write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));

			uint32_t num_meshes = Native2LE(static_cast<uint32_t>(pos_bbs.size()));
			ss.write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));

			uint32_t num_joints = Native2LE(static_cast<uint32_t>(joints.size()));
			ss.write(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));

			uint32_t num_kfs = Native2LE(kfs ? static_cast<uint32_t>(kfs->size()) : 0);
			ss.write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));

			uint32_t num_actions = Native2LE(actions ? std::max(static_cast<uint32_t>(actions->size()), 1U) : 0);
			ss.write(reinterpret_cast<char*>(&num_actions), sizeof(num_actions));
		}

		if (!mtls.empty())
		{
			WriteMaterialsChunk(mtls, ss);
		}

		if (!mesh_names.empty())
		{
			WriteMeshesChunk(mesh_names, mtl_ids, mesh_lods, pos_bbs, tc_bbs,
				mesh_num_vertices, mesh_base_vertices, mesh_num_indices, mesh_base_indices,
				merged_ves, merged_buffs, merged_indices, all_is_index_16_bit, ss);
		}

		if (!joints.empty())
		{
			WriteBonesChunk(joints, ss);
		}

		if (kfs && !kfs->empty())
		{
			WriteKeyFramesChunk(num_frames, frame_rate, *kfs, ss);

			std::vector<AABBKeyFrames> bb_kfss;

			AABBKeyFrames bb_kfs;
			bb_kfs.frame_id.resize(2);
			bb_kfs.bb.resize(2);

			bb_kfs.frame_id[0] = 0;
			bb_kfs.frame_id[1] = num_frames - 1;

			for (uint32_t mesh_index = 0; mesh_index < pos_bbs.size(); ++ mesh_index)
			{
				bb_kfs.bb[0] = pos_bbs[mesh_index];
				bb_kfs.bb[1] = pos_bbs[mesh_index];

				bb_kfss.push_back(bb_kfs);
			}
			WriteBBKeyFramesChunk(bb_kfss, ss);

			WriteActionsChunk(*actions, ss);
		}

		std::ofstream ofs(jit_name.c_str(), std::ios_base::binary);
		BOOST_ASSERT(ofs);
		uint32_t fourcc = Native2LE(MakeFourCC<'K', 'L', 'M', ' '>::value);
		ofs.write(reinterpret_cast<char*>(&fourcc), sizeof(fourcc));

		uint32_t ver = Native2LE(MODEL_BIN_VERSION);
		ofs.write(reinterpret_cast<char*>(&ver), sizeof(ver));

		uint64_t original_len = Native2LE(static_cast<uint64_t>(ss.str().size()));
		ofs.write(reinterpret_cast<char*>(&original_len), sizeof(original_len));

		std::ofstream::pos_type p = ofs.tellp();
		uint64_t len = 0;
		ofs.write(reinterpret_cast<char*>(&len), sizeof(len));

		LZMACodec lzma;
		len = lzma.Encode(ofs, ss.str().c_str(), ss.str().size());

		ofs.seekp(p, std::ios_base::beg);
		len = Native2LE(len);
		ofs.write(reinterpret_cast<char*>(&len), sizeof(len));
	}

	void SaveModel(std::string const & meshml_name, std::vector<RenderMaterialPtr> const & mtls,
		std::vector<VertexElement> const & merged_ves, char all_is_index_16_bit,
		std::vector<std::vector<uint8_t>> const & merged_buffs, std::vector<uint8_t> const & merged_indices,
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<uint32_t> const & mesh_lods,
		std::vector<AABBox> const & pos_bbs, std::vector<AABBox> const & tc_bbs,
		std::vector<uint32_t> const & mesh_num_vertices, std::vector<uint32_t> const & mesh_base_vertices,
		std::vector<uint32_t> const & mesh_num_indices, std::vector<uint32_t> const & mesh_base_indices,
		std::vector<Joint> const & joints, std::shared_ptr<AnimationActionsType> const & actions,
		std::shared_ptr<KeyFramesType> const & kfs, uint32_t num_frames, uint32_t frame_rate)
	{
		if (meshml_name.find(jit_ext_name) != std::string::npos)
		{
			SaveModelToJIT(meshml_name, mtls, merged_ves, all_is_index_16_bit, merged_buffs, merged_indices,
				mesh_names, mtl_ids, mesh_lods, pos_bbs, tc_bbs, mesh_num_vertices, mesh_base_vertices,
				mesh_num_indices, mesh_base_indices, joints, actions,
				kfs, num_frames, frame_rate);
		}
		else
		{
			SaveModelToMeshML(meshml_name, mtls, merged_ves, all_is_index_16_bit, merged_buffs, merged_indices,
				mesh_names, mtl_ids, mesh_lods, pos_bbs, tc_bbs, mesh_num_vertices, mesh_base_vertices,
				mesh_num_indices, mesh_base_indices, joints, actions,
				kfs, num_frames, frame_rate);
		}
	}

	void SaveModel(RenderModelPtr const & model, std::string const & meshml_name)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		std::vector<RenderMaterialPtr> mtls(model->NumMaterials());
		if (!mtls.empty())
		{
			for (uint32_t i = 0; i < mtls.size(); ++ i)
			{
				mtls[i] = model->GetMaterial(i);
			}
		}

		std::vector<VertexElement> merged_ves;
		std::vector<std::vector<uint8_t>> merged_buffs;
		char all_is_index_16_bit = false;
		std::vector<uint8_t> merged_indices;
		std::vector<std::string> mesh_names(model->NumSubrenderables());
		std::vector<int32_t> mtl_ids(mesh_names.size());
		std::vector<uint32_t> mesh_lods(mesh_names.size());
		std::vector<AABBox> pos_bbs(mesh_names.size());
		std::vector<AABBox> tc_bbs(mesh_names.size());
		std::vector<uint32_t> mesh_num_vertices;
		std::vector<uint32_t> mesh_base_vertices;
		std::vector<uint32_t> mesh_num_indices;
		std::vector<uint32_t> mesh_base_indices;
		if (!mesh_names.empty())
		{
			{
				StaticMesh const & mesh = *checked_pointer_cast<StaticMesh>(model->Subrenderable(0));

				RenderLayout const & rl = mesh.GetRenderLayout();
				merged_ves.resize(rl.NumVertexStreams());
				for (uint32_t j = 0; j < rl.NumVertexStreams(); ++ j)
				{
					merged_ves[j] = rl.VertexStreamFormat(j)[0];
				}

				merged_buffs.resize(merged_ves.size());
				for (uint32_t j = 0; j < rl.NumVertexStreams(); ++ j)
				{
					GraphicsBufferPtr const & vb = rl.GetVertexStream(j);
					uint32_t size = vb->Size();
					GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, size, nullptr);
					vb->CopyToBuffer(*vb_cpu);

					merged_buffs[j].resize(size);

					GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
					std::memcpy(&merged_buffs[j][0], mapper.Pointer<uint8_t>(), size);
				}

				if (EF_R16UI == rl.IndexStreamFormat())
				{
					all_is_index_16_bit = true;
				}
				else
				{
					BOOST_ASSERT(EF_R32UI == rl.IndexStreamFormat());
					all_is_index_16_bit = false;
				}

				{
					GraphicsBufferPtr ib = rl.GetIndexStream();
					uint32_t size = ib->Size();
					GraphicsBufferPtr ib_cpu = rf.MakeIndexBuffer(BU_Static, EAH_CPU_Read, size, nullptr);
					ib->CopyToBuffer(*ib_cpu);

					merged_indices.resize(size);

					GraphicsBuffer::Mapper mapper(*ib_cpu, BA_Read_Only);
					std::memcpy(&merged_indices[0], mapper.Pointer<uint8_t>(), size);
				}
			}

			for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
			{
				StaticMesh const & mesh = *checked_pointer_cast<StaticMesh>(model->Subrenderable(mesh_index));

				Convert(mesh_names[mesh_index], mesh.Name());
				mtl_ids[mesh_index] = mesh.MaterialID();

				mesh_lods[mesh_index] = mesh.NumLods();

				pos_bbs[mesh_index] = mesh.PosBound();
				tc_bbs[mesh_index] = mesh.TexcoordBound();

				for (uint32_t lod = 0; lod < mesh_lods[mesh_index]; ++ lod)
				{
					mesh_num_vertices.push_back(mesh.NumVertices(lod));
					mesh_base_vertices.push_back(mesh.StartVertexLocation(lod));
					mesh_num_indices.push_back(mesh.NumIndices(lod));
					mesh_base_indices.push_back(mesh.StartIndexLocation(lod));
				}
			}
		}

		std::vector<Joint> joints;
		std::shared_ptr<AnimationActionsType> actions;
		std::shared_ptr<KeyFramesType> kfs;
		uint32_t num_frame = 0;
		uint32_t frame_rate = 0;
		if (model->IsSkinned())
		{
			SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(model);

			uint32_t num_joints = skinned->NumJoints();
			joints.resize(num_joints);
			for (uint32_t i = 0; i < num_joints; ++ i)
			{
				Joint joint = skinned->GetJoint(i);

				float flip = MathLib::SignBit(joint.inverse_origin_scale);

				joint.bind_scale = 1 / joint.inverse_origin_scale;
				if (flip > 0)
				{
					std::pair<Quaternion, Quaternion> inv = MathLib::inverse(joint.inverse_origin_real, joint.inverse_origin_dual);
					joint.bind_real = inv.first;
					joint.bind_dual = inv.second;
				}
				else
				{
					float4x4 tmp_mat = MathLib::scaling(-joint.inverse_origin_scale, -joint.inverse_origin_scale, joint.inverse_origin_scale)
						* MathLib::to_matrix(joint.inverse_origin_real)
						* MathLib::translation(MathLib::udq_to_trans(joint.inverse_origin_real, joint.inverse_origin_dual));
					tmp_mat = MathLib::inverse(tmp_mat);
					tmp_mat(2, 0) = -tmp_mat(2, 0);
					tmp_mat(2, 1) = -tmp_mat(2, 1);
					tmp_mat(2, 2) = -tmp_mat(2, 2);

					float3 scale;
					Quaternion rot;
					float3 trans;
					MathLib::decompose(scale, rot, trans, tmp_mat);

					joint.bind_real = rot;
					joint.bind_dual = MathLib::quat_trans_to_udq(rot, trans);
					joint.bind_scale = -scale.x();
				}

				joints[i] = joint;
			}

			actions = skinned->GetActions();

			num_frame = skinned->NumFrames();
			frame_rate = skinned->FrameRate();

			kfs = skinned->GetKeyFrames();
		}

		SaveModel(meshml_name, mtls, merged_ves, all_is_index_16_bit, merged_buffs, merged_indices,
			mesh_names, mtl_ids, mesh_lods, pos_bbs, tc_bbs,
			mesh_num_vertices, mesh_base_vertices, mesh_num_indices, mesh_base_indices,
			joints, actions, kfs, num_frame, frame_rate);
	}


	RenderableLightSourceProxy::RenderableLightSourceProxy(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
	{
		auto effect = SyncLoadRenderEffect("LightSourceProxy.fxml");
		this->Technique(effect, effect->TechniqueByName("LightSourceProxy"));
		effect_attrs_ |= EA_SimpleForward;
	}

	void RenderableLightSourceProxy::Technique(RenderEffectPtr const & effect, RenderTechnique* tech)
	{
		StaticMesh::Technique(effect, tech);

		simple_forward_tech_ = tech;
		if (tech)
		{
			mvp_param_ = effect_->ParameterByName("mvp");
			model_param_ = effect_->ParameterByName("model");
			pos_center_param_ = effect_->ParameterByName("pos_center");
			pos_extent_param_ = effect_->ParameterByName("pos_extent");
			tc_center_param_ = effect_->ParameterByName("tc_center");
			tc_extent_param_ = effect_->ParameterByName("tc_extent");

			light_color_param_ = effect_->ParameterByName("light_color");
			light_is_projective_param_ = effect_->ParameterByName("light_is_projective");
			projective_map_2d_tex_param_ = effect_->ParameterByName("projective_map_2d_tex");
			projective_map_cube_tex_param_ = effect_->ParameterByName("projective_map_cube_tex");

			select_mode_object_id_param_ = effect_->ParameterByName("object_id");
			select_mode_tech_ = effect_->TechniqueByName("SelectModeTech");
		}
	}

	void RenderableLightSourceProxy::Update()
	{
		if (light_color_param_)
		{
			*light_color_param_ = light_->Color();
		}
		if (light_is_projective_param_)
		{
			*light_is_projective_param_ = int2(light_->ProjectiveTexture() ? 1 : 0,
				LightSource::LT_Point == light_->Type());
		}
		if (LightSource::LT_Point == light_->Type())
		{
			if (projective_map_cube_tex_param_)
			{
				*projective_map_cube_tex_param_ = light_->ProjectiveTexture();
			}
		}
		else
		{
			if (projective_map_2d_tex_param_)
			{
				*projective_map_2d_tex_param_ = light_->ProjectiveTexture();
			}
		}
	}

	void RenderableLightSourceProxy::OnRenderBegin()
	{
		StaticMesh::OnRenderBegin();

		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

		float4x4 const & view = camera.ViewMatrix();
		float4x4 const & proj = camera.ProjMatrix();

		float4x4 mv = model_mat_ * view;
		*mvp_param_ = mv * proj;
		*model_param_ = model_mat_;

		AABBox const & pos_bb = this->PosBound();
		*pos_center_param_ = pos_bb.Center();
		*pos_extent_param_ = pos_bb.HalfSize();

		AABBox const & tc_bb = this->TexcoordBound();
		*tc_center_param_ = float2(tc_bb.Center().x(), tc_bb.Center().y());
		*tc_extent_param_ = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
	}

	void RenderableLightSourceProxy::AttachLightSrc(LightSourcePtr const & light)
	{
		light_ = light;
	}


	RenderableCameraProxy::RenderableCameraProxy(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
	{
		auto effect = SyncLoadRenderEffect("CameraProxy.fxml");
		this->Technique(effect, effect->TechniqueByName("CameraProxy"));
		effect_attrs_ |= EA_SimpleForward;
	}

	void RenderableCameraProxy::Technique(RenderEffectPtr const & effect, RenderTechnique* tech)
	{
		StaticMesh::Technique(effect, tech);

		simple_forward_tech_ = tech;
		if (tech)
		{
			mvp_param_ = effect_->ParameterByName("mvp");
			pos_center_param_ = effect_->ParameterByName("pos_center");
			pos_extent_param_ = effect_->ParameterByName("pos_extent");
			tc_center_param_ = effect_->ParameterByName("tc_center");
			tc_extent_param_ = effect_->ParameterByName("tc_extent");

			select_mode_object_id_param_ = effect_->ParameterByName("object_id");
			select_mode_tech_ = effect_->TechniqueByName("SelectModeTech");
		}
	}

	void RenderableCameraProxy::AttachCamera(CameraPtr const & camera)
	{
		camera_ = camera;
	}
}
