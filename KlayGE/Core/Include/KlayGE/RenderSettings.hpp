#ifndef _RENDERSETTINGS_HPP
#define _RENDERSETTINGS_HPP

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	// 建立渲染窗口的设置
	/////////////////////////////////////////////////////////////////////////////////
	class RenderSettings
	{
	public:
		RenderSettings()
			: left(0), top(0),
				depthBuffer(true)
			{ }
		virtual ~RenderSettings()
			{ }

		int		width;
		int		height;
		int		colorDepth;
		bool	fullScreen;
		int		left;
		int		top;
		bool	depthBuffer;
	};
}

#endif			// _RENDERSETTINGS_HPP
