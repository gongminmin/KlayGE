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
#include <KlayGE/thread.hpp>
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

	uint32_t const MODEL_BIN_VERSION = 5;

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
			std::vector<Box> bbs;
			std::vector<uint32_t> mesh_num_vertices;
			std::vector<uint32_t> mesh_base_vertices;
			std::vector<uint32_t> mesh_num_indices;
			std::vector<uint32_t> mesh_start_indices;
			std::vector<Joint> joints;
			boost::shared_ptr<KeyFramesType> kfs;
			int32_t start_frame;
			int32_t end_frame;
			int32_t frame_rate;

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
				model_desc_.mesh_names, model_desc_.mtl_ids, model_desc_.bbs, 
				model_desc_.mesh_num_vertices, model_desc_.mesh_base_vertices, model_desc_.mesh_num_indices, model_desc_.mesh_start_indices, 
				model_desc_.joints, model_desc_.kfs,
				model_desc_.start_frame, model_desc_.end_frame, model_desc_.frame_rate);

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
				mesh->SetBound(model_desc_.bbs[mesh_index]);

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
					SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(model);

					skinned->AssignJoints(model_desc_.joints.begin(), model_desc_.joints.end());
					skinned->AttachKeyFrames(model_desc_.kfs);

					skinned->StartFrame(model_desc_.start_frame);
					skinned->EndFrame(model_desc_.end_frame);
					skinned->FrameRate(model_desc_.frame_rate);
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

	void RenderModel::UpdateBoundBox()
	{
		box_ = Box(float3(0, 0, 0), float3(0, 0, 0));
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			box_ |= mesh->GetBound();
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
	
	bool RenderModel::AlphaBlend() const
	{
		bool ab = false;
		typedef BOOST_TYPEOF(meshes_) MeshesType;
		BOOST_FOREACH(MeshesType::const_reference mesh, meshes_)
		{
			ab |= mesh->AlphaBlend();
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
		special_shading_ = false;
		need_alpha_blend_ = false;
		need_alpha_test_ = false;

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
						need_alpha_test_ = true;
					}
					else
					{
						need_alpha_blend_ = true;
					}
				}
			}				
		}
		
		if (!need_alpha_test_ && (mtl_->opacity < 1))
		{
			need_alpha_blend_ = true;
		}
		if ((mtl_->emit.x() > 0) || (mtl_->emit.y() > 0) || (mtl_->emit.z() > 0) || emit_tex_ || need_alpha_blend_)
		{
			special_shading_ = true;
		}
	}

	std::wstring const & StaticMesh::Name() const
	{
		return name_;
	}

	Box const & StaticMesh::GetBound() const
	{
		return box_;
	}

	void StaticMesh::SetBound(Box const & box)
	{
		box_ = box;
	}

	void StaticMesh::AddVertexStream(void const * buf, uint32_t size, vertex_element const & ve, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		if (VEU_Position == ve.usage)
		{
			switch (ve.format)
			{
			case EF_BGR32F:
				box_ = MathLib::compute_bounding_box<float>(static_cast<float3 const *>(buf),
					static_cast<float3 const *>(buf) + size / sizeof(float3));
				break;

			case EF_ABGR32F:
				box_ = MathLib::compute_bounding_box<float>(static_cast<float4 const *>(buf),
					static_cast<float4 const *>(buf) + size / sizeof(float4));
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

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


	std::pair<Quaternion, Quaternion> KeyFrames::Frame(float frame) const
	{
		frame = std::fmod(frame, static_cast<float>(frame_id.back() + 1));

		std::vector<uint32_t>::const_iterator iter = std::upper_bound(frame_id.begin(), frame_id.end(), frame);
		int index = iter - frame_id.begin();

		int index0 = index - 1;
		int index1 = index % frame_id.size();
		int frame0 = frame_id[index0];
		int frame1 = frame_id[index1];
		return MathLib::sclerp(bind_real[index0], bind_dual[index0], bind_real[index1], bind_dual[index1], (frame - frame0) / (frame1 - frame0));
	}


	SkinnedModel::SkinnedModel(std::wstring const & name)
		: RenderModel(name),
			last_frame_(-1),
			start_frame_(0), end_frame_(1), frame_rate_(0)
	{
	}
	
	void SkinnedModel::BuildBones(float frame)
	{
		typedef BOOST_TYPEOF(joints_) JointsType;
		BOOST_FOREACH(JointsType::reference joint, joints_)
		{
			KeyFrames const & kf = key_frames_->find(joint.name)->second;
			std::pair<Quaternion, Quaternion> key_dq = kf.Frame(frame);

			if (joint.parent != -1)
			{
				Joint const & parent(joints_[joint.parent]);

				if (MathLib::dot(key_dq.first, parent.bind_real) < 0)
				{
					key_dq.first = -key_dq.first;
					key_dq.second = -key_dq.second;
				}

				joint.bind_real = MathLib::mul_real(key_dq.first, parent.bind_real);
				joint.bind_dual = MathLib::mul_dual(key_dq.first, key_dq.second, parent.bind_real, parent.bind_dual);
			}
			else
			{
				joint.bind_real = key_dq.first;
				joint.bind_dual = key_dq.second;
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
			Quaternion real = MathLib::mul_real(joints_[i].inverse_origin_real, joints_[i].bind_real);
			bind_reals_[i] = float4(real.x(), real.y(), real.z(), real.w());

			Quaternion dual = MathLib::mul_dual(joints_[i].inverse_origin_real, joints_[i].inverse_origin_dual,
				joints_[i].bind_real, joints_[i].bind_dual);
			bind_duals_[i] = float4(dual.x(), dual.y(), dual.z(), dual.w());
		}
	}

	void SkinnedModel::AttachKeyFrames(boost::shared_ptr<KlayGE::KeyFramesType> const & key_frames)
	{
		key_frames_ = key_frames;
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


	SkinnedMesh::SkinnedMesh(RenderModelPtr const & model, std::wstring const & name)
		: StaticMesh(model, name)
	{
	}


	std::string const jit_ext_name = ".model_bin";

	void ReadShortString(ResIdentifierPtr& file, std::string& str)
	{
		uint8_t len;
		file->read(reinterpret_cast<char*>(&len), sizeof(len));
		str.resize(len);
		file->read(reinterpret_cast<char*>(&str[0]), len * sizeof(str[0]));
	}

	void WriteShortString(std::ostream& os, std::string const & str)
	{
		BOOST_ASSERT(str.size() < 256);

		uint8_t len = static_cast<uint8_t>(str.length());
		os.write(reinterpret_cast<char const *>(&len), sizeof(len));
		os.write(reinterpret_cast<char const *>(&str[0]), str.size() * sizeof(str[0]));
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
			uint32_t ver;
			lzma_file->read(&ver, sizeof(ver));
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

			XMLNodePtr materials_chunk = root->FirstNode("materials_chunk");
			if (materials_chunk)
			{
				uint32_t num_mtls = 0;
				for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"))
				{
					++ num_mtls;
				}
				ss->write(reinterpret_cast<char*>(&num_mtls), sizeof(num_mtls));
			}
			else
			{
				uint32_t num_mtls = 0;
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
				ss->write(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
			}
			else
			{
				uint32_t num_meshes = 0;
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
				ss->write(reinterpret_cast<char*>(&num_joints), sizeof(num_joints));
			}
			else
			{
				uint32_t num_joints = 0;
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
				ss->write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));
			}
			else
			{
				uint32_t num_kfs = 0;
				ss->write(reinterpret_cast<char*>(&num_kfs), sizeof(num_kfs));
			}

			if (materials_chunk)
			{
				uint32_t mtl_index = 0;
				for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node; mtl_node = mtl_node->NextSibling("material"), ++ mtl_index)
				{
					RenderMaterialPtr mtl = MakeSharedPtr<RenderMaterial>();
					float ambient_r = mtl_node->Attrib("ambient_r")->ValueFloat();
					float ambient_g = mtl_node->Attrib("ambient_g")->ValueFloat();
					float ambient_b = mtl_node->Attrib("ambient_b")->ValueFloat();
					float diffuse_r = mtl_node->Attrib("diffuse_r")->ValueFloat();
					float diffuse_g = mtl_node->Attrib("diffuse_g")->ValueFloat();
					float diffuse_b = mtl_node->Attrib("diffuse_b")->ValueFloat();
					float specular_r = mtl_node->Attrib("specular_r")->ValueFloat();
					float specular_g = mtl_node->Attrib("specular_g")->ValueFloat();
					float specular_b = mtl_node->Attrib("specular_b")->ValueFloat();
					float emit_r = mtl_node->Attrib("emit_r")->ValueFloat();
					float emit_g = mtl_node->Attrib("emit_g")->ValueFloat();
					float emit_b = mtl_node->Attrib("emit_b")->ValueFloat();
					float opacity = mtl_node->Attrib("opacity")->ValueFloat();
					float specular_level = mtl_node->Attrib("specular_level")->ValueFloat();
					float shininess = mtl_node->Attrib("shininess")->ValueFloat();

					ss->write(reinterpret_cast<char*>(&ambient_r), sizeof(ambient_r));
					ss->write(reinterpret_cast<char*>(&ambient_g), sizeof(ambient_g));
					ss->write(reinterpret_cast<char*>(&ambient_b), sizeof(ambient_b));
					ss->write(reinterpret_cast<char*>(&diffuse_r), sizeof(diffuse_r));
					ss->write(reinterpret_cast<char*>(&diffuse_g), sizeof(diffuse_g));
					ss->write(reinterpret_cast<char*>(&diffuse_b), sizeof(diffuse_b));
					ss->write(reinterpret_cast<char*>(&specular_r), sizeof(specular_r));
					ss->write(reinterpret_cast<char*>(&specular_g), sizeof(specular_g));
					ss->write(reinterpret_cast<char*>(&specular_b), sizeof(specular_b));
					ss->write(reinterpret_cast<char*>(&emit_r), sizeof(emit_r));
					ss->write(reinterpret_cast<char*>(&emit_g), sizeof(emit_g));
					ss->write(reinterpret_cast<char*>(&emit_b), sizeof(emit_b));
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
						ss->write(reinterpret_cast<char*>(&num_texs), sizeof(num_texs));
					}
				}
			}

			if (meshes_chunk)
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				ElementFormat tbn_format;
				if (rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_A2BGR10))
				{
					tbn_format = EF_A2BGR10;
				}
				else
				{
					BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ARGB8));

					tbn_format = EF_ARGB8;
				}

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

						bool has_position = false;
						bool has_normal = false;
						bool has_diffuse = false;
						bool has_specular = false;
						bool has_weight = false;
						std::vector<uint32_t> max_num_tc_components;
						bool has_tangent = false;
						bool has_binormal = false;

						uint32_t num_vertices = 0;
						for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
						{
							++ num_vertices;

							has_position = true;
						
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

							for (XMLNodePtr weight_node = vertex_node->FirstNode("weight"); weight_node; weight_node = weight_node->NextSibling("weight"))
							{
								has_weight = true;
							}

							uint32_t num_tex_coord = 0;
							for (XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord"); tex_coord_node; tex_coord_node = tex_coord_node->NextSibling("tex_coord"))
							{
								++ num_tex_coord;

								if (num_tex_coord >= max_num_tc_components.size())
								{
									max_num_tc_components.resize(num_tex_coord, 0);
								}

								uint32_t num_components = 0;
								if (tex_coord_node->Attrib("u"))
								{
									num_components = 1;
								}
								if (tex_coord_node->Attrib("v"))
								{
									num_components = 2;
								}
								if (tex_coord_node->Attrib("w"))
								{
									num_components = 3;
								}
								max_num_tc_components[num_tex_coord - 1] = std::max(max_num_tc_components[num_tex_coord - 1], num_components);
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
						}

						mesh_num_vertices.push_back(num_vertices);
						mesh_base_vertices.push_back(mesh_base_vertices.back() + num_vertices);

						ves.push_back(std::vector<vertex_element>());
						std::vector<vertex_element>& vertex_elements = ves.back();
						{
							vertex_element ve;

							{
								BOOST_ASSERT(has_position);

								ve.usage = VEU_Position;
								ve.usage_index = 0;
								ve.format = EF_BGR32F;
								vertex_elements.push_back(ve);
							}

							if (has_normal)
							{
								ve.usage = VEU_Normal;
								ve.usage_index = 0;
								ve.format = tbn_format;
								vertex_elements.push_back(ve);
							}

							if (has_diffuse)
							{
								ve.usage = VEU_Diffuse;
								ve.usage_index = 0;
								ve.format = EF_ABGR32F;
								vertex_elements.push_back(ve);
							}

							if (has_specular)
							{
								ve.usage = VEU_Specular;
								ve.usage_index = 0;
								ve.format = EF_ABGR32F;
								vertex_elements.push_back(ve);
							}

							if (has_weight)
							{
								ve.usage = VEU_BlendWeight;
								ve.usage_index = 0;
								ve.format = EF_ABGR32F;
								vertex_elements.push_back(ve);

								ve.usage = VEU_BlendIndex;
								ve.usage_index = 0;
								ve.format = EF_ABGR8UI;
								vertex_elements.push_back(ve);
							}

							for (uint32_t usage = 0; usage < max_num_tc_components.size(); ++ usage)
							{
								ve.usage = VEU_TextureCoord;
								ve.usage_index = static_cast<uint8_t>(usage);
								switch (max_num_tc_components[usage])
								{
								case 1:
									ve.format = EF_R32F;
									break;

								case 2:
									ve.format = EF_GR32F;
									break;

								case 3:
									ve.format = EF_BGR32F;
									break;

								default:
									ve.format = EF_ABGR32F;
									break;
								}
								vertex_elements.push_back(ve);
							}

							if (has_tangent)
							{
								ve.usage = VEU_Tangent;
								ve.usage_index = 0;
								ve.format = tbn_format;
								vertex_elements.push_back(ve);
							}

							if (has_binormal)
							{
								ve.usage = VEU_Binormal;
								ve.usage_index = 0;
								ve.format = tbn_format;
								vertex_elements.push_back(ve);
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
							uint32_t a = static_cast<uint32_t>(tri_node->Attrib("a")->ValueUInt());
							uint32_t b = static_cast<uint32_t>(tri_node->Attrib("b")->ValueUInt());
							uint32_t c = static_cast<uint32_t>(tri_node->Attrib("c")->ValueUInt());
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
				std::vector<Box> bounding_boxes;
				mesh_index = 0;
				for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"), ++ mesh_index)
				{
					float3 min_bb, max_bb;
						
					{
						XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");

						uint32_t index = 0;
						for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
						{
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_Position == ves[mesh_index][i].usage)
									{
										float3 pos(vertex_node->Attrib("x")->ValueFloat(),
											vertex_node->Attrib("y")->ValueFloat(), vertex_node->Attrib("z")->ValueFloat());
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &pos, sizeof(pos));
										if (0 == index)
										{
											min_bb = max_bb = pos;
										}
										else
										{
											min_bb = MathLib::minimize(min_bb, pos);
											max_bb = MathLib::maximize(max_bb, pos);
										}
										break;
									}
								}
							}

							XMLNodePtr normal_node = vertex_node->FirstNode("normal");
							if (normal_node)
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_Normal == ves[mesh_index][i].usage)
									{
										float3 normal(normal_node->Attrib("x")->ValueFloat(),
											normal_node->Attrib("y")->ValueFloat(), normal_node->Attrib("z")->ValueFloat());
										normal = MathLib::normalize(normal) * 0.5f + 0.5f;

										uint32_t compact;
										if (EF_A2BGR10 == ves[mesh_index][i].format)
										{	
											compact = MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.x() * 1023), 0, 1023)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.y() * 1023), 0, 1023) << 10)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.z() * 1023), 0, 1023) << 20);
										}
										else
										{
											BOOST_ASSERT(EF_ARGB8 == ves[mesh_index][i].format);

											compact = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.x() * 255), 0, 255) << 16)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.y() * 255), 0, 255) << 8)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.z() * 255), 0, 255) << 0);
										}

										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &compact, sizeof(compact));
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
										float4 diffuse(diffuse_node->Attrib("r")->ValueFloat(), diffuse_node->Attrib("g")->ValueFloat(),
											diffuse_node->Attrib("b")->ValueFloat(), diffuse_node->Attrib("a")->ValueFloat());
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &diffuse, sizeof(diffuse));
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
										float4 specular(specular_node->Attrib("r")->ValueFloat(), specular_node->Attrib("g")->ValueFloat(),
											specular_node->Attrib("b")->ValueFloat(), specular_node->Attrib("a")->ValueFloat());
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &specular, sizeof(specular));
										break;
									}
								}
							}

							uint32_t num_blend = 0;
							for (XMLNodePtr weight_node = vertex_node->FirstNode("weight"); weight_node; weight_node = weight_node->NextSibling("weight"))
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_BlendIndex == ves[mesh_index][i].usage)
									{
										uint8_t bone_index = static_cast<uint8_t>(weight_node->Attrib("bone_index")->ValueUInt());
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size() + num_blend * sizeof(bone_index)], &bone_index, sizeof(bone_index));
										break;
									}
								}
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_BlendWeight == ves[mesh_index][i].usage)
									{
										float weight = weight_node->Attrib("weight")->ValueFloat();
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size() + num_blend * sizeof(weight)], &weight, sizeof(weight));
										break;
									}
								}
								++ num_blend;
							}
							for (uint32_t b = num_blend; b < 4; ++ b)
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_BlendIndex == ves[mesh_index][i].usage)
									{
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memset(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size() + b * sizeof(uint8_t)], 0, sizeof(uint8_t));
										break;
									}
								}
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_BlendWeight == ves[mesh_index][i].usage)
									{
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memset(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size() + b * sizeof(float)], 0, sizeof(float));
										break;
									}
								}
							}

							uint32_t usage = 0;
							for (XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord"); tex_coord_node; tex_coord_node = tex_coord_node->NextSibling("tex_coord"))
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if ((VEU_TextureCoord == ves[mesh_index][i].usage) && (usage == ves[mesh_index][i].usage_index))
									{
										float3 tex_coord(0, 0, 0);
										XMLAttributePtr attr = tex_coord_node->Attrib("u");
										if (attr)
										{
											tex_coord.x() = attr->ValueFloat();
										}
										attr = tex_coord_node->Attrib("v");
										if (attr)
										{
											tex_coord.y() = attr->ValueFloat();
										}
										attr = tex_coord_node->Attrib("w");
										if (attr)
										{
											tex_coord.z() = attr->ValueFloat();
										}

										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &tex_coord, merged_ves[buf_index].element_size());
										break;
									}
								}
								++ usage;
							}

							XMLNodePtr tangent_node = vertex_node->FirstNode("tangent");
							if (tangent_node)
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_Tangent == ves[mesh_index][i].usage)
									{
										float k = 1;
										XMLAttributePtr attr = tangent_node->Attrib("w");
										if (attr)
										{
											k = attr->ValueFloat();
										}

										float3 tangent(tangent_node->Attrib("x")->ValueFloat(),
											tangent_node->Attrib("y")->ValueFloat(), tangent_node->Attrib("z")->ValueFloat());
										tangent = MathLib::normalize(tangent) * 0.5f + 0.5f;

										uint32_t compact;
										if (EF_A2BGR10 == ves[mesh_index][i].format)
										{	
											compact = MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent.x() * 1023), 0, 1023)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent.y() * 1023), 0, 1023) << 10)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent.z() * 1023), 0, 1023) << 20)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((k * 0.5f + 0.5f) * 3), 0, 3) << 30);
										}
										else
										{
											BOOST_ASSERT(EF_ARGB8 == ves[mesh_index][i].format);

											compact = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent.x() * 255), 0, 255) << 16)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent.y() * 255), 0, 255) << 8)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent.z() * 255), 0, 255) << 0)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((k * 0.5f + 0.5f) * 255), 0, 255) << 24);
										}
										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &compact, sizeof(compact));
										break;
									}
								}
							}

							XMLNodePtr binormal_node = vertex_node->FirstNode("binormal");
							if (binormal_node)
							{
								for (size_t i = 0; i < ves[mesh_index].size(); ++ i)
								{
									if (VEU_Binormal == ves[mesh_index][i].usage)
									{
										float3 binormal(tangent_node->Attrib("x")->ValueFloat(),
											tangent_node->Attrib("y")->ValueFloat(), tangent_node->Attrib("z")->ValueFloat());
										binormal = MathLib::normalize(binormal) * 0.5f + 0.5f;

										uint32_t compact;
										if (EF_A2BGR10 == ves[mesh_index][i].format)
										{	
											compact = MathLib::clamp<uint32_t>(static_cast<uint32_t>(binormal.x() * 1023), 0, 1023)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(binormal.y() * 1023), 0, 1023) << 10)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(binormal.z() * 1023), 0, 1023) << 20);
										}
										else
										{
											BOOST_ASSERT(EF_ARGB8 == ves[mesh_index][i].format);

											compact = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(binormal.x() * 255), 0, 255) << 16)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(binormal.y() * 255), 0, 255) << 8)
												| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(binormal.z() * 255), 0, 255) << 0);
										}

										uint32_t buf_index = ves_mapping[mesh_index][i];
										memcpy(&merged_buff[buf_index][(mesh_base_vertices[mesh_index] + index) * merged_ves[buf_index].element_size()], &compact, sizeof(compact));
										break;
									}
								}
							}

							++ index;
						}
					}

					bounding_boxes.push_back(Box(min_bb, max_bb));

					{
						XMLNodePtr triangles_chunk = mesh_node->FirstNode("triangles_chunk");

						uint32_t index = 0;
						for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
						{
							if (is_index_16_bit)
							{
								uint16_t a = static_cast<uint16_t>(tri_node->Attrib("a")->ValueUInt());
								uint16_t b = static_cast<uint16_t>(tri_node->Attrib("b")->ValueUInt());
								uint16_t c = static_cast<uint16_t>(tri_node->Attrib("c")->ValueUInt());
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 0) * index_elem_size], &a, sizeof(a));
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 1) * index_elem_size], &b, sizeof(b));
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 2) * index_elem_size], &c, sizeof(c));
							}
							else
							{
								uint32_t a = static_cast<uint32_t>(tri_node->Attrib("a")->ValueUInt());
								uint32_t b = static_cast<uint32_t>(tri_node->Attrib("b")->ValueUInt());
								uint32_t c = static_cast<uint32_t>(tri_node->Attrib("c")->ValueUInt());
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 0) * index_elem_size], &a, sizeof(a));
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 1) * index_elem_size], &b, sizeof(b));
								memcpy(&merged_indices[(mesh_start_indices[mesh_index] + index * 3 + 2) * index_elem_size], &c, sizeof(c));
							}

							++ index;
						}
					}
				}

				uint32_t num_merged_ves = static_cast<uint32_t>(merged_ves.size());
				ss->write(reinterpret_cast<char*>(&num_merged_ves), sizeof(num_merged_ves));
				for (size_t i = 0; i < merged_ves.size(); ++ i)
				{
					ss->write(reinterpret_cast<char*>(&merged_ves[i]), sizeof(merged_ves[i]));
				}

				ss->write(reinterpret_cast<char*>(&mesh_base_vertices.back()), sizeof(mesh_base_vertices.back()));
				ss->write(reinterpret_cast<char*>(&mesh_start_indices.back()), sizeof(mesh_start_indices.back()));
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
					ss->write(reinterpret_cast<char*>(&mtl_id), sizeof(mtl_id));

					float3 min_bb = bounding_boxes[mesh_index].Min();
					ss->write(reinterpret_cast<char*>(&min_bb), sizeof(min_bb));
					float3 max_bb = bounding_boxes[mesh_index].Max();
					ss->write(reinterpret_cast<char*>(&max_bb), sizeof(max_bb));

					ss->write(reinterpret_cast<char*>(&mesh_num_vertices[mesh_index]), sizeof(mesh_num_vertices[mesh_index]));
					ss->write(reinterpret_cast<char*>(&mesh_base_vertices[mesh_index]), sizeof(mesh_base_vertices[mesh_index]));
					ss->write(reinterpret_cast<char*>(&mesh_num_indices[mesh_index]), sizeof(mesh_num_indices[mesh_index]));
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
					ss->write(reinterpret_cast<char*>(&joint_parent), sizeof(joint_parent));

					XMLNodePtr bind_pos_node = bone_node->FirstNode("bind_pos");
					if (bind_pos_node)
					{
						float3 bind_pos(bind_pos_node->Attrib("x")->ValueFloat(), bind_pos_node->Attrib("y")->ValueFloat(),
							bind_pos_node->Attrib("z")->ValueFloat());

						XMLNodePtr bind_quat_node = bone_node->FirstNode("bind_quat");
						Quaternion bind_quat(bind_quat_node->Attrib("x")->ValueFloat(), bind_quat_node->Attrib("y")->ValueFloat(),
							bind_quat_node->Attrib("z")->ValueFloat(), bind_quat_node->Attrib("w")->ValueFloat());

						Quaternion bind_dual = MathLib::quat_trans_to_udq(bind_quat, bind_pos);

						ss->write(reinterpret_cast<char*>(&bind_quat), sizeof(bind_quat));
						ss->write(reinterpret_cast<char*>(&bind_dual), sizeof(bind_dual));
					}
					else
					{
						XMLNodePtr bind_real_node = bone_node->FirstNode("bind_real");
						Quaternion bind_real(bind_real_node->Attrib("x")->ValueFloat(), bind_real_node->Attrib("y")->ValueFloat(),
							bind_real_node->Attrib("z")->ValueFloat(), bind_real_node->Attrib("w")->ValueFloat());
						ss->write(reinterpret_cast<char*>(&bind_real), sizeof(bind_real));
							
						XMLNodePtr bind_dual_node = bone_node->FirstNode("bind_dual");
						Quaternion bind_dual(bind_dual_node->Attrib("x")->ValueFloat(), bind_dual_node->Attrib("y")->ValueFloat(),
							bind_dual_node->Attrib("z")->ValueFloat(), bind_dual_node->Attrib("w")->ValueFloat());
						ss->write(reinterpret_cast<char*>(&bind_dual), sizeof(bind_dual));
					}
				}
			}

			if (key_frames_chunk)
			{
				int32_t start_frame = key_frames_chunk->Attrib("start_frame")->ValueInt();
				int32_t end_frame = key_frames_chunk->Attrib("end_frame")->ValueInt();
				int32_t frame_rate = key_frames_chunk->Attrib("frame_rate")->ValueInt();
				ss->write(reinterpret_cast<char*>(&start_frame), sizeof(start_frame));
				ss->write(reinterpret_cast<char*>(&end_frame), sizeof(end_frame));
				ss->write(reinterpret_cast<char*>(&frame_rate), sizeof(frame_rate));

				KeyFrames kfs;
				for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
				{
					WriteShortString(*ss, kf_node->Attrib("joint")->ValueString());

					kfs.frame_id.clear();
					kfs.bind_real.clear();
					kfs.bind_dual.clear();

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
						Quaternion bind_dual;
						XMLNodePtr pos_node = key_node->FirstNode("pos");
						if (pos_node)
						{
							float3 bind_pos(pos_node->Attrib("x")->ValueFloat(), pos_node->Attrib("y")->ValueFloat(),
								pos_node->Attrib("z")->ValueFloat());

							XMLNodePtr quat_node = key_node->FirstNode("quat");
							bind_real = Quaternion(quat_node->Attrib("x")->ValueFloat(), quat_node->Attrib("y")->ValueFloat(),
								quat_node->Attrib("z")->ValueFloat(), quat_node->Attrib("w")->ValueFloat());
					
					        bind_dual = MathLib::quat_trans_to_udq(bind_real, bind_pos);
						}
						else
						{
							XMLNodePtr bind_real_node = key_node->FirstNode("bind_real");
							bind_real = Quaternion(bind_real_node->Attrib("x")->ValueFloat(), bind_real_node->Attrib("y")->ValueFloat(),
								bind_real_node->Attrib("z")->ValueFloat(), bind_real_node->Attrib("w")->ValueFloat());
							
							XMLNodePtr bind_dual_node = key_node->FirstNode("bind_dual");
							bind_dual = Quaternion(bind_dual_node->Attrib("x")->ValueFloat(), bind_dual_node->Attrib("y")->ValueFloat(),
								bind_dual_node->Attrib("z")->ValueFloat(), bind_dual_node->Attrib("w")->ValueFloat());
						}

						kfs.bind_real.push_back(bind_real);
						kfs.bind_dual.push_back(bind_dual);
					}

					// compress the key frame data
					uint32_t base = 0;
					while (base < kfs.frame_id.size() - 2)
					{
						uint32_t frame0 = kfs.frame_id[base + 0];
						uint32_t frame1 = kfs.frame_id[base + 1];
						uint32_t frame2 = kfs.frame_id[base + 2];
						std::pair<Quaternion, Quaternion> interpolate = MathLib::sclerp(kfs.bind_real[base + 0], kfs.bind_dual[base + 0],
							kfs.bind_real[base + 2], kfs.bind_dual[base + 2], static_cast<float>(frame1 - frame0) / (frame2 - frame0));

						float quat_dot = MathLib::dot(kfs.bind_real[base + 1], interpolate.first);
						Quaternion to_sign_corrected_real = interpolate.first;
						Quaternion to_sign_corrected_dual = interpolate.second;
						if (quat_dot < 0)
						{
							to_sign_corrected_real = -to_sign_corrected_real;
							to_sign_corrected_dual = -to_sign_corrected_dual;
						}

						std::pair<Quaternion, Quaternion> dif_dq = MathLib::inverse(kfs.bind_real[base + 1], kfs.bind_dual[base + 1]);
						dif_dq.second = MathLib::mul_dual(dif_dq.first, dif_dq.second, to_sign_corrected_real, to_sign_corrected_dual);
						dif_dq.first = MathLib::mul_real(dif_dq.first, to_sign_corrected_real);

						if ((abs(dif_dq.first.x()) < 1e-5f) && (abs(dif_dq.first.y()) < 1e-5f)
							&& (abs(dif_dq.first.z()) < 1e-5f) && (abs(dif_dq.first.w() - 1) < 1e-5f)
							&& (abs(dif_dq.second.x()) < 1e-5f) && (abs(dif_dq.second.y()) < 1e-5f)
							&& (abs(dif_dq.second.z()) < 1e-5f) && (abs(dif_dq.second.w()) < 1e-5f))
						{
							kfs.frame_id.erase(kfs.frame_id.begin() + base + 1);
							kfs.bind_real.erase(kfs.bind_real.begin() + base + 1);
							kfs.bind_dual.erase(kfs.bind_dual.begin() + base + 1);
						}
						else
						{
							++ base;
						}
					}

					uint32_t num_kf = static_cast<uint32_t>(kfs.frame_id.size());
					ss->write(reinterpret_cast<char*>(&num_kf), sizeof(num_kf));
					for (uint32_t i = 0; i < num_kf; ++ i)
					{
						ss->write(reinterpret_cast<char*>(&kfs.frame_id[i]), sizeof(kfs.frame_id[i]));
						ss->write(reinterpret_cast<char*>(&kfs.bind_real[i]), sizeof(kfs.bind_real[i]));
						ss->write(reinterpret_cast<char*>(&kfs.bind_dual[i]), sizeof(kfs.bind_dual[i]));
					}
				}
			}

			std::ofstream ofs((path_name + jit_ext_name).c_str(), std::ios_base::binary);
			BOOST_ASSERT(ofs);
			uint32_t fourcc = MakeFourCC<'K', 'L', 'M', ' '>::value;
			ofs.write(reinterpret_cast<char*>(&fourcc), sizeof(fourcc));

			uint32_t ver = MODEL_BIN_VERSION;
			ofs.write(reinterpret_cast<char*>(&ver), sizeof(ver));

			uint64_t original_len = ss->str().size();
			ofs.write(reinterpret_cast<char*>(&original_len), sizeof(original_len));

			std::ofstream::pos_type p = ofs.tellp();
			uint64_t len = 0;
			ofs.write(reinterpret_cast<char*>(&len), sizeof(len));

			LZMACodec lzma;
			len = lzma.Encode(ofs, ss->str().c_str(), ss->str().size());

			ofs.seekp(p, std::ios_base::beg);
			ofs.write(reinterpret_cast<char*>(&len), sizeof(len));
		}
	}

	void LoadModel(std::string const & meshml_name, std::vector<RenderMaterialPtr>& mtls,
		std::vector<vertex_element>& merged_ves, char& all_is_index_16_bit,
		std::vector<std::vector<uint8_t> >& merged_buff, std::vector<uint8_t>& merged_indices,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids, std::vector<Box>& bbs,
		std::vector<uint32_t>& mesh_num_vertices, std::vector<uint32_t>& mesh_base_vertices,
		std::vector<uint32_t>& mesh_num_triangles, std::vector<uint32_t>& mesh_base_triangles,
		std::vector<Joint>& joints, boost::shared_ptr<KeyFramesType>& kfs,
		int32_t& start_frame, int32_t& end_frame, int32_t& frame_rate)
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
		BOOST_ASSERT((fourcc == MakeFourCC<'K', 'L', 'M', ' '>::value));

		uint32_t ver;
		lzma_file->read(&ver, sizeof(ver));
		BOOST_ASSERT(MODEL_BIN_VERSION == ver);

		boost::shared_ptr<std::stringstream> ss = MakeSharedPtr<std::stringstream>();

		uint64_t original_len, len;
		lzma_file->read(&original_len, sizeof(original_len));
		lzma_file->read(&len, sizeof(len));

		LZMACodec lzma;
		lzma.Decode(*ss, lzma_file, len, original_len);

		ResIdentifierPtr decoded = MakeSharedPtr<ResIdentifier>(lzma_file->ResName(), lzma_file->Timestamp(), ss);

		uint32_t num_mtls;
		decoded->read(&num_mtls, sizeof(num_mtls));
		uint32_t num_meshes;
		decoded->read(&num_meshes, sizeof(num_meshes));
		uint32_t num_joints;
		decoded->read(&num_joints, sizeof(num_joints));
		uint32_t num_kfs;
		decoded->read(&num_kfs, sizeof(num_kfs));

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
		merged_ves.resize(num_merged_ves);
		for (size_t i = 0; i < merged_ves.size(); ++ i)
		{
			decoded->read(&merged_ves[i], sizeof(merged_ves[i]));
		}

		uint32_t all_num_vertices;
		uint32_t all_num_indices;
		decoded->read(&all_num_vertices, sizeof(all_num_vertices));
		decoded->read(&all_num_indices, sizeof(all_num_indices));
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
		bbs.resize(num_meshes);
		mesh_num_vertices.resize(num_meshes);
		mesh_base_vertices.resize(num_meshes);
		mesh_num_triangles.resize(num_meshes);
		mesh_base_triangles.resize(num_meshes);
		for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
		{
			ReadShortString(decoded, mesh_names[mesh_index]);

			decoded->read(&mtl_ids[mesh_index], sizeof(mtl_ids[mesh_index]));

			float3 min_bb, max_bb;
			decoded->read(&min_bb, sizeof(min_bb));
			decoded->read(&max_bb, sizeof(max_bb));
			bbs[mesh_index] = Box(min_bb, max_bb);

			decoded->read(&mesh_num_vertices[mesh_index], sizeof(mesh_num_vertices[mesh_index]));
			decoded->read(&mesh_base_vertices[mesh_index], sizeof(mesh_base_vertices[mesh_index]));
			decoded->read(&mesh_num_triangles[mesh_index], sizeof(mesh_num_triangles[mesh_index]));
			decoded->read(&mesh_base_triangles[mesh_index], sizeof(mesh_base_triangles[mesh_index]));
		}

		joints.resize(num_joints);
		for (uint32_t joint_index = 0; joint_index < num_joints; ++ joint_index)
		{
			Joint& joint = joints[joint_index];

			ReadShortString(decoded, joint.name);
			decoded->read(&joint.parent, sizeof(joint.parent));

			decoded->read(&joint.bind_real, sizeof(joint.bind_real));
			decoded->read(&joint.bind_dual, sizeof(joint.bind_dual)); 

			std::pair<Quaternion, Quaternion> inv = MathLib::inverse(joint.bind_real, joint.bind_dual);
			joint.inverse_origin_real = inv.first;
			joint.inverse_origin_dual = inv.second;
		}

		if (num_kfs > 0)
		{
			decoded->read(&start_frame, sizeof(start_frame));
			decoded->read(&end_frame, sizeof(end_frame));
			decoded->read(&frame_rate, sizeof(frame_rate));

			kfs = MakeSharedPtr<KeyFramesType>();
			for (uint32_t kf_index = 0; kf_index < num_kfs; ++ kf_index)
			{
				std::string name;
				ReadShortString(decoded, name);

				uint32_t num_kf;
				decoded->read(&num_kf, sizeof(num_kf));

				KeyFrames kf;
				kf.frame_id.resize(num_kf);
				kf.bind_real.resize(num_kf);
				kf.bind_dual.resize(num_kf);
				for (uint32_t k_index = 0; k_index < num_kf; ++ k_index)
				{
					decoded->read(&kf.frame_id[k_index], sizeof(kf.frame_id[k_index]));
					decoded->read(&kf.bind_real[k_index], sizeof(kf.bind_real[k_index]));
					decoded->read(&kf.bind_dual[k_index], sizeof(kf.bind_dual[k_index]));
				}

				kfs->insert(std::make_pair(name, kf));
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
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<std::vector<vertex_element> > const & ves,
		std::vector<std::vector<std::vector<uint8_t> > > const & buffs,
		std::vector<char> const & is_index_16_bit, std::vector<std::vector<uint8_t> > const & indices,
		std::vector<Joint> const & joints, boost::shared_ptr<KeyFramesType> const & kfs,
		int32_t start_frame, int32_t end_frame, int32_t frame_rate)
	{
		KlayGE::XMLDocument doc;

		XMLNodePtr root = doc.AllocNode(XNT_Element, "model");
		doc.RootNode(root);
		root->AppendAttrib(doc.AllocAttribUInt("version", 4));

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

				Quaternion const & bind_real = joint.bind_real;
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
						case VEU_Tangent:
						case VEU_Binormal:
							{
								std::string node_name;
								switch (ve.usage)
								{
								case VEU_Normal:
									node_name = "normal";
									break;

								case VEU_Tangent:
									node_name = "tangent";
									break;

								default:
									node_name = "binormal";
									break;
								}

								XMLNodePtr sub_node = doc.AllocNode(XNT_Element, node_name);
								vertex_node->AppendNode(sub_node);

								if (EF_A2BGR10 == ve.format)
								{
									uint32_t const * p = reinterpret_cast<uint32_t const *>(&buffs[mesh_index][k][0]);
									sub_node->AppendAttrib(doc.AllocAttribFloat("x", ((p[j] >>  0) & 0x3FF) / 1023.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("y", ((p[j] >> 10) & 0x3FF) / 1023.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("z", ((p[j] >> 20) & 0x3FF) / 1023.0f * 2 - 1));
									if (VEU_Tangent == ve.usage)
									{
										sub_node->AppendAttrib(doc.AllocAttribFloat("w", ((p[j] >> 30) & 0x3) / 3.0f * 2 - 1));
									}
								}
								else if (EF_ARGB8 == ve.format)
								{
									uint32_t const * p = reinterpret_cast<uint32_t const *>(&buffs[mesh_index][k][0]);
									sub_node->AppendAttrib(doc.AllocAttribFloat("x", ((p[j] >> 16) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("y", ((p[j] >>  8) & 0xFF) / 255.0f * 2 - 1));
									sub_node->AppendAttrib(doc.AllocAttribFloat("z", ((p[j] >>  0) & 0xFF) / 255.0f * 2 - 1));
									if (VEU_Tangent == ve.usage)
									{
										sub_node->AppendAttrib(doc.AllocAttribFloat("w", ((p[j] >> 24) & 0xFF) / 255.0f * 2 - 1));
									}
								}
								else if (EF_BGR32F == ve.format)
								{
									float3 const * p = reinterpret_cast<float3 const *>(&buffs[mesh_index][k][0]);
									sub_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
									sub_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
									sub_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
								}
								else if (EF_ABGR32F == ve.format)
								{
									BOOST_ASSERT(EF_ABGR32F == ve.format);

									float4 const * p = reinterpret_cast<float4 const *>(&buffs[mesh_index][k][0]);
									sub_node->AppendAttrib(doc.AllocAttribFloat("x", p[j].x()));
									sub_node->AppendAttrib(doc.AllocAttribFloat("y", p[j].y()));
									sub_node->AppendAttrib(doc.AllocAttribFloat("z", p[j].z()));
									sub_node->AppendAttrib(doc.AllocAttribFloat("w", p[j].w()));
								}
							}
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
										float const * weights = reinterpret_cast<float const *>(&buffs[mesh_index][weight_stream][0]);
										weight_node->AppendAttrib(doc.AllocAttribFloat("weight", weights[j * num_components + c]));
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

		if (kfs)
		{
			if (!kfs->empty())
			{
				uint32_t num_key_frames = static_cast<uint32_t>(kfs->size());

				XMLNodePtr key_frames_chunk = doc.AllocNode(XNT_Element, "key_frames_chunk");
				root->AppendNode(key_frames_chunk);

				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("start_frame", start_frame));
				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("end_frame", end_frame));
				key_frames_chunk->AppendAttrib(doc.AllocAttribUInt("frame_rate", frame_rate));

				KeyFramesType::const_iterator iter = kfs->begin();

				for (uint32_t i = 0; i < num_key_frames; ++ i, ++ iter)
				{
					XMLNodePtr key_frame_node = doc.AllocNode(XNT_Element, "key_frame");
					key_frames_chunk->AppendNode(key_frame_node);

					key_frame_node->AppendAttrib(doc.AllocAttribString("joint", iter->first));

					for (size_t j = 0; j < iter->second.bind_real.size(); ++ j)
					{
						XMLNodePtr key_node = doc.AllocNode(XNT_Element, "key");
						key_frame_node->AppendNode(key_node);

						key_frame_node->AppendAttrib(doc.AllocAttribUInt("id", iter->second.frame_id[j]));

						XMLNodePtr bind_real_node = doc.AllocNode(XNT_Element, "bind_real");
						key_node->AppendNode(bind_real_node); 
						bind_real_node->AppendAttrib(doc.AllocAttribFloat("x", iter->second.bind_real[j].x()));
						bind_real_node->AppendAttrib(doc.AllocAttribFloat("y", iter->second.bind_real[j].y()));
						bind_real_node->AppendAttrib(doc.AllocAttribFloat("z", iter->second.bind_real[j].z()));
						bind_real_node->AppendAttrib(doc.AllocAttribFloat("w", iter->second.bind_real[j].w()));

						XMLNodePtr bind_dual_node = doc.AllocNode(XNT_Element, "bind_dual");
						key_node->AppendNode(bind_dual_node);
						bind_dual_node->AppendAttrib(doc.AllocAttribFloat("x", iter->second.bind_dual[j].x()));
						bind_dual_node->AppendAttrib(doc.AllocAttribFloat("y", iter->second.bind_dual[j].y()));
						bind_dual_node->AppendAttrib(doc.AllocAttribFloat("z", iter->second.bind_dual[j].z()));
						bind_dual_node->AppendAttrib(doc.AllocAttribFloat("w", iter->second.bind_dual[j].w()));
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
		boost::shared_ptr<KeyFramesType> kfs;
		int32_t start_frame = 0;
		int32_t end_frame = 0;
		int32_t frame_rate = 0;
		if (model->IsSkinned())
		{
			SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(model);
			uint32_t num_joints = skinned->NumJoints();
			joints.resize(num_joints);
			for (uint32_t i = 0; i < num_joints; ++ i)
			{
				joints[i] = checked_pointer_cast<SkinnedModel>(model)->GetJoint(i);
			}

			start_frame = skinned->StartFrame();
			end_frame = skinned->EndFrame();
			frame_rate = skinned->FrameRate();

			kfs = skinned->GetKeyFrames();
		}

		SaveModel(meshml_name, mtls, mesh_names, mtl_ids, ves, buffs, is_index_16_bit, indices, joints, kfs, start_frame, end_frame, frame_rate);
	}


	RenderableLightSourceProxy::RenderableLightSourceProxy(RenderModelPtr const & model, std::wstring const & name)
			: StaticMesh(model, name)
	{
		this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("LightSourceProxy.fxml")->TechniqueByName("LightSourceProxy"));
	}

	void RenderableLightSourceProxy::Technique(RenderTechniquePtr const & tech)
	{
		technique_ = tech;
		if (tech)
		{
			mvp_param_ = technique_->Effect().ParameterByName("mvp");
			model_param_ = technique_->Effect().ParameterByName("model");

			light_color_param_ = technique_->Effect().ParameterByName("light_color");
			light_falloff_param_ = technique_->Effect().ParameterByName("light_falloff");
			light_is_projective_param_ = technique_->Effect().ParameterByName("light_is_projective");
			light_projective_tex_param_ = technique_->Effect().ParameterByName("light_projective_tex");
		}
	}

	void RenderableLightSourceProxy::Update()
	{
		if (!deferred_effect_)
		{
			if (light_color_param_)
			{
				*light_color_param_ = light_->Color();
			}
			if (light_falloff_param_)
			{
				*light_falloff_param_ = light_->Falloff();
			}
			if (light_->ProjectiveTexture() && light_is_projective_param_)
			{
				*light_is_projective_param_ = static_cast<int32_t>(light_->ProjectiveTexture() ? 1 : 0);
			}
			if (light_projective_tex_param_)
			{
				*light_projective_tex_param_ = light_->ProjectiveTexture();
			}
		}
	}

	void RenderableLightSourceProxy::OnRenderBegin()
	{
		if (deferred_effect_)
		{
			StaticMesh::OnRenderBegin();

			if ((PT_OpaqueShading == type_) || (PT_OpaqueSpecialShading == type_))
			{
				float4 clr = light_->Color();
				clr.w() = 0;
				*emit_clr_param_ = clr;
			}
		}
		else
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			float4x4 mv = model_mat_ * view;
			*mvp_param_ = mv * proj;
			*model_param_ = model_mat_;
		}
	}

	void RenderableLightSourceProxy::AttachLightSrc(LightSourcePtr const & light)
	{
		light_ = light;
	}
}
