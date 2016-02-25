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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <boost/functional/hash.hpp>

#include <MeshMLLib/MeshMLLib.hpp>

#include <KlayGE/Mesh.hpp>

namespace
{
	using namespace KlayGE;

	uint32_t const MODEL_BIN_VERSION = 9;

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
				std::vector<RenderMaterialPtr> mtls;
				std::vector<vertex_element> merged_ves;
				char all_is_index_16_bit;
				std::vector<std::vector<uint8_t>> merged_buff;
				std::vector<uint8_t> merged_indices;
				std::vector<GraphicsBufferPtr> merged_vbs;
				GraphicsBufferPtr merged_ib;
				std::vector<std::string> mesh_names;
				std::vector<int32_t> mtl_ids;
				std::vector<AABBox> pos_bbs;
				std::vector<AABBox> tc_bbs;
				std::vector<uint32_t> mesh_num_vertices;
				std::vector<uint32_t> mesh_base_vertices;
				std::vector<uint32_t> mesh_num_indices;
				std::vector<uint32_t> mesh_start_indices;
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

		uint64_t Type() const
		{
			static uint64_t const type = CT_HASH("RenderModelLoadingDesc");
			return type;
		}

		bool StateLess() const
		{
			return false;
		}

		virtual std::shared_ptr<void> CreateResource() override
		{
			RenderModelPtr model = model_desc_.CreateModelFactoryFunc(L"Model");
			*model_desc_.model = model;
			return model;
		}

		void SubThreadStage()
		{
			LoadModel(model_desc_.res_name, model_desc_.model_data->mtls, model_desc_.model_data->merged_ves,
				model_desc_.model_data->all_is_index_16_bit,
				model_desc_.model_data->merged_buff, model_desc_.model_data->merged_indices,
				model_desc_.model_data->mesh_names, model_desc_.model_data->mtl_ids,
				model_desc_.model_data->pos_bbs, model_desc_.model_data->tc_bbs,
				model_desc_.model_data->mesh_num_vertices, model_desc_.model_data->mesh_base_vertices,
				model_desc_.model_data->mesh_num_indices, model_desc_.model_data->mesh_start_indices, 
				model_desc_.model_data->joints, model_desc_.model_data->actions, model_desc_.model_data->kfs,
				model_desc_.model_data->num_frames, model_desc_.model_data->frame_rate,
				model_desc_.model_data->frame_pos_bbs);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if (caps.multithread_res_creating_support)
			{
				this->MainThreadStage();
			}
		}

		std::shared_ptr<void> MainThreadStage()
		{
			RenderModelPtr const & model = *model_desc_.model;
			if (!model || !model->HWResourceReady())
			{
				this->FillModel();

				for (size_t i = 0; i < model_desc_.model_data->merged_buff.size(); ++i)
				{
					model_desc_.model_data->merged_vbs[i]->CreateHWResource(&model_desc_.model_data->merged_buff[i][0]);
				}
				model_desc_.model_data->merged_ib->CreateHWResource(&model_desc_.model_data->merged_indices[0]);

				model->BuildModelInfo();
				for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
				{
					checked_pointer_cast<StaticMesh>(model->Subrenderable(i))->BuildMeshInfo();
				}

				model_desc_.model_data.reset();
			}
			return std::static_pointer_cast<void>(model);
		}

		bool HasSubThreadStage() const
		{
			return true;
		}

		bool Match(ResLoadingDesc const & rhs) const
		{
			if (this->Type() == rhs.Type())
			{
				RenderModelLoadingDesc const & rmld = static_cast<RenderModelLoadingDesc const &>(rhs);
				return (model_desc_.res_name == rmld.model_desc_.res_name)
					&& (model_desc_.access_hint == rmld.model_desc_.access_hint);
			}
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs)
		{
			BOOST_ASSERT(this->Type() == rhs.Type());

			RenderModelLoadingDesc const & rmld = static_cast<RenderModelLoadingDesc const &>(rhs);
			model_desc_.res_name = rmld.model_desc_.res_name;
			model_desc_.access_hint = rmld.model_desc_.access_hint;
			model_desc_.model_data = rmld.model_desc_.model_data;
			model_desc_.model = rmld.model_desc_.model;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource)
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
				RenderLayout const & rhs_rl = rhs_model->Subrenderable(0)->GetRenderLayout();
			
