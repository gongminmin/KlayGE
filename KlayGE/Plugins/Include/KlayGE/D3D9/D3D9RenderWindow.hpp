#ifndef _D3D9RENDERWINDOW_HPP
#define _D3D9RENDERWINDOW_HPP

#include <d3d9.h>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/D3D9/D3D9Adapter.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	// 建立渲染窗口的设置
	/////////////////////////////////////////////////////////////////////////////////
	struct D3D9RenderWindowSettings : public RenderWindowSettings
	{
		D3D9RenderWindowSettings()
			: multiSampleQuality(0)
			{ }
		virtual ~D3D9RenderWindowSettings()
			{ }

		U32 multiSampleQuality;

		bool ConfirmDevice(const D3DCAPS9& caps, U32 behavior, D3DFORMAT format) const
		{
			if (caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
			{
				return false;
			}
			return this->DoConfirmDevice(caps, behavior, format);
		}

	private:
		virtual bool DoConfirmDevice(const D3DCAPS9& /*caps*/, U32 /*behavior*/, D3DFORMAT /*format*/) const
			{ return true; }
	};

	class D3D9RenderWindow : public RenderWindow
	{
	public:
		D3D9RenderWindow(const COMPtr<IDirect3D9>& d3d, const D3D9Adapter& adapter,
			const String& name, const D3D9RenderWindowSettings& settings);
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

		const WString& Description() const;

		const D3D9Adapter& Adapter() const;
		const COMPtr<IDirect3DDevice9>& D3DDevice() const;

		void CustomAttribute(const String& name, void* pData);

		bool RequiresTextureFlipping() const;

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

	protected:
		String	name_;

		HWND	hWnd_;				// Win32 Window handle
		bool	active_;			// Is active i.e. visible
		bool	ready_;				// Is ready i.e. available for update
		bool	closed_;

		U32		multiSampleQuality_;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam );

		// -------------------------------------------------------
		// DirectX-specific
		// -------------------------------------------------------

		D3D9Adapter					adapter_;

		// Pointer to the 3D device specific for this window
		COMPtr<IDirect3D9>			d3d_;
		COMPtr<IDirect3DDevice9>	d3dDevice_;
		D3DPRESENT_PARAMETERS		d3dpp_;
		
		COMPtr<IDirect3DSurface9>	renderSurface_;
		COMPtr<IDirect3DSurface9>	renderZBuffer_;

		WString		description_;
	};
}

#endif			// _D3D9RENDERWINDOW_HPP
