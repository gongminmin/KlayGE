// RenderEngine.cpp
// KlayGE 渲染引擎类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 简化了StencilBuffer相关操作 (2005.7.20)
//
// 2.7.1
// ViewMatrix和ProjectionMatrix改为const (2005.7.10)
//
// 2.4.0
// 增加了NumPrimitivesJustRendered和NumVerticesJustRendered (2005.3.21)
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
#include <KlayGE/VertexBuffer.hpp>
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

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::ActiveRenderTarget(uint32_t n, RenderTargetPtr renderTarget)
	{
		BOOST_ASSERT(n < renderTargets_.size());

		renderTargets_[n] = renderTarget;
		this->DoActiveRenderTarget(n, renderTarget);
	}

	// 获取当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	RenderTargetPtr RenderEngine::ActiveRenderTarget(uint32_t n) const
	{
		BOOST_ASSERT(n < renderTargets_.size());

		return renderTargets_[n];
	}

	// 设置渲染状态
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::SetRenderEffect(RenderEffectPtr const & effect)
	{
		renderEffect_ = (!effect) ? RenderEffect::NullObject() : effect;
	}

	// 获取渲染状态
	/////////////////////////////////////////////////////////////////////////////////
	RenderEffectPtr RenderEngine::GetRenderEffect() const
	{
		return renderEffect_;
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
	Matrix4 RenderEngine::ViewMatrix() const
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
	Matrix4 RenderEngine::ProjectionMatrix() const
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

	// 渲染一个vb
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::Render(VertexBuffer const & vb)
	{
		renderPasses_ = renderEffect_->Begin();
		this->DoRender(vb);
		renderEffect_->End();
	}

	// 上次Render()所渲染的图元数
	/////////////////////////////////////////////////////////////////////////////////
	size_t RenderEngine::NumPrimitivesJustRendered()
	{
		size_t const ret = numPrimitivesJustRendered_;
		numPrimitivesJustRendered_ = 0;
		return ret;
	}

	// 上次Render()所渲染的顶点数
	/////////////////////////////////////////////////////////////////////////////////
	size_t RenderEngine::NumVerticesJustRendered()
	{
		size_t const ret = numVerticesJustRendered_;
		numVerticesJustRendered_ = 0;
		return ret;
	}

	// 获取渲染设备能力
	/////////////////////////////////////////////////////////////////////////////////
	RenderDeviceCaps const & RenderEngine::DeviceCaps() const
	{
		return caps_;
	}
}
