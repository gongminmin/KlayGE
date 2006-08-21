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
#include <boost/tuple/tuple.hpp>

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
		: name_(name)
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

	Box StaticMesh::GetBound() const
	{
		return box_;
	}

	void StaticMesh::AddVertexStream(void const * buf, uint32_t size, vertex_element const & ve)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static);
		vb->Resize(size);
		{
			GraphicsBuffer::Mapper mapper(*vb, BA_Write_Only);
			std::copy(static_cast<uint8_t const *>(buf), static_cast<uint8_t const *>(buf) + size,
				mapper.Pointer<uint8_t>());
		}
		rl_->BindVertexStream(vb, boost::make_tuple(ve));

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
	}

	void StaticMesh::AddIndexStream(void const * buf, uint32_t size, ElementFormat format)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
		ib->Resize(size);
		{
			GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
			std::copy(static_cast<uint8_t const *>(buf), static_cast<uint8_t const *>(buf) + size,
				mapper.Pointer<uint8_t>());
		}
		rl_->BindIndexStream(ib, format);
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
			float3 const & key_pos = kf.FramePos(frame);
			Quaternion const & key_quat = kf.FrameQuat(frame);

			if (joint.parent != -1)
			{
				Joint& parent(joints_[joint.parent]);

				joint.bind_quat = key_quat * parent.bind_quat;
				joint.bind_pos = MathLib::transform_quat(key_pos, parent.bind_quat) + parent.bind_pos;
			}
			else
			{
				joint.bind_quat = key_quat;
				joint.bind_pos = MathLib::transform_quat(key_pos, key_quat);
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
			float3 pos = MathLib::transform_quat(joints_[i].inverse_origin_pos, joints_[i].bind_quat) + joints_[i].bind_pos;
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
}