				std::vector<StaticMeshPtr> meshes(rhs_model->NumSubrenderables());
				for (uint32_t mesh_index = 0; mesh_index < rhs_model->NumSubrenderables(); ++ mesh_index)
				{
					StaticMeshPtr rhs_mesh = checked_pointer_cast<StaticMesh>(rhs_model->Subrenderable(mesh_index));

					meshes[mesh_index] = model_desc_.CreateMeshFactoryFunc(model, rhs_mesh->Name());
					StaticMeshPtr& mesh = meshes[mesh_index];

					mesh->MaterialID(rhs_mesh->MaterialID());
					mesh->PosBound(rhs_mesh->PosBound());
					mesh->TexcoordBound(rhs_mesh->TexcoordBound());

					for (uint32_t ve_index = 0; ve_index < rhs_rl.NumVertexStreams(); ++ ve_index)
					{
						mesh->AddVertexStream(rhs_rl.GetVertexStream(ve_index),
							rhs_rl.VertexStreamFormat(ve_index)[0]);
					}
					mesh->AddIndexStream(rhs_rl.GetIndexStream(), rhs_rl.IndexStreamFormat());

					mesh->NumVertices(rhs_mesh->NumVertices());
					mesh->NumTriangles(rhs_mesh->NumTriangles());
					mesh->StartVertexLocation(rhs_mesh->StartVertexLocation());
					mesh->StartIndexLocation(rhs_mesh->StartIndexLocation());
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

			model->BuildModelInfo();
			for (uint32_t i = 0; i < model->NumSubrenderables(); ++ i)
			{
				checked_pointer_cast<StaticMesh>(model->Subrenderable(i))->BuildMeshInfo();
			}

			return std::static_pointer_cast<void>(model);
		}

		virtual std::shared_ptr<void> Resource() const override
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
			for (size_t i = 0; i < model_desc_.model_data->merged_buff.size(); ++i)
			{
				model_desc_.model_data->merged_vbs[i] = rf.MakeDelayCreationVertexBuffer(BU_Static, model_desc_.access_hint,
					static_cast<uint32_t>(model_desc_.model_data->merged_buff[i].size()));
			}
			model_desc_.model_data->merged_ib = rf.MakeDelayCreationIndexBuffer(BU_Static, model_desc_.access_hint,
				static_cast<uint32_t>(model_desc_.model_data->merged_indices.size()));

			std::vector<StaticMeshPtr> meshes(model_desc_.model_data->mesh_names.size());
			for (uint32_t mesh_index = 0; mesh_index < model_desc_.model_data->mesh_names.size(); ++ mesh_index)
			{
				std::wstring wname;
				Convert(wname, model_desc_.model_data->mesh_names[mesh_index]);

				meshes[mesh_index] = model_desc_.CreateMeshFactoryFunc(model, wname);
				StaticMeshPtr& mesh = meshes[mesh_index];

				mesh->MaterialID(model_desc_.model_data->mtl_ids[mesh_index]);
				mesh->PosBound(model_desc_.model_data->pos_bbs[mesh_index]);
				mesh->TexcoordBound(model_desc_.model_data->tc_bbs[mesh_index]);

				for (uint32_t ve_index = 0; ve_index < model_desc_.model_data->merged_buff.size(); ++ ve_index)
				{
					mesh->AddVertexStream(model_desc_.model_data->merged_vbs[ve_index], model_desc_.model_data->merged_ves[ve_index]);
				}
				mesh->AddIndexStream(model_desc_.model_data->merged_ib, model_desc_.model_data->all_is_index_16_bit ? EF_R16UI : EF_R32UI);

				mesh->NumVertices(model_desc_.model_data->mesh_num_vertices[mesh_index]);
				mesh->NumTriangles(model_desc_.model_data->mesh_num_indices[mesh_index] / 3);
				mesh->StartVertexLocation(model_desc_.model_data->mesh_base_vertices[mesh_index]);
				mesh->StartIndexLocation(model_desc_.model_data->mesh_start_indices[mesh_index]);
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

	private:
		ModelDesc model_desc_;
	};
}

