// D3D9RenderWindow.hpp
// KlayGE D3D9渲染窗口类 头文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 增加了ResetDevice (2005.7.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDERWINDOW_HPP
#define _D3D9RENDERWINDOW_HPP

#include <d3d9.h>
#include <boost/smart_ptr.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/D3D9/D3D9Adapter.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	struct RenderSettings;

	class D3D9RenderWindow : public RenderWindow
	{
	public:
		D3D9RenderWindow(ID3D9Ptr const & d3d, D3D9Adapter const & adapter,
			std::string const & name, RenderSettings const & settings);
		~D3D9RenderWindow();

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void Destroy();

		bool Closed() const;

		bool Ready() const;
		void Ready(bool ready);

		void SwapBuffers();

		HWND WindowHandle() const;

		std::wstring const & Description() const;

		D3D9Adapter const & Adapter() const;
		ID3D9DevicePtr D3DDevice() const;
		ID3D9SurfacePtr D3DRenderSurface(uint32_t n) const;
		ID3D9SurfacePtr D3DRenderZBuffer() const;

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

	private:
		void UpdateSurfacesPtrs();
		void ResetDevice();

		void DoReposition(uint32_t left, uint32_t top);
		void DoResize(uint32_t width, uint32_t height);

	private:
		std::string	name_;

		HWND	hWnd_;				// Win32 Window handle
		bool	ready_;				// Is ready i.e. available for update
		bool	closed_;

		D3DMULTISAMPLE_TYPE multiSample_;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam );


		D3D9Adapter				adapter_;

		// Pointer to the 3D device specific for this window
		ID3D9Ptr				d3d_;
		ID3D9DevicePtr			d3dDevice_;
		D3DPRESENT_PARAMETERS	d3dpp_;
		ID3D9SwapChainPtr		d3d_swap_chain_;
		bool					main_wnd_;

		ID3D9SurfacePtr			renderSurface_;
		ID3D9SurfacePtr			renderZBuffer_;

		std::wstring			description_;
	};

	typedef boost::shared_ptr<D3D9RenderWindow> D3D9RenderWindowPtr;
}

#endif			// _D3D9RENDERWINDOW_HPP
