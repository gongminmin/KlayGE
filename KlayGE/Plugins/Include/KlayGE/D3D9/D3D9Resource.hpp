// D3D9Resource.hpp
// KlayGE D3D9资源类 头文件
// Ver 2.3.1
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.3.1
// 初次建立 (2005.3.2)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RESOURCE_HPP
#define _D3D9RESOURCE_HPP

#include <boost/smart_ptr.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9Resource
	{
	public:
		virtual ~D3D9Resource()
		{
		}

		virtual void OnLostDevice() = 0;
		virtual void OnResetDevice() = 0;
	};

	typedef boost::shared_ptr<D3D9Resource> D3D9ResourcePtr;
}

#endif			// _D3D9RESOURCE_HPP
