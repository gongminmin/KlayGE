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
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/XMLDom.hpp>
#include <KlayGE/LZMACodec.hpp>
#include <KlayGE/Light.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <boost/tuple/tuple.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/Mesh.hpp>

namespace
{
	using namespace KlayGE;

	uint32_t const MODEL_BIN_VERSION = 8;

	class RenderModelLoadingDesc : public ResLoadingDesc
	{
	private:
		struct ModelDesc
		{
			std::string res_name;

			uint32_t access_hint;
			boost::function<RenderModelPtr(std::wstring const &)> CreateModelFactoryFunc;
			boost::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc;

			std::vector<RenderMaterialPtr> mtls;
			std::vector<vertex_element> merged_ves;
			char all_is_index_16_bit;
			std::vector<std::vector<uint8_t> > merged_buff;
			std::vector<uint8_t> merged_indices;
			std::vector<std::string> mesh_names;
			std::vector<int32_t> mtl_ids;
			std::vector<AABBox> pos_bbs;
			std::vector<AABBox> tc_bbs;
			std::vector<uint32_t> mesh_num_vertices;
			std::vector<uint32_t> mesh_base_vertices;
			std::vector<uint32_t> mesh_num_indices;
			std::vector<uint32_t> mesh_start_indices;
			std::vector<Joint> joints;
			boost::shared_ptr<AnimationActionsType> actions;
			boost::shared_ptr<KeyFramesType> kfs;
			uint32_t num_frames;
			uint32_t frame_rate;
			std::vector<boost::shared_ptr<AABBKeyFrames> > frame_pos_bbs;

			RenderModelPtr model;
		};

	public:
		RenderModelLoadingDesc(std::string const & res_name, uint32_t access_hint, 
			boost::function<RenderModelPtr(std::wstring const &)> CreateModelFactoryFunc,
			boost::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
		{
			model_desc_.res_name = res_name;
			model_desc_.access_hint = access_hint;
			model_desc_.CreateModelFactoryFunc = CreateModelFactoryFunc;
			model_desc_.CreateMeshFactoryFunc = CreateMeshFactoryFunc;
		}

		void SubThreadStage()
		{
			this->LoadKModel();
		}

		void* MainThreadStage()
		{
			if (!model_desc_.model)
			{
				this->CreateModel();
			}

			model_desc_.model->BuildModelInfo();
			for (uint32_t i = 0; i < model_desc_.model->NumMeshes(); ++ i)
			{
				model_desc_.model->Mesh(i)->BuildMeshInfo();
			}

			return &model_desc_.model;
		}

		bool HasSubThreadStage() const
		{
			return true;
		}

	private:
		void LoadKModel()
		{
			LoadModel(model_desc_.res_name, model_desc_.mtls, model_desc_.merged_ves, model_desc_.all_is_index_16_bit,
				model_desc_.merged_buff, model_desc_.merged_indices,
				model_desc_.mesh_names, model_desc_.mtl_ids, model_desc_.pos_bbs, model_desc_.tc_bbs,
				model_desc_.mesh_num_vertices, model_desc_.mesh_base_vertices, model_desc_.mesh_num_indices, model_desc_.mesh_start_indices, 
				model_desc_.joints, model_desc_.actions, model_desc_.kfs,
				model_desc_.num_frames, model_desc_.frame_rate, model_desc_.frame_pos_bbs);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			if (rf.RenderEngineInstance().DeviceCaps().multithread_res_creating_support)
			{
				this->CreateModel();
			}
		}

		void CreateModel()
		{
			std::wstring model_name;
			if (!model_desc_.joints.empty())
			{
				model_name = L"SkinnedMesh";
			}
			else
			{
				model_name = L"Mesh";
			}
			RenderModelPtr model = model_desc_.CreateModelFactoryFunc(model_name);

			model->NumMaterials(model_desc_.mtls.size());
			for (uint32_t mtl_index = 0; mtl_index < model_desc_.mtls.size(); ++ mtl_index)
			{
				model->GetMaterial(mtl_index) = model_desc_.mtls[mtl_index];
			}

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			ElementInitData init_data;
			std::vector<GraphicsBufferPtr> merged_vbs(model_desc_.merged_buff.size());
			for (size_t i = 0; i < model_desc_.merged_buff.size(); ++ i)
			{
				init_data.data = &model_desc_.merged_buff[i][0];
				init_data.row_pitch = static_cast<uint32_t>(model_desc_.merged_buff[i].size());
				init_data.slice_pitch = 0;
				merged_vbs[i] = rf.MakeVertexBuffer(BU_Static, model_desc_.access_hint, &init_data);
			}

			GraphicsBufferPtr merged_ib;
			{
				init_data.data = &model_desc_.merged_indices[0];
				init_data.row_pitch = static_cast<uint32_t>(model_desc_.merged_indices.size());
				init_data.slice_pitch = 0;
				merged_ib = rf.MakeIndexBuffer(BU_Static, model_desc_.access_hint, &init_data);
			}

			std::vector<StaticMeshPtr> meshes(model_desc_.mesh_names.size());
			for (uint32_t mesh_index = 0; mesh_index < model_desc_.mesh_names.size(); ++ mesh_index)
			{
				std::wstring wname;
				Convert(wname, model_desc_.mesh_names[mesh_index]);

				meshes[mesh_index] = model_desc_.CreateMeshFactoryFunc(model, wname);
				StaticMeshPtr& mesh = meshes[mesh_index];

				mesh->MaterialID(model_desc_.mtl_ids[mesh_index]);
				mesh->PosBound(model_desc_.pos_bbs[mesh_index]);
				mesh->TexcoordBound(model_desc_.tc_bbs[mesh_index]);

				for (uint32_t ve_index = 0; ve_index < model_desc_.merged_buff.size(); ++ ve_index)
				{
					mesh->AddVertexStream(merged_vbs[ve_index], model_desc_.merged_ves[ve_index]);
				}
				mesh->AddIndexStream(merged_ib, model_desc_.all_is_index_16_bit ? EF_R16UI : EF_R32UI);

				mesh->NumVertices(model_desc_.mesh_num_vertices[mesh_index]);
				mesh->NumTriangles(model_desc_.mesh_num_indices[mesh_index] / 3);
				mesh->BaseVertexLocation(model_desc_.mesh_base_vertices[mesh_index]);
				mesh->StartVertexLocation(model_desc_.mesh_base_vertices[mesh_index]);
				mesh->StartIndexLocation(model_desc_.mesh_start_indices[mesh_index]);
			}

			if (model_desc_.kfs && !model_desc_.kfs->empty())
			{
				if (model->IsSkinned())
				{
					SkinnedModelPtr skinned_model = checked_pointer_cast<SkinnedModel>(model);

					skinned_model->AssignJoints(model_desc_.joints.begin(), model_desc_.joints.end());
					skinned_model->AttachKeyFrames(model_desc_.kfs);

					skinned_model->NumFrames(model_desc_.num_frames);
					skinned_model->FrameRate(model_desc_.frame_rate);

					for (size_t mesh_index = 0; mesh_index < meshes.size(); ++ mesh_index)
					{
						SkinnedMeshPtr skinned_mesh = checked_pointer_cast<SkinnedMesh>(meshes[mesh_index]);
						skinned_mesh->AttachFramePosBounds(model_desc_.frame_pos_bbs[mesh_index]);
					}
				}
			}

			model->AssignMeshes(meshes.begin(), meshes.end());

			model_desc_.model = model;
		}

	private:
		ModelDesc model_desc_;
	};
}

namespace KlayGE
{
	RenderModel::RenderModel(std::wstring const & name)
		: name_(name)
	{
	}

	void RenderModel::AddToRenderQueue()
	{
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::reference mesh, meshes_)
		{
			mesh->AddToRenderQueue();
		}
	}

