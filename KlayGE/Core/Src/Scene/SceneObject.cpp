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
#include <KlayGE/Math.hpp>
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

	AABBox const & SceneObject::Bound() const
	{
		BOOST_ASSERT(renderable_);
		return renderable_->Bound();
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

	void SceneObject::BindUpdateFunc(boost::function<void(SceneObject&, float, float)> const & update_func)
	{
		update_func_ = update_func;
	}

	void SceneObject::Update(float app_time, float elapsed_time)
	{
		if (update_func_)
		{
			update_func_(*this, app_time, elapsed_time);
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
		return (0 == (attrib_ & SOA_Unvisible));
	}

	void SceneObject::Visible(bool vis)
	{
		if (vis)
		{
			attrib_ &= ~SOA_Unvisible;
		}
		else
		{
			attrib_ |= SOA_Unvisible;
		}
	}

	vertex_elements_type const & SceneObject::InstanceFormat() const
	{
		return instance_format_;
	}

	void const * SceneObject::InstanceData() const
	{
		return NULL;
	}

	void SceneObject::Pass(PassType type)
	{
		renderable_->Pass(type);
		if (attrib_ & SOA_NotCastShadow)
		{
			this->Visible((PT_GenShadowMap != type) && (PT_GenShadowMapWODepthTexture != type) && (PT_GenReflectiveShadowMap != type));
		}
	}

	void SceneObject::LightingTex(TexturePtr const & tex)
	{
		renderable_->LightingTex(tex);
	}
}
