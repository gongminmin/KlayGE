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
#include <KFL/CXX17/filesystem.hpp>
#include <KFL/CXX2a/span.hpp>
#include <KFL/ErrorHandling.hpp>
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
#include <KlayGE/DevHelper.hpp>
#include <KFL/Hash.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/SceneManager.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>

#include <KlayGE/Mesh.hpp>

namespace
{
	using namespace KlayGE;

	uint32_t const MODEL_BIN_VERSION = 18;

	class RenderModelLoadingDesc : public ResLoadingDesc
	{
	private:
		struct ModelDesc
		{
			std::string res_name;
			uint32_t access_hint;
			uint32_t node_attrib;
			std::function<void(RenderModel&)> OnFinishLoading;
			std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc;
			std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc;

			RenderModelPtr sw_model;

			std::shared_ptr<RenderModelPtr> model;
		};

	public:
		RenderModelLoadingDesc(std::string_view res_name, uint32_t access_hint, uint32_t node_attrib,
			std::function<void(RenderModel&)> OnFinishLoading,
			std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc,
			std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc)
		{
			model_desc_.res_name = std::string(res_name);
			model_desc_.access_hint = access_hint;
			model_desc_.node_attrib = node_attrib;
			model_desc_.OnFinishLoading = OnFinishLoading;
			model_desc_.CreateModelFactoryFunc = CreateModelFactoryFunc;
			model_desc_.CreateMeshFactoryFunc = CreateMeshFactoryFunc;
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
			RenderModelPtr model = model_desc_.CreateModelFactoryFunc(L"Model", model_desc_.node_attrib);
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

			model_desc_.sw_model = LoadSoftwareModel(model_desc_.res_name);

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
			KFL_UNUSED(rhs);
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs) override
		{
			BOOST_ASSERT(this->Type() == rhs.Type());

			RenderModelLoadingDesc const & rmld = static_cast<RenderModelLoadingDesc const &>(rhs);
			model_desc_.res_name = rmld.model_desc_.res_name;
			model_desc_.access_hint = rmld.model_desc_.access_hint;
			model_desc_.sw_model = rmld.model_desc_.sw_model;
			model_desc_.model = rmld.model_desc_.model;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
		{
			auto rhs_model = std::static_pointer_cast<RenderModel>(resource);
			auto model = model_desc_.CreateModelFactoryFunc(rhs_model->RootNode()->Name(), rhs_model->RootNode()->Attrib());
			model->CloneDataFrom(*rhs_model, model_desc_.CreateMeshFactoryFunc);

			this->AddsSubPath();

			model->BuildModelInfo();
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<StaticMesh>(model->Mesh(i))->BuildMeshInfo(*model);
			}
			
			if (model_desc_.OnFinishLoading)
			{
				model_desc_.OnFinishLoading(*model);
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
			auto const & model = *model_desc_.model;
			auto const & sw_model = *model_desc_.sw_model;

			model->CloneDataFrom(sw_model, model_desc_.CreateMeshFactoryFunc);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			auto const & sw_rl = checked_pointer_cast<StaticMesh>(sw_model.Mesh(0))->GetRenderLayout(0);

			std::vector<GraphicsBufferPtr> merged_vbs(sw_rl.NumVertexStreams());
			for (uint32_t i = 0; i < sw_rl.NumVertexStreams(); ++ i)
			{
				merged_vbs[i] = rf.MakeDelayCreationVertexBuffer(BU_Static, model_desc_.access_hint, sw_rl.GetVertexStream(i)->Size());
			}
			auto merged_ib = rf.MakeDelayCreationIndexBuffer(BU_Static, model_desc_.access_hint, sw_rl.GetIndexStream()->Size());

			for (uint32_t mesh_index = 0; mesh_index < model->NumMeshes(); ++ mesh_index)
			{
				for (uint32_t lod = 0; lod < model->Mesh(mesh_index)->NumLods(); ++ lod)
				{
					auto& rl = model->Mesh(mesh_index)->GetRenderLayout(lod);
					for (uint32_t i = 0; i < sw_rl.NumVertexStreams(); ++ i)
					{
						rl.SetVertexStream(i, merged_vbs[i]);
					}
					rl.BindIndexStream(merged_ib, rl.IndexStreamFormat());
				}
			}
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

				auto const & sw_model = *model_desc_.sw_model;

				auto const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
				auto const & rl = checked_pointer_cast<StaticMesh>(model->Mesh(0))->GetRenderLayout();
				auto const & sw_rl = checked_pointer_cast<StaticMesh>(sw_model.Mesh(0))->GetRenderLayout();

				for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
				{
					GraphicsBuffer::Mapper mapper(*sw_rl.GetVertexStream(i), BA_Read_Only);

					uint32_t const num_vertices = sw_rl.GetVertexStream(i)->Size() / sizeof(uint32_t);

					void* ptr = mapper.Pointer<void>();
					auto ve = sw_rl.VertexStreamFormat(i)[0];

					std::vector<uint8_t> buff;
					if (!caps.VertexFormatSupport(ve.format))
					{
						buff.resize(sw_rl.GetVertexStream(i)->Size());
						ptr = buff.data();

						uint32_t const * src = mapper.Pointer<uint32_t>();
						uint32_t* dst = static_cast<uint32_t*>(ptr);

						if (ve.format == EF_A2BGR10)
						{
							ve.format = caps.BestMatchVertexFormat(MakeSpan({EF_ARGB8, EF_ABGR8}));

							if (ve.format == EF_ARGB8)
							{
								for (uint32_t j = 0; j < num_vertices; ++ j)
								{
									float x = ((src[j] >> 0) & 0x3FF) / 1023.0f;
									float y = ((src[j] >> 10) & 0x3FF) / 1023.0f;
									float z = ((src[j] >> 20) & 0x3FF) / 1023.0f;
									float w = ((src[j] >> 30) & 0x3) / 3.0f;

									dst[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(x * 255), 0, 255) << 16)
										| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(y * 255), 0, 255) << 8)
										| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(z * 255), 0, 255) << 0)
										| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(w * 255), 0, 255) << 24);
								}
							}
							else
							{
								for (uint32_t j = 0; j < num_vertices; ++ j)
								{
									float x = ((src[j] >> 0) & 0x3FF) / 1023.0f;
									float y = ((src[j] >> 10) & 0x3FF) / 1023.0f;
									float z = ((src[j] >> 20) & 0x3FF) / 1023.0f;
									float w = ((src[j] >> 30) & 0x3) / 3.0f;

									dst[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(x * 255), 0, 255) << 0)
										| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(y * 255), 0, 255) << 8)
										| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(z * 255), 0, 255) << 16)
										| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(w * 255), 0, 255) << 24);
								}
							}
						}
						else if (ve.format == EF_ARGB8)
						{
							BOOST_ASSERT(caps.VertexFormatSupport(EF_ABGR8));

							ve.format = EF_ABGR8;

							for (uint32_t j = 0; j < num_vertices; ++ j)
							{
								float x = ((src[j] >> 16) & 0xFF) / 255.0f;
								float y = ((src[j] >> 8) & 0xFF) / 255.0f;
								float z = ((src[j] >> 0) & 0xFF) / 255.0f;
								float w = ((src[j] >> 24) & 0xFF) / 255.0f;

								dst[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(x * 255), 0, 255) << 0)
									| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(y * 255), 0, 255) << 8)
									| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(z * 255), 0, 255) << 16)
									| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(w * 255), 0, 255) << 24);
							}
						}
						else
						{
							KFL_UNREACHABLE("Invalid vertex format");
						}
					}

					rl.GetVertexStream(i)->CreateHWResource(ptr);
				}
				{
					GraphicsBuffer::Mapper mapper(*sw_rl.GetIndexStream(), BA_Read_Only);
					rl.GetIndexStream()->CreateHWResource(mapper.Pointer<void>());
				}

				this->AddsSubPath();

				model->BuildModelInfo();
				for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
				{
					checked_pointer_cast<StaticMesh>(model->Mesh(i))->BuildMeshInfo(*model);
				}

				model_desc_.sw_model.reset();

				if (model_desc_.OnFinishLoading)
				{
					model_desc_.OnFinishLoading(*model);
				}
			}
		}

	private:
		ModelDesc model_desc_;
		std::mutex main_thread_stage_mutex_;
	};
}

