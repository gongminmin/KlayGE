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
		: attrib_(attrib), model_(float4x4::Identity())
	{
	}

	SceneObject::~SceneObject()
	{
	}

	RenderablePtr const & SceneObject::GetRenderable() const
	{
		return renderable_;
	}

	AABBox const & SceneObject::PosBound() const
	{
		BOOST_ASSERT(renderable_);
		return renderable_->PosBound();
	}

	AABBox const & SceneObject::TexcoordBound() const
	{
		BOOST_ASSERT(renderable_);
		return renderable_->TexcoordBound();
	}

	void SceneObject::ModelMatrix(float4x4 const & mat)
	{
		model_ = mat;
		renderable_->ModelMatrix(model_);
	}

	float4x4 const & SceneObject::ModelMatrix() const
	{
		return model_;
	}

	void SceneObject::BindSubThreadUpdateFunc(function<void(SceneObject&, float, float)> const & update_func)
	{
		sub_thread_update_func_ = update_func;
	}

	void SceneObject::BindMainThreadUpdateFunc(function<void(SceneObject&, float, float)> const & update_func)
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

	void SceneObject::MainThreadUpdate(float app_time, float elapsed_time)
	{
		if (main_thread_update_func_)
		{
			main_thread_update_func_(*this, app_time, elapsed_time);
		}
	}

	void SceneObject::AddToSceneManager()
	{
		Context::Instance().SceneManagerInstance().AddSceneObject(this->shared_from_this());
	}

	void SceneObject::DelFromSceneManager()
	{
		Context::Instance().SceneManagerInstance().DelSceneObject(this->shared_from_this());
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
	}

	vertex_elements_type const & SceneObject::InstanceFormat() const
	{
		return instance_format_;
	}

	void const * SceneObject::InstanceData() const
	{
		return nullptr;
	}

	void SceneObject::Pass(PassType type)
	{
		renderable_->Pass(type);
		if (attrib_ & SOA_NotCastShadow)
		{
			this->Visible(PC_ShadowMap != GetPassCategory(type));
		}
	}
}
