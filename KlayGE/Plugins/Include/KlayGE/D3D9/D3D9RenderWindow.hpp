#ifndef _D3D9RENDERWINDOW_HPP
#define _D3D9RENDERWINDOW_HPP

#include <d3d9.h>
#include <boost/smart_ptr.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/D3D9/D3D9Adapter.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	struct D3D9RenderSettings;

	class D3D9RenderWindow : public RenderWindow
	{
	public:
		D3D9RenderWindow(boost::shared_ptr<IDirect3D9> const & d3d, D3D9Adapter const & adapter,
			std::string const & name, D3D9RenderSettings const & settings);
		~D3D9RenderWindow();

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void Destroy();

		bool Active() const;

		bool Closed() const;

		bool Ready() const;
		void Ready(bool ready);

		void Reposition(int left, int top);
		void Resize(int width, int height);
		void SwapBuffers();

		HWND WindowHandle() const;

		std::wstring const & Description() const;

		D3D9Adapter const & Adapter() const;
		boost::shared_ptr<IDirect3DDevice9> const & D3DDevice() const;

		void CustomAttribute(std::string const & name, void* pData);

		bool RequiresTextureFlipping() const;

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

	protected:
		std::string	name_;

		HWND	hWnd_;				// Win32 Window handle
		bool	active_;			// Is active i.e. visible
		bool	ready_;				// Is ready i.e. available for update
		bool	closed_;

		D3DMULTISAMPLE_TYPE multiSample_;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam );


		D3D9Adapter					adapter_;

		// Pointer to the 3D device specific for this window
		boost::shared_ptr<IDirect3D9>			d3d_;
		boost::shared_ptr<IDirect3DDevice9>		d3dDevice_;
		D3DPRESENT_PARAMETERS					d3dpp_;
		
		boost::shared_ptr<IDirect3DSurface9>	renderSurface_;
		boost::shared_ptr<IDirect3DSurface9>	renderZBuffer_;

		std::wstring		description_;
	};
}

#endif			// _D3D9RENDERWINDOW_HPP