	void RenderModel::OnRenderBegin()
	{
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::reference mesh, meshes_)
		{
			mesh->OnRenderBegin();
		}
	}

	void RenderModel::OnRenderEnd()
	{
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::reference mesh, meshes_)
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
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			pos_aabb_ |= mesh->PosBound();
			tc_aabb_ |= mesh->TexcoordBound();
		}
	}

	TexturePtr const & RenderModel::RetriveTexture(std::string const & name)
	{
		BOOST_AUTO(iter, tex_pool_.find(name));
		if (tex_pool_.end() == iter)
		{
			TexturePtr tex = SyncLoadTexture(name, EAH_GPU_Read | EAH_Immutable);
			iter = tex_pool_.insert(std::make_pair(name, tex)).first;
		}

		return iter->second;
	}

	bool RenderModel::SpecialShading() const
	{
		bool ss = false;
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			ss |= mesh->SpecialShading();
		}
		return ss;
	}
	
	bool RenderModel::TransparencyBackFace() const
	{
		bool ab = false;
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			ab |= mesh->TransparencyBackFace();
		}
		return ab;
	}

	bool RenderModel::TransparencyFrontFace() const
	{
		bool ab = false;
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			ab |= mesh->TransparencyFrontFace();
		}
		return ab;
	}

	bool RenderModel::Reflection() const
	{
		bool ab = false;
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			ab |= mesh->Reflection();
		}
		return ab;
	}

	bool RenderModel::SimpleForward() const
	{
		bool ab = false;
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			ab |= mesh->SimpleForward();
		}
		return ab;
	}


	StaticMesh::StaticMesh(RenderModelPtr const & model, std::wstring const & name)
		: name_(name), model_(model)
	{
		rl_ = Context::Instance().RenderFactoryInstance().MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);
	}

	StaticMesh::~StaticMesh()
	{
	}

	void StaticMesh::BuildMeshInfo()
	{
		opacity_map_enabled_ = false;
		effect_attrs_ = 0;

		RenderModelPtr model = model_.lock();

		mtl_ = model->GetMaterial(this->MaterialID());
		TextureSlotsType const & texture_slots = mtl_->texture_slots;
		for (TextureSlotsType::const_iterator iter = texture_slots.begin();
			iter != texture_slots.end(); ++ iter)
		{
			TexturePtr tex;
			if (!ResLoader::Instance().Locate(iter->second).empty())
			{
				tex = model->RetriveTexture(iter->second);
			}

			if (("Diffuse Color" == iter->first) || ("Diffuse Color Map" == iter->first))
			{
				diffuse_tex_ = tex;
			}
			else if (("Specular Level" == iter->first) || ("Reflection Glossiness Map" == iter->first))
			{
				specular_tex_ = tex;
			}
			else if (("Bump" == iter->first) || ("Bump Map" == iter->first))
			{
				normal_tex_ = tex;
			}
			else if (("Height" == iter->first) || ("Height Map" == iter->first))
			{
				height_tex_ = tex;
			}
			else if ("Self-Illumination" == iter->first)
			{
				emit_tex_ = tex;
			}
			else if ("Opacity" == iter->first)
			{
				if (tex)
				{
					opacity_map_enabled_ = true;

					if ((EF_BC1 == tex->Format()) || (EF_BC1_SRGB == tex->Format()))
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
		ElementInitData init_data;
		init_data.data = buf;
		init_data.row_pitch = size;
		init_data.slice_pitch = 0;
		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, access_hint, &init_data);
		this->AddVertexStream(vb, ve);
	}

	void StaticMesh::AddVertexStream(GraphicsBufferPtr const & buffer, vertex_element const & ve)
	{
		rl_->BindVertexStream(buffer, boost::make_tuple(ve));
	}

	void StaticMesh::AddIndexStream(void const * buf, uint32_t size, ElementFormat format, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		ElementInitData init_data;
		init_data.data = buf;
		init_data.row_pitch = size;
		init_data.slice_pitch = 0;
		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, access_hint, &init_data);
		this->AddIndexStream(ib, format);
	}

	void StaticMesh::AddIndexStream(GraphicsBufferPtr const & index_stream, ElementFormat format)
	{
		rl_->BindIndexStream(index_stream, format);
	}


	std::pair<std::pair<Quaternion, Quaternion>, float> KeyFrames::Frame(float frame) const
	{
		frame = std::fmod(frame, static_cast<float>(frame_id.back() + 1));

		std::vector<uint32_t>::const_iterator iter = std::upper_bound(frame_id.begin(), frame_id.end(), frame);
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

		std::vector<uint32_t>::const_iterator iter = std::upper_bound(frame_id.begin(), frame_id.end(), frame);
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
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			pos_aabb |= checked_pointer_cast<SkinnedMesh>(mesh)->FramePosBound(frame);
		}

		return pos_aabb;
	}

	void SkinnedModel::AttachActions(boost::shared_ptr<AnimationActionsType> const & actions)
	{
		actions_ = actions;
	}
	
	uint32_t SkinnedModel::NumActions() const
	{
		return actions_ ? actions_->size() : 1;
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
	
	void SkinnedMesh::AttachFramePosBounds(boost::shared_ptr<AABBKeyFrames> const & frame_pos_aabbs)
	{
		frame_pos_aabbs_ = frame_pos_aabbs;
	}


	std::string const jit_ext_name = ".model_bin";

	void ReadShortString(ResIdentifierPtr& file, std::string& str)
	{
		uint8_t len;
		file->read(reinterpret_cast<char*>(&len), sizeof(len));
		str.resize(len);
		if (len > 0)
		{
			file->read(reinterpret_cast<char*>(&str[0]), len * sizeof(str[0]));
		}
	}

	void WriteShortString(std::ostream& os, std::string const & str)
	{
		BOOST_ASSERT(str.size() < 256);

		uint8_t len = static_cast<uint8_t>(str.length());
		os.write(reinterpret_cast<char const *>(&len), sizeof(len));
		if (len > 0)
		{
			os.write(reinterpret_cast<char const *>(&str[0]), str.size() * sizeof(str[0]));
		}
	}

	void ModelJIT(std::string const & meshml_name)
	{
		std::string::size_type const pkt_offset(meshml_name.find("//"));
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
				path_name = pkt_name.substr(0, offset + 1);
			}

			std::string const file_name = meshml_name.substr(pkt_offset + 2);
			path_name += file_name;
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
			LittleEndianToNative<sizeof(fourcc)>(&fourcc);
			uint32_t ver;
			lzma_file->read(&ver, sizeof(ver));
			LittleEndianToNative<sizeof(ver)>(&ver);
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

		if (jit)
		{
			boost::shared_ptr<std::stringstream> ss = MakeSharedPtr<std::stringstream>();

			ResIdentifierPtr file = ResLoader::Instance().Open(meshml_name);
			XMLDocument doc;
			XMLNodePtr root = doc.Parse(file);

			BOOST_ASSERT(root->Attrib("version") && (root->Attrib("version")->ValueInt() >= 6));

			XMLNodePtr materials_chunk = root->FirstNode("materials_chunk");
			if (materials_chunk)
			{
				uint32_t num_mtls = 0;
				for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"))
				{
					++ num_mtls;
				}
				NativeToLittleEndian<sizeof(num_mtls)>(&num_mtls);
				ss->write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));
			}
			else
			{
				uint32_t num_mtls = 0;
				NativeToLittleEndian<sizeof(num_mtls)>(&num_mtls);
				ss->write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));
			}

			XMLNodePtr meshes_chunk = root->FirstNode("meshes_chunk");
			if (meshes_chunk)
			{
				uint32_t num_meshes = 0;
				for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"))
				{
					++ num_meshes;
				}
				NativeToLittleEndian<sizeof(num_meshes)>(&num_meshes);
				ss->write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
			}
			else
			{
				uint32_t num_meshes = 0;
				NativeToLittleEndian<sizeof(num_meshes)>(&num_meshes);
				ss->write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
			}

			XMLNodePtr bones_chunk = root->FirstNode("bones_chunk");
			if (bones_chunk)
			{
				uint32_t num_joints = 0;
				for (XMLNodePtr bone_node = bones_chunk->FirstNode("bone"); bone_node; bone_node = bone_node->NextSibling("bone"))
				{
					++ num_joints;
				}
				NativeToLittleEndian<sizeof(num_joints)>(&num_joints);
				ss->write(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
			}
			else
			{
				uint32_t num_joints = 0;
				NativeToLittleEndian<sizeof(num_joints)>(&num_joints);
				ss->write(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
			}

			XMLNodePtr key_frames_chunk = root->FirstNode("key_frames_chunk");
			if (key_frames_chunk)
			{
				uint32_t num_kfs = 0;
				for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
				{
					++ num_kfs;
				}
				NativeToLittleEndian<sizeof(num_kfs)>(&num_kfs);
				ss->write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));
			}
			else
			{
				uint32_t num_kfs = 0;
				NativeToLittleEndian<sizeof(num_kfs)>(&num_kfs);
				ss->write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));
			}

			XMLNodePtr action_set_chunk = root->FirstNode("actions_chunk");
			if (action_set_chunk)
			{
				uint32_t num_actions = 0;
				for (XMLNodePtr action_node = action_set_chunk->FirstNode("action"); action_node; action_node = action_node->NextSibling("action"))
				{
					++ num_actions;
				}
				num_actions = std::max(num_actions, 1U);
				NativeToLittleEndian<sizeof(num_actions)>(&num_actions);
				ss->write(reinterpret_cast<char*>(&num_actions), sizeof(num_actions));
			}
			else
			{
				uint32_t num_actions = key_frames_chunk ? 1 : 0;
				NativeToLittleEndian<sizeof(num_actions)>(&num_actions);
				ss->write(reinterpret_cast<char*>(&num_actions), sizeof(num_actions));
			}

			if (materials_chunk)
			{
				uint32_t mtl_index = 0;
				for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"), ++ mtl_index)
				{
					std::istringstream attr_ss;
					float3 ambient, diffuse, specular, emit;
					attr_ss.str(mtl_node->Attrib("ambient")->ValueString());
					attr_ss.seekg(0);
					attr_ss >> ambient.x() >> ambient.y() >> ambient.z();
					attr_ss.str(mtl_node->Attrib("diffuse")->ValueString());
					attr_ss.seekg(0);
					attr_ss >> diffuse.x() >> diffuse.y() >> diffuse.z();
					attr_ss.str(mtl_node->Attrib("specular")->ValueString());
					attr_ss.seekg(0);
					attr_ss >> specular.x() >> specular.y() >> specular.z();
					attr_ss.str(mtl_node->Attrib("emit")->ValueString());
					attr_ss.seekg(0);
					attr_ss >> emit.x() >> emit.y() >> emit.z();
					float opacity = mtl_node->Attrib("opacity")->ValueFloat();
					float specular_level = mtl_node->Attrib("specular_level")->ValueFloat();
					float shininess = mtl_node->Attrib("shininess")->ValueFloat();

					NativeToLittleEndian<sizeof(ambient[0])>(&ambient[0]);
					NativeToLittleEndian<sizeof(ambient[1])>(&ambient[1]);
					NativeToLittleEndian<sizeof(ambient[2])>(&ambient[2]);
					NativeToLittleEndian<sizeof(diffuse[0])>(&diffuse[0]);
					NativeToLittleEndian<sizeof(diffuse[1])>(&diffuse[1]);
					NativeToLittleEndian<sizeof(diffuse[2])>(&diffuse[2]);
					NativeToLittleEndian<sizeof(specular[0])>(&specular[0]);
					NativeToLittleEndian<sizeof(specular[1])>(&specular[1]);
					NativeToLittleEndian<sizeof(specular[2])>(&specular[2]);
					NativeToLittleEndian<sizeof(emit[0])>(&emit[0]);
					NativeToLittleEndian<sizeof(emit[1])>(&emit[1]);
					NativeToLittleEndian<sizeof(emit[2])>(&emit[2]);
					NativeToLittleEndian<sizeof(opacity)>(&opacity);
					NativeToLittleEndian<sizeof(specular_level)>(&specular_level);
					NativeToLittleEndian<sizeof(shininess)>(&shininess);

					ss->write(reinterpret_cast<char*>(&ambient), sizeof(ambient));
					ss->write(reinterpret_cast<char*>(&diffuse), sizeof(diffuse));
					ss->write(reinterpret_cast<char*>(&specular), sizeof(specular));
					ss->write(reinterpret_cast<char*>(&emit), sizeof(emit));
					ss->write(reinterpret_cast<char*>(&opacity), sizeof(opacity));
					ss->write(reinterpret_cast<char*>(&specular_level), sizeof(specular_level));
					ss->write(reinterpret_cast<char*>(&shininess), sizeof(shininess));

					XMLNodePtr textures_chunk = mtl_node->FirstNode("textures_chunk");
					if (textures_chunk)
					{
						uint32_t num_texs = 0;
						for (XMLNodePtr tex_node = textures_chunk->FirstNode("texture"); tex_node; tex_node = tex_node->NextSibling("texture"))
						{
							++ num_texs;
						}
						NativeToLittleEndian<sizeof(num_texs)>(&num_texs);
						ss->write(reinterpret_cast<char*>(&num_texs), sizeof(num_texs));

						for (XMLNodePtr tex_node = textures_chunk->FirstNode("texture"); tex_node; tex_node = tex_node->NextSibling("texture"))
						{
							WriteShortString(*ss, tex_node->Attrib("type")->ValueString());
							WriteShortString(*ss, tex_node->Attrib("name")->ValueString());
						}
					}
					else
					{
						uint32_t num_texs = 0;
						NativeToLittleEndian<sizeof(num_texs)>(&num_texs);
						ss->write(reinterpret_cast<char*>(&num_texs), sizeof(num_texs));
					}
				}
			}

			std::vector<AABBox> pos_bbs;
			if (meshes_chunk)
			{
				std::vector<std::vector<vertex_element> > ves;
				std::vector<uint32_t> mesh_num_vertices;
				std::vector<uint32_t> mesh_base_vertices(1, 0);
				std::vector<uint32_t> mesh_num_indices;
				std::vector<uint32_t> mesh_start_indices(1, 0);
				char is_index_16_bit = true;
				for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"))
				{
					{
						XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");

						bool has_normal = false;
						bool has_diffuse = false;
						bool has_specular = false;
						bool has_weight = false;
						bool has_tex_coord = false;
						bool has_tangent = false;
						bool has_binormal = false;
						bool has_tangent_quat = false;

						uint32_t num_vertices = 0;
						for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
						{
							++ num_vertices;
						
							XMLNodePtr normal_node = vertex_node->FirstNode("normal");
							if (normal_node)
							{
								has_normal = true;
							}

							XMLNodePtr diffuse_node = vertex_node->FirstNode("diffuse");
							if (diffuse_node)
							{
								has_diffuse = true;
							}

							XMLNodePtr specular_node = vertex_node->FirstNode("specular");
							if (specular_node)
							{
								has_specular = true;
							}

							XMLNodePtr weight_node = vertex_node->FirstNode("weight");
							if (weight_node)
							{
								has_weight = true;
							}

							XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord");
							if (tex_coord_node)
							{
								has_tex_coord = true;
							}

							XMLNodePtr tangent_node = vertex_node->FirstNode("tangent");
							if (tangent_node)
							{
								has_tangent = true;
							}

							XMLNodePtr binormal_node = vertex_node->FirstNode("binormal");
							if (binormal_node)
							{
								has_binormal = true;
							}

							XMLNodePtr tangent_quat_node = vertex_node->FirstNode("tangent_quat");
							if (tangent_quat_node)
							{
								has_tangent_quat = true;
							}
						}

						mesh_num_vertices.push_back(num_vertices);
						mesh_base_vertices.push_back(mesh_base_vertices.back() + num_vertices);

						ves.push_back(std::vector<vertex_element>());
						std::vector<vertex_element>& vertex_elements = ves.back();
						{
							vertex_element ve;

							{
								ve.usage = VEU_Position;
								ve.usage_index = 0;
								ve.format = EF_SIGNED_ABGR16;
								vertex_elements.push_back(ve);
							}

							if (has_diffuse)
							{
								ve.usage = VEU_Diffuse;
								ve.usage_index = 0;
								ve.format = EF_ABGR8;
								vertex_elements.push_back(ve);
							}

							if (has_specular)
							{
								ve.usage = VEU_Specular;
								ve.usage_index = 0;
								ve.format = EF_ABGR8;
								vertex_elements.push_back(ve);
							}

							if (has_weight)
							{
								ve.usage = VEU_BlendWeight;
								ve.usage_index = 0;
								ve.format = EF_ABGR8;
								vertex_elements.push_back(ve);

								ve.usage = VEU_BlendIndex;
								ve.usage_index = 0;
								ve.format = EF_ABGR8UI;
								vertex_elements.push_back(ve);
							}

							if (has_tex_coord)
							{
								ve.usage = VEU_TextureCoord;
								ve.usage_index = 0;
								ve.format = EF_SIGNED_GR16;
								vertex_elements.push_back(ve);
							}

							if (has_tangent_quat)
							{
								ve.usage = VEU_Tangent;
								ve.usage_index = 0;
								ve.format = EF_ABGR8;
								vertex_elements.push_back(ve);
							}
							else
							{
								if (has_normal && !has_tangent && !has_binormal)
								{
									ve.usage = VEU_Normal;
									ve.usage_index = 0;
									ve.format = EF_A2BGR10;
									vertex_elements.push_back(ve);
								}
								else
								{
									if ((has_normal && has_tangent) || (has_normal && has_binormal)
										|| (has_tangent && has_binormal))
									{
										ve.usage = VEU_Tangent;
										ve.usage_index = 0;
										ve.format = EF_ABGR8;
										vertex_elements.push_back(ve);
									}
								}
							}
						}
					}

					{
						XMLNodePtr triangles_chunk = mesh_node->FirstNode("triangles_chunk");

						uint32_t num_indices = 0;
						for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
						{
							num_indices += 3;
						}

						char is_index_16 = true;
						for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
						{
							uint32_t a, b, c;
							std::istringstream attr_ss(tri_node->Attrib("index")->ValueString());
							attr_ss >> a >> b >> c;
							if ((a > 0xFFFF) || (b > 0xFFFF) || (c > 0xFFFF))
							{
								is_index_16 = false;
							}
						}
						is_index_16_bit &= is_index_16;

						mesh_num_indices.push_back(num_indices);
						mesh_start_indices.push_back(mesh_start_indices.back() + num_indices);
					}
				}

				std::vector<vertex_element> merged_ves;
				std::vector<std::vector<uint32_t> > ves_mapping(ves.size());
				uint32_t mesh_index = 0;
				for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"), ++ mesh_index)
				{
					XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");

					ves_mapping[mesh_index].resize(ves[mesh_index].size());
					for (uint32_t ve_index = 0; ve_index < ves[mesh_index].size(); ++ ve_index)
					{
						bool found = false;
						for (uint32_t mve_index = 0; mve_index < merged_ves.size(); ++ mve_index)
						{
							if (ves[mesh_index][ve_index] == merged_ves[mve_index])
							{
								ves_mapping[mesh_index][ve_index] = mve_index;
								found = true;
								break;
							}
						}
						if (!found)
						{
							ves_mapping[mesh_index][ve_index] = static_cast<uint32_t>(merged_ves.size());
							merged_ves.push_back(ves[mesh_index][ve_index]);
						}
					}
				}

				std::vector<std::vector<uint8_t> > merged_buff(merged_ves.size());
				for (size_t i = 0; i < merged_buff.size(); ++ i)
				{
					merged_buff[i].resize(mesh_base_vertices.back() * merged_ves[i].element_size(), 0);
				}

				int const index_elem_size = is_index_16_bit ? 2 : 4;

				std::vector<uint8_t> merged_indices(mesh_start_indices.back() * index_elem_size);
				std::vector<AABBox> tc_bbs;
				mesh_index = 0;
				for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"), ++ mesh_index)
				{
					float3 pos_min_bb, pos_max_bb;
					float3 tc_min_bb, tc_max_bb;
						
					{
						XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");

						XMLNodePtr pos_bb_node = vertices_chunk->FirstNode("pos_bb");
						if (pos_bb_node)
						{
							std::istringstream attr_ss;
							XMLAttributePtr attr = pos_bb_node->Attrib("min");
							attr_ss.str(attr->ValueString());
							attr_ss.seekg(0);
							attr_ss >> pos_min_bb[0] >> pos_min_bb[1] >> pos_min_bb[2];
							attr = pos_bb_node->Attrib("max");
							attr_ss.str(attr->ValueString());
							attr_ss.seekg(0);
							attr_ss >> pos_max_bb[0] >> pos_max_bb[1] >> pos_max_bb[2];
						}
						else
						{
							uint32_t index = 0;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								float3 pos;
								std::istringstream attr_ss(vertex_node->Attrib("v")->ValueString());
								attr_ss >> pos.x() >> pos.y() >> pos.z();
								if (0 == index)
								{
									pos_min_bb = pos_max_bb = pos;
								}
								else
								{
									pos_min_bb = MathLib::minimize(pos_min_bb, pos);
									pos_max_bb = MathLib::maximize(pos_max_bb, pos);
								}
								++ index;
							}
						}

						XMLNodePtr tc_bb_node = vertices_chunk->FirstNode("tc_bb");
						if (tc_bb_node)
						{
							XMLAttributePtr attr = tc_bb_node->Attrib("min");
							std::istringstream attr_ss(attr->ValueString());
							attr_ss >> tc_min_bb[0] >> tc_min_bb[1];
							attr = tc_bb_node->Attrib("max");
							attr_ss.str(attr->ValueString());
							attr_ss.seekg(0);
							attr_ss >> tc_max_bb[0] >> tc_max_bb[1];

							tc_min_bb.z() = 0;
							tc_max_bb.z() = 0;
						}
						else
						{
							uint32_t index = 0;
							for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
							{
								XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord");
								float3 tex_coord;
								std::istringstream attr_ss(tex_coord_node->Attrib("v")->ValueString());
								attr_ss >> tex_coord.x() >> tex_coord.y();
								tex_coord.z() = 0;
								if (0 == index)
								{
									tc_min_bb = tc_max_bb = tex_coord;
								}
								else
								{
									tc_min_bb = MathLib::minimize(tc_min_bb, tex_coord);
									tc_max_bb = MathLib::maximize(tc_max_bb, tex_coord);
								}
								++ index;
							}
						}

						pos_bbs.push_back(AABBox(pos_min_bb, pos_max_bb));
						tc_bbs.push_back(AABBox(tc_min_bb, tc_max_bb));

						float3 const pos_center = pos_bbs.back().Center();
						float3 const pos_extent = pos_bbs.back().HalfSize();
						float3 const tc_center = tc_bbs.back().Center();
						float3 const tc_extent = tc_bbs.back().HalfSize();

						uint32_t index = 0;
						for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
						{
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_Position == ves[mesh_index][i].usage)
									{
										float3 pos;
										std::istringstream attr_ss(vertex_node->Attrib("v")->ValueString());
										attr_ss >> pos.x() >> pos.y() >> pos.z();
										pos = (pos - pos_center) / pos_extent * 0.5f + 0.5f;
										int16_t s_pos[4] = 
										{
											static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.x() * 65535 - 32768), -32768, 32767)),
											static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.y() * 65535 - 32768), -32768, 32767)),
											static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.z() * 65535 - 32768), -32768, 32767)),
											32767
										};
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], s_pos, sizeof(s_pos));
										break;
									}
								}
							}

							XMLNodePtr diffuse_node = vertex_node->FirstNode("diffuse");
							if (diffuse_node)
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_Diffuse == ves[mesh_index][i].usage)
									{
										float4 diffuse;
										std::istringstream attr_ss(diffuse_node->Attrib("v")->ValueString());
										attr_ss >> diffuse.x() >> diffuse.y() >> diffuse.z() >> diffuse.w();
										uint32_t compact = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((diffuse.x() * 0.5f + 0.5f) * 255), 0, 255) << 0)
											| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((diffuse.y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
											| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((diffuse.z() * 0.5f + 0.5f) * 255), 0, 255) << 16)
											| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((diffuse.w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &compact, sizeof(compact));
										break;
									}
								}
							}

							XMLNodePtr specular_node = vertex_node->FirstNode("specular");
							if (specular_node)
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_Specular == ves[mesh_index][i].usage)
									{
										float4 specular;
										std::istringstream attr_ss(specular_node->Attrib("v")->ValueString());
										attr_ss >> specular.x() >> specular.y() >> specular.z() >> specular.w();
										uint32_t compact = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((specular.x() * 0.5f + 0.5f) * 255), 0, 255) << 0)
											| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((specular.y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
											| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((specular.z() * 0.5f + 0.5f) * 255), 0, 255) << 16)
											| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((specular.w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &compact, sizeof(compact));
										break;
									}
								}
							}

							XMLNodePtr weight_node = vertex_node->FirstNode("weight");
							if (weight_node)
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_BlendIndex == ves[mesh_index][i].usage)
									{
										std::istringstream attr_ss(weight_node->Attrib("bone_index")->ValueString());
										uint32_t bone_index32[4] = { 0, 0, 0, 0 };
										uint32_t num_blend = 0;
										while (attr_ss && (num_blend < 4))
										{
											attr_ss >> bone_index32[num_blend];
											++ num_blend;
										}
										for (int j = 0; j < 4; ++ j)
										{
											uint8_t bone_index = static_cast<uint8_t>(bone_index32[j]);
											uint32_t buf_index = ves_mapping[mesh_index][i];
											memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size() + j * sizeof(bone_index)], &bone_index, sizeof(bone_index));
										}
										break;
									}
								}
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_BlendWeight == ves[mesh_index][i].usage)
									{
										std::istringstream attr_ss(weight_node->Attrib("weight")->ValueString());
										float bone_weight32[4] = { 0, 0, 0, 0 };
										uint32_t num_blend = 0;
										while (attr_ss && (num_blend < 4))
										{
											attr_ss >> bone_weight32[num_blend];
											++ num_blend;
										}
										for (int j = 0; j < 4; ++ j)
										{
											uint8_t bone_weight = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(bone_weight32[j] * 255), 0, 255));
											uint32_t buf_index = ves_mapping[mesh_index][i];
											memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size() + j * sizeof(bone_weight)], &bone_weight, sizeof(bone_weight));
										}
										break;
									}
								}
							}

							XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord");
							if (tex_coord_node)
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_TextureCoord == ves[mesh_index][i].usage)
									{
										float3 tex_coord;
										std::istringstream attr_ss(tex_coord_node->Attrib("v")->ValueString());
										attr_ss >> tex_coord.x() >> tex_coord.y();
										tex_coord.z() = 0;

										tex_coord = (tex_coord - tc_center) / tc_extent * 0.5f + 0.5f;
										int16_t s_tc[2] = 
										{
											static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tex_coord.x() * 65535 - 32768), -32768, 32767)),
											static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tex_coord.y() * 65535 - 32768), -32768, 32767)),
										};

										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], s_tc, merged_ves[buf_index].element_size());
										break;
									}
								}
							}

							XMLNodePtr tangent_quat_node = vertex_node->FirstNode("tangent_quat");
							if (tangent_quat_node)
							{
								Quaternion tangent_quat;
								std::istringstream attr_ss(tangent_quat_node->Attrib("v")->ValueString());
								attr_ss >> tangent_quat.x() >> tangent_quat.y() >> tangent_quat.z() >> tangent_quat.w();
								uint32_t compact = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.x() * 0.5f + 0.5f) * 255), 0, 255) << 0)
									| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
									| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.z() * 0.5f + 0.5f) * 255), 0, 255) << 16)
									| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_Tangent == ves[mesh_index][i].usage)
									{
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &compact, sizeof(compact));
										break;
									}
								}
							}
							else
							{
								XMLNodePtr normal_node = vertex_node->FirstNode("normal");
								if (normal_node)
								{
									for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
									{
										if (VEU_Normal == ves[mesh_index][i].usage)
										{
											float3 normal;
											std::istringstream attr_ss(normal_node->Attrib("v")->ValueString());
											attr_ss >> normal.x() >> normal.y() >> normal.z();
											normal = MathLib::normalize(normal) * 0.5f + 0.5f;

											uint32_t compact = MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.x() * 1023), 0, 1023)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.y() * 1023), 0, 1023) << 10)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.z() * 1023), 0, 1023) << 20);

											uint32_t buf_index = ves_mapping[mesh_index][i];
											memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &compact, sizeof(compact));
											break;
										}
									}
								}
							}

							++ index;
						}
					}

					{
						XMLNodePtr triangles_chunk = mesh_node->FirstNode("triangles_chunk");

						uint32_t index = 0;
						for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
						{
							std::istringstream attr_ss(tri_node->Attrib("index")->ValueString());
							if (is_index_16_bit)
							{
								uint16_t a, b, c;
								attr_ss >> a >> b >> c;
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 0) * index_elem_size], &a, sizeof(a));
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 1) * index_elem_size], &b, sizeof(b));
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 2) * index_elem_size], &c, sizeof(c));
							}
							else
							{
								uint32_t a, b, c;
								attr_ss >> a >> b >> c;
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 0) * index_elem_size], &a, sizeof(a));
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 1) * index_elem_size], &b, sizeof(b));
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 2) * index_elem_size], &c, sizeof(c));
							}

							++ index;
						}
					}
				}

				uint32_t num_merged_ves = static_cast<uint32_t>(merged_ves.size());
				NativeToLittleEndian<sizeof(num_merged_ves)>(&num_merged_ves);
				ss->write(reinterpret_cast<char*>(&num_merged_ves), sizeof(num_merged_ves));
				for (size_t i = 0; i < merged_ves.size(); ++ i)
				{
					NativeToLittleEndian<sizeof(merged_ves[i].usage)>(&merged_ves[i].usage);
					NativeToLittleEndian<sizeof(merged_ves[i].format)>(&merged_ves[i].format);
					ss->write(reinterpret_cast<char*>(&merged_ves[i]), sizeof(merged_ves[i]));
				}

				uint32_t num_vertices = mesh_base_vertices.back();
				NativeToLittleEndian<sizeof(num_vertices)>(&num_vertices);
				ss->write(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));
				uint32_t num_indices = mesh_start_indices.back();
				NativeToLittleEndian<sizeof(num_indices)>(&num_indices);
				ss->write(reinterpret_cast<char*>(&num_indices), sizeof(num_indices));
				ss->write(&is_index_16_bit, sizeof(is_index_16_bit));

				for (size_t i = 0; i < merged_buff.size(); ++ i)
				{
					ss->write(reinterpret_cast<char*>(&merged_buff[i][0]), merged_buff[i].size() * sizeof(merged_buff[i][0]));
				}
				ss->write(reinterpret_cast<char*>(&merged_indices[0]), merged_indices.size() * sizeof(merged_indices[0]));

				mesh_index = 0;
				for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"))
				{
					WriteShortString(*ss, mesh_node->Attrib("name")->ValueString());

					int32_t mtl_id = mesh_node->Attrib("mtl_id")->ValueInt();
					NativeToLittleEndian<sizeof(mtl_id)>(&mtl_id);
					ss->write(reinterpret_cast<char*>(&mtl_id), sizeof(mtl_id));

					float3 min_bb = pos_bbs[mesh_index].Min();
					NativeToLittleEndian<sizeof(min_bb[0])>(&min_bb[0]);
					NativeToLittleEndian<sizeof(min_bb[1])>(&min_bb[1]);
					NativeToLittleEndian<sizeof(min_bb[2])>(&min_bb[2]);
					ss->write(reinterpret_cast<char*>(&min_bb), sizeof(min_bb));
					float3 max_bb = pos_bbs[mesh_index].Max();
					NativeToLittleEndian<sizeof(max_bb[0])>(&max_bb[0]);
					NativeToLittleEndian<sizeof(max_bb[1])>(&max_bb[1]);
					NativeToLittleEndian<sizeof(max_bb[2])>(&max_bb[2]);
					ss->write(reinterpret_cast<char*>(&max_bb), sizeof(max_bb));

					min_bb = tc_bbs[mesh_index].Min();
					NativeToLittleEndian<sizeof(min_bb[0])>(&min_bb[0]);
					NativeToLittleEndian<sizeof(min_bb[1])>(&min_bb[1]);
					ss->write(reinterpret_cast<char*>(&min_bb[0]), sizeof(min_bb[0]));
					ss->write(reinterpret_cast<char*>(&min_bb[1]), sizeof(min_bb[1]));
					max_bb = tc_bbs[mesh_index].Max();
					NativeToLittleEndian<sizeof(max_bb[0])>(&max_bb[0]);
					NativeToLittleEndian<sizeof(max_bb[1])>(&max_bb[1]);
					ss->write(reinterpret_cast<char*>(&max_bb[0]), sizeof(max_bb[0]));
					ss->write(reinterpret_cast<char*>(&max_bb[1]), sizeof(max_bb[1]));

					NativeToLittleEndian<sizeof(mesh_num_vertices[mesh_index])>(&mesh_num_vertices[mesh_index]);
					ss->write(reinterpret_cast<char*>(&mesh_num_vertices[mesh_index]), sizeof(mesh_num_vertices[mesh_index]));
					NativeToLittleEndian<sizeof(mesh_base_vertices[mesh_index])>(&mesh_base_vertices[mesh_index]);
					ss->write(reinterpret_cast<char*>(&mesh_base_vertices[mesh_index]), sizeof(mesh_base_vertices[mesh_index]));
					NativeToLittleEndian<sizeof(mesh_num_indices[mesh_index])>(&mesh_num_indices[mesh_index]);
					ss->write(reinterpret_cast<char*>(&mesh_num_indices[mesh_index]), sizeof(mesh_num_indices[mesh_index]));
					NativeToLittleEndian<sizeof(mesh_start_indices[mesh_index])>(&mesh_start_indices[mesh_index]);
					ss->write(reinterpret_cast<char*>(&mesh_start_indices[mesh_index]), sizeof(mesh_start_indices[mesh_index]));

					++ mesh_index;
				}
			}

			if (bones_chunk)
			{
				for (XMLNodePtr bone_node = bones_chunk->FirstNode("bone"); bone_node; bone_node = bone_node->NextSibling("bone"))
				{
					WriteShortString(*ss, bone_node->Attrib("name")->ValueString());

					int16_t joint_parent = static_cast<int16_t>(bone_node->Attrib("parent")->ValueInt());
					NativeToLittleEndian<sizeof(joint_parent)>(&joint_parent);
					ss->write(reinterpret_cast<char*>(&joint_parent), sizeof(joint_parent));

					Quaternion bind_real;
					XMLNodePtr bind_real_node = bone_node->FirstNode("bind_real");
					std::istringstream attr_ss(bind_real_node->Attrib("v")->ValueString());
					attr_ss >> bind_real.x() >> bind_real.y() >> bind_real.z() >> bind_real.w();
							
					Quaternion bind_dual;
					XMLNodePtr bind_dual_node = bone_node->FirstNode("bind_dual");
					attr_ss.str(bind_dual_node->Attrib("v")->ValueString());
					attr_ss.seekg(0);
					attr_ss >> bind_dual.x() >> bind_dual.y() >> bind_dual.z() >> bind_dual.w();

					NativeToLittleEndian<sizeof(bind_real[0])>(&bind_real[0]);
					NativeToLittleEndian<sizeof(bind_real[1])>(&bind_real[1]);
					NativeToLittleEndian<sizeof(bind_real[2])>(&bind_real[2]);
					NativeToLittleEndian<sizeof(bind_real[3])>(&bind_real[3]);
					ss->write(reinterpret_cast<char*>(&bind_real), sizeof(bind_real));
					NativeToLittleEndian<sizeof(bind_dual[0])>(&bind_dual[0]);
					NativeToLittleEndian<sizeof(bind_dual[1])>(&bind_dual[1]);
					NativeToLittleEndian<sizeof(bind_dual[2])>(&bind_dual[2]);
					NativeToLittleEndian<sizeof(bind_dual[3])>(&bind_dual[3]);
					ss->write(reinterpret_cast<char*>(&bind_dual), sizeof(bind_dual));
				}
			}

			if (key_frames_chunk)
			{
				XMLAttributePtr nf_attr = key_frames_chunk->Attrib("num_frames");
				uint32_t num_frames = nf_attr->ValueUInt();
				int32_t frame_rate = key_frames_chunk->Attrib("frame_rate")->ValueInt();
				NativeToLittleEndian<sizeof(num_frames)>(&num_frames);
				ss->write(reinterpret_cast<char*>(&num_frames), sizeof(num_frames));
				NativeToLittleEndian<sizeof(frame_rate)>(&frame_rate);
				ss->write(reinterpret_cast<char*>(&frame_rate), sizeof(frame_rate));

				KeyFrames kfs;
				for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
				{
					kfs.frame_id.clear();
					kfs.bind_real.clear();
					kfs.bind_dual.clear();
					kfs.bind_scale.clear();

					int32_t frame_id = -1;
					for (XMLNodePtr key_node = kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
					{
						XMLAttributePtr id_attr = key_node->Attrib("id");
						if (id_attr)
						{
							frame_id = id_attr->ValueInt();
						}
						else
						{
							++ frame_id;
						}
						kfs.frame_id.push_back(frame_id);

						Quaternion bind_real;
						XMLNodePtr bind_real_node = key_node->FirstNode("bind_real");
						std::istringstream attr_ss(bind_real_node->Attrib("v")->ValueString());
						attr_ss >> bind_real.x() >> bind_real.y() >> bind_real.z() >> bind_real.w();
							
						Quaternion bind_dual;
						XMLNodePtr bind_dual_node = key_node->FirstNode("bind_dual");
						attr_ss.str(bind_dual_node->Attrib("v")->ValueString());
						attr_ss.seekg(0);
						attr_ss >> bind_dual.x() >> bind_dual.y() >> bind_dual.z() >> bind_dual.w();

						float bind_scale = MathLib::length(bind_real);
						bind_real /= bind_scale;
						if (bind_real.w() < 0)
						{
							bind_real = -bind_real;
							bind_scale = -bind_scale;
						}

						kfs.bind_real.push_back(bind_real);
						kfs.bind_dual.push_back(bind_dual);
						kfs.bind_scale.push_back(bind_scale);
					}

					// compress the key frame data
					uint32_t base = 0;
					while (base < kfs.frame_id.size() - 2)
					{
						uint32_t frame0 = kfs.frame_id[base + 0];
						uint32_t frame1 = kfs.frame_id[base + 1];
						uint32_t frame2 = kfs.frame_id[base + 2];
						float factor = static_cast<float>(frame1 - frame0) / (frame2 - frame0);
						std::pair<Quaternion, Quaternion> interpolate = MathLib::sclerp(kfs.bind_real[base + 0], kfs.bind_dual[base + 0],
							kfs.bind_real[base + 2], kfs.bind_dual[base + 2], factor);
						float scale = MathLib::lerp(kfs.bind_scale[base + 0], kfs.bind_scale[base + 2], factor);

						if (MathLib::dot(kfs.bind_real[base + 1], interpolate.first) < 0)
						{
							interpolate.first = -interpolate.first;
							interpolate.second = -interpolate.second;
						}

						std::pair<Quaternion, Quaternion> dif_dq = MathLib::inverse(kfs.bind_real[base + 1], kfs.bind_dual[base + 1]);
						dif_dq.second = MathLib::mul_dual(dif_dq.first, dif_dq.second * scale, interpolate.first, interpolate.second);
						dif_dq.first = MathLib::mul_real(dif_dq.first, interpolate.first);
						float dif_scale = scale * kfs.bind_scale[base + 1];

						if ((MathLib::abs(dif_dq.first.x()) < 1e-5f) && (MathLib::abs(dif_dq.first.y()) < 1e-5f)
							&& (MathLib::abs(dif_dq.first.z()) < 1e-5f) && (MathLib::abs(dif_dq.first.w() - 1) < 1e-5f)
							&& (MathLib::abs(dif_dq.second.x()) < 1e-5f) && (MathLib::abs(dif_dq.second.y()) < 1e-5f)
							&& (MathLib::abs(dif_dq.second.z()) < 1e-5f) && (MathLib::abs(dif_dq.second.w()) < 1e-5f)
							&& (MathLib::abs(dif_scale - 1) < 1e-5f))
						{
							kfs.frame_id.erase(kfs.frame_id.begin() + base + 1);
							kfs.bind_real.erase(kfs.bind_real.begin() + base + 1);
							kfs.bind_dual.erase(kfs.bind_dual.begin() + base + 1);
							kfs.bind_scale.erase(kfs.bind_scale.begin() + base + 1);
						}
						else
						{
							++ base;
						}
					}

					uint32_t num_kf = static_cast<uint32_t>(kfs.frame_id.size());
					NativeToLittleEndian<sizeof(num_kf)>(&num_kf);
					ss->write(reinterpret_cast<char*>(&num_kf), sizeof(num_kf));
					for (uint32_t i = 0; i < num_kf; ++ i)
					{
						NativeToLittleEndian<sizeof(kfs.frame_id[i])>(&kfs.frame_id[i]);
						ss->write(reinterpret_cast<char*>(&kfs.frame_id[i]), sizeof(kfs.frame_id[i]));
						kfs.bind_real[i] *= kfs.bind_scale[i];
						NativeToLittleEndian<sizeof(kfs.bind_real[i][0])>(&kfs.bind_real[i][0]);
						NativeToLittleEndian<sizeof(kfs.bind_real[i][1])>(&kfs.bind_real[i][1]);
						NativeToLittleEndian<sizeof(kfs.bind_real[i][2])>(&kfs.bind_real[i][2]);
						NativeToLittleEndian<sizeof(kfs.bind_real[i][3])>(&kfs.bind_real[i][3]);
						ss->write(reinterpret_cast<char*>(&kfs.bind_real[i]), sizeof(kfs.bind_real[i]));
						NativeToLittleEndian<sizeof(kfs.bind_dual[i][0])>(&kfs.bind_dual[i][0]);
						NativeToLittleEndian<sizeof(kfs.bind_dual[i][1])>(&kfs.bind_dual[i][1]);
						NativeToLittleEndian<sizeof(kfs.bind_dual[i][2])>(&kfs.bind_dual[i][2]);
						NativeToLittleEndian<sizeof(kfs.bind_dual[i][3])>(&kfs.bind_dual[i][3]);
						ss->write(reinterpret_cast<char*>(&kfs.bind_dual[i]), sizeof(kfs.bind_dual[i]));
					}
				}

				AABBKeyFrames bb_kfs;
				XMLNodePtr bb_kfs_chunk = root->FirstNode("bb_key_frames_chunk");
				if (bb_kfs_chunk)
				{
					for (XMLNodePtr bb_kf_node = bb_kfs_chunk->FirstNode("bb_key_frame"); bb_kf_node; bb_kf_node = bb_kf_node->NextSibling("bb_key_frame"))
					{
						bb_kfs.frame_id.clear();
						bb_kfs.bb.clear();

						int32_t frame_id = -1;
						for (XMLNodePtr key_node = bb_kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
						{
							XMLAttributePtr id_attr = key_node->Attrib("id");
							if (id_attr)
							{
								frame_id = id_attr->ValueInt();
							}
							else
							{
								++ frame_id;
							}
							bb_kfs.frame_id.push_back(frame_id);

							float3 bb_min, bb_max;
							std::istringstream attr_ss;
							XMLAttributePtr attr = key_node->Attrib("min");
							attr_ss.str(attr->ValueString());
							attr_ss.seekg(0);
							attr_ss >> bb_min[0] >> bb_min[1] >> bb_min[2];
							attr = key_node->Attrib("max");
							attr_ss.str(attr->ValueString());
							attr_ss.seekg(0);
							attr_ss >> bb_max[0] >> bb_max[1] >> bb_max[2];

							bb_kfs.bb.push_back(AABBox(bb_min, bb_max));
						}

						// compress the bounding box key frame data
						uint32_t base = 0;
						while (base < bb_kfs.frame_id.size() - 2)
						{
							uint32_t frame0 = kfs.frame_id[base + 0];
							uint32_t frame1 = kfs.frame_id[base + 1];
							uint32_t frame2 = kfs.frame_id[base + 2];
							float factor = static_cast<float>(frame1 - frame0) / (frame2 - frame0);
							AABBox interpolate(MathLib::lerp(bb_kfs.bb[base + 0].Min(), bb_kfs.bb[base + 2].Min(), factor),
								MathLib::lerp(bb_kfs.bb[base + 0].Max(), bb_kfs.bb[base + 2].Max(), factor));

							float3 diff_min = bb_kfs.bb[base + 1].Min() - interpolate.Min();
							float3 diff_max = bb_kfs.bb[base + 1].Max() - interpolate.Max();

							if ((MathLib::abs(diff_min.x()) < 1e-5f) && (MathLib::abs(diff_min.y()) < 1e-5f)
								&& (MathLib::abs(diff_min.z()) < 1e-5f)
								&& (MathLib::abs(diff_max.x()) < 1e-5f) && (MathLib::abs(diff_max.y()) < 1e-5f)
								&& (MathLib::abs(diff_max.z()) < 1e-5f))
							{
								bb_kfs.frame_id.erase(bb_kfs.frame_id.begin() + base + 1);
								bb_kfs.bb.erase(bb_kfs.bb.begin() + base + 1);
							}
							else
							{
								++ base;
							}
						}

						uint32_t num_bb_kf = static_cast<uint32_t>(bb_kfs.frame_id.size());
						NativeToLittleEndian<sizeof(num_bb_kf)>(&num_bb_kf);
						ss->write(reinterpret_cast<char*>(&num_bb_kf), sizeof(num_bb_kf));
						for (uint32_t i = 0; i < num_bb_kf; ++ i)
						{
							NativeToLittleEndian<sizeof(bb_kfs.frame_id[i])>(&bb_kfs.frame_id[i]);
							ss->write(reinterpret_cast<char*>(&bb_kfs.frame_id[i]), sizeof(bb_kfs.frame_id[i]));
							float3 bb_min = bb_kfs.bb[i].Min();
							float3 bb_max = bb_kfs.bb[i].Max();
							NativeToLittleEndian<sizeof(bb_min[0])>(&bb_min[0]);
							NativeToLittleEndian<sizeof(bb_min[1])>(&bb_min[1]);
							NativeToLittleEndian<sizeof(bb_min[2])>(&bb_min[2]);
							ss->write(reinterpret_cast<char*>(&bb_min), sizeof(bb_min));
							NativeToLittleEndian<sizeof(bb_max[0])>(&bb_max[0]);
							NativeToLittleEndian<sizeof(bb_max[1])>(&bb_max[1]);
							NativeToLittleEndian<sizeof(bb_max[2])>(&bb_max[2]);
							ss->write(reinterpret_cast<char*>(&bb_max), sizeof(bb_max));
						}
					}
				}
				else
				{
					bb_kfs.frame_id.resize(2);
					bb_kfs.bb.resize(2);

					bb_kfs.frame_id[0] = 0;
					bb_kfs.frame_id[1] = num_frames - 1;

					uint32_t mesh_index = 0;
					for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"), ++ mesh_index)
					{
						bb_kfs.bb[0] = pos_bbs[mesh_index];
						bb_kfs.bb[1] = pos_bbs[mesh_index];

						uint32_t num_bb_kf = static_cast<uint32_t>(bb_kfs.frame_id.size());
						NativeToLittleEndian<sizeof(num_bb_kf)>(&num_bb_kf);
						ss->write(reinterpret_cast<char*>(&num_bb_kf), sizeof(num_bb_kf));
						for (uint32_t i = 0; i < num_bb_kf; ++ i)
						{
							NativeToLittleEndian<sizeof(bb_kfs.frame_id[i])>(&bb_kfs.frame_id[i]);
							ss->write(reinterpret_cast<char*>(&bb_kfs.frame_id[i]), sizeof(bb_kfs.frame_id[i]));
							float3 bb_min = bb_kfs.bb[i].Min();
							float3 bb_max = bb_kfs.bb[i].Max();
							NativeToLittleEndian<sizeof(bb_min[0])>(&bb_min[0]);
							NativeToLittleEndian<sizeof(bb_min[1])>(&bb_min[1]);
							NativeToLittleEndian<sizeof(bb_min[2])>(&bb_min[2]);
							ss->write(reinterpret_cast<char*>(&bb_min), sizeof(bb_min));
							NativeToLittleEndian<sizeof(bb_max[0])>(&bb_max[0]);
							NativeToLittleEndian<sizeof(bb_max[1])>(&bb_max[1]);
							NativeToLittleEndian<sizeof(bb_max[2])>(&bb_max[2]);
							ss->write(reinterpret_cast<char*>(&bb_max), sizeof(bb_max));
						}
					}
				}

				XMLNodePtr action_node;
				if (action_set_chunk)
				{
					action_node = action_set_chunk->FirstNode("action");
				}
				if (action_node)
				{
					for (; action_node; action_node = action_node->NextSibling("action"))
					{
						WriteShortString(*ss, action_node->Attrib("name")->ValueString());

						uint32_t sf = action_node->Attrib("start_frame")->ValueUInt();
						uint32_t ef = action_node->Attrib("end_frame")->ValueUInt();

						NativeToLittleEndian<sizeof(sf)>(&sf);
						ss->write(reinterpret_cast<char*>(&sf), sizeof(sf));

						NativeToLittleEndian<sizeof(ef)>(&ef);
						ss->write(reinterpret_cast<char*>(&ef), sizeof(ef));
					}
				}
				else
				{
					WriteShortString(*ss, "root");

					uint32_t sf = 0;
					LittleEndianToNative<sizeof(num_frames)>(&num_frames);
					uint32_t ef = num_frames;

					NativeToLittleEndian<sizeof(sf)>(&sf);
					ss->write(reinterpret_cast<char*>(&sf), sizeof(sf));

					NativeToLittleEndian<sizeof(ef)>(&ef);
					ss->write(reinterpret_cast<char*>(&ef), sizeof(ef));
				}
			}

			std::ofstream ofs((path_name + jit_ext_name).c_str(), std::ios_base::binary);
			BOOST_ASSERT(ofs);
			uint32_t fourcc = MakeFourCC<'K', 'L', 'M', ' '>::value;
			NativeToLittleEndian<sizeof(fourcc)>(&fourcc);
			ofs.write(reinterpret_cast<char*>(&fourcc), sizeof(fourcc));

			uint32_t ver = MODEL_BIN_VERSION;
			NativeToLittleEndian<sizeof(ver)>(&ver);
			ofs.write(reinterpret_cast<char*>(&ver), sizeof(ver));

			uint64_t original_len = ss->str().size();
			NativeToLittleEndian<sizeof(original_len)>(&original_len);
			ofs.write(reinterpret_cast<char*>(&original_len), sizeof(original_len));

			std::ofstream::pos_type p = ofs.tellp();
			uint64_t len = 0;
			ofs.write(reinterpret_cast<char*>(&len), sizeof(len));

			LZMACodec lzma;
			len = lzma.Encode(ofs, ss->str().c_str(), ss->str().size());

			ofs.seekp(p, std::ios_base::beg);
			NativeToLittleEndian<sizeof(len)>(&len);
			ofs.write(reinterpret_cast<char*>(&len), sizeof(len));
		}
	}

	void LoadModel(std::string const & meshml_name, std::vector<RenderMaterialPtr>& mtls,
		std::vector<vertex_element>& merged_ves, char& all_is_index_16_bit,
		std::vector<std::vector<uint8_t> >& merged_buff, std::vector<uint8_t>& merged_indices,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids,
		std::vector<AABBox>& pos_bbs, std::vector<AABBox>& tc_bbs,
		std::vector<uint32_t>& mesh_num_vertices, std::vector<uint32_t>& mesh_base_vertices,
		std::vector<uint32_t>& mesh_num_triangles, std::vector<uint32_t>& mesh_base_triangles,
		std::vector<Joint>& joints, boost::shared_ptr<AnimationActionsType>& actions,
		boost::shared_ptr<KeyFramesType>& kfs, uint32_t& num_frames, uint32_t& frame_rate,
		std::vector<boost::shared_ptr<AABBKeyFrames> >& frame_pos_bbs)
	{
		ResIdentifierPtr lzma_file;
		if (meshml_name.rfind(jit_ext_name) + jit_ext_name.size() == meshml_name.size())
		{
			lzma_file = ResLoader::Instance().Open(meshml_name);
		}
		else
		{
			std::string full_meshml_name = ResLoader::Instance().Locate(meshml_name);
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
		LittleEndianToNative<sizeof(fourcc)>(&fourcc);
		BOOST_ASSERT((fourcc == MakeFourCC<'K', 'L', 'M', ' '>::value));

		uint32_t ver;
		lzma_file->read(&ver, sizeof(ver));
		LittleEndianToNative<sizeof(ver)>(&ver);
		BOOST_ASSERT(MODEL_BIN_VERSION == ver);

		boost::shared_ptr<std::stringstream> ss = MakeSharedPtr<std::stringstream>();

		uint64_t original_len, len;
		lzma_file->read(&original_len, sizeof(original_len));
		LittleEndianToNative<sizeof(original_len)>(&original_len);
		lzma_file->read(&len, sizeof(len));
		LittleEndianToNative<sizeof(len)>(&len);

		LZMACodec lzma;
		lzma.Decode(*ss, lzma_file, len, original_len);

		ResIdentifierPtr decoded = MakeSharedPtr<ResIdentifier>(lzma_file->ResName(), lzma_file->Timestamp(), ss);

		uint32_t num_mtls;
		decoded->read(&num_mtls, sizeof(num_mtls));
		LittleEndianToNative<sizeof(num_mtls)>(&num_mtls);
		uint32_t num_meshes;
		decoded->read(&num_meshes, sizeof(num_meshes));
		LittleEndianToNative<sizeof(num_meshes)>(&num_meshes);
		uint32_t num_joints;
		decoded->read(&num_joints, sizeof(num_joints));
		LittleEndianToNative<sizeof(num_joints)>(&num_joints);
		uint32_t num_kfs;
		decoded->read(&num_kfs, sizeof(num_kfs));
		LittleEndianToNative<sizeof(num_kfs)>(&num_kfs);
		uint32_t num_actions;
		decoded->read(&num_actions, sizeof(num_actions));
		LittleEndianToNative<sizeof(num_actions)>(&num_actions);

		mtls.resize(num_mtls);
		for (uint32_t mtl_index = 0; mtl_index < num_mtls; ++ mtl_index)
		{
			RenderMaterialPtr mtl = MakeSharedPtr<RenderMaterial>();
			mtls[mtl_index] = mtl;

			decoded->read(&mtl->ambient.x(), sizeof(float));
			decoded->read(&mtl->ambient.y(), sizeof(float));
			decoded->read(&mtl->ambient.z(), sizeof(float));
			decoded->read(&mtl->diffuse.x(), sizeof(float));
			decoded->read(&mtl->diffuse.y(), sizeof(float));
			decoded->read(&mtl->diffuse.z(), sizeof(float));
			decoded->read(&mtl->specular.x(), sizeof(float));
			decoded->read(&mtl->specular.y(), sizeof(float));
			decoded->read(&mtl->specular.z(), sizeof(float));
			decoded->read(&mtl->emit.x(), sizeof(float));
			decoded->read(&mtl->emit.y(), sizeof(float));
			decoded->read(&mtl->emit.z(), sizeof(float));
			decoded->read(&mtl->opacity, sizeof(float));
			decoded->read(&mtl->specular_level, sizeof(float));
			decoded->read(&mtl->shininess, sizeof(float));

			LittleEndianToNative<sizeof(mtl->ambient[0])>(&mtl->ambient[0]);
			LittleEndianToNative<sizeof(mtl->ambient[1])>(&mtl->ambient[1]);
			LittleEndianToNative<sizeof(mtl->ambient[2])>(&mtl->ambient[2]);
			LittleEndianToNative<sizeof(mtl->diffuse[0])>(&mtl->diffuse[0]);
			LittleEndianToNative<sizeof(mtl->diffuse[1])>(&mtl->diffuse[1]);
			LittleEndianToNative<sizeof(mtl->diffuse[2])>(&mtl->diffuse[2]);
			LittleEndianToNative<sizeof(mtl->specular[0])>(&mtl->specular[0]);
			LittleEndianToNative<sizeof(mtl->specular[1])>(&mtl->specular[1]);
			LittleEndianToNative<sizeof(mtl->specular[2])>(&mtl->specular[2]);
			LittleEndianToNative<sizeof(mtl->emit[0])>(&mtl->emit[0]);
			LittleEndianToNative<sizeof(mtl->emit[1])>(&mtl->emit[1]);
			LittleEndianToNative<sizeof(mtl->emit[2])>(&mtl->emit[2]);
			LittleEndianToNative<sizeof(mtl->opacity)>(&mtl->opacity);
			LittleEndianToNative<sizeof(mtl->specular_level)>(&mtl->specular_level);
			LittleEndianToNative<sizeof(mtl->shininess)>(&mtl->shininess);

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
			LittleEndianToNative<sizeof(num_texs)>(&num_texs);

			for (uint32_t tex_index = 0; tex_index < num_texs; ++ tex_index)
			{
				std::string type, name;
				ReadShortString(decoded, type);
				ReadShortString(decoded, name);
				mtl->texture_slots.push_back(std::make_pair(type, name));
			}
		}

		uint32_t num_merged_ves;
		decoded->read(&num_merged_ves, sizeof(num_merged_ves));
		LittleEndianToNative<sizeof(num_merged_ves)>(&num_merged_ves);
		merged_ves.resize(num_merged_ves);
		for (size_t i = 0; i < merged_ves.size(); ++ i)
		{
			decoded->read(&merged_ves[i], sizeof(merged_ves[i]));

			LittleEndianToNative<sizeof(merged_ves[i].usage)>(&merged_ves[i].usage);
			LittleEndianToNative<sizeof(merged_ves[i].format)>(&merged_ves[i].format);
		}

		uint32_t all_num_vertices;
		uint32_t all_num_indices;
		decoded->read(&all_num_vertices, sizeof(all_num_vertices));
		LittleEndianToNative<sizeof(all_num_vertices)>(&all_num_vertices);
		decoded->read(&all_num_indices, sizeof(all_num_indices));
		LittleEndianToNative<sizeof(all_num_indices)>(&all_num_indices);
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
			ReadShortString(decoded, mesh_names[mesh_index]);

			decoded->read(&mtl_ids[mesh_index], sizeof(mtl_ids[mesh_index]));
			LittleEndianToNative<sizeof(mtl_ids[mesh_index])>(&mtl_ids[mesh_index]);

			float3 min_bb, max_bb;
			decoded->read(&min_bb, sizeof(min_bb));
			LittleEndianToNative<sizeof(min_bb.x())>(&min_bb.x());
			LittleEndianToNative<sizeof(min_bb.y())>(&min_bb.y());
			LittleEndianToNative<sizeof(min_bb.z())>(&min_bb.z());
			decoded->read(&max_bb, sizeof(max_bb));
			LittleEndianToNative<sizeof(max_bb.x())>(&max_bb.x());
			LittleEndianToNative<sizeof(max_bb.y())>(&max_bb.y());
			LittleEndianToNative<sizeof(max_bb.z())>(&max_bb.z());
			pos_bbs[mesh_index] = AABBox(min_bb, max_bb);

			decoded->read(&min_bb[0], sizeof(min_bb[0]));
			decoded->read(&min_bb[1], sizeof(min_bb[1]));
			LittleEndianToNative<sizeof(min_bb.x())>(&min_bb.x());
			LittleEndianToNative<sizeof(min_bb.y())>(&min_bb.y());
			min_bb.z() = 0;
			decoded->read(&max_bb[0], sizeof(max_bb[0]));
			decoded->read(&max_bb[1], sizeof(max_bb[1]));
			LittleEndianToNative<sizeof(max_bb.x())>(&max_bb.x());
			LittleEndianToNative<sizeof(max_bb.y())>(&max_bb.y());
			max_bb.z() = 0;
			tc_bbs[mesh_index] = AABBox(min_bb, max_bb);

			decoded->read(&mesh_num_vertices[mesh_index], sizeof(mesh_num_vertices[mesh_index]));
			LittleEndianToNative<sizeof(mesh_num_vertices[mesh_index])>(&mesh_num_vertices[mesh_index]);
			decoded->read(&mesh_base_vertices[mesh_index], sizeof(mesh_base_vertices[mesh_index]));
			LittleEndianToNative<sizeof(mesh_base_vertices[mesh_index])>(&mesh_base_vertices[mesh_index]);
			decoded->read(&mesh_num_triangles[mesh_index], sizeof(mesh_num_triangles[mesh_index]));
			LittleEndianToNative<sizeof(mesh_num_triangles[mesh_index])>(&mesh_num_triangles[mesh_index]);
			decoded->read(&mesh_base_triangles[mesh_index], sizeof(mesh_base_triangles[mesh_index]));
			LittleEndianToNative<sizeof(mesh_base_triangles[mesh_index])>(&mesh_base_triangles[mesh_index]);
		}

		joints.resize(num_joints);
		for (uint32_t joint_index = 0; joint_index < num_joints; ++ joint_index)
		{
			Joint& joint = joints[joint_index];

			ReadShortString(decoded, joint.name);
			decoded->read(&joint.parent, sizeof(joint.parent));
			LittleEndianToNative<sizeof(joint.parent)>(&joint.parent);

			decoded->read(&joint.bind_real, sizeof(joint.bind_real));
			LittleEndianToNative<sizeof(joint.bind_real[0])>(&joint.bind_real[0]);
			LittleEndianToNative<sizeof(joint.bind_real[1])>(&joint.bind_real[1]);
			LittleEndianToNative<sizeof(joint.bind_real[2])>(&joint.bind_real[2]);
			LittleEndianToNative<sizeof(joint.bind_real[3])>(&joint.bind_real[3]);
			decoded->read(&joint.bind_dual, sizeof(joint.bind_dual));
			LittleEndianToNative<sizeof(joint.bind_dual[0])>(&joint.bind_dual[0]);
			LittleEndianToNative<sizeof(joint.bind_dual[1])>(&joint.bind_dual[1]);
			LittleEndianToNative<sizeof(joint.bind_dual[2])>(&joint.bind_dual[2]);
			LittleEndianToNative<sizeof(joint.bind_dual[3])>(&joint.bind_dual[3]);

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
			LittleEndianToNative<sizeof(num_frames)>(&num_frames);
			decoded->read(&frame_rate, sizeof(frame_rate));
			LittleEndianToNative<sizeof(frame_rate)>(&frame_rate);

			kfs = MakeSharedPtr<KeyFramesType>(joints.size());
			for (uint32_t kf_index = 0; kf_index < num_kfs; ++ kf_index)
			{
				uint32_t joint_index = kf_index;

				uint32_t num_kf;
				decoded->read(&num_kf, sizeof(num_kf));
				LittleEndianToNative<sizeof(num_kf)>(&num_kf);

				KeyFrames kf;
				kf.frame_id.resize(num_kf);
				kf.bind_real.resize(num_kf);
				kf.bind_dual.resize(num_kf);
				kf.bind_scale.resize(num_kf);
				for (uint32_t k_index = 0; k_index < num_kf; ++ k_index)
				{
					decoded->read(&kf.frame_id[k_index], sizeof(kf.frame_id[k_index]));
					LittleEndianToNative<sizeof(kf.frame_id[k_index])>(&kf.frame_id[k_index]);
					decoded->read(&kf.bind_real[k_index], sizeof(kf.bind_real[k_index]));
					LittleEndianToNative<sizeof(kf.bind_real[k_index][0])>(&kf.bind_real[k_index][0]);
					LittleEndianToNative<sizeof(kf.bind_real[k_index][1])>(&kf.bind_real[k_index][1]);
					LittleEndianToNative<sizeof(kf.bind_real[k_index][2])>(&kf.bind_real[k_index][2]);
					LittleEndianToNative<sizeof(kf.bind_real[k_index][3])>(&kf.bind_real[k_index][3]);
					decoded->read(&kf.bind_dual[k_index], sizeof(kf.bind_dual[k_index]));
					LittleEndianToNative<sizeof(kf.bind_dual[k_index][0])>(&kf.bind_dual[k_index][0]);
					LittleEndianToNative<sizeof(kf.bind_dual[k_index][1])>(&kf.bind_dual[k_index][1]);
					LittleEndianToNative<sizeof(kf.bind_dual[k_index][2])>(&kf.bind_dual[k_index][2]);
					LittleEndianToNative<sizeof(kf.bind_dual[k_index][3])>(&kf.bind_dual[k_index][3]);

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
				LittleEndianToNative<sizeof(num_bb_kf)>(&num_bb_kf);

				frame_pos_bbs[mesh_index] = MakeSharedPtr<AABBKeyFrames>();
				frame_pos_bbs[mesh_index]->frame_id.resize(num_bb_kf);
				frame_pos_bbs[mesh_index]->bb.resize(num_bb_kf);

				for (uint32_t bb_k_index = 0; bb_k_index < num_bb_kf; ++ bb_k_index)
				{
					decoded->read(&frame_pos_bbs[mesh_index]->frame_id[bb_k_index], sizeof(frame_pos_bbs[mesh_index]->frame_id[bb_k_index]));
					LittleEndianToNative<sizeof(frame_pos_bbs[mesh_index]->frame_id[bb_k_index])>(&frame_pos_bbs[mesh_index]->frame_id[bb_k_index]);

					float3 bb_min, bb_max;
					decoded->read(&bb_min, sizeof(bb_min));
					LittleEndianToNative<sizeof(bb_min[0])>(&bb_min[0]);
					LittleEndianToNative<sizeof(bb_min[1])>(&bb_min[1]);
					LittleEndianToNative<sizeof(bb_min[2])>(&bb_min[2]);
					decoded->read(&bb_max, sizeof(bb_max));
					LittleEndianToNative<sizeof(bb_max[0])>(&bb_max[0]);
					LittleEndianToNative<sizeof(bb_max[1])>(&bb_max[1]);
					LittleEndianToNative<sizeof(bb_max[2])>(&bb_max[2]);
					frame_pos_bbs[mesh_index]->bb[bb_k_index] = AABBox(bb_min, bb_max);
				}
			}
			
			if (num_actions > 0)
			{
				actions = MakeSharedPtr<AnimationActionsType>(num_actions);
				for (uint32_t action_index = 0; action_index < num_actions; ++ action_index)
				{
					AnimationAction action;
					ReadShortString(decoded, action.name);
					decoded->read(&action.start_frame, sizeof(action.start_frame));
					LittleEndianToNative<sizeof(action.start_frame)>(&action.start_frame);
					decoded->read(&action.end_frame, sizeof(action.end_frame));
					LittleEndianToNative<sizeof(action.end_frame)>(&action.end_frame);
					actions->push_back(action);
				}
			}
		}
	}

	RenderModelPtr SyncLoadModel(std::string const & meshml_name, uint32_t access_hint,
		boost::function<RenderModelPtr(std::wstring const &)> CreateModelFactoryFunc,
		boost::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		return ResLoader::Instance().SyncQueryT<RenderModel>(MakeSharedPtr<RenderModelLoadingDesc>(meshml_name,
			access_hint, CreateModelFactoryFunc, CreateMeshFactoryFunc));
	}

	boost::function<RenderModelPtr()> ASyncLoadModel(std::string const & meshml_name, uint32_t access_hint,
		boost::function<RenderModelPtr(std::wstring const &)> CreateModelFactoryFunc,
		boost::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		return ResLoader::Instance().ASyncQueryT<RenderModel>(MakeSharedPtr<RenderModelLoadingDesc>(meshml_name,
			access_hint, CreateModelFactoryFunc, CreateMeshFactoryFunc));
	}

	void SaveModel(std::string const & meshml_name, std::vector<RenderMaterialPtr> const & mtls,
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids,
		std::vector<AABBox> const & /*pos_bbs*/, std::vector<AABBox> const & /*tc_bbs*/,
		std::vector<std::vector<vertex_element> > const & ves,
		std::vector<std::vector<std::vector<uint8_t> > > const & buffs,
		std::vector<char> const & is_index_16_bit, std::vector<std::vector<uint8_t> > const & indices,
		std::vector<Joint> const & joints, boost::shared_ptr<AnimationActionsType> const & actions,
		boost::shared_ptr<KeyFramesType> const & kfs, uint32_t num_frames, uint32_t frame_rate,
		std::vector<boost::shared_ptr<AABBKeyFrames> > const & /*frame_pos_bbs*/)
	{
		XMLDocument doc;

		XMLNodePtr root = doc.AllocNode(XNT_Element, "model");
		doc.RootNode(root);
		root->AppendAttrib(doc.AllocAttribUInt("version", 6));

		if (kfs)
		{
			XMLNodePtr bones_chunk = doc.AllocNode(XNT_Element, "bones_chunk");
			root->AppendNode(bones_chunk);

			uint32_t num_joints = static_cast<uint32_t>(joints.size());
			for (uint32_t i = 0; i < num_joints; ++ i)
			{
				XMLNodePtr bone_node = doc.AllocNode(XNT_Element, "bone");
				bones_chunk->AppendNode(bone_node);

				Joint const & joint = joints[i];

				bone_node->AppendAttrib(doc.AllocAttribString("name", joint.name));
				bone_node->AppendAttrib(doc.AllocAttribInt("parent", joint.parent));

				Quaternion const & bind_real = joint.bind_real * MathLib::abs(joint.bind_scale);
				Quaternion const & bind_dual = joint.bind_dual;

				XMLNodePtr bind_real_node = doc.AllocNode(XNT_Element, "bind_real");
				bone_node->AppendNode(bind_real_node);
				bind_real_node->AppendAttrib(doc.AllocAttribFloat("x", bind_real.x()));
				bind_real_node->AppendAttrib(doc.AllocAttribFloat("y", bind_real.y()));
				bind_real_node->AppendAttrib(doc.AllocAttribFloat("z", bind_real.z()));
				bind_real_node->AppendAttrib(doc.AllocAttribFloat("w", bind_real.w()));

				XMLNodePtr bind_dual_node = doc.AllocNode(XNT_Element, "bind_dual");
				bone_node->AppendNode(bind_dual_node);
				bind_dual_node->AppendAttrib(doc.AllocAttribFloat("x", bind_dual.x()));
				bind_dual_node->AppendAttrib(doc.AllocAttribFloat("y", bind_dual.y()));
				bind_dual_node->AppendAttrib(doc.AllocAttribFloat("z", bind_dual.z()));
				bind_dual_node->AppendAttrib(doc.AllocAttribFloat("w", bind_dual.w()));
			}
		}

		if (!mtls.empty())
		{
			XMLNodePtr materials_chunk = doc.AllocNode(XNT_Element, "materials_chunk");
			root->AppendNode(materials_chunk);

			for (uint32_t i = 0; i < mtls.size(); ++ i)
			{
				RenderMaterialPtr const & mtl = mtls[i];

				XMLNodePtr mtl_node = doc.AllocNode(XNT_Element, "material");
				materials_chunk->AppendNode(mtl_node);

				float3 ambient, diffuse;
				if (Context::Instance().Config().graphics_cfg.gamma)
				{
					ambient.x() = MathLib::linear_to_srgb(mtl->ambient.x());
					ambient.y() = MathLib::linear_to_srgb(mtl->ambient.y());
					ambient.z() = MathLib::linear_to_srgb(mtl->ambient.z());
					diffuse.x() = MathLib::linear_to_srgb(mtl->diffuse.x());
					diffuse.y() = MathLib::linear_to_srgb(mtl->diffuse.y());
					diffuse.z() = MathLib::linear_to_srgb(mtl->diffuse.z());
				}
				else
				{
					ambient = mtl->ambient;
					diffuse = mtl->diffuse;
				}

				mtl_node->AppendAttrib(doc.AllocAttribFloat("ambient_r", ambient.x()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("ambient_g", ambient.y()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("ambient_b", ambient.z()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("diffuse_r", diffuse.x()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("diffuse_g", diffuse.y()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("diffuse_b", diffuse.z()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("specular_r", mtl->specular.x()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("specular_g", mtl->specular.y()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("specular_b", mtl->specular.z()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("emit_r", mtl->emit.x()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("emit_g", mtl->emit.y()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("emit_b", mtl->emit.z()));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("opacity", mtl->opacity));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("specular_level", mtl->specular_level));
				mtl_node->AppendAttrib(doc.AllocAttribFloat("shininess", mtl->shininess));

				uint32_t const num_textures = static_cast<uint32_t const>(mtl->texture_slots.size());
				if (num_textures > 0)
				{
					XMLNodePtr textures_chunk = doc.AllocNode(XNT_Element, "textures_chunk");
					mtl_node->AppendNode(textures_chunk);

					for (uint8_t j = 0; j < num_textures; ++ j)
					{
						XMLNodePtr texture_node = doc.AllocNode(XNT_Element, "texture");
						textures_chunk->AppendNode(texture_node);

						texture_node->AppendAttrib(doc.AllocAttribString("type", mtl->texture_slots[j].first));
						texture_node->AppendAttrib(doc.AllocAttribString("name", mtl->texture_slots[j].second));
					}
				}
			}
		}

		if (!mesh_names.empty())
		{
			XMLNodePtr meshes_chunk = doc.AllocNode(XNT_Element, "meshes_chunk");
			root->AppendNode(meshes_chunk);

			for (uint32_t mesh_index = 0; mesh_index < static_cast<uint32_t>(mesh_names.size()); ++ mesh_index)
			{
				XMLNodePtr mesh_node = doc.AllocNode(XNT_Element, "mesh");
				meshes_chunk->AppendNode(mesh_node);

				mesh_node->AppendAttrib(doc.AllocAttribString("name", mesh_names[mesh_index]));
				mesh_node->AppendAttrib(doc.AllocAttribInt("mtl_id", mtl_ids[mesh_index]));

				XMLNodePtr vertex_elements_chunk = doc.AllocNode(XNT_Element, "vertex_elements_chunk");
				mesh_node->AppendNode(vertex_elements_chunk);

				uint32_t num_vertices = 0;
				for (uint32_t j = 0; j < ves[mesh_index].size(); ++ j)
				{
					vertex_element const & ve = ves[mesh_index][j];

					XMLNodePtr ve_node = doc.AllocNode(XNT_Element, "vertex_element");
					vertex_elements_chunk->AppendNode(ve_node);

					ve_node->AppendAttrib(doc.AllocAttribUInt("usage", static_cast<uint32_t>(ve.usage)));
					ve_node->AppendAttrib(doc.AllocAttribUInt("usage_index", ve.usage_index));
					ve_node->AppendAttrib(doc.AllocAttribUInt("num_components", NumComponents(ve.format)));

					num_vertices = static_cast<uint32_t>(buffs[mesh_index][j].size() / ve.element_size());
				}

				XMLNodePtr vertices_chunk = doc.AllocNode(XNT_Element, "vertices_chunk");
				mesh_node->AppendNode(vertices_chunk);

				for (uint32_t j = 0; j < num_vertices; ++ j)
				{
					XMLNodePtr vertex_node = doc.AllocNode(XNT_Element, "vertex");
					vertices_chunk->AppendNode(vertex_node);

					for (size_t k = 0; k < ves[mesh_index].size(); ++ k)
					{
						vertex_element const & ve = ves[mesh_index][k];

						switch (ve.usage)
						{
						case VEU_Position:
							{
								float3 const * p = reinterpret_cast<float3 const *>(&buffs[mesh_index][k][0]);
								vertex_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
								vertex_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
								vertex_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
							}
							break;

						case VEU_Normal:
							{
								XMLNodePtr sub_node = doc.AllocNode(XNT_Element, "normal");
								vertex_node->AppendNode(sub_node);

								if (EF_A2BGR10 == ve.format)
								{
									uint32_t const * p = reinterpret_cast<uint32_t const *>(&buffs[mesh_index][k][0]);
									sub_node->AppendAttrib(doc.AllocAttribFloat("x", ((p[j] >>  0) & 0x3FF) / 1023.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("y", ((p[j] >> 10) & 0x3FF) / 1023.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("z", ((p[j] >> 20) & 0x3FF) / 1023.0f * 2 - 1));
								}
								else if (EF_ABGR8 == ve.format)
								{
									uint32_t const * p = reinterpret_cast<uint32_t const *>(&buffs[mesh_index][k][0]);
									sub_node->AppendAttrib(doc.AllocAttribFloat("x", ((p[j] >>  0) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("y", ((p[j] >>  8) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("z", ((p[j] >> 16) & 0xFF) / 255.0f * 2 - 1));
								}
								else
								{
									BOOST_ASSERT(EF_ARGB8 == ve.format);

									uint32_t const * p = reinterpret_cast<uint32_t const *>(&buffs[mesh_index][k][0]);
									sub_node->AppendAttrib(doc.AllocAttribFloat("x", ((p[j] >> 16) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("y", ((p[j] >>  8) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("z", ((p[j] >>  0) & 0xFF) / 255.0f * 2 - 1));
								}
							}
							break;

						case VEU_Tangent:
							{
								XMLNodePtr sub_node = doc.AllocNode(XNT_Element, "tangent_quat");
								vertex_node->AppendNode(sub_node);

								if (EF_ABGR8 == ve.format)
								{
									uint32_t const * p = reinterpret_cast<uint32_t const *>(&buffs[mesh_index][k][0]);
									sub_node->AppendAttrib(doc.AllocAttribFloat("x", ((p[j] >>  0) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("y", ((p[j] >>  8) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("z", ((p[j] >> 16) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("w", ((p[j] >> 24) & 0xFF) / 255.0f * 2 - 1));
								}
								else
								{
									BOOST_ASSERT(EF_ARGB8 == ve.format);

									uint32_t const * p = reinterpret_cast<uint32_t const *>(&buffs[mesh_index][k][0]);
									sub_node->AppendAttrib(doc.AllocAttribFloat("x", ((p[j] >> 16) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("y", ((p[j] >>  8) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("z", ((p[j] >>  0) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("w", ((p[j] >> 24) & 0xFF) / 255.0f * 2 - 1));
								}
							}
							break;
							
						case VEU_Binormal:
							break;

						case VEU_Diffuse:
						case VEU_Specular:
							{
								std::string node_name;
								switch (ve.usage)
								{
								case VEU_Diffuse:
									node_name = "diffuse";
									break;

								default:
									node_name = "specular";
									break;
								}

								XMLNodePtr sub_node = doc.AllocNode(XNT_Element, node_name);
								vertex_node->AppendNode(sub_node);

								float4 const * p = reinterpret_cast<float4 const *>(&buffs[mesh_index][k][0]);
								sub_node->AppendAttrib(doc.AllocAttribFloat("r", p[j].x()));
								sub_node->AppendAttrib(doc.AllocAttribFloat("g", p[j].y()));
								sub_node->AppendAttrib(doc.AllocAttribFloat("b", p[j].z()));
								sub_node->AppendAttrib(doc.AllocAttribFloat("a", p[j].w()));
							}
							break;

						case VEU_BlendIndex:
							{
								int weight_stream = -1;
								for (uint32_t l = 0; l < ves[mesh_index].size(); ++ l)
								{
									vertex_element const & other_ve = ves[mesh_index][l];
									if (VEU_BlendWeight == other_ve.usage)
									{
										weight_stream = l;
										break;
									}
								}

								uint32_t num_components = NumComponents(ve.format);
								for (uint32_t c = 0; c < num_components; ++ c)
								{
									XMLNodePtr weight_node = doc.AllocNode(XNT_Element, "weight");
									vertex_node->AppendNode(weight_node);

									uint8_t const * bone_indices = &buffs[mesh_index][k][0];
									weight_node->AppendAttrib(doc.AllocAttribFloat("bone_index", bone_indices[j * num_components + c]));

									if (weight_stream != -1)
									{
										uint8_t const * bone_weights = &buffs[mesh_index][weight_stream][0];
										weight_node->AppendAttrib(doc.AllocAttribFloat("weight", bone_weights[j * num_components + c] / 255.0f));
									}
								}
							}
							break;

						case VEU_BlendWeight:
							break;

						case VEU_TextureCoord:
							{
								XMLNodePtr tex_coord_node = doc.AllocNode(XNT_Element, "tex_coord");
								vertex_node->AppendNode(tex_coord_node);
								if (ve.usage_index != 0)
								{
									tex_coord_node->AppendAttrib(doc.AllocAttribInt("usage_index", ve.usage_index));
								}

								float const * p = reinterpret_cast<float const *>(&buffs[mesh_index][k][0]);
								uint32_t num_components = NumComponents(ve.format);
								if (num_components >= 1)
								{
									tex_coord_node->AppendAttrib(doc.AllocAttribFloat("u", p[j * num_components + 0]));
								}
								if (num_components >= 2)
								{
									tex_coord_node->AppendAttrib(doc.AllocAttribFloat("v", p[j * num_components + 1]));
								}
								if (num_components >= 3)
								{
									tex_coord_node->AppendAttrib(doc.AllocAttribFloat("w", p[j * num_components + 2]));
								}
							}
							break;
						}
					}
				}

				XMLNodePtr triangles_chunk = doc.AllocNode(XNT_Element, "triangles_chunk");
				mesh_node->AppendNode(triangles_chunk);
				{
					if (is_index_16_bit[mesh_index])
					{
						size_t size = indices[mesh_index].size() / 2;
						uint16_t const * indices_16 = reinterpret_cast<uint16_t const *>(&indices[mesh_index][0]);
						for (size_t j = 0; j < size; j += 3)
						{
							XMLNodePtr triangle_node = doc.AllocNode(XNT_Element, "triangle");
							triangles_chunk->AppendNode(triangle_node);

							triangle_node->AppendAttrib(doc.AllocAttribInt("a", indices_16[j + 0]));
							triangle_node->AppendAttrib(doc.AllocAttribInt("b", indices_16[j + 1]));
							triangle_node->AppendAttrib(doc.AllocAttribInt("c", indices_16[j + 2]));
						}
					}
					else
					{
						size_t size = indices[mesh_index].size() / 4;
						uint32_t const * indices_32 = reinterpret_cast<uint32_t const *>(&indices[mesh_index][0]);
						for (size_t j = 0; j < size; j += 3)
						{
							XMLNodePtr triangle_node = doc.AllocNode(XNT_Element, "triangle");
							triangles_chunk->AppendNode(triangle_node);

							triangle_node->AppendAttrib(doc.AllocAttribInt("a", indices_32[j + 0]));
							triangle_node->AppendAttrib(doc.AllocAttribInt("b", indices_32[j + 1]));
							triangle_node->AppendAttrib(doc.AllocAttribInt("c", indices_32[j + 2]));
						}
					}
				}
			}
		}

		if (actions)
		{
			XMLNodePtr actions_chunk = doc.AllocNode(XNT_Element, "actions_chunk");
			root->AppendNode(actions_chunk);

			for (AnimationActionsType::const_iterator iter = actions->begin(); iter != actions->end(); ++ iter)
			{
				XMLNodePtr action_node = doc.AllocNode(XNT_Element, "action");
				action_node->AppendNode(action_node);

				action_node->AppendAttrib(doc.AllocAttribString("name", iter->name));
				action_node->AppendAttrib(doc.AllocAttribUInt("start_frame", iter->start_frame));
				action_node->AppendAttrib(doc.AllocAttribUInt("end_frame", iter->end_frame));
			}
		}

		if (kfs)
		{
			if (!kfs->empty())
			{
				uint32_t num_key_frames = static_cast<uint32_t>(kfs->size());

				XMLNodePtr key_frames_chunk = doc.AllocNode(XNT_Element, "key_frames_chunk");
				root->AppendNode(key_frames_chunk);

				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("num_frame", num_frames));
				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("frame_rate", frame_rate));

				KeyFramesType::const_iterator iter = kfs->begin();
				for (uint32_t i = 0; i < num_key_frames; ++ i, ++ iter)
				{
					XMLNodePtr key_frame_node = doc.AllocNode(XNT_Element, "key_frame");
					key_frames_chunk->AppendNode(key_frame_node);

					key_frame_node->AppendAttrib(doc.AllocAttribString("joint", joints[i].name));

					for (size_t j = 0; j < iter->bind_real.size(); ++ j)
					{
						XMLNodePtr key_node = doc.AllocNode(XNT_Element, "key");
						key_frame_node->AppendNode(key_node);

						key_frame_node->AppendAttrib(doc.AllocAttribUInt("id", iter->frame_id[j]));

						Quaternion const & bind_real = iter->bind_real[j] * MathLib::abs(iter->bind_scale[j]);
						Quaternion const & bind_dual = iter->bind_dual[j];

						XMLNodePtr bind_real_node = doc.AllocNode(XNT_Element, "bind_real");
						key_node->AppendNode(bind_real_node); 
						bind_real_node->AppendAttrib(doc.AllocAttribFloat("x", bind_real.x()));
						bind_real_node->AppendAttrib(doc.AllocAttribFloat("y", bind_real.y()));
						bind_real_node->AppendAttrib(doc.AllocAttribFloat("z", bind_real.z()));
						bind_real_node->AppendAttrib(doc.AllocAttribFloat("w", bind_real.w()));

						XMLNodePtr bind_dual_node = doc.AllocNode(XNT_Element, "bind_dual");
						key_node->AppendNode(bind_dual_node);
						bind_dual_node->AppendAttrib(doc.AllocAttribFloat("x", bind_dual.x()));
						bind_dual_node->AppendAttrib(doc.AllocAttribFloat("y", bind_dual.y()));
						bind_dual_node->AppendAttrib(doc.AllocAttribFloat("z", bind_dual.z()));
						bind_dual_node->AppendAttrib(doc.AllocAttribFloat("w", bind_dual.w()));
					}
				}
			}
		}

		std::ofstream file(meshml_name.c_str());
		doc.Print(file);
	}

	void SaveModel(RenderModelPtr const & model, std::string const & meshml_name)
	{
		std::vector<RenderMaterialPtr> mtls(model->NumMaterials());
		if (!mtls.empty())
		{
			for (uint32_t i = 0; i < mtls.size(); ++ i)
			{
				mtls[i] = model->GetMaterial(i);
			}
		}

		std::vector<std::string> mesh_names(model->NumMeshes());
		std::vector<int32_t> mtl_ids(mesh_names.size());
		std::vector<AABBox> pos_bbs(mesh_names.size());
		std::vector<AABBox> tc_bbs(mesh_names.size());
		std::vector<std::vector<vertex_element> > ves(mesh_names.size());
		std::vector<std::vector<std::vector<uint8_t> > > buffs(mesh_names.size());
		std::vector<char> is_index_16_bit(mesh_names.size());
		std::vector<std::vector<uint8_t> > indices(mesh_names.size());
		if (!mesh_names.empty())
		{
			for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
			{
				StaticMesh const & mesh = *model->Mesh(mesh_index);

				Convert(mesh_names[mesh_index], mesh.Name());
				mtl_ids[mesh_index] = mesh.MaterialID();

				pos_bbs[mesh_index] = mesh.PosBound();
				tc_bbs[mesh_index] = mesh.TexcoordBound();

				RenderLayoutPtr const & rl = mesh.GetRenderLayout();
				ves[mesh_index].resize(rl->NumVertexStreams());
				for (uint32_t j = 0; j < rl->NumVertexStreams(); ++ j)
				{
					ves[mesh_index][j] = rl->VertexStreamFormat(j)[0];
				}

				buffs[mesh_index].resize(ves[mesh_index].size());
				for (uint32_t j = 0; j < rl->NumVertexStreams(); ++ j)
				{
					GraphicsBufferPtr const & vb = rl->GetVertexStream(j);
					GraphicsBufferPtr vb_cpu = Context::Instance().RenderFactoryInstance().MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
					vb_cpu->Resize(vb->Size());
					vb->CopyToBuffer(*vb_cpu);

					buffs[mesh_index][j].resize(vb->Size());

					GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
					memcpy(&buffs[mesh_index][j][0], mapper.Pointer<uint8_t>(), vb->Size());
				}

				if (EF_R16UI == rl->IndexStreamFormat())
				{
					is_index_16_bit[mesh_index] = true;
				}
				else
				{
					BOOST_ASSERT(EF_R32UI == rl->IndexStreamFormat());
					is_index_16_bit[mesh_index] = false;
				}

				indices[mesh_index].resize((is_index_16_bit[mesh_index] ? 2 : 4) * mesh.NumTriangles() * 3);
				{
					GraphicsBufferPtr ib = rl->GetIndexStream();
					GraphicsBufferPtr ib_cpu = Context::Instance().RenderFactoryInstance().MakeIndexBuffer(BU_Static, EAH_CPU_Read, NULL);
					ib_cpu->Resize(ib->Size());
					ib->CopyToBuffer(*ib_cpu);

					GraphicsBuffer::Mapper mapper(*ib_cpu, BA_Read_Only);
					memcpy(&indices[mesh_index][0], mapper.Pointer<uint8_t>(), ib->Size());
				}
			}
		}

		std::vector<Joint> joints;
		boost::shared_ptr<AnimationActionsType> actions;
		boost::shared_ptr<KeyFramesType> kfs;
		uint32_t num_frame = 0;
		uint32_t frame_rate = 0;
		std::vector<boost::shared_ptr<AABBKeyFrames> > frame_pos_bbs;
		if (model->IsSkinned())
		{
			SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(model);

			uint32_t num_joints = skinned->NumJoints();
			joints.resize(num_joints);
			for (uint32_t i = 0; i < num_joints; ++ i)
			{
				joints[i] = skinned->GetJoint(i);
			}

			actions = skinned->GetActions();

			num_frame = skinned->NumFrames();
			frame_rate = skinned->FrameRate();

			kfs = skinned->GetKeyFrames();

			frame_pos_bbs.resize(model->NumMeshes());
			for (size_t i = 0; i < frame_pos_bbs.size(); ++ i)
			{
				frame_pos_bbs[i] = checked_pointer_cast<SkinnedMesh>(skinned->Mesh(i))->GetFramePosBounds();
			}
		}

		SaveModel(meshml_name, mtls, mesh_names, mtl_ids, pos_bbs, tc_bbs, ves, buffs, is_index_16_bit, indices,
			joints, actions, kfs, num_frame, frame_rate, frame_pos_bbs);
	}


	RenderableLightSourceProxy::RenderableLightSourceProxy(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
	{
		RenderEffectPtr effect = Context::Instance().RenderFactoryInstance().LoadEffect("LightSourceProxy.fxml");

		if (deferred_effect_)
		{
			this->BindDeferredEffect(effect);
			special_shading_tech_ = effect->TechniqueByName("LightSourceProxySpecialShadingTech");
		}

		this->Technique(effect->TechniqueByName("LightSourceProxy"));
	}

	void RenderableLightSourceProxy::Technique(RenderTechniquePtr const & tech)
	{
		technique_ = tech;
		if (tech)
		{
			mvp_param_ = technique_->Effect().ParameterByName("mvp");
			model_param_ = technique_->Effect().ParameterByName("model");
			pos_center_param_ = technique_->Effect().ParameterByName("pos_center");
			pos_extent_param_ = technique_->Effect().ParameterByName("pos_extent");
			tc_center_param_ = technique_->Effect().ParameterByName("tc_center");
			tc_extent_param_ = technique_->Effect().ParameterByName("tc_extent");

			light_color_param_ = technique_->Effect().ParameterByName("light_color");
			light_falloff_param_ = technique_->Effect().ParameterByName("light_falloff");
			light_is_projective_param_ = technique_->Effect().ParameterByName("light_is_projective");
			projective_map_tex_param_ = technique_->Effect().ParameterByName("projective_map_tex");
			projective_map_cube_tex_param_ = technique_->Effect().ParameterByName("projective_map_cube_tex");
		}
	}

	void RenderableLightSourceProxy::Update()
	{
		if (light_color_param_)
		{
			*light_color_param_ = light_->Color();
		}
		if (light_falloff_param_)
		{
			*light_falloff_param_ = light_->Falloff();
		}
		if (light_is_projective_param_)
		{
			*light_is_projective_param_ = int2(light_->ProjectiveTexture() ? 1 : 0, LT_Point == light_->Type());
		}
		if (LT_Point == light_->Type())
		{
			if (projective_map_cube_tex_param_)
			{
				*projective_map_cube_tex_param_ = light_->ProjectiveTexture();
			}
		}
		else
		{
			if (projective_map_tex_param_)
			{
				*projective_map_tex_param_ = light_->ProjectiveTexture();
			}
		}
	}

	void RenderableLightSourceProxy::OnRenderBegin()
	{
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
}
