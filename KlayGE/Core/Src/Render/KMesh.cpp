// KMesh.cpp
// KlayGE KMesh类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 支持蒙皮模型的载入和保存 (2006.8.23)
//
// 2.7.1
// LoadKMesh可以使用自定义类 (2005.7.13)
//
// 2.7.0
// 初次建立 (2005.6.17)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>

#include <fstream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/KMesh.hpp>

namespace
{
	std::string ReadShortString(std::istream& is)
	{
		KlayGE::uint8_t len;
		is.read(reinterpret_cast<char*>(&len), sizeof(len));
		std::vector<char> str(len, 0);
		is.read(&str[0], static_cast<std::streamsize>(str.size()));

		return std::string(str.begin(), str.end());
	}

	void WriteShortString(std::ostream& os, std::string const & str)
	{
		KlayGE::uint8_t const len = static_cast<KlayGE::uint8_t const>(str.size());
		os.write(reinterpret_cast<char const *>(&len), sizeof(len));
		os.write(&str[0], static_cast<std::streamsize>(str.size()));
	}
}

namespace KlayGE
{
	KMesh::KMesh(RenderModelPtr model, std::wstring const & name)
						: StaticMesh(model, name),
							model_(float4x4::Identity())
	{
		// 载入fx
		RenderEffectPtr effect;
		if (!ResLoader::Instance().Locate("KMesh.kfx").empty())
		{
			effect = Context::Instance().RenderFactoryInstance().LoadEffect("KMesh.kfx");
		}
		else
		{
			effect = RenderEffect::NullObject();
		}

		technique_ = effect->TechniqueByName("KMeshNoTexTec");

		texSampler_ep_ = technique_->Effect().ParameterByName("texSampler");
		modelviewproj_ep_ = technique_->Effect().ParameterByName("modelviewproj");
		modelIT_ep_ = technique_->Effect().ParameterByName("modelIT");
	}

	KMesh::~KMesh()
	{
	}

	void KMesh::BuildMeshInfo()
	{
		TexturePtr tex;
		if (!texture_slots_.empty())
		{
			tex = LoadTexture(texture_slots_[0].second, EAH_GPU_Read);
		}

		if (tex)
		{
			technique_ = technique_->Effect().TechniqueByName("KMeshTec");
			*texSampler_ep_ = tex;
		}
		else
		{
			technique_ = technique_->Effect().TechniqueByName("KMeshNoTexTec");
		}
	}