namespace KlayGE
{
	void AddToSceneHelper(SceneNode& node, RenderModel& model)
	{
		auto& scene_mgr = Context::Instance().SceneManagerInstance();
		std::lock_guard<std::mutex> lock(scene_mgr.MutexForUpdate());
		node.AddChild(model.RootNode());
	}

	void AddToSceneRootHelper(RenderModel& model)
	{
		AddToSceneHelper(Context::Instance().SceneManagerInstance().SceneRootNode(), model);
	}

	RenderModel::RenderModel(SceneNodePtr const & root_node)
		: root_node_(root_node),
			hw_res_ready_(false)
	{
	}

	RenderModel::RenderModel(std::wstring_view name, uint32_t node_attrib)
		: RenderModel(MakeSharedPtr<SceneNode>(name, node_attrib))
	{
	}

	RenderModel::~RenderModel()
	{
	}

	void RenderModel::BuildModelInfo()
	{
		this->DoBuildModelInfo();
		root_node_->UpdatePosBoundSubtree();

		hw_res_ready_ = true;
	}

	uint32_t RenderModel::NumLods() const
	{
		uint32_t max_lod = 0;
		this->ForEachMesh([&max_lod](Renderable& mesh)
			{
				max_lod = std::max(max_lod, mesh.NumLods());
			});
		return max_lod;
	}

	void RenderModel::ActiveLod(int32_t lod)
	{
		this->ForEachMesh([lod](Renderable& mesh)
			{
				mesh.ActiveLod(lod);
			});
	}

	bool RenderModel::HWResourceReady() const
	{
		bool ready = hw_res_ready_;
		if (ready)
		{
			for (auto const & mesh : meshes_)
			{
				ready &= mesh->HWResourceReady();
				if (!ready)
				{
					break;
				}
			}
		}
		return ready;
	}

	void RenderModel::ForEachMesh(std::function<void(Renderable&)> const & callback) const
	{
		for (auto const & mesh : meshes_)
		{
			callback(*mesh);
		}
	}

	RenderModelPtr RenderModel::Clone(std::function<RenderModelPtr(std::wstring_view, uint32_t)> const & CreateModelFactoryFunc,
		std::function<StaticMeshPtr(std::wstring_view)> const & CreateMeshFactoryFunc)
	{
		auto ret_model = CreateModelFactoryFunc(root_node_->Name(), root_node_->Attrib());
		ret_model->CloneDataFrom(*this, CreateMeshFactoryFunc);

		ret_model->BuildModelInfo();
		for (auto const & ret_mesh : ret_model->meshes_)
		{
			checked_pointer_cast<StaticMesh>(ret_mesh)->BuildMeshInfo(*ret_model);
		}

		return ret_model;
	}

	void RenderModel::CloneDataFrom(RenderModel const & source,
		std::function<StaticMeshPtr(std::wstring_view)> const & CreateMeshFactoryFunc)
	{
		this->NumMaterials(source.NumMaterials());
		for (uint32_t mtl_index = 0; mtl_index < source.NumMaterials(); ++ mtl_index)
		{
			this->GetMaterial(mtl_index) = source.GetMaterial(mtl_index)->Clone();
		}

		if (source.NumMeshes() > 0)
		{
			std::vector<StaticMeshPtr> meshes(source.NumMeshes());
			for (uint32_t mesh_index = 0; mesh_index < source.NumMeshes(); ++ mesh_index)
			{
				auto const& src_mesh = checked_cast<StaticMesh&>(*source.Mesh(mesh_index));

				meshes[mesh_index] = CreateMeshFactoryFunc(src_mesh.Name());
				auto& mesh = *meshes[mesh_index];

				mesh.MaterialID(src_mesh.MaterialID());
				mesh.NumLods(src_mesh.NumLods());
				mesh.PosBound(src_mesh.PosBound());
				mesh.TexcoordBound(src_mesh.TexcoordBound());

				for (uint32_t lod = 0; lod < src_mesh.NumLods(); ++ lod)
				{
					auto const & src_rl = src_mesh.GetRenderLayout(lod);

					for (uint32_t ve_index = 0; ve_index < src_rl.NumVertexStreams(); ++ ve_index)
					{
						mesh.AddVertexStream(lod, src_rl.GetVertexStream(ve_index), src_rl.VertexStreamFormat(ve_index)[0]);
					}
					mesh.AddIndexStream(lod, src_rl.GetIndexStream(), src_rl.IndexStreamFormat());

					mesh.NumVertices(lod, src_mesh.NumVertices(lod));
					mesh.NumIndices(lod, src_mesh.NumIndices(lod));
					mesh.StartVertexLocation(lod, src_mesh.StartVertexLocation(lod));
					mesh.StartIndexLocation(lod, src_mesh.StartIndexLocation(lod));
				}
			}

			this->AssignMeshes(meshes.begin(), meshes.end());
		}

		std::vector<SceneNode const *> source_nodes;
		std::vector<SceneNodePtr> new_nodes;
		source.RootNode()->Traverse([this, &source, &source_nodes, &new_nodes](SceneNode& node)
			{
				source_nodes.push_back(&node);

				SceneNodePtr new_node;
				if (new_nodes.empty())
				{
					new_node = root_node_;
				}
				else
				{
					new_node = MakeSharedPtr<SceneNode>(node.Name(), node.Attrib());

					for (size_t j = 0; j < source_nodes.size() - 1; ++ j)
					{
						if (node.Parent() == source_nodes[j])
						{
							new_nodes[j]->AddChild(new_node);
						}
					}
				}
				new_nodes.push_back(new_node);

				for (uint32_t i = 0; i < node.NumComponents(); ++ i)
				{
					for (uint32_t mesh_index = 0; mesh_index < source.NumMeshes(); ++ mesh_index)
					{
						if (&checked_cast<RenderableComponent&>(*node.ComponentByIndex(i)).BoundRenderable() ==
							source.Mesh(mesh_index).get())
						{
							new_node->AddComponent(MakeSharedPtr<RenderableComponent>(this->Mesh(mesh_index)));
							break;
						}
					}
				}
				new_node->TransformToParent(node.TransformToParent());

				return true;
			});

		root_node_->UpdatePosBoundSubtree();
	}


	StaticMesh::StaticMesh(std::wstring_view name)
		: Renderable(name),
			hw_res_ready_(false)
	{
	}
	
