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
#include <KlayGE/RenderWindow.hpp>

#include <KlayGE/RenderEngine.hpp>

namespace KlayGE
{
	class NullRenderEngine : public RenderEngine
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Render Engine");
			return name;
		}

		void StartRendering()
		{
		}

		void BeginFrame()
		{
		}
		void EndFrame()
		{
		}

		void ClearColor(Color const & clr)
		{
		}

		void ShadingType(ShadeOptions so)
		{
		}

		RenderWindowPtr CreateRenderWindow(std::string const & name, RenderSettings const & settings)
		{
			return RenderWindow::NullObject();
		}

		void CullingMode(CullMode mode)
		{
		}
		void PolygonMode(FillMode mode)
		{
		}

		void AlphaBlend(bool enabled)
		{
		}
		void AlphaBlendFunction(AlphaBlendFactor src_factor, AlphaBlendFactor dst_factor)
		{
		}

		void DepthBufferDepthTest(bool enabled)
		{
		}
		void DepthBufferDepthWrite(bool enabled)
		{
		}
		void DepthBufferFunction(CompareFunction depthFunction)
		{
		}
		void DepthBias(uint16_t bias)
		{
		}

		void AlphaTest(bool enabled)
		{
		}
		void AlphaFunction(CompareFunction alphaFunction, float refValue)
		{
		}

		void SetSampler(uint32_t stage, SamplerPtr const & sampler)
		{
		}
		void DisableSampler(uint32_t stage)
		{
		}

		void StencilCheckEnabled(bool enabled)
		{
		}
		bool HasHardwareStencil()
		{
			return false;
		}

		uint16_t StencilBufferBitDepth()
		{
			return 0;
		}

		void StencilBufferFunction(CompareFunction func, uint32_t refValue, uint32_t mask)
		{
		}
		void StencilBufferOperation(StencilOperation fail, StencilOperation depth_fail, StencilOperation pass)
		{
		}

		void PointSpriteEnable(bool enable)
		{
		}
		void PointDistanceAttenuation(float quadratic0, float quadratic1, float quadratic2)
		{
		}
		void PointSize(float size)
		{
		}
		void PointMinMaxSize(float min_size, float max_size)
		{
		}

		void ScissorTest(bool enabled)
		{
		}
		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
		}

	private:
		void DoActiveRenderTarget(uint32_t n, RenderTargetPtr renderTarget)
		{
		}

		void DoRender(VertexBuffer const & vb)
		{
		}

		void FillRenderDeviceCaps()
		{
		}
	};

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderEngine()
		: renderEffect_(RenderEffect::NullObject())
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::~RenderEngine()
	{
	}

	// 返回空对象
	/////////////////////////////////////////////////////////////////////////////////
	RenderEnginePtr RenderEngine::NullObject()
	{
		static RenderEnginePtr obj(new NullRenderEngine);
		return obj;
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

	// 渲染一个vb
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::Render(VertexBuffer const & vb)
	{
		renderEffect_->Begin();
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
