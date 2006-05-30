// RenderEngine.cpp
// KlayGE 渲染引擎类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 统一了RenderState (2006.5.21)
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
#include <KlayGE/GraphicsBuffer.hpp>
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

		void ClearColor(Color const & /*clr*/)
		{
		}
		void Clear(uint32_t /*masks*/)
		{
		}

		RenderWindowPtr CreateRenderWindow(std::string const & /*name*/, RenderSettings const & /*settings*/)
		{
			return RenderWindow::NullObject();
		}

		void SetSampler(uint32_t /*stage*/, SamplerPtr const & /*sampler*/)
		{
		}
		void DisableSampler(uint32_t /*stage*/)
		{
		}

		uint16_t StencilBufferBitDepth()
		{
			return 0;
		}

		void ScissorRect(uint32_t /*x*/, uint32_t /*y*/, uint32_t /*width*/, uint32_t /*height*/)
		{
		}

	private:
		void DoBindRenderTarget(RenderTargetPtr /*rt*/)
		{
		}

		void DoRender(RenderLayout const & /*rl*/)
		{
		}

		void DoFlushRenderStates()
		{
		}

		void InitRenderStates()
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
	void RenderEngine::BindRenderTarget(RenderTargetPtr rt)
	{
		if (!rt)
		{
			cur_render_target_ = default_render_window_;
		}
		else
		{
			cur_render_target_ = rt;
		}

		this->DoBindRenderTarget(cur_render_target_);
	}

	// 获取当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	RenderTargetPtr RenderEngine::CurRenderTarget() const
	{
		return cur_render_target_;
	}

	// 设置渲染特效
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::SetRenderEffect(RenderEffectPtr const & effect)
	{
		renderEffect_ = (!effect) ? RenderEffect::NullObject() : effect;
	}

	// 获取渲染特效
	/////////////////////////////////////////////////////////////////////////////////
	RenderEffectPtr RenderEngine::GetRenderEffect() const
	{
		return renderEffect_;
	}

	// 设置渲染状态
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::SetRenderState(RenderStateType rst, uint32_t state)
	{
		BOOST_ASSERT(rst < render_states_.size());

		if (render_states_[rst] != state)
		{
			render_states_[rst] = state;
			dirty_render_states_[rst] = true;
		}
	}

	// 获取渲染状态
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t RenderEngine::GetRenderState(RenderStateType rst)
	{
		return render_states_[rst];
	}

	// 渲染一个vb
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::Render(RenderLayout const & rl)
	{
		this->DoFlushRenderStates();

		renderEffect_->Begin();
		this->DoRender(rl);
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