	void StaticMesh::BuildMeshInfo(RenderModel const & model)
	{
		this->DoBuildMeshInfo(model);
		this->UpdateBoundBox();

		hw_res_ready_ = true;
	}

	void StaticMesh::NumLods(uint32_t lods)
	{
		Renderable::NumLods(lods);

		for (auto& rl : rls_)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				auto& rf = Context::Instance().RenderFactoryInstance();
				rl = rf.MakeRenderLayout();
			}
			else
			{
				rl = MakeSharedPtr<RenderLayout>();
			}
			rl->TopologyType(RenderLayout::TT_TriangleList);
		}
	}

	void StaticMesh::DoBuildMeshInfo(RenderModel const & model)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		mtl_ = model.GetMaterial(this->MaterialID());

		for (size_t i = 0; i < RenderMaterial::TS_NumTextureSlots; ++ i)
		{
			auto slot = static_cast<RenderMaterial::TextureSlot>(i);
			if (!mtl_->TextureName(slot).empty())
			{
				if (!ResLoader::Instance().Locate(mtl_->TextureName(slot)).empty()
					|| !ResLoader::Instance().Locate(mtl_->TextureName(slot) + ".dds").empty())
				{
					mtl_->Texture(slot, rf.MakeTextureSrv(ASyncLoadTexture(mtl_->TextureName(slot), EAH_GPU_Read | EAH_Immutable)));
				}
			}
		}

		if (mtl_->Transparent())
		{
			effect_attrs_ |= EA_TransparencyBack;
			effect_attrs_ |= EA_TransparencyFront;
		}
		if (mtl_->AlphaTestThreshold() > 0)
		{
			effect_attrs_ |= EA_AlphaTest;
		}
		if (mtl_->Sss())
		{
			effect_attrs_ |= EA_SSS;
		}

		if ((mtl_->Emissive().x() > 0) || (mtl_->Emissive().y() > 0) || (mtl_->Emissive().z() > 0) ||
			mtl_->Texture(RenderMaterial::TS_Emissive) || (effect_attrs_ & EA_TransparencyBack) || (effect_attrs_ & EA_TransparencyFront) ||
			(effect_attrs_ & EA_Reflection))
		{
			effect_attrs_ |= EA_SpecialShading;
		}

		auto drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			this->BindDeferredEffect(drl->GBufferEffect(mtl_.get(), false, model.IsSkinned()));
		}
	}

	void StaticMesh::PosBound(AABBox const & aabb)
	{
		pos_aabb_ = aabb;
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


	std::tuple<Quaternion, Quaternion, float> KeyFrameSet::Frame(float frame) const
	{
		std::tuple<Quaternion, Quaternion, float> ret;
		if (frame_id.size() == 1)
		{
			ret = std::make_tuple(bind_real[0], bind_dual[0], bind_scale[0]);
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
			auto dq = MathLib::sclerp(bind_real[index0], bind_dual[index0], bind_real[index1], bind_dual[index1], factor);
			ret = std::make_tuple(dq.first, dq.second, MathLib::lerp(bind_scale[index0], bind_scale[index1], factor));
		}
		return ret;
	}

	AABBox AABBKeyFrameSet::Frame(float frame) const
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


	SkinnedModel::SkinnedModel(SceneNodePtr const & root_node)
		: RenderModel(root_node),
			last_frame_(-1),
			num_frames_(0), frame_rate_(0)
	{
	}

	SkinnedModel::SkinnedModel(std::wstring_view name, uint32_t node_attrib)
		: SkinnedModel(MakeSharedPtr<SceneNode>(name, node_attrib))
	{
	}

	void SkinnedModel::BuildBones(float frame)
	{
		for (size_t i = 0; i < joints_.size(); ++ i)
		{
			Joint& joint = joints_[i];
			KeyFrameSet const & kf = (*key_frame_sets_)[i];

			std::tuple<Quaternion, Quaternion, float> key_dq = kf.Frame(frame);

			if (joint.parent != -1)
			{
				Joint const & parent(joints_[joint.parent]);

				if (MathLib::dot(std::get<0>(key_dq), parent.bind_real) < 0)
				{
					std::get<0>(key_dq) = -std::get<0>(key_dq);
					std::get<1>(key_dq) = -std::get<1>(key_dq);
				}

				if ((MathLib::SignBit(std::get<2>(key_dq)) > 0) && (MathLib::SignBit(parent.bind_scale) > 0))
				{
					joint.bind_real = MathLib::mul_real(std::get<0>(key_dq), parent.bind_real);
					joint.bind_dual = MathLib::mul_dual(std::get<0>(key_dq), std::get<1>(key_dq) * parent.bind_scale,
						parent.bind_real, parent.bind_dual);
					joint.bind_scale = std::get<2>(key_dq) * parent.bind_scale;
				}
				else
				{
					float const key_scale = std::get<2>(key_dq);
					float4x4 tmp_mat = MathLib::scaling(MathLib::abs(key_scale), MathLib::abs(key_scale), key_scale)
						* MathLib::to_matrix(std::get<0>(key_dq))
						* MathLib::translation(MathLib::udq_to_trans(std::get<0>(key_dq), std::get<1>(key_dq)))
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
				joint.bind_real = std::get<0>(key_dq);
				joint.bind_dual = std::get<1>(key_dq);
				joint.bind_scale = std::get<2>(key_dq);
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
		this->ForEachMesh([&pos_aabb, frame](Renderable& mesh)
			{
				pos_aabb |= checked_cast<SkinnedMesh&>(mesh).FramePosBound(frame);
			});

		return pos_aabb;
	}

	void SkinnedModel::AttachActions(std::shared_ptr<std::vector<AnimationAction>> const & actions)
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

	void SkinnedModel::CloneDataFrom(RenderModel const & source,
		std::function<StaticMeshPtr(std::wstring_view)> const & CreateMeshFactoryFunc)
	{
		RenderModel::CloneDataFrom(source, CreateMeshFactoryFunc);

		if (source.IsSkinned())
		{
			auto const& src_skinned_model = checked_cast<SkinnedModel const&>(source);
			auto& skinned_model = checked_cast<SkinnedModel&>(*this);

			std::vector<Joint> joints(src_skinned_model.NumJoints());
			for (uint32_t i = 0; i < src_skinned_model.NumJoints(); ++ i)
			{
				joints[i] = src_skinned_model.GetJoint(i);
			}
			skinned_model.AssignJoints(joints.begin(), joints.end());
			skinned_model.AttachKeyFrameSets(src_skinned_model.GetKeyFrameSets());

			skinned_model.NumFrames(src_skinned_model.NumFrames());
			skinned_model.FrameRate(src_skinned_model.FrameRate());

			for (size_t mesh_index = 0; mesh_index < src_skinned_model.NumMeshes(); ++ mesh_index)
			{
				auto const& src_skinned_mesh = checked_cast<SkinnedMesh const&>(*src_skinned_model.Mesh(mesh_index));
				auto& skinned_mesh = checked_cast<SkinnedMesh&>(*skinned_model.Mesh(mesh_index));
				skinned_mesh.AttachFramePosBounds(src_skinned_mesh.GetFramePosBounds());
			}

			skinned_model.AttachActions(src_skinned_model.GetActions());
		}
	}


	SkinnedMesh::SkinnedMesh(std::wstring_view name)
		: StaticMesh(name)
	{
	}

	AABBox SkinnedMesh::FramePosBound(uint32_t frame) const
	{
		BOOST_ASSERT(frame_pos_aabbs_);
		return frame_pos_aabbs_->Frame(static_cast<float>(frame));
	}
	
	void SkinnedMesh::AttachFramePosBounds(std::shared_ptr<AABBKeyFrameSet> const & frame_pos_aabbs)
	{
		frame_pos_aabbs_ = frame_pos_aabbs;
	}


	RenderModelPtr SyncLoadModel(std::string_view model_name, uint32_t access_hint, uint32_t node_attrib,
		std::function<void(RenderModel&)> OnFinishLoading,
		std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc,
		std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		return ResLoader::Instance().SyncQueryT<RenderModel>(MakeSharedPtr<RenderModelLoadingDesc>(model_name,
			access_hint, node_attrib, OnFinishLoading, CreateModelFactoryFunc, CreateMeshFactoryFunc));
	}

	RenderModelPtr ASyncLoadModel(std::string_view model_name, uint32_t access_hint, uint32_t node_attrib,
		std::function<void(RenderModel&)> OnFinishLoading,
		std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc,
		std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		// Hacky code. During model creation, shaders are created, too. On devices without multirhead resource creating support, shaeder
		// creation will failed. It can be removed once we have async effect loading.
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
		if (caps.multithread_res_creating_support)
		{
			return ResLoader::Instance().ASyncQueryT<RenderModel>(MakeSharedPtr<RenderModelLoadingDesc>(model_name,
				access_hint, node_attrib, OnFinishLoading, CreateModelFactoryFunc, CreateMeshFactoryFunc));
		}
		else
		{
			return SyncLoadModel(model_name, access_hint, node_attrib, OnFinishLoading, CreateModelFactoryFunc, CreateMeshFactoryFunc);
		}
	}

	RenderModelPtr LoadSoftwareModel(std::string_view model_name)
	{
		char const * JIT_EXT_NAME = ".model_bin";

		std::string runtime_name(model_name);
		if (std::filesystem::path(runtime_name).extension() != JIT_EXT_NAME)
		{
			std::string const metadata_name = runtime_name + ".kmeta";
			runtime_name += JIT_EXT_NAME;

			bool jit = false;
			if (ResLoader::Instance().Locate(runtime_name).empty())
			{
				jit = true;
			}
			else
			{
				ResIdentifierPtr runtime_file = ResLoader::Instance().Open(runtime_name);
				uint32_t fourcc;
				runtime_file->read(&fourcc, sizeof(fourcc));
				fourcc = LE2Native(fourcc);
				uint32_t ver;
				runtime_file->read(&ver, sizeof(ver));
				ver = LE2Native(ver);
				if ((fourcc != MakeFourCC<'K', 'L', 'M', ' '>::value) || (ver != MODEL_BIN_VERSION))
				{
					jit = true;
				}
				else
				{
					uint64_t const runtime_file_timestamp = runtime_file->Timestamp();
					uint64_t const input_file_timestamp = ResLoader::Instance().Timestamp(model_name);
					uint64_t const metadata_timestamp = ResLoader::Instance().Timestamp(metadata_name);
					if (((input_file_timestamp > 0) && (runtime_file_timestamp < input_file_timestamp))
						|| ((metadata_timestamp > 0) && (runtime_file_timestamp < metadata_timestamp)))
					{
						jit = true;
					}
				}
			}

			if (jit)
			{
#if KLAYGE_IS_DEV_PLATFORM
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

				return Context::Instance().DevHelperInstance().ConvertModel(model_name, metadata_name, runtime_name, &caps);
#else
				LogError() << "Could NOT locate " << runtime_name << std::endl;
				return RenderModelPtr();
#endif
			}
		}

		std::vector<RenderMaterialPtr> mtls;
		std::vector<VertexElement> merged_ves;
		char all_is_index_16_bit;
		std::vector<std::vector<uint8_t>> merged_buff;
		std::vector<uint8_t> merged_indices;
		std::vector<std::string> mesh_names;
		std::vector<int32_t> mtl_ids;
		std::vector<uint32_t> mesh_lods;
		std::vector<AABBox> pos_bbs;
		std::vector<AABBox> tc_bbs;
		std::vector<uint32_t> mesh_num_vertices;
		std::vector<uint32_t> mesh_base_vertices;
		std::vector<uint32_t> mesh_num_indices;
		std::vector<uint32_t> mesh_start_indices;
		std::vector<std::pair<SceneNodePtr, std::vector<uint16_t>>> nodes;
		std::vector<Joint> joints;
		std::shared_ptr<std::vector<AnimationAction>> actions;
		std::shared_ptr<std::vector<KeyFrameSet>> kfs;
		uint32_t num_frames = 0;
		uint32_t frame_rate = 0;
		std::vector<std::shared_ptr<AABBKeyFrameSet>> frame_pos_bbs;

		ResIdentifierPtr runtime_file = ResLoader::Instance().Open(runtime_name);

		uint32_t fourcc;
		runtime_file->read(&fourcc, sizeof(fourcc));
		fourcc = LE2Native(fourcc);
		BOOST_ASSERT((fourcc == MakeFourCC<'K', 'L', 'M', ' '>::value));

		uint32_t ver;
		runtime_file->read(&ver, sizeof(ver));
		ver = LE2Native(ver);
		BOOST_ASSERT(MODEL_BIN_VERSION == ver);

		std::shared_ptr<std::stringstream> ss = MakeSharedPtr<std::stringstream>();

		uint64_t original_len, len;
		runtime_file->read(&original_len, sizeof(original_len));
		original_len = LE2Native(original_len);
		runtime_file->read(&len, sizeof(len));
		len = LE2Native(len);

		LZMACodec lzma;
		lzma.Decode(*ss, runtime_file, len, original_len);

		ResIdentifierPtr decoded = MakeSharedPtr<ResIdentifier>(runtime_file->ResName(), runtime_file->Timestamp(), ss);

		uint32_t num_mtls;
		decoded->read(&num_mtls, sizeof(num_mtls));
		num_mtls = LE2Native(num_mtls);
		uint32_t num_meshes;
		decoded->read(&num_meshes, sizeof(num_meshes));
		num_meshes = LE2Native(num_meshes);
		uint32_t num_nodes;
		decoded->read(&num_nodes, sizeof(num_nodes));
		num_nodes = LE2Native(num_nodes);
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

			mtl->Name(ReadShortString(decoded));

			float4 albedo;
			decoded->read(&albedo, sizeof(albedo));
			albedo.x() = LE2Native(albedo.x());
			albedo.y() = LE2Native(albedo.y());
			albedo.z() = LE2Native(albedo.z());
			albedo.w() = LE2Native(albedo.w());
			mtl->Albedo(albedo);

			float metalness;
			decoded->read(&metalness, sizeof(metalness));
			metalness = LE2Native(metalness);
			mtl->Metalness(metalness);

			float glossiness;
			decoded->read(&glossiness, sizeof(glossiness));
			glossiness = LE2Native(glossiness);
			mtl->Glossiness(glossiness);

			float3 emissive;
			decoded->read(&emissive, sizeof(emissive));
			emissive.x() = LE2Native(emissive.x());
			emissive.y() = LE2Native(emissive.y());
			emissive.z() = LE2Native(emissive.z());
			mtl->Emissive(emissive);

			uint8_t transparent;
			decoded->read(&transparent, sizeof(transparent));
			mtl->Transparent(transparent ? true : false);

			uint8_t alpha_test;
			decoded->read(&alpha_test, sizeof(uint8_t));
			mtl->AlphaTestThreshold(alpha_test / 255.0f);

			uint8_t sss;
			decoded->read(&sss, sizeof(sss));
			mtl->Sss(sss ? true : false);

			uint8_t two_sided;
			decoded->read(&two_sided, sizeof(two_sided));
			mtl->TwoSided(two_sided ? true : false);

			for (size_t i = 0; i < RenderMaterial::TS_NumTextureSlots; ++ i)
			{
				mtl->TextureName(static_cast<RenderMaterial::TextureSlot>(i), ReadShortString(decoded));
			}
			if (!mtl->TextureName(RenderMaterial::TS_Normal).empty())
			{
				float normal_scale;
				decoded->read(&normal_scale, sizeof(normal_scale));
				mtl->NormalScale(LE2Native(normal_scale));
			}
			if (!mtl->TextureName(RenderMaterial::TS_Height).empty())
			{
				float height_offset;
				decoded->read(&height_offset, sizeof(height_offset));
				mtl->HeightOffset(LE2Native(height_offset));
				float height_scale;
				decoded->read(&height_scale, sizeof(height_scale));
				mtl->HeightScale(LE2Native(height_scale));
			}
			if (!mtl->TextureName(RenderMaterial::TS_Occlusion).empty())
			{
				float occlusion_strength;
				decoded->read(&occlusion_strength, sizeof(occlusion_strength));
				mtl->OcclusionStrength(LE2Native(occlusion_strength));
			}

			uint8_t detail_mode;
			decoded->read(&detail_mode, sizeof(detail_mode));
			mtl->DetailMode(static_cast<RenderMaterial::SurfaceDetailMode>(detail_mode));
			if (mtl->DetailMode() != RenderMaterial::SDM_Parallax)
			{
				float tess_factor;
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->EdgeTessHint(LE2Native(tess_factor));
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->InsideTessHint(LE2Native(tess_factor));
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->MinTessFactor(LE2Native(tess_factor));
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->MaxTessFactor(LE2Native(tess_factor));
			}
			else
			{
				mtl->EdgeTessHint(5);
				mtl->InsideTessHint(5);
				mtl->MinTessFactor(1);
				mtl->MaxTessFactor(9);
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

		merged_buff.resize(merged_ves.size());
		for (size_t i = 0; i < merged_buff.size(); ++ i)
		{
			merged_buff[i].resize(all_num_vertices * merged_ves[i].element_size());
			decoded->read(&merged_buff[i][0], merged_buff[i].size() * sizeof(merged_buff[i][0]));
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
		mesh_start_indices.clear();
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
				mesh_start_indices.push_back(LE2Native(tmp));
			}
		}

		nodes.resize(num_nodes);
		for (auto& node : nodes)
		{
			auto node_name = ReadShortString(decoded);
			std::wstring node_wname;
			Convert(node_wname, node_name);

			uint32_t attrib;
			decoded->read(&attrib, sizeof(attrib));

			node.first = MakeSharedPtr<SceneNode>(node_wname, attrib);

			int16_t parent_index;
			decoded->read(&parent_index, sizeof(parent_index));
			parent_index = LE2Native(parent_index);

			if (parent_index >= 0)
			{
				nodes[parent_index].first->AddChild(node.first);
			}

			uint16_t num_mesh_indices;
			decoded->read(&num_mesh_indices, sizeof(num_mesh_indices));
			num_mesh_indices = LE2Native(num_mesh_indices);

			if (num_mesh_indices > 0)
			{
				node.second.resize(num_mesh_indices);
				decoded->read(node.second.data(), node.second.size() * sizeof(node.second[0]));
				for (auto& mesh_index : node.second)
				{
					mesh_index = LE2Native(mesh_index);
				}
			}

			float4x4 xform_to_parent;
			decoded->read(&xform_to_parent, sizeof(xform_to_parent));
			for (auto& item : xform_to_parent)
			{
				item = LE2Native(item);
			}
			node.first->TransformToParent(xform_to_parent);
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

			kfs = MakeSharedPtr<std::vector<KeyFrameSet>>(joints.size());
			for (uint32_t kf_index = 0; kf_index < num_kfs; ++ kf_index)
			{
				uint32_t joint_index = kf_index;

				uint32_t num_kf;
				decoded->read(&num_kf, sizeof(num_kf));
				num_kf = LE2Native(num_kf);

				KeyFrameSet kf;
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

				frame_pos_bbs[mesh_index] = MakeSharedPtr<AABBKeyFrameSet>();
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
				actions = MakeSharedPtr<std::vector<AnimationAction>>(num_actions);
				for (uint32_t action_index = 0; action_index < num_actions; ++ action_index)
				{
					AnimationAction action;
					action.name = ReadShortString(decoded);
					decoded->read(&action.start_frame, sizeof(action.start_frame));
					action.start_frame = LE2Native(action.start_frame);
					decoded->read(&action.end_frame, sizeof(action.end_frame));
					action.end_frame = LE2Native(action.end_frame);
					(*actions)[action_index] = action;
				}
			}
		}

		bool const skinned = kfs && !kfs->empty();

		RenderModelPtr model;
		if (skinned)
		{
			model = MakeSharedPtr<SkinnedModel>(nodes[0].first);
		}
		else
		{
			model = MakeSharedPtr<RenderModel>(nodes[0].first);
		}

		model->NumMaterials(mtls.size());
		for (uint32_t mtl_index = 0; mtl_index < mtls.size(); ++ mtl_index)
		{
			model->GetMaterial(mtl_index) = mtls[mtl_index];
		}

		std::vector<GraphicsBufferPtr> merged_vbs(merged_buff.size());
		for (size_t i = 0; i < merged_buff.size(); ++ i)
		{
			auto vb = MakeSharedPtr<SoftwareGraphicsBuffer>(static_cast<uint32_t>(merged_buff[i].size()), false);
			vb->CreateHWResource(merged_buff[i].data());

			merged_vbs[i] = vb;
		}
		auto merged_ib = MakeSharedPtr<SoftwareGraphicsBuffer>(static_cast<uint32_t>(merged_indices.size()), false);
		merged_ib->CreateHWResource(merged_indices.data());

		uint32_t mesh_lod_index = 0;
		std::vector<StaticMeshPtr> meshes(num_meshes);
		for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
		{
			std::wstring wname;
			Convert(wname, mesh_names[mesh_index]);

			if (skinned)
			{
				meshes[mesh_index] = MakeSharedPtr<SkinnedMesh>(wname);
			}
			else
			{
				meshes[mesh_index] = MakeSharedPtr<StaticMesh>(wname);
			}
			StaticMeshPtr& mesh = meshes[mesh_index];

			mesh->MaterialID(mtl_ids[mesh_index]);
			mesh->PosBound(pos_bbs[mesh_index]);
			mesh->TexcoordBound(tc_bbs[mesh_index]);

			uint32_t const lods = mesh_lods[mesh_index];
			mesh->NumLods(lods);
			for (uint32_t lod = 0; lod < lods; ++ lod, ++ mesh_lod_index)
			{
				for (uint32_t ve_index = 0; ve_index < merged_buff.size(); ++ ve_index)
				{
					mesh->AddVertexStream(lod, merged_vbs[ve_index], merged_ves[ve_index]);
				}
				mesh->AddIndexStream(lod, merged_ib, all_is_index_16_bit ? EF_R16UI : EF_R32UI);

				mesh->NumVertices(lod, mesh_num_vertices[mesh_lod_index]);
				mesh->NumIndices(lod, mesh_num_indices[mesh_lod_index]);
				mesh->StartVertexLocation(lod, mesh_base_vertices[mesh_lod_index]);
				mesh->StartIndexLocation(lod, mesh_start_indices[mesh_lod_index]);
			}
		}

		if (kfs && !kfs->empty())
		{
			if (!joints.empty())
			{
				SkinnedModelPtr skinned_model = checked_pointer_cast<SkinnedModel>(model);

				skinned_model->AssignJoints(joints.begin(), joints.end());
				skinned_model->AttachKeyFrameSets(kfs);

				skinned_model->NumFrames(num_frames);
				skinned_model->FrameRate(frame_rate);

				for (size_t mesh_index = 0; mesh_index < meshes.size(); ++ mesh_index)
				{
					SkinnedMeshPtr skinned_mesh = checked_pointer_cast<SkinnedMesh>(meshes[mesh_index]);
					skinned_mesh->AttachFramePosBounds(frame_pos_bbs[mesh_index]);
				}

				skinned_model->AttachActions(actions);
			}
		}

		model->AssignMeshes(meshes.begin(), meshes.end());

		for (auto& node : nodes)
		{
			for (auto mesh_index : node.second)
			{
				node.first->AddComponent(MakeSharedPtr<RenderableComponent>(meshes[mesh_index]));
			}
		}

		model->RootNode()->UpdatePosBoundSubtree();

		return model;
	}

	void WriteMaterialsChunk(std::vector<RenderMaterialPtr> const & mtls, std::ostream& os)
	{
		for (size_t i = 0; i < mtls.size(); ++ i)
		{
			auto& mtl = *mtls[i];

			WriteShortString(os, mtl.Name());

			for (uint32_t j = 0; j < 4; ++ j)
			{
				float const value = Native2LE(mtl.Albedo()[j]);
				os.write(reinterpret_cast<char const *>(&value), sizeof(value));
			}

			float metalness = Native2LE(mtl.Metalness());
			os.write(reinterpret_cast<char*>(&metalness), sizeof(metalness));

			float glossiness = Native2LE(mtl.Glossiness());
			os.write(reinterpret_cast<char*>(&glossiness), sizeof(glossiness));

			for (uint32_t j = 0; j < 3; ++ j)
			{
				float const value = Native2LE(mtl.Emissive()[j]);
				os.write(reinterpret_cast<char const *>(&value), sizeof(value));
			}

			uint8_t transparent = mtl.Transparent();
			os.write(reinterpret_cast<char*>(&transparent), sizeof(transparent));

			uint8_t alpha_test = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(mtl.AlphaTestThreshold() * 255.0f + 0.5f), 0, 255));
			os.write(reinterpret_cast<char*>(&alpha_test), sizeof(alpha_test));

			uint8_t sss = mtl.Sss();
			os.write(reinterpret_cast<char*>(&sss), sizeof(sss));

			uint8_t two_sided = mtl.TwoSided();
			os.write(reinterpret_cast<char*>(&two_sided), sizeof(two_sided));

			for (size_t j = 0; j < RenderMaterial::TS_NumTextureSlots; ++ j)
			{
				WriteShortString(os, mtl.TextureName(static_cast<RenderMaterial::TextureSlot>(j)));
			}
			if (!mtl.TextureName(RenderMaterial::TS_Normal).empty())
			{
				float normal_scale = Native2LE(mtl.NormalScale());
				os.write(reinterpret_cast<char*>(&normal_scale), sizeof(normal_scale));
			}
			if (!mtl.TextureName(RenderMaterial::TS_Height).empty())
			{
				float height_offset = Native2LE(mtl.HeightOffset());
				os.write(reinterpret_cast<char*>(&height_offset), sizeof(height_offset));
				float height_scale = Native2LE(mtl.HeightScale());
				os.write(reinterpret_cast<char*>(&height_scale), sizeof(height_scale));
			}
			if (!mtl.TextureName(RenderMaterial::TS_Occlusion).empty())
			{
				float occlusion_strength = Native2LE(mtl.OcclusionStrength());
				os.write(reinterpret_cast<char*>(&occlusion_strength), sizeof(occlusion_strength));
			}

			uint8_t detail_mode = static_cast<uint8_t>(mtl.DetailMode());
			os.write(reinterpret_cast<char*>(&detail_mode), sizeof(detail_mode));
			if (mtl.DetailMode() != RenderMaterial::SDM_Parallax)
			{
				float tess_factor = Native2LE(mtl.EdgeTessHint());
				os.write(reinterpret_cast<char*>(&tess_factor), sizeof(tess_factor));
				tess_factor = Native2LE(mtl.InsideTessHint());
				os.write(reinterpret_cast<char*>(&tess_factor), sizeof(tess_factor));
				tess_factor = Native2LE(mtl.MinTessFactor());
				os.write(reinterpret_cast<char*>(&tess_factor), sizeof(tess_factor));
				tess_factor = Native2LE(mtl.MaxTessFactor());
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

	void WriteNodesChunk(std::vector<SceneNode const *> const & nodes, std::vector<Renderable const *> const & renderables,
		std::ostream& os)
	{
		for (size_t i = 0; i < nodes.size(); ++ i)
		{
			std::string name;
			Convert(name, nodes[i]->Name());
			WriteShortString(os, name);

			uint32_t attrib = Native2LE(nodes[i]->Attrib());
			os.write(reinterpret_cast<char*>(&attrib), sizeof(attrib));

			int16_t node_parent = -1;
			if (nodes[i]->Parent() != nullptr)
			{
				for (size_t j = 0; j < i; ++ j)
				{
					if (nodes[j] == nodes[i]->Parent())
					{
						node_parent = static_cast<int16_t>(j);
						break;
					}
				}
			}

			node_parent = Native2LE(node_parent);
			os.write(reinterpret_cast<char*>(&node_parent), sizeof(node_parent));

			std::vector<uint16_t> mesh_indices;
			nodes[i]->ForEachComponentOfType<RenderableComponent>([&renderables, &mesh_indices](RenderableComponent& mesh_comp)
				{
					auto& mesh = mesh_comp.BoundRenderable();
					for (size_t i = 0; i < renderables.size(); ++ i)
					{
						if (renderables[i] == &mesh)
						{
							uint16_t index = static_cast<uint16_t>(i);
							index = Native2LE(index);
							mesh_indices.push_back(index);
							break;
						}
					}
				});
			uint16_t num_meshes = static_cast<uint16_t>(mesh_indices.size());
			num_meshes = Native2LE(num_meshes);
			os.write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
			os.write(reinterpret_cast<char*>(mesh_indices.data()), mesh_indices.size() * sizeof(mesh_indices[0]));

			float4x4 xform_to_parent = nodes[i]->TransformToParent();
			for (auto& item : xform_to_parent)
			{
				item = Native2LE(item);
			}
			os.write(reinterpret_cast<char*>(&xform_to_parent), sizeof(xform_to_parent));
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

	void WriteKeyFramesChunk(uint32_t num_frames, uint32_t frame_rate, std::vector<KeyFrameSet>& kfs,
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

	void WriteBBKeyFramesChunk(std::vector<std::shared_ptr<AABBKeyFrameSet>> const & frame_pos_bbs, std::ostream& os)
	{
		for (size_t i = 0; i < frame_pos_bbs.size(); ++ i)
		{
			auto& bb_kf = *frame_pos_bbs[i];

			uint32_t num_bb_kf = Native2LE(static_cast<uint32_t>(bb_kf.frame_id.size()));
			os.write(reinterpret_cast<char*>(&num_bb_kf), sizeof(num_bb_kf));

			for (uint32_t j = 0; j < bb_kf.frame_id.size(); ++ j)
			{
				uint32_t frame_id = Native2LE(bb_kf.frame_id[j]);
				os.write(reinterpret_cast<char*>(&frame_id), sizeof(frame_id));
				float3 bb_min;
				bb_min.x() = Native2LE(bb_kf.bb[j].Min().x());
				bb_min.y() = Native2LE(bb_kf.bb[j].Min().y());
				bb_min.z() = Native2LE(bb_kf.bb[j].Min().z());
				os.write(reinterpret_cast<char*>(&bb_min), sizeof(bb_min));
				float3 bb_max = bb_kf.bb[j].Max();
				bb_max.x() = Native2LE(bb_kf.bb[j].Max().x());
				bb_max.y() = Native2LE(bb_kf.bb[j].Max().y());
				bb_max.z() = Native2LE(bb_kf.bb[j].Max().z());
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

	void SaveModel(std::string const & jit_name, std::vector<RenderMaterialPtr> const & mtls,
		std::vector<VertexElement> const & merged_ves, char all_is_index_16_bit,
		std::vector<std::vector<uint8_t>> const & merged_buffs, std::vector<uint8_t> const & merged_indices,
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<uint32_t> const & mesh_lods,
		std::vector<AABBox> const & pos_bbs, std::vector<AABBox> const & tc_bbs,
		std::vector<uint32_t> const & mesh_num_vertices, std::vector<uint32_t> const & mesh_base_vertices,
		std::vector<uint32_t> const & mesh_num_indices, std::vector<uint32_t> const & mesh_base_indices,
		std::vector<SceneNode const *> const & nodes, std::vector<Renderable const *> const & renderables,
		std::vector<Joint> const & joints, std::shared_ptr<std::vector<AnimationAction>> const & actions,
		std::shared_ptr<std::vector<KeyFrameSet>> const & kfs, uint32_t num_frames, uint32_t frame_rate,
		std::vector<std::shared_ptr<AABBKeyFrameSet>> const & frame_pos_bbs)
	{
		std::ostringstream ss;

		{
			uint32_t num_mtls = Native2LE(static_cast<uint32_t>(mtls.size()));
			ss.write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));

			uint32_t num_meshes = Native2LE(static_cast<uint32_t>(pos_bbs.size()));
			ss.write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));

			uint32_t num_nodes = Native2LE(static_cast<uint32_t>(nodes.size()));
			ss.write(reinterpret_cast<char*>(&num_nodes), sizeof(num_nodes));

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

		if (!nodes.empty())
		{
			WriteNodesChunk(nodes, renderables, ss);
		}

		if (!joints.empty())
		{
			WriteBonesChunk(joints, ss);
		}

		if (kfs && !kfs->empty())
		{
			WriteKeyFramesChunk(num_frames, frame_rate, *kfs, ss);

			WriteBBKeyFramesChunk(frame_pos_bbs, ss);

			WriteActionsChunk(*actions, ss);
		}

		std::ofstream ofs(jit_name.c_str(), std::ios_base::binary);
		BOOST_ASSERT(ofs);
		uint32_t fourcc = Native2LE(MakeFourCC<'K', 'L', 'M', ' '>::value);
		ofs.write(reinterpret_cast<char*>(&fourcc), sizeof(fourcc));

		uint32_t ver = Native2LE(MODEL_BIN_VERSION);
		ofs.write(reinterpret_cast<char*>(&ver), sizeof(ver));

		auto const & ss_str = ss.str();
		uint64_t original_len = Native2LE(static_cast<uint64_t>(ss_str.size()));
		ofs.write(reinterpret_cast<char*>(&original_len), sizeof(original_len));

		std::ofstream::pos_type p = ofs.tellp();
		uint64_t len = 0;
		ofs.write(reinterpret_cast<char*>(&len), sizeof(len));

		LZMACodec lzma;
		len = lzma.Encode(ofs, MakeSpan(reinterpret_cast<uint8_t const *>(ss_str.c_str()), ss_str.size()));

		ofs.seekp(p, std::ios_base::beg);
		len = Native2LE(len);
		ofs.write(reinterpret_cast<char*>(&len), sizeof(len));
	}

	void SaveModel(RenderModel const & model, std::string_view model_name)
	{
		std::filesystem::path output_path(model_name.begin(), model_name.end());
		auto const output_ext = output_path.extension().string();
		bool need_conversion = false;
		if (output_ext != ".model_bin")
		{
			output_path += ".model_bin";
			need_conversion = true;
		}

		std::vector<RenderMaterialPtr> mtls(model.NumMaterials());
		if (!mtls.empty())
		{
			for (uint32_t i = 0; i < mtls.size(); ++ i)
			{
				mtls[i] = model.GetMaterial(i);
			}
		}

		std::vector<VertexElement> merged_ves;
		std::vector<std::vector<uint8_t>> merged_buffs;
		char all_is_index_16_bit = false;
		std::vector<uint8_t> merged_indices;
		std::vector<std::string> mesh_names(model.NumMeshes());
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
				auto const& mesh = checked_cast<StaticMesh&>(*model.Mesh(0));

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
					GraphicsBufferPtr vb_cpu;
					if (vb->AccessHint() & EAH_CPU_Read)
					{
						vb_cpu = vb;
					}
					else
					{
						auto& rf = Context::Instance().RenderFactoryInstance();
						vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, size, nullptr);
						vb->CopyToBuffer(*vb_cpu);
					}

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
					GraphicsBufferPtr const & ib = rl.GetIndexStream();
					uint32_t size = ib->Size();
					GraphicsBufferPtr ib_cpu;
					if (ib->AccessHint() & EAH_CPU_Read)
					{
						ib_cpu = ib;
					}
					else
					{
						auto& rf = Context::Instance().RenderFactoryInstance();
						ib_cpu = rf.MakeIndexBuffer(BU_Static, EAH_CPU_Read, size, nullptr);
						ib->CopyToBuffer(*ib_cpu);
					}

					merged_indices.resize(size);

					GraphicsBuffer::Mapper mapper(*ib_cpu, BA_Read_Only);
					std::memcpy(&merged_indices[0], mapper.Pointer<uint8_t>(), size);
				}
			}

			for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
			{
				auto const& mesh = checked_cast<StaticMesh&>(*model.Mesh(mesh_index));

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

			mesh_base_vertices.push_back(mesh_base_vertices.back() + mesh_num_vertices.back());
			mesh_base_indices.push_back(mesh_base_indices.back() + mesh_num_indices.back());
		}

		std::vector<SceneNode const *> nodes;
		model.RootNode()->Traverse([&nodes](SceneNode& node)
			{
				nodes.push_back(&node);
				return true;
			});

		std::vector<Renderable const *> renderables;
		for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
		{
			renderables.push_back(model.Mesh(mesh_index).get());
		}

		std::vector<Joint> joints;
		std::shared_ptr<std::vector<AnimationAction>> actions;
		std::shared_ptr<std::vector<KeyFrameSet>> kfs;
		uint32_t num_frame = 0;
		uint32_t frame_rate = 0;
		std::vector<std::shared_ptr<AABBKeyFrameSet>> frame_pos_bbs;
		if (model.IsSkinned())
		{
			auto const& skinned_model = checked_cast<SkinnedModel const&>(model);

			uint32_t num_joints = skinned_model.NumJoints();
			joints.resize(num_joints);
			for (uint32_t i = 0; i < num_joints; ++ i)
			{
				Joint joint = skinned_model.GetJoint(i);

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

			actions = skinned_model.GetActions();

			num_frame = skinned_model.NumFrames();
			frame_rate = skinned_model.FrameRate();

			kfs = skinned_model.GetKeyFrameSets();

			frame_pos_bbs.resize(mesh_names.size());
			for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
			{
				auto& skinned_mesh = checked_cast<SkinnedMesh&>(*skinned_model.Mesh(mesh_index));
				frame_pos_bbs[mesh_index] = skinned_mesh.GetFramePosBounds();
			}
		}

		SaveModel(output_path.string(), mtls, merged_ves, all_is_index_16_bit, merged_buffs, merged_indices,
			mesh_names, mtl_ids, mesh_lods, pos_bbs, tc_bbs,
			mesh_num_vertices, mesh_base_vertices, mesh_num_indices, mesh_base_indices,
			nodes, renderables,
			joints, actions, kfs, num_frame, frame_rate, frame_pos_bbs);

#if KLAYGE_IS_DEV_PLATFORM
		if (need_conversion)
		{
			RenderDeviceCaps const * caps = nullptr;
			if (Context::Instance().RenderFactoryValid())
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				caps = &rf.RenderEngineInstance().DeviceCaps();
			}

			Context::Instance().DevHelperInstance().ConvertModel(output_path.string(), "", model_name, caps);
		}
#endif
	}


	RenderableLightSourceProxy::RenderableLightSourceProxy(std::wstring_view name)
		: StaticMesh(name)
	{
		auto effect = SyncLoadRenderEffect("LightSourceProxy.fxml");
		this->Technique(effect, effect->TechniqueByName("LightSourceProxy"));
		effect_attrs_ |= EA_SimpleForward;

		mtl_ = MakeSharedPtr<RenderMaterial>();
	}

	void RenderableLightSourceProxy::Technique(RenderEffectPtr const & effect, RenderTechnique* tech)
	{
		StaticMesh::Technique(effect, tech);

		simple_forward_tech_ = tech;
		if (tech)
		{
			light_is_projective_param_ = effect_->ParameterByName("light_is_projective");
			projective_map_2d_tex_param_ = effect_->ParameterByName("projective_map_2d_tex");
			projective_map_cube_tex_param_ = effect_->ParameterByName("projective_map_cube_tex");

			select_mode_object_id_param_ = effect_->ParameterByName("object_id");
			select_mode_tech_ = effect_->TechniqueByName("SelectModeTech");
		}
	}

	void RenderableLightSourceProxy::AttachLightSrc(LightSourcePtr const & light)
	{
		light_ = light;
	}

	void RenderableLightSourceProxy::OnRenderBegin()
	{
		mtl_->Albedo(light_->Color());
		if (light_is_projective_param_)
		{
			*light_is_projective_param_ = int2(light_->ProjectiveTexture() ? 1 : 0, LightSource::LT_Point == light_->Type());
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

		StaticMesh::OnRenderBegin();
	}


	RenderableCameraProxy::RenderableCameraProxy(std::wstring_view name)
		: StaticMesh(name)
	{
		auto effect = SyncLoadRenderEffect("CameraProxy.fxml");
		this->Technique(effect, effect->TechniqueByName("CameraProxy"));
		effect_attrs_ |= EA_SimpleForward;

		mtl_ = MakeSharedPtr<RenderMaterial>();
		mtl_->Albedo(float4(1, 1, 1, 1));
	}

	void RenderableCameraProxy::Technique(RenderEffectPtr const & effect, RenderTechnique* tech)
	{
		StaticMesh::Technique(effect, tech);

		simple_forward_tech_ = tech;
		if (tech)
		{
			select_mode_object_id_param_ = effect_->ParameterByName("object_id");
			select_mode_tech_ = effect_->TechniqueByName("SelectModeTech");
		}
	}

	void RenderableCameraProxy::AttachCamera(CameraPtr const & camera)
	{
		camera_ = camera;
	}


	RenderModelPtr LoadLightSourceProxyModel(LightSourcePtr const& light)
	{
		char const* mesh_name;
		switch (light->Type())
		{
		case LightSource::LT_Ambient:
			mesh_name = "AmbientLightProxy.glb";
			break;

		case LightSource::LT_Point:
		case LightSource::LT_SphereArea:
			mesh_name = "PointLightProxy.glb";
			break;

		case LightSource::LT_Directional:
			mesh_name = "DirectionalLightProxy.glb";
			break;

		case LightSource::LT_Spot:
			mesh_name = "SpotLightProxy.glb";
			break;

		case LightSource::LT_TubeArea:
			mesh_name = "TubeLightProxy.glb";
			break;

		default:
			KFL_UNREACHABLE("Invalid light type");
		}

		auto light_model = SyncLoadModel(mesh_name, EAH_GPU_Read | EAH_Immutable,
			SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow, nullptr, CreateModelFactory<RenderModel>,
			CreateMeshFactory<RenderableLightSourceProxy>);

		for (uint32_t i = 0; i < light_model->NumMeshes(); ++i)
		{
			checked_pointer_cast<RenderableLightSourceProxy>(light_model->Mesh(i))->AttachLightSrc(light);
		}

		if (light->Type() == LightSource::LT_Spot)
		{
			float const radius = light->CosOuterInner().w();
			light_model->RootNode()->TransformToParent(
				MathLib::scaling(radius, radius, 1.0f) * light_model->RootNode()->TransformToParent());
		}

		return light_model;
	}

	RenderModelPtr LoadCameraProxyModel(CameraPtr const& camera)
	{
		auto camera_model = SyncLoadModel("CameraProxy.glb", EAH_GPU_Read | EAH_Immutable,
			SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow, nullptr, CreateModelFactory<RenderModel>,
			CreateMeshFactory<RenderableCameraProxy>);

		for (uint32_t i = 0; i < camera_model->NumMeshes(); ++i)
		{
			checked_pointer_cast<RenderableCameraProxy>(camera_model->Mesh(i))->AttachCamera(camera);
		}

		camera_model->RootNode()->OnMainThreadUpdate().Connect([&camera](SceneNode& node, float app_time, float elapsed_time) {
			KFL_UNUSED(app_time);
			KFL_UNUSED(elapsed_time);

			node.TransformToParent(camera->InverseViewMatrix());
		});

		return camera_model;
	}
}
