// KMesh.cpp
// KlayGE KMesh类 实现文件
// Ver 2.7.1
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/App3D.hpp>

#include <boost/assert.hpp>

#include <KlayGE/KMesh.hpp>

namespace KlayGE
{
	KMesh::KMesh(std::wstring const & name, TexturePtr tex)
						: StaticMesh(name),
							sampler_(new Sampler),
							model_(float4x4::Identity())
	{
		// 载入fx
		RenderEffectPtr effect;
		if (!ResLoader::Instance().Locate("KMesh.fx").empty())
		{
			effect = Context::Instance().RenderFactoryInstance().LoadEffect("KMesh.fx");
		}
		else
		{
			effect = RenderEffect::NullObject();
		}

		if (tex)
		{
			technique_ = effect->Technique("KMeshTec");

			sampler_->SetTexture(tex);
			sampler_->Filtering(Sampler::TFO_Bilinear);
			sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
			sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("texSampler")) = sampler_;
		}
		else
		{
			technique_ = effect->Technique("KMeshNoTexTec");
		}
	}

	KMesh::~KMesh()
	{
	}

	void KMesh::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		*(technique_->Effect().ParameterByName("modelviewproj")) = model_ * camera.ViewMatrix() * camera.ProjMatrix();
		*(technique_->Effect().ParameterByName("modelIT")) = MathLib::transpose(MathLib::inverse(model_));
	}

	void KMesh::SetModelMatrix(float4x4 const & model)
	{
		model_ = model;
	}


	RenderModelPtr LoadKMesh(const std::string& kmeshName,
		boost::function<KMeshPtr (std::wstring const &, TexturePtr)> CreateKMeshFactoryFunc)
	{
		BOOST_ASSERT(CreateKMeshFactoryFunc);

		typedef std::vector<StaticMeshPtr> MeshesType;
		MeshesType meshes;

		ResIdentifierPtr file(ResLoader::Instance().Load(kmeshName));

		char fourcc[4];
		file->read(fourcc, sizeof(fourcc));

		uint32_t version;
		file->read(reinterpret_cast<char*>(&version), sizeof(version));

		uint8_t num_meshes;
		file->read(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));
		for (uint8_t i = 0; i < num_meshes; ++ i)
		{
			uint8_t len;
			file->read(reinterpret_cast<char*>(&len), sizeof(len));
			std::string name(len, 0);
			file->read(&name[0], static_cast<std::streamsize>(name.size()));
			std::wstring wname;
			Convert(wname, name);

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
						ve.format = EF_ARGB8;
						break;
					}
				}

				vertex_elements.push_back(ve);
			}

			uint8_t num_textures;
			file->read(reinterpret_cast<char*>(&num_textures), sizeof(num_textures));

			typedef std::vector<std::pair<std::string, std::string> > TextureSlotsType;
			TextureSlotsType texture_slots(num_textures);

			for (uint8_t j = 0; j < num_textures; ++ j)
			{
				std::string& texture_type = texture_slots[j].first;
				std::string& texture_name = texture_slots[j].second;

				uint8_t len;
				file->read(reinterpret_cast<char*>(&len), sizeof(len));
				texture_type.resize(len, 0);
				file->read(&texture_type[0], static_cast<std::streamsize>(texture_type.size()));
				file->read(reinterpret_cast<char*>(&len), sizeof(len));
				texture_name.resize(len, 0);
				file->read(&texture_name[0], static_cast<std::streamsize>(texture_name.size()));
			}

			TexturePtr texture;
			if (!texture_slots.empty() && !texture_slots[0].second.empty()
				&& !ResLoader::Instance().Locate(texture_slots[0].second).empty())
			{
				texture = LoadTexture(texture_slots[0].second);
			}

			StaticMeshPtr mesh = CreateKMeshFactoryFunc(wname, texture);

			uint32_t num_vertices;
			file->read(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));
			uint32_t num_tex_coords_per_ver;
			file->read(reinterpret_cast<char*>(&num_tex_coords_per_ver), sizeof(num_tex_coords_per_ver));

			StaticMesh::PositionsType positions(num_vertices);
			StaticMesh::NormalsType normals(num_vertices);
			StaticMesh::MultiTexCoordsType multi_tex_coords(num_tex_coords_per_ver);
			StaticMesh::DiffusesType diffuses(num_vertices);
			StaticMesh::SpecularsType speculars(num_vertices);
			StaticMesh::BlendIndicesType blend_indices;
			StaticMesh::BlendWeightsType blend_weights;
			StaticMesh::TangentsType tangents(num_vertices);
			StaticMesh::BinormalsType binormals(num_vertices);

			for (uint32_t k = 0; k < num_tex_coords_per_ver; ++ k)
			{
				multi_tex_coords[k].resize(num_vertices);
			}

			for (uint8_t k = 0; k < num_vertex_elems; ++ k)
			{
				if ((VEU_BlendWeight == vertex_elements[k].usage) || (VEU_BlendIndex == vertex_elements[k].usage))
				{
					blend_indices.resize(num_vertices * NumComponents(vertex_elements[k].format));
					blend_weights.resize(num_vertices * NumComponents(vertex_elements[k].format));
					break;
				}
			}

			for (uint32_t j = 0; j < num_vertices; ++ j)
			{
				for (uint8_t k = 0; k < num_vertex_elems; ++ k)
				{
					switch (vertex_elements[k].usage)
					{
					case VEU_Position:
						file->read(reinterpret_cast<char*>(&positions[j]), sizeof(positions[j]));
						break;

					case VEU_Normal:
						file->read(reinterpret_cast<char*>(&normals[j]), sizeof(normals[j]));
						break;

					case VEU_Diffuse:
						file->read(reinterpret_cast<char*>(&diffuses[j]), sizeof(diffuses[j]));
						break;

					case VEU_Specular:
						file->read(reinterpret_cast<char*>(&speculars[j]), sizeof(speculars[j]));
						break;

					case VEU_BlendIndex:
						file->read(reinterpret_cast<char*>(&blend_indices[j]), sizeof(blend_indices[j]) * NumComponents(vertex_elements[k].format));
						break;

					case VEU_BlendWeight:
						file->read(reinterpret_cast<char*>(&blend_weights[j]), sizeof(blend_weights[j]) * NumComponents(vertex_elements[k].format));
						break;

					case VEU_TextureCoord:
						file->read(reinterpret_cast<char*>(&multi_tex_coords[vertex_elements[k].usage_index][j]), sizeof(multi_tex_coords[vertex_elements[k].usage_index][j]));
						break;

					case VEU_Tangent:
						file->read(reinterpret_cast<char*>(&tangents[j]), sizeof(tangents[j]));
						break;

					case VEU_Binormal:
						file->read(reinterpret_cast<char*>(&binormals[j]), sizeof(binormals[j]));
						break;
					}
				}
			}

			uint32_t num_triangles;
			file->read(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));
			StaticMesh::IndicesType indices(num_triangles * 3);
			file->read(reinterpret_cast<char*>(&indices[0]),
				static_cast<std::streamsize>(sizeof(indices[0]) * indices.size()));

			mesh->AssignPositions(positions.begin(), positions.end());
			mesh->AssignNormals(normals.begin(), normals.end());
			mesh->AssignDiffuses(diffuses.begin(), diffuses.end());
			mesh->AssignSpeculars(speculars.begin(), speculars.end());
			mesh->AssignBlendIndices(blend_indices.begin(), blend_indices.end());
			mesh->AssignBlendWeights(blend_weights.begin(), blend_weights.end());
			mesh->AssignMultiTexs(multi_tex_coords.begin(), multi_tex_coords.end());
			mesh->AssignTangents(tangents.begin(), tangents.end());
			mesh->AssignBinormals(binormals.begin(), binormals.end());
			mesh->AssignIndices(indices.begin(), indices.end());

			meshes.push_back(mesh);
		}

		RenderModelPtr ret(new RenderModel(L"KMesh"));
		ret->AssignMeshes(meshes.begin(), meshes.end());
		return ret;
	}
}
