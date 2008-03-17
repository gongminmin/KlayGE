// RenderSettings.hpp
// KlayGE 渲染设置类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2005-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 去掉了ConfirmDevice的参数 (2008.3.17)
//
// 2.8.0
// 加入了ConfirmDevice (2005.7.17)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERSETTINGS_HPP
#define _RENDERSETTINGS_HPP

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <boost/function.hpp>

#include <KlayGE/ElementFormat.hpp>

namespace KlayGE
{
	// 建立渲染窗口的设置
	/////////////////////////////////////////////////////////////////////////////////
	struct RenderSettings
	{
		RenderSettings()
			: left(0), top(0),
				depth_stencil_fmt(EF_D16),
				multi_sample(0)
		{
		}

		boost::function<bool ()> ConfirmDevice;

		bool	full_screen;
		int		left;
		int		top;
		int		width;
		int		height;
		ElementFormat color_fmt;
		ElementFormat depth_stencil_fmt;
		uint32_t multi_sample;
	};
}

#endif			// _RENDERSETTINGS_HPP
