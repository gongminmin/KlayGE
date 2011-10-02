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
		: attrib_(attrib)
	{
	}

	SceneObject::~SceneObject()
	{
	}

	RenderablePtr const & SceneObject::GetRenderable() const
	{
		return renderable_;
	}

	Box const & SceneObject::GetBound() const
	{
		BOOST_ASSERT(renderable_);
		return renderable_->GetBound();
	}

	float4x4 const & SceneObject::GetModelMatrix() const
	{
		static float4x4 iden = float4x4::Identity();
		return iden;
	}

	void SceneObject::Update()
	{
	}

	void SceneObject::AddToSceneManager()
	{
		Context::Instance().SceneManagerInstance().AddSceneObject(this->shared_from_this());
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

		if (PT_SpecialShading == type)
		{
			if (this->Visible())
			{
				this->Visible(renderable_->SpecialShading());
			}
		}
	}

	void SceneObject::LightingTex(TexturePtr const & tex)
	{
		renderable_->LightingTex(tex);
	}
}