namespace KlayGE
{
	RenderModel::RenderModel(std::wstring const & name)
		: name_(name),
			hw_res_ready_(false)
	{
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
			for (uint32_t i = 0; i < this->NumSubrenderables(); ++i)
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
		rl_ = Context::Instance().RenderFactoryInstance().MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);
	}

	StaticMesh::~StaticMesh()
	{
	}

	void StaticMesh::DoBuildMeshInfo()
	{
		opacity_map_enabled_ = false;

		RenderModelPtr model = model_.lock();

		mtl_ = model->GetMaterial(this->MaterialID());
		for (auto const & texture_slot : mtl_->texture_slots)
		{
			TexturePtr tex;
			if (!ResLoader::Instance().Locate(texture_slot.second).empty())
			{
				tex = ASyncLoadTexture(texture_slot.second, EAH_GPU_Read | EAH_Immutable);
			}

			size_t const slot_type_hash = RT_HASH(texture_slot.first.c_str());

			if ((CT_HASH("Color") == slot_type_hash) || (CT_HASH("Diffuse Color") == slot_type_hash)
				|| (CT_HASH("Diffuse Color Map") == slot_type_hash))
			{
				diffuse_tex_ = tex;
			}
			else if ((CT_HASH("Specular Level") == slot_type_hash) || (CT_HASH("Specular Color") == slot_type_hash))
			{
				specular_tex_ = tex;
			}
			else if ((CT_HASH("Glossiness") == slot_type_hash) || (CT_HASH("Reflection Glossiness Map") == slot_type_hash))
			{
				shininess_tex_ = tex;
			}
			else if ((CT_HASH("Bump") == slot_type_hash) || (CT_HASH("Bump Map") == slot_type_hash))
			{
				normal_tex_ = tex;
			}
			else if ((CT_HASH("Height") == slot_type_hash) || (CT_HASH("Height Map") == slot_type_hash))
			{
				height_tex_ = tex;
			}
			else if (CT_HASH("Self-Illumination") == slot_type_hash)
			{
				emit_tex_ = tex;
			}
			else if (CT_HASH("Opacity") == slot_type_hash)
			{
				ResIdentifierPtr tex_file = ResLoader::Instance().Open(texture_slot.second);
				if (tex_file)
				{
					Texture::TextureType type;
					uint32_t width, height, depth;
					uint32_t num_mipmaps;
					uint32_t array_size;
					ElementFormat format;
					uint32_t row_pitch, slice_pitch;
					GetImageInfo(tex_file, type, width, height, depth, num_mipmaps, array_size, format,
						row_pitch, slice_pitch);

					opacity_map_enabled_ = true;

					if ((EF_BC1 == format) || (EF_BC1_SRGB == format))
					{
						effect_attrs_ |= EA_AlphaTest;
					}
					else
					{
						effect_attrs_ |= EA_TransparencyBack;
						effect_attrs_ |= EA_TransparencyFront;
					}
				}
			}
		}

		if (!(effect_attrs_ & EA_AlphaTest) && (mtl_->opacity < 1))
		{
			effect_attrs_ |= EA_TransparencyBack;
			effect_attrs_ |= EA_TransparencyFront;
		}
		if ((mtl_->emit.x() > 0) || (mtl_->emit.y() > 0) || (mtl_->emit.z() > 0) || emit_tex_
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

	void StaticMesh::AddVertexStream(void const * buf, uint32_t size, vertex_element const & ve, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, access_hint, size, buf);
		this->AddVertexStream(vb, ve);
	}

	void StaticMesh::AddVertexStream(GraphicsBufferPtr const & buffer, vertex_element const & ve)
	{
		rl_->BindVertexStream(buffer, std::make_tuple(ve));
	}

	void StaticMesh::AddIndexStream(void const * buf, uint32_t size, ElementFormat format, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, access_hint, size, buf);
		this->AddIndexStream(ib, format);
	}

	void StaticMesh::AddIndexStream(GraphicsBufferPtr const & index_stream, ElementFormat format)
	{
		rl_->BindIndexStream(index_stream, format);
	}


	std::pair<std::pair<Quaternion, Quaternion>, float> KeyFrames::Frame(float frame) const
	{
		frame = std::fmod(frame, static_cast<float>(frame_id.back() + 1));

		auto iter = std::upper_bound(frame_id.begin(), frame_id.end(), frame);
		int index = static_cast<int>(iter - frame_id.begin());

		int index0 = index - 1;
		int index1 = index % frame_id.size();
		int frame0 = frame_id[index0];
		int frame1 = frame_id[index1];
		float factor = (frame - frame0) / (frame1 - frame0);
		std::pair<std::pair<Quaternion, Quaternion>, float> ret;
		ret.first = MathLib::sclerp(bind_real[index0], bind_dual[index0], bind_real[index1], bind_dual[index1], factor);
		ret.second = MathLib::lerp(bind_scale[index0], bind_scale[index1], factor);
		return ret;
	}

	AABBox AABBKeyFrames::Frame(float frame) const
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

				if ((key_dq.second > 0) && (parent.bind_scale > 0))
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
			if ((joint.inverse_origin_scale > 0) && (joint.bind_scale > 0))
			{
				bind_real = MathLib::mul_real(joint.inverse_origin_real, joint.bind_real);
				bind_dual = MathLib::mul_dual(joint.inverse_origin_real, joint.inverse_origin_dual,
					joint.bind_real, joint.bind_dual);
				bind_scale = joint.inverse_origin_scale * joint.bind_scale;

				if (bind_real.w() < 0)
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

				if (flip * bind_real.w() < 0)
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
			std::string::size_type offset = meshml_name.rfind("/");
			if (offset != std::string::npos)
			{
				folder_name = meshml_name.substr(0, offset + 1);
			}

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
		std::vector<vertex_element>& merged_ves, char& all_is_index_16_bit,
		std::vector<std::vector<uint8_t>>& merged_buff, std::vector<uint8_t>& merged_indices,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids,
		std::vector<AABBox>& pos_bbs, std::vector<AABBox>& tc_bbs,
		std::vector<uint32_t>& mesh_num_vertices, std::vector<uint32_t>& mesh_base_vertices,
		std::vector<uint32_t>& mesh_num_triangles, std::vector<uint32_t>& mesh_base_triangles,
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

			uint8_t rgb[3];
			float specular_level;
			decoded->read(&rgb[0], sizeof(uint8_t));
			decoded->read(&rgb[1], sizeof(uint8_t));
			decoded->read(&rgb[2], sizeof(uint8_t));
			mtl->ambient.x() = rgb[0] / 255.0f;
			mtl->ambient.y() = rgb[1] / 255.0f;
			mtl->ambient.z() = rgb[2] / 255.0f;
			decoded->read(&rgb[0], sizeof(uint8_t));
			decoded->read(&rgb[1], sizeof(uint8_t));
			decoded->read(&rgb[2], sizeof(uint8_t));
			mtl->diffuse.x() = rgb[0] / 255.0f;
			mtl->diffuse.y() = rgb[1] / 255.0f;
			mtl->diffuse.z() = rgb[2] / 255.0f;
			decoded->read(&rgb[0], sizeof(uint8_t));
			decoded->read(&rgb[1], sizeof(uint8_t));
			decoded->read(&rgb[2], sizeof(uint8_t));
			decoded->read(&specular_level, sizeof(float));
			specular_level = LE2Native(specular_level);
			mtl->specular.x() = rgb[0] / 255.0f * specular_level;
			mtl->specular.y() = rgb[1] / 255.0f * specular_level;
			mtl->specular.z() = rgb[2] / 255.0f * specular_level;
			decoded->read(&rgb[0], sizeof(uint8_t));
			decoded->read(&rgb[1], sizeof(uint8_t));
			decoded->read(&rgb[2], sizeof(uint8_t));
			mtl->emit.x() = rgb[0] / 255.0f;
			mtl->emit.y() = rgb[1] / 255.0f;
			mtl->emit.z() = rgb[2] / 255.0f;

			decoded->read(&mtl->opacity, sizeof(float));
			mtl->opacity = LE2Native(mtl->opacity);

			decoded->read(&mtl->shininess, sizeof(float));
			mtl->shininess = LE2Native(mtl->shininess);

			if (Context::Instance().Config().graphics_cfg.gamma)
			{
				mtl->ambient.x() = MathLib::srgb_to_linear(mtl->ambient.x());
				mtl->ambient.y() = MathLib::srgb_to_linear(mtl->ambient.y());
				mtl->ambient.z() = MathLib::srgb_to_linear(mtl->ambient.z());
				mtl->diffuse.x() = MathLib::srgb_to_linear(mtl->diffuse.x());
				mtl->diffuse.y() = MathLib::srgb_to_linear(mtl->diffuse.y());
				mtl->diffuse.z() = MathLib::srgb_to_linear(mtl->diffuse.z());
			}

			uint32_t num_texs;
			decoded->read(&num_texs, sizeof(num_texs));
			num_texs = LE2Native(num_texs);

			for (uint32_t tex_index = 0; tex_index < num_texs; ++ tex_index)
			{
				std::string type = ReadShortString(decoded);
				std::string name = ReadShortString(decoded);
				mtl->texture_slots.emplace_back(type, name);
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
		pos_bbs.resize(num_meshes);
		tc_bbs.resize(num_meshes);
		mesh_num_vertices.resize(num_meshes);
		mesh_base_vertices.resize(num_meshes);
		mesh_num_triangles.resize(num_meshes);
		mesh_base_triangles.resize(num_meshes);
		for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
		{
			mesh_names[mesh_index] = ReadShortString(decoded);

			decoded->read(&mtl_ids[mesh_index], sizeof(mtl_ids[mesh_index]));
			mtl_ids[mesh_index] = LE2Native(mtl_ids[mesh_index]);

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

			decoded->read(&mesh_num_vertices[mesh_index], sizeof(mesh_num_vertices[mesh_index]));
			mesh_num_vertices[mesh_index] = LE2Native(mesh_num_vertices[mesh_index]);
			decoded->read(&mesh_base_vertices[mesh_index], sizeof(mesh_base_vertices[mesh_index]));
			mesh_base_vertices[mesh_index] = LE2Native(mesh_base_vertices[mesh_index]);
			decoded->read(&mesh_num_triangles[mesh_index], sizeof(mesh_num_triangles[mesh_index]));
			mesh_num_triangles[mesh_index] = LE2Native(mesh_num_triangles[mesh_index]);
			decoded->read(&mesh_base_triangles[mesh_index], sizeof(mesh_base_triangles[mesh_index]));
			mesh_base_triangles[mesh_index] = LE2Native(mesh_base_triangles[mesh_index]);
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

			float flip = MathLib::sgn(joint.bind_real.w());

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

					float flip = MathLib::sgn(kf.bind_real[k_index].w());

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

	void SaveModel(std::string const & meshml_name, std::vector<RenderMaterialPtr> const & mtls,
		std::vector<vertex_element> const & merged_ves, char all_is_index_16_bit, 
		std::vector<std::vector<uint8_t>> const & merged_buffs, std::vector<uint8_t> const & merged_indices,
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids,
		std::vector<AABBox> const & pos_bbs, std::vector<AABBox> const & tc_bbs,
		std::vector<uint32_t>& mesh_num_vertices, std::vector<uint32_t>& mesh_base_vertices,
		std::vector<uint32_t>& mesh_num_triangles, std::vector<uint32_t>& mesh_base_triangles,
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

			float3 ambient = mtls[i]->ambient;
			float3 diffuse = mtls[i]->diffuse;
			if (Context::Instance().Config().graphics_cfg.gamma)
			{
				ambient.x() = MathLib::linear_to_srgb(ambient.x());
				ambient.y() = MathLib::linear_to_srgb(ambient.y());
				ambient.z() = MathLib::linear_to_srgb(ambient.z());
				diffuse.x() = MathLib::linear_to_srgb(diffuse.x());
				diffuse.y() = MathLib::linear_to_srgb(diffuse.y());
				diffuse.z() = MathLib::linear_to_srgb(diffuse.z());
			}

			obj.SetMaterial(mtl_id, ambient, diffuse, mtls[i]->specular, mtls[i]->emit,
				mtls[i]->opacity, mtls[i]->shininess);

			for (size_t ts = 0; ts < mtls[i]->texture_slots.size(); ++ ts)
			{
				int slot_id = obj.AllocTextureSlot(mtl_id);
				obj.SetTextureSlot(mtl_id, slot_id, mtls[i]->texture_slots[ts].first, mtls[i]->texture_slots[ts].second);
			}
		}

		for (size_t i = 0; i < mesh_names.size(); ++ i)
		{
			int mesh_id = obj.AllocMesh();
			
			obj.SetMesh(mesh_id, mtl_map[mtl_ids[i]], mesh_names[i]);

			AABBox const & pos_bb = pos_bbs[i];
			AABBox const & tc_bb = tc_bbs[i];
			float3 const pos_center = pos_bb.Center();
			float3 const pos_extent = pos_bb.HalfSize();
			float3 const tc_center = tc_bb.Center();
			float3 const tc_extent = tc_bb.HalfSize();

			for (size_t v = 0; v < mesh_num_vertices[i]; ++ v)
			{
				int vert_id = obj.AllocVertex(mesh_id);

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
					obj.SetVertex(mesh_id, vert_id, pos, normal, 2, texcoords);
				}
				else
				{
					obj.SetVertex(mesh_id, vert_id, pos, tangent_quat, 2, texcoords);
				}

				for (size_t b = 0; b < bindings.size(); ++ b)
				{
					if (bindings[b].second > 0)
					{
						int binding_id = obj.AllocJointBinding(mesh_id, vert_id);
						obj.SetJointBinding(mesh_id, vert_id, binding_id, joint_map[bindings[b].first], bindings[b].second);
					}
				}
			}

			for (size_t t = 0; t < mesh_num_triangles[i]; ++ t)
			{
				int tri_id = obj.AllocTriangle(mesh_id);
				int index[3];
				if (all_is_index_16_bit)
				{
					uint16_t const * src = reinterpret_cast<uint16_t const *>(&merged_indices[(mesh_base_triangles[i] + t * 3) * sizeof(uint16_t)]);
					index[0] = src[0];
					index[1] = src[1];
					index[2] = src[2];
				}
				else
				{
					uint32_t const * src = reinterpret_cast<uint32_t const *>(&merged_indices[(mesh_base_triangles[i] + t * 3)* sizeof(uint32_t)]);
					index[0] = src[0];
					index[1] = src[1];
					index[2] = src[2];
				}
				obj.SetTriangle(mesh_id, tri_id, index[0], index[1], index[2]);
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
		obj.WriteMeshML(ofs);
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

		std::vector<vertex_element> merged_ves;
		std::vector<std::vector<uint8_t>> merged_buffs;
		char all_is_index_16_bit = false;
		std::vector<uint8_t> merged_indices;
		std::vector<std::string> mesh_names(model->NumSubrenderables());
		std::vector<int32_t> mtl_ids(mesh_names.size());
		std::vector<AABBox> pos_bbs(mesh_names.size());
		std::vector<AABBox> tc_bbs(mesh_names.size());
		std::vector<uint32_t> mesh_num_vertices(mesh_names.size());
		std::vector<uint32_t> mesh_base_vertices(mesh_names.size());
		std::vector<uint32_t> mesh_num_triangles(mesh_names.size());
		std::vector<uint32_t> mesh_base_triangles(mesh_names.size());
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

				pos_bbs[mesh_index] = mesh.PosBound();
				tc_bbs[mesh_index] = mesh.TexcoordBound();

				mesh_num_vertices[mesh_index] = mesh.NumVertices();
				mesh_base_vertices[mesh_index] = mesh.StartVertexLocation();
				mesh_num_triangles[mesh_index] = mesh.NumTriangles();
				mesh_base_triangles[mesh_index] =  mesh.StartIndexLocation();
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

				float flip = MathLib::sgn(joint.inverse_origin_scale);

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
			mesh_names, mtl_ids, pos_bbs, tc_bbs,
			mesh_num_vertices, mesh_base_vertices, mesh_num_triangles, mesh_base_triangles,
			joints, actions, kfs, num_frame, frame_rate);
	}


	RenderableLightSourceProxy::RenderableLightSourceProxy(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
	{
		this->Technique(SyncLoadRenderEffect("LightSourceProxy.fxml")->TechniqueByName("LightSourceProxy"));
		effect_attrs_ |= EA_SimpleForward;
	}

	void RenderableLightSourceProxy::Technique(RenderTechniquePtr const & tech)
	{
		technique_ = simple_forward_tech_ = tech;
		if (tech)
		{
			mvp_param_ = technique_->Effect().ParameterByName("mvp");
			model_param_ = technique_->Effect().ParameterByName("model");
			pos_center_param_ = technique_->Effect().ParameterByName("pos_center");
			pos_extent_param_ = technique_->Effect().ParameterByName("pos_extent");
			tc_center_param_ = technique_->Effect().ParameterByName("tc_center");
			tc_extent_param_ = technique_->Effect().ParameterByName("tc_extent");

			light_color_param_ = technique_->Effect().ParameterByName("light_color");
			light_is_projective_param_ = technique_->Effect().ParameterByName("light_is_projective");
			projective_map_2d_tex_param_ = technique_->Effect().ParameterByName("projective_map_2d_tex");
			projective_map_cube_tex_param_ = technique_->Effect().ParameterByName("projective_map_cube_tex");
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
		this->Technique(SyncLoadRenderEffect("CameraProxy.fxml")->TechniqueByName("CameraProxy"));
		effect_attrs_ |= EA_SimpleForward;
	}

	void RenderableCameraProxy::Technique(RenderTechniquePtr const & tech)
	{
		technique_ = simple_forward_tech_ = tech;
		if (tech)
		{
			mvp_param_ = technique_->Effect().ParameterByName("mvp");
			pos_center_param_ = technique_->Effect().ParameterByName("pos_center");
			pos_extent_param_ = technique_->Effect().ParameterByName("pos_extent");
			tc_center_param_ = technique_->Effect().ParameterByName("tc_center");
			tc_extent_param_ = technique_->Effect().ParameterByName("tc_extent");
		}
	}

	void RenderableCameraProxy::AttachCamera(CameraPtr const & camera)
	{
		camera_ = camera;
	}
}
