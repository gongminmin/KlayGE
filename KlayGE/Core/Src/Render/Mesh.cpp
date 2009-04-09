// Mesh.cpp
// KlayGE Mesh类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2004-2006
// Homepage: http://klayge.sourceforge.net
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

#include <algorithm>
#include <boost/mem_fn.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

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

	void RenderModel::UpdateBoundBox()
	{
		box_ = Box(float3(0, 0, 0), float3(0, 0, 0));
		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::const_reference mesh, meshes_)
		{
			box_ |= mesh->GetBound();
		}
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

	std::wstring const & StaticMesh::Name() const
	{
		return name_;
	}

	Box const & StaticMesh::GetBound() const
	{
		return box_;
	}

	void StaticMesh::AddVertexStream(void const * buf, uint32_t size, vertex_element const & ve, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		ElementInitData init_data;
		init_data.data = buf;
		init_data.row_pitch = size;
		init_data.slice_pitch = 0;
		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, access_hint, &init_data);
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

	void StaticMesh::AddIndexStream(void const * buf, uint32_t size, ElementFormat format, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		ElementInitData init_data;
		init_data.data = buf;
		init_data.row_pitch = size;
		init_data.slice_pitch = 0;
		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, access_hint, &init_data);
		rl_->BindIndexStream(ib, format);
	}


	SkinnedModel::SkinnedModel(std::wstring const & name)
		: RenderModel(name),
			last_frame_(0xFFFFFFFF)
	{
	}
	
	void SkinnedModel::BuildBones(uint32_t frame)
	{
		BOOST_FOREACH(BOOST_TYPEOF(joints_)::reference joint, joints_)
		{
			KeyFrames const & kf = key_frames_->find(joint.name)->second;
			float3 const & key_pos = kf.FramePos(frame);
			Quaternion const & key_quat = kf.FrameQuat(frame);

			if (joint.parent != -1)
			{
				Joint const & parent(joints_[joint.parent]);

				joint.bind_quat = key_quat * parent.bind_quat;
				joint.bind_pos = MathLib::transform_quat(key_pos, parent.bind_quat) + parent.bind_pos;
			}
			else
			{
				joint.bind_quat = key_quat;
				joint.bind_pos = key_pos;
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
			bind_poss_[i].w() = 1;
		}
	}

	void SkinnedModel::AttachKeyFrames(boost::shared_ptr<KlayGE::KeyFramesType> const & key_frames)
	{
		key_frames_ = key_frames;
	}

	uint32_t SkinnedModel::GetFrame() const
	{
		return last_frame_;
	}

	void SkinnedModel::SetFrame(uint32_t frame)
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
		for (size_t i = 0; i < bind_rots_.size(); ++ i)
		{
			bind_rots_[i] = float4(0, 0, 0, 1);
			bind_poss_[i] = float4(0, 0, 0, 1);
		}
	}


	SkinnedMesh::SkinnedMesh(RenderModelPtr const & model, std::wstring const & name)
		: StaticMesh(model, name)
	{
	}
}
