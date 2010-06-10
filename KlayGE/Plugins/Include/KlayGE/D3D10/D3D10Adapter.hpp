// D3D10Adapter.hpp
// KlayGE D3D10适配器 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10ADAPTER_HPP
#define _D3D10ADAPTER_HPP

#pragma once

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>
#include <vector>
#include <string>
#include <KlayGE/D3D10/D3D10Typedefs.hpp>
#include <KlayGE/D3D10/D3D10VideoMode.hpp>

namespace KlayGE
{
	// 保存适配器的信息，包含该适配器支持的设备列表
	/////////////////////////////////////////////////////////////////////////////////
	class D3D10Adapter
	{
	public:
		D3D10Adapter();
		D3D10Adapter(uint32_t adapter_no, IDXGIAdapterPtr const & adapter);

		void Enumerate();

		// 访问设备描述字符串
		std::wstring const Description() const;
		void ResetAdapter(IDXGIAdapterPtr const & ada);

		uint32_t AdapterNo() const
		{
			return adapter_no_;
		}

		IDXGIAdapterPtr const & Adapter() const
		{
			return adapter_;
		}

		DXGI_FORMAT DesktopFormat() const
		{
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		size_t NumVideoMode() const;
		D3D10VideoMode const & VideoMode(size_t index) const;

	private:
		// 这是第几块适配器
		uint32_t			adapter_no_;

		// 适配器信息
		IDXGIAdapterPtr		adapter_;
		DXGI_ADAPTER_DESC	adapter_desc_;

		// 显示模式列表
		typedef std::vector<D3D10VideoMode> ModeType;
		ModeType modes_;
	};

	typedef boost::shared_ptr<D3D10Adapter> D3D10AdapterPtr;
}

#endif			// _D3D10ADAPTER_HPP