	void KMesh::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		*modelviewproj_ep_ = model_ * camera.ViewMatrix() * camera.ProjMatrix();
	}

	void KMesh::SetModelMatrix(float4x4 const & model)
	{
		model_ = model;
		*modelIT_ep_ = MathLib::transpose(MathLib::inverse(model_));
	}


	RenderModelPtr LoadKModel(std::string const & kmodel_name, uint32_t access_hint,
		boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc,
		boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateModelFactoryFunc);
		BOOST_ASSERT(CreateMeshFactoryFunc);

		typedef std::vector<StaticMeshPtr> MeshesType;
		MeshesType meshes;

		ResIdentifierPtr file(ResLoader::Instance().Load(kmodel_name));

		char fourcc[4];
		file->read(fourcc, sizeof(fourcc));
		BOOST_ASSERT(('M' == fourcc[0]) && ('E' == fourcc[1]) && ('S' == fourcc[2]) && ('H' == fourcc[3]));

		uint32_t header_size;
		file->read(reinterpret_cast<char*>(&header_size), sizeof(header_size));

		KModelHeader header;
		file->read(reinterpret_cast<char*>(&header), sizeof(header));
		BOOST_ASSERT(3 == header.version);

		RenderModelPtr ret;
		if ((header.num_joints > 0) && (header.num_key_frames > 0))
		{
			ret = CreateModelFactoryFunc(L"KSkinnedMesh");
		}
		else
		{
			ret = CreateModelFactoryFunc(L"KMesh");
		}

		for (uint8_t i = 0; i < header.num_meshes; ++ i)
		{
			std::wstring wname;
			Convert(wname, ReadShortString(*file));
			StaticMeshPtr mesh = CreateMeshFactoryFunc(ret, wname);

			uint8_t num_vertex_elems;
			file->read(reinterpret_cast<char*>(&num_vertex_elems), sizeof(num_vertex_elems));

			std::vector<vertex_element> vertex_elements;
			for (uint8_t j = 0; j < num_vertex_elems; ++ j)
			{
				vertex_element ve;

				uint8_t usage;
				file->read(reinterpret_cast<char*>(&usage), 1);
				ve.usage = static_cast<VertexElementUsage>(usage);
				file->read(reinterpret_cast<char*>(&ve.usage_index), 1);
				uint8_t num_components;
				file->read(reinterpret_cast<char*>(&num_components), 1);

				if (ve.usage != VEU_BlendIndex)
				{
					switch (num_components)
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

					case 4:
						ve.format = EF_ABGR32F;
						break;
					}
				}
				else
				{
					switch (num_components)
					{
					case 1:
						ve.format = EF_L8;
						break;

					case 2:
						ve.format = EF_AL8;
						break;

					case 3:
						ve.format = EF_RGB8;
						break;

					case 4:
						ve.format = EF_ABGR8;
						break;
					}
				}

				vertex_elements.push_back(ve);
			}

			uint8_t num_textures;
			file->read(reinterpret_cast<char*>(&num_textures), sizeof(num_textures));

			StaticMesh::TextureSlotsType texture_slots(num_textures);
			for (uint8_t j = 0; j < num_textures; ++ j)
			{
				texture_slots[j].first = ReadShortString(*file);
				texture_slots[j].second = ReadShortString(*file);
			}

			mesh->TextureSlots(texture_slots);

			uint32_t num_vertices;
			file->read(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));

			for (uint8_t k = 0; k < num_vertex_elems; ++ k)
			{
				std::vector<uint8_t> buf(num_vertices * vertex_elements[k].element_size());
				file->read(reinterpret_cast<char*>(&buf[0]), static_cast<std::streamsize>(sizeof(buf[0]) * buf.size()));

				mesh->AddVertexStream(&buf[0], static_cast<uint32_t>(buf.size()), vertex_elements[k], access_hint);
			}

			uint32_t num_triangles;
			file->read(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));
			{
				std::vector<uint16_t> indices(num_triangles * 3);
				file->read(reinterpret_cast<char*>(&indices[0]),
					static_cast<std::streamsize>(sizeof(indices[0]) * indices.size()));

				mesh->AddIndexStream(&indices[0], static_cast<uint32_t>(indices.size() * sizeof(indices[0])), EF_R16, access_hint);
			}

			meshes.push_back(mesh);
		}

		std::vector<Joint> joints;
		for (uint8_t i = 0; i < header.num_joints; ++ i)
		{
			Joint joint;
			joint.name = ReadShortString(*file);
			file->read(reinterpret_cast<char*>(&joint.parent), sizeof(joint.parent));
			file->read(reinterpret_cast<char*>(&joint.bind_pos), sizeof(joint.bind_pos));
			file->read(reinterpret_cast<char*>(&joint.bind_quat), sizeof(joint.bind_quat));

			joint.inverse_origin_quat = MathLib::inverse(joint.bind_quat);
			joint.inverse_origin_pos = MathLib::transform_quat(-joint.bind_pos, joint.inverse_origin_quat);

			joints.push_back(joint);
		}

		boost::shared_ptr<KeyFramesType> kfs = MakeSharedPtr<KeyFramesType>();
		for (uint8_t i = 0; i < header.num_key_frames; ++ i)
		{
			std::string name = ReadShortString(*file);

			KeyFrames kf;
			kf.bind_pos.resize(header.end_frame - header.start_frame);
			kf.bind_quat.resize(header.end_frame - header.start_frame);

			file->read(reinterpret_cast<char*>(&kf.bind_pos[0]),
				static_cast<std::streamsize>(sizeof(kf.bind_pos[0]) * kf.bind_pos.size()));
			file->read(reinterpret_cast<char*>(&kf.bind_quat[0]),
				static_cast<std::streamsize>(sizeof(kf.bind_quat[0]) * kf.bind_quat.size()));

			kfs->insert(std::make_pair(name, kf));
		}

		if (ret->IsSkinned())
		{
			SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(ret);

			skinned->AssignJoints(joints.begin(), joints.end());
			skinned->AttachKeyFrames(kfs);

			skinned->StartFrame(header.start_frame);
			skinned->EndFrame(header.end_frame);
			skinned->FrameRate(header.frame_rate);
		}

		BOOST_FOREACH(BOOST_TYPEOF(meshes)::reference mesh, meshes)
		{
			mesh->BuildMeshInfo();
		}
		ret->AssignMeshes(meshes.begin(), meshes.end());

		return ret;
	}

	void SaveKModel(RenderModelPtr model, std::string const & kmodel_name)
	{
		std::ofstream file(kmodel_name.c_str(), std::ios_base::binary);

		char const fourcc[4] = { 'M', 'E', 'S', 'H' };
		file.write(fourcc, sizeof(fourcc));

		KModelHeader header;
		header.version = 3;
		header.num_meshes = static_cast<uint8_t const>(model->NumMeshes());
		if (model->IsSkinned())
		{
			SkinnedModelPtr skinned = checked_pointer_cast<SkinnedModel>(model);

			header.num_joints = static_cast<uint8_t>(skinned->NumJoints());
			header.num_key_frames = static_cast<uint8_t>(skinned->GetKeyFrames().size());
			header.start_frame = skinned->StartFrame();
			header.end_frame = skinned->EndFrame();
			header.frame_rate = skinned->FrameRate();
		}
		else
		{
			header.num_joints = 0;
			header.num_key_frames = 0;
			header.start_frame = 0;
			header.end_frame = 0;
			header.frame_rate = 0;
		}

		uint32_t header_size = sizeof(header);
		file.write(reinterpret_cast<char*>(&header_size), sizeof(header_size));
		file.write(reinterpret_cast<char*>(&header), sizeof(header));

		for (uint8_t i = 0; i < header.num_meshes; ++ i)
		{
			StaticMesh const & mesh = *model->Mesh(i);

			std::string name;
			Convert(name, mesh.Name());
			WriteShortString(file, name);

			RenderLayoutPtr rl = mesh.GetRenderLayout();
			uint8_t const num_vertex_elems = static_cast<uint8_t const>(rl->NumVertexStreams());
			file.write(reinterpret_cast<char const *>(&num_vertex_elems), sizeof(num_vertex_elems));

			for (uint8_t j = 0; j < num_vertex_elems; ++ j)
			{
				vertex_element const & ve = rl->VertexStreamFormat(j)[0];

				uint8_t const usage = static_cast<uint8_t const>(ve.usage);
				file.write(reinterpret_cast<char const *>(&usage), sizeof(usage));
				file.write(reinterpret_cast<char const *>(&ve.usage_index), sizeof(ve.usage_index));
				uint8_t const num_components = static_cast<uint8_t const>(NumComponents(ve.format));
				file.write(reinterpret_cast<char const *>(&num_components), sizeof(num_components));
			}

			uint8_t const num_textures = static_cast<uint8_t const>(mesh.TextureSlots().size());
			file.write(reinterpret_cast<char const *>(&num_textures), sizeof(num_textures));

			StaticMesh::TextureSlotsType const & texture_slots = mesh.TextureSlots();
			for (uint8_t j = 0; j < num_textures; ++ j)
			{
				WriteShortString(file, texture_slots[j].first);
				WriteShortString(file, texture_slots[j].second);
			}

			uint32_t const num_vertices = static_cast<uint32_t const>(mesh.NumVertices());
			file.write(reinterpret_cast<char const *>(&num_vertices), sizeof(num_vertices));
			for (uint8_t k = 0; k < num_vertex_elems; ++ k)
			{
				GraphicsBufferPtr vb = rl->GetVertexStream(k);
				{
					GraphicsBuffer::Mapper mapper(*vb, BA_Read_Only);
					file.write(mapper.Pointer<char const>(), vb->Size());
				}
			}

			uint32_t const num_triangles = mesh.NumTriangles();
			file.write(reinterpret_cast<char const *>(&num_triangles), sizeof(num_triangles));
			{
				GraphicsBufferPtr ib = rl->GetIndexStream();
				{
					GraphicsBuffer::Mapper mapper(*ib, BA_Read_Only);
					file.write(mapper.Pointer<char const>(), ib->Size());
				}
			}
		}

		for (uint8_t i = 0; i < header.num_joints; ++ i)
		{
			Joint const & joint = checked_pointer_cast<SkinnedModel>(model)->GetJoint(i);

			WriteShortString(file, joint.name);

			file.write(reinterpret_cast<char const *>(&joint.parent), sizeof(joint.parent));
			file.write(reinterpret_cast<char const *>(&joint.bind_pos), sizeof(joint.bind_pos));
			file.write(reinterpret_cast<char const *>(&joint.bind_quat), sizeof(joint.bind_quat));
		}

		if (header.num_key_frames > 0)
		{
			KeyFramesType const & kfs = checked_pointer_cast<SkinnedModel>(model)->GetKeyFrames();
			KeyFramesType::const_iterator iter = kfs.begin();

			for (uint8_t i = 0; i < header.num_key_frames; ++ i, ++ iter)
			{
				WriteShortString(file, iter->first);

				file.write(reinterpret_cast<char const *>(&iter->second.bind_pos[0]),
					static_cast<std::streamsize>(sizeof(iter->second.bind_pos[0]) * iter->second.bind_pos.size()));
				file.write(reinterpret_cast<char const *>(&iter->second.bind_quat[0]),
					static_cast<std::streamsize>(sizeof(iter->second.bind_quat[0]) * iter->second.bind_quat.size()));
			}
		}
	}
}
