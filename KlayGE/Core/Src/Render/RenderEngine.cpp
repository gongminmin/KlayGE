// RenderEngine.cpp
// KlayGE 渲染引擎类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 优化了RenderEffect的设置 (2004.2.16)
// 去掉了VO_2D (2004.3.1)
// 去掉了SoftwareBlend (2004.3.10)
//
// 2.0.0
// 初次建立(2003.10.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/RenderBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/RenderEngine.hpp>

namespace KlayGE
{
	RenderEngine::RenderEngine()
		: renderEffect_(RenderEffect::NullObject()),
			renderPasses_(1),
			worldMat_(Matrix4::Identity()),
			viewMat_(Matrix4::Identity()),
			projMat_(Matrix4::Identity())
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::~RenderEngine()
	{
	}

	// 增加渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderTargetListIterator RenderEngine::AddRenderTarget(RenderTargetPtr const & target)
	{
		renderTargetList_.push_back(target);
		RenderTargetListIterator iter(renderTargetList_.end());
		-- iter;

		return iter;
	}

	// 渲染目标列表的Begin迭代器
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderTargetListIterator RenderEngine::RenderTargetListBegin()
	{
		return renderTargetList_.begin();
	}

	// 显示目标列表的End迭代器
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderTargetListIterator RenderEngine::RenderTargetListEnd()
	{
		return renderTargetList_.end();
	}

	// 从渲染目标列表中删除渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	RenderTargetPtr RenderEngine::RemoveRenderTarget(RenderTargetListIterator iter)
	{
		RenderTargetPtr ret(*iter);
		renderTargetList_.erase(iter);
		return ret;
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::ActiveRenderTarget(RenderTargetListIterator iter)
	{
		activeRenderTarget_ = iter;
	}

	// 获取当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderTargetListIterator const & RenderEngine::ActiveRenderTarget() const
	{
		return activeRenderTarget_;
	}

	// 设置渲染状态
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::SetRenderEffect(RenderEffectPtr const & effect)
	{
		if (renderEffect_ != effect)
		{
			renderEffect_->End();
			renderEffect_ = (!effect) ? RenderEffect::NullObject() : effect;
			renderPasses_ = renderEffect_->Begin();
		}
	}

	// 获取渲染状态
	/////////////////////////////////////////////////////////////////////////////////
	RenderEffectPtr RenderEngine::GetRenderEffect() const
	{
		return renderEffect_;
	}
	

	// 获取最大坐标数
	/////////////////////////////////////////////////////////////////////////////////
	U32 RenderEngine::MaxVertexBlendMatrices()
	{
		// TODO: implement vertex blending support in DX8 & possibly GL_ARB_VERTEX_BLEND (in subclasses)
		return 1;
	}

	// 获取世界矩阵
	/////////////////////////////////////////////////////////////////////////////////
	Matrix4 RenderEngine::WorldMatrix() const
	{
		return worldMat_;
	}

	// 设置世界矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::WorldMatrix(Matrix4 const & mat)
	{
		worldMat_ = mat;
		this->DoWorldMatrix();
	}

	// 获取观察矩阵
	/////////////////////////////////////////////////////////////////////////////////
	Matrix4 RenderEngine::ViewMatrix()
	{
		return viewMat_;
	}

	// 设置观察矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::ViewMatrix(Matrix4 const & mat)
	{
		viewMat_ = mat;
		this->DoViewMatrix();
	}

	// 获取投射矩阵
	/////////////////////////////////////////////////////////////////////////////////
	Matrix4 RenderEngine::ProjectionMatrix()
	{
		return projMat_;
	}

	// 设置投射矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::ProjectionMatrix(Matrix4 const & mat)
	{
		projMat_ = mat;
		this->DoProjectionMatrix();
	}
}
