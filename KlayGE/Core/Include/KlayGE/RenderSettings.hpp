// RenderEngine.hpp
// KlayGE 渲染引擎类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 加入了ConfirmDevice (2005.7.17)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERSETTINGS_HPP
#define _RENDERSETTINGS_HPP

#include <KlayGE/RenderDeviceCaps.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	// 建立渲染窗口的设置
	/////////////////////////////////////////////////////////////////////////////////
	struct RenderSettings
	{
		RenderSettings()
			: left(0), top(0),
				depthBuffer(true),
				multiSample(0)
		{
		}
		virtual ~RenderSettings()
		{
		}

		virtual bool ConfirmDevice(RenderDeviceCaps const & /*caps*/) const
		{
			return true;
		}

		int		width;
		int		height;
		int		colorDepth;
		bool	fullScreen;
		int		left;
		int		top;
		bool	depthBuffer;
		uint32_t multiSample;
	};
}

#endif			// _RENDERSETTINGS_HPP
