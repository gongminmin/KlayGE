#ifndef _OGLRENDERSETTINGS_HPP
#define _OGLRENDERSETTINGS_HPP

#include <KlayGE/RenderSettings.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	// 建立渲染窗口的设置
	/////////////////////////////////////////////////////////////////////////////////
	struct OGLRenderSettings : public RenderSettings
	{
		OGLRenderSettings()
			{ }
		virtual ~OGLRenderSettings()
			{ }
	};
}

#endif		// _OGLRENDERSETTINGS_HPP
