// Mesh.cpp
// KlayGE Mesh类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2004-2006
// Homepage: http://klayge.sourceforge.net
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
				pos_vb->Resize(static_cast<uint32_t>(xyzs_.size() * sizeof(xyzs_[0])));
				{
					GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
					std::copy(xyzs_.begin(), xyzs_.end(), mapper.Pointer<Vector3>());
				}
				rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, sizeof(float), 3)));

				if (!normals_.empty())
				{
					// 建立法线坐标
					GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static);
					normal_vb->Resize(static_cast<uint32_t>(normals_.size() * sizeof(normals_[0])));
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
					tex_vb->Resize(static_cast<uint32_t>(multi_tex_coords_[i].size() * sizeof(multi_tex_coords_[i][0])));
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


	SkinnedModel::SkinnedModel(std::wstring const & name)
		: RenderModel(name),
			last_frame_(-1)
	{
	}
	
	void SkinnedModel::BuildBones(int frame)
	{
		for (JointsType::iterator iter = joints_.begin(); iter != joints_.end(); ++ iter)
		{
			Joint& joint(*iter);

			KeyFrames const & kf = key_frames_->find(joint.name)->second;
			Vector3 const & key_pos = kf.FramePos(frame);
			Quaternion const & key_quat = kf.FrameQuat(frame);

			if (joint.parent != -1)
			{
				Joint& parent(joints_[joint.parent]);

				joint.bind_quat = key_quat * parent.bind_quat;
				joint.bind_pos = MathLib::TransQuat(key_pos, parent.bind_quat) + parent.bind_pos;
			}
			else
			{
				joint.bind_quat = key_quat;
				joint.bind_pos = MathLib::TransQuat(key_pos, key_quat);
			}
		}

		this->UpdateBinds();
	}

	void SkinnedModel::UpdateBinds()
	{
		bind_rots_.resize(joints_.size());
		bind_poss_.resize(joints_.size());
		for (size_t i = 0; i < joints_.size(); ++ i)
		{
			Quaternion quat = joints_[i].inverse_origin_quat * joints_[i].bind_quat;
			Vector3 pos = MathLib::TransQuat(joints_[i].inverse_origin_pos, joints_[i].bind_quat) + joints_[i].bind_pos;
			bind_rots_[i].x() = quat.x();
			bind_rots_[i].y() = quat.y();
			bind_rots_[i].z() = quat.z();
			bind_rots_[i].w() = quat.w();
			bind_poss_[i].x() = pos.x();
			bind_poss_[i].y() = pos.y();
			bind_poss_[i].z() = pos.z();
		}
	}

	void SkinnedModel::AttachKeyFrames(boost::shared_ptr<KlayGE::KeyFramesType> const & key_frames)
	{
		key_frames_ = key_frames;
	}

	void SkinnedModel::SetFrame(int frame)
	{
		if (last_frame_ != frame)
		{
			last_frame_ = frame;

			this->BuildBones(frame);
		}
	}


	SkinnedMesh::SkinnedMesh(std::wstring const & name)
		: StaticMesh(name)
	{
	}

	void SkinnedMesh::BuildRenderable()
	{
		if (!beBuilt_)
		{
			// 填充混合信息
			GraphicsBufferPtr bw = Context::Instance().RenderFactoryInstance().MakeVertexBuffer(BU_Static);
			bw->Resize(static_cast<uint32_t>(blend_weights_.size() * sizeof(blend_weights_[0])));
			{
				GraphicsBuffer::Mapper mapper(*bw, BA_Write_Only);
				std::copy(blend_weights_.begin(), blend_weights_.end(), mapper.Pointer<float>());
			}
			rl_->BindVertexStream(bw, boost::make_tuple(vertex_element(VEU_BlendWeight, 0, sizeof(float), 4)));
			GraphicsBufferPtr bi = Context::Instance().RenderFactoryInstance().MakeVertexBuffer(BU_Static);
			bi->Resize(static_cast<uint32_t>(blend_indices_.size() * sizeof(blend_indices_[0])));
			{
				GraphicsBuffer::Mapper mapper(*bi, BA_Write_Only);
				std::copy(blend_indices_.begin(), blend_indices_.end(), mapper.Pointer<uint8_t>());
			}
			rl_->BindVertexStream(bi, boost::make_tuple(vertex_element(VEU_BlendIndex, 0, sizeof(uint8_t), 4)));
		}

		StaticMesh::BuildRenderable();
	}
}
