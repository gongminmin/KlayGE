#ifndef _OGLRENDERWINDOW_HPP
#define _OGLRENDERWINDOW_HPP

#include <KlayGE/RenderWindow.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")

namespace KlayGE
{
	struct OGLRenderSettings;

	class OGLRenderWindow : public RenderWindow
	{
	public:
		OGLRenderWindow(const String& name, const OGLRenderSettings& settings);
		~OGLRenderWindow();

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

		void CustomAttribute(const String& name, void* pData);

		bool RequiresTextureFlipping() const;

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

	protected:
		String	name_;

		HWND	hWnd_;
		HGLRC	hRC_;
		HDC		hDC_;

		bool	active_;			// Is active i.e. visible
		bool	ready_;				// Is ready i.e. available for update
		bool	closed_;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam );

		WString			description_;
	};
}

#endif			// _OGLRENDERWINDOW_HPP
