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

#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>
#pragma warning(push)
#pragma warning(disable: 4127 4189)
#include <boost/function.hpp>
#pragma warning(pop)

#include <KlayGE/ElementFormat.hpp>
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
				depth_stencil_fmt(EF_D16),
				multi_sample(0)
		{
		}

		boost::function<bool (RenderDeviceCaps const &)> ConfirmDevice;

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
