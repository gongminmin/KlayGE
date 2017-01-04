// SceneObject.cpp
// KlayGE 场景对象类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2003-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 增加了Overlay标志 (2009.5.13)
// 增加了Update (2009.5.14)
//
// 3.1.0
// 初次建立 (2005.10.31)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Renderable.hpp>

#include <boost/assert.hpp>

#include <KlayGE/SceneObject.hpp>

namespace KlayGE
{
	SceneObject::SceneObject(uint32_t attrib)
		: attrib_(attrib), parent_(nullptr), renderable_hw_res_ready_(false),
			model_(float4x4::Identity()), abs_model_(float4x4::Identity()),
			visible_mark_(BO_No)
	{
		if (!(attrib & SOA_Overlay) && (attrib & (SOA_Cullable | SOA_Moveable)))
		{
			pos_aabb_ws_ = MakeUniquePtr<AABBox>();
		}
	}

	SceneObject::~SceneObject()
	{
	}

	SceneObject* SceneObject::Parent() const
	{
		return parent_;
	}

	void SceneObject::Parent(SceneObject* so)
	{
		parent_ = so;
	}

	uint32_t SceneObject::NumChildren() const
	{
		return static_cast<uint32_t>(children_.size());
	}

	const SceneObjectPtr& SceneObject::Child(uint32_t index) const
	{
		BOOST_ASSERT(index < children_.size());
		return children_[index];
	}

	RenderablePtr const & SceneObject::GetRenderable() const
	{
		return renderable_;
	}

	void SceneObject::ModelMatrix(float4x4 const & mat)
	{
		model_ = mat;
	}

	float4x4 const & SceneObject::ModelMatrix() const
	{
		return model_;
	}

	float4x4 const & SceneObject::AbsModelMatrix() const
	{
		return abs_model_;
	}

	AABBox const & SceneObject::PosBoundWS() const
	{
		return *pos_aabb_ws_;
	}

	void SceneObject::UpdateAbsModelMatrix()
	{
		if (parent_)
		{
			abs_model_ = parent_->ModelMatrix() * model_;
		}
		else
		{
			abs_model_ = model_;
		}

		if (renderable_)
		{
			if (pos_aabb_ws_)
			{
				*pos_aabb_ws_ = MathLib::transform_aabb(renderable_->PosBound(), abs_model_);
			}

			renderable_->ModelMatrix(abs_model_);
		}
	}

	void SceneObject::VisibleMark(BoundOverlap vm)
	{
		visible_mark_ = vm;
	}

	BoundOverlap SceneObject::VisibleMark() const
	{
		return visible_mark_;
	}

	void SceneObject::BindSubThreadUpdateFunc(std::function<void(SceneObject&, float, float)> const & update_func)
	{
		sub_thread_update_func_ = update_func;
	}

	void SceneObject::BindMainThreadUpdateFunc(std::function<void(SceneObject&, float, float)> const & update_func)
	{
		main_thread_update_func_ = update_func;
	}

	void SceneObject::SubThreadUpdate(float app_time, float elapsed_time)
	{
		if (sub_thread_update_func_)
		{
			sub_thread_update_func_(*this, app_time, elapsed_time);
		}
	}

	bool SceneObject::MainThreadUpdate(float app_time, float elapsed_time)
	{
		bool refreshed = false;
		if (renderable_ && !renderable_hw_res_ready_ && renderable_->HWResourceReady())
		{
			this->OnAttachRenderable(false);
			this->UpdateAbsModelMatrix();
			refreshed = true;
			renderable_hw_res_ready_ = true;
		}

		if (main_thread_update_func_)
		{
			main_thread_update_func_(*this, app_time, elapsed_time);
		}

		return refreshed;
	}

	void SceneObject::AddToSceneManager()
	{
		Context::Instance().SceneManagerInstance().AddSceneObject(this->shared_from_this());
		for (auto const & child : children_)
		{
			child->AddToSceneManager();
		}
	}

	void SceneObject::AddToSceneManagerLocked()
	{
		Context::Instance().SceneManagerInstance().AddSceneObjectLocked(this->shared_from_this());
		for (auto const & child : children_)
		{
			child->AddToSceneManagerLocked();
		}
	}

	void SceneObject::DelFromSceneManager()
	{
		for (auto const & child : children_)
		{
			child->DelFromSceneManager();
		}
		Context::Instance().SceneManagerInstance().DelSceneObject(this->shared_from_this());
	}

	void SceneObject::DelFromSceneManagerLocked()
	{
		for (auto const & child : children_)
		{
			child->DelFromSceneManagerLocked();
		}
		Context::Instance().SceneManagerInstance().DelSceneObjectLocked(this->shared_from_this());
	}

	uint32_t SceneObject::Attrib() const
	{
		return attrib_;
	}

	bool SceneObject::Visible() const
	{
		return (0 == (attrib_ & SOA_Invisible));
	}

	void SceneObject::Visible(bool vis)
	{
		if (vis)
		{
			attrib_ &= ~SOA_Invisible;
		}
		else
		{
			attrib_ |= SOA_Invisible;
		}

		for (auto const & child : children_)
		{
			child->Visible(vis);
		}
	}

	vertex_elements_type const & SceneObject::InstanceFormat() const
	{
		return instance_format_;
	}

	void const * SceneObject::InstanceData() const
	{
		return nullptr;
	}

	void SceneObject::SelectMode(bool select_mode)
	{
		if (renderable_)
		{
			renderable_->SelectMode(select_mode);
		}
	}

	void SceneObject::ObjectID(uint32_t id)
	{
		if (renderable_)
		{
			renderable_->ObjectID(id);
		}
	}

	bool SceneObject::SelectMode() const
	{
		if (renderable_)
		{
			return renderable_->SelectMode();
		}
		else
		{
			return false;
		}
	}

	void SceneObject::Pass(PassType type)
	{
		if (renderable_)
		{
			renderable_->Pass(type);
			if (attrib_ & SOA_NotCastShadow)
			{
				this->Visible(PC_ShadowMap != GetPassCategory(type));
			}
		}
	}

	bool SceneObject::TransparencyBackFace() const
	{
		if (renderable_)
		{
			return renderable_->TransparencyBackFace();
		}
		else
		{
			return false;
		}
	}

	bool SceneObject::TransparencyFrontFace() const
	{
		if (renderable_)
		{
			return renderable_->TransparencyFrontFace();
		}
		else
		{
			return false;
		}
	}

	bool SceneObject::SSS() const
	{
		if (renderable_)
		{
			return renderable_->SSS();
		}
		else
		{
			return false;
		}
	}

	bool SceneObject::Reflection() const
	{
		if (renderable_)
		{
			return renderable_->Reflection();
		}
		else
		{
			return false;
		}
	}

	bool SceneObject::SimpleForward() const
	{
		if (renderable_)
		{
			return renderable_->SimpleForward();
		}
		else
		{
			return false;
		}
	}

	bool SceneObject::VDM() const
	{
		if (renderable_)
		{
			return renderable_->VDM();
		}
		else
		{
			return false;
		}
	}

	void SceneObject::OnAttachRenderable(bool add_to_scene)
	{
		if (add_to_scene)
		{
			this->AddToSceneManagerLocked();
		}
	}
}
