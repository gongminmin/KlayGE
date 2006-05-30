#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderTarget.hpp>

#include <KlayGE/RenderWindow.hpp>

namespace KlayGE
{
	class NullRenderWindow : public RenderWindow
	{
	public:
		std::wstring const & Description() const
		{
			static std::wstring const desc(L"Null Render Window");
			return desc;
		}

		void Destroy()
		{
		}

		void Resize(uint32_t /*width*/, uint32_t /*height*/)
		{
		}
		void Reposition(uint32_t /*left*/, uint32_t /*top*/)
		{
		}

		bool Closed() const
		{
			return true;
		}

		void SwapBuffers()
		{
		}

		void DoResize(uint32_t /*width*/, uint32_t /*height*/)
		{
		}

		void DoReposition(uint32_t /*left*/, uint32_t /*top*/)
		{
		}
	};

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderWindow::RenderWindow()
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderWindow::~RenderWindow()
	{
	}

	// 返回空对象
	/////////////////////////////////////////////////////////////////////////////////
	RenderWindowPtr RenderWindow::NullObject()
	{
		static RenderWindowPtr obj(new NullRenderWindow);
		return obj;
	}

	// 获取是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	bool RenderWindow::FullScreen() const
	{
		return isFullScreen_;
	}

	// 改变窗口大小
	/////////////////////////////////////////////////////////////////////////////////
	void RenderWindow::Resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_.width = width;
		viewport_.height = height;

		this->DoResize(width, height);

		App3DFramework& app = Context::Instance().AppInstance();
		app.OnResize(width, height);
	}

	// 改变窗口位置
	/////////////////////////////////////////////////////////////////////////////////
	void RenderWindow::Reposition(uint32_t left, uint32_t top)
	{
		left_ = left;
		top_ = top;
	}
}
