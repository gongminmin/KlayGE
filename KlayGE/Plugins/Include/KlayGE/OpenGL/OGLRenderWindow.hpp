#ifndef _OGLRENDERWINDOW_HPP
#define _OGLRENDERWINDOW_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderWindow.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	struct OGLRenderSettings;

	class OGLRenderWindow : public RenderWindow
	{
	public:
		OGLRenderWindow(std::string const & name, OGLRenderSettings const & settings);
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

		std::wstring const & Description() const;

		void CustomAttribute(std::string const & name, void* pData);

		bool RequiresTextureFlipping() const;

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

	protected:
		std::string	name_;

		HWND	hWnd_;
		HGLRC	hRC_;
		HDC		hDC_;

		bool	active_;			// Is active i.e. visible
		bool	ready_;				// Is ready i.e. available for update
		bool	closed_;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam );

		std::wstring			description_;
	};
}

#endif			// _OGLRENDERWINDOW_HPP
