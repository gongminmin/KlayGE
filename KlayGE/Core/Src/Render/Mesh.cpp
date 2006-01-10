// Mesh.cpp
// KlayGE Mesh类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
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

#include <algorithm>
#include <boost/mem_fn.hpp>

#include <KlayGE/Mesh.hpp>

namespace KlayGE
{
	RenderModel::RenderModel(std::wstring const & name)
		: name_(name)
	{
	}

	void RenderModel::AddToRenderQueue()
	{
		std::for_each(meshes_.begin(), meshes_.end(), boost::mem_fn(&StaticMesh::AddToRenderQueue));
	}

	void RenderModel::OnRenderBegin()
	{
		std::for_each(meshes_.begin(), meshes_.end(), boost::mem_fn(&StaticMesh::OnRenderBegin));
	}

	void RenderModel::OnRenderEnd()
	{
		std::for_each(meshes_.begin(), meshes_.end(), boost::mem_fn(&StaticMesh::OnRenderEnd));
	}


	StaticMesh::StaticMesh(std::wstring const & name)
		: name_(name),
			beBuilt_(false)
	{
		rl_ = Context::Instance().RenderFactoryInstance().MakeRenderLayout(RenderLayout::BT_TriangleList);
	}

	StaticMesh::~StaticMesh()
	{
	}

	std::wstring const & StaticMesh::Name() const
	{
		return name_;
	}

	void StaticMesh::AddToRenderQueue()
	{
		this->BuildRenderable();

		if (!xyzs_.empty())
		{
			Renderable::AddToRenderQueue();
		}
	}

	Box StaticMesh::GetBound() const
	{
		return box_;
	}

	void StaticMesh::BuildRenderable()
	{
		if (!beBuilt_)
		{
			if (!xyzs_.empty())
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();

				// 建立顶点坐标
				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static);
				pos_vb->Resize(xyzs_.size() * sizeof(xyzs_[0]));
				{
					GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
					std::copy(xyzs_.begin(), xyzs_.end(), mapper.Pointer<Vector3>());
				}
				rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)));

				if (!normals_.empty())
				{
					// 建立法线坐标
					GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static);
					normal_vb->Resize(normals_.size() * sizeof(normals_[0]));
					{
						GraphicsBuffer::Mapper mapper(*normal_vb, BA_Write_Only);
						std::copy(normals_.begin(), normals_.end(), mapper.Pointer<Vector3>());
					}
					rl_->BindVertexStream(normal_vb, boost::make_tuple(vertex_element(VEU_Normal, 0, sizeof(float), 3)));
				}

				// 建立纹理坐标
				for (size_t i = 0; i < multi_tex_coords_.size(); ++ i)
				{
					GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static);
					tex_vb->Resize(multi_tex_coords_[i].size() * sizeof(multi_tex_coords_[i][0]));
					{
						GraphicsBuffer::Mapper mapper(*tex_vb, BA_Write_Only);
						std::copy(multi_tex_coords_[i].begin(), multi_tex_coords_[i].end(), mapper.Pointer<Vector2>());
					}
					rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_TextureCoord,
						static_cast<uint8_t>(i), sizeof(float), 2)));
				}

				// 建立索引
				GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
				ib->Resize(static_cast<uint32_t>(indices_.size() * sizeof(indices_[0])));
				{
					GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
					std::copy(indices_.begin(), indices_.end(), mapper.Pointer<uint16_t>());
				}
				rl_->BindIndexStream(ib, IF_Index16);
			}

			beBuilt_ = true;
		}
	}


	BoneMesh::~BoneMesh()
	{
	}

	std::wstring const & BoneMesh::Name() const
	{
		static std::wstring name(L"Bone Mesh");
		return name;
	}
}
