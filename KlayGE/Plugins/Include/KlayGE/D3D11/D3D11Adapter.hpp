// D3D11Adapter.hpp
// KlayGE D3D11适配器 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11ADAPTER_HPP
#define _D3D11ADAPTER_HPP

#pragma once

#include <vector>
#include <string>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11VideoMode.hpp>

namespace KlayGE
{
	// 保存适配器的信息，包含该适配器支持的设备列表
	/////////////////////////////////////////////////////////////////////////////////
	class D3D11Adapter
	{
	public:
		D3D11Adapter(uint32_t adapter_no, IDXGIAdapter1Ptr const & adapter);

		void Enumerate();

		// 访问设备描述字符串
		std::wstring const Description() const;
		void ResetAdapter(IDXGIAdapter1Ptr const & ada);

		uint32_t AdapterNo() const
		{
			return adapter_no_;
		}

		IDXGIAdapter1Ptr const & DXGIAdapter() const
		{
			return adapter_;
		}

		DXGI_FORMAT DesktopFormat() const
		{
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		size_t NumVideoMode() const;
		D3D11VideoMode const & VideoMode(size_t index) const;

	private:
		// 这是第几块适配器
		uint32_t			adapter_no_;

		// 适配器信息
		IDXGIAdapter1Ptr adapter_;
		DXGI_ADAPTER_DESC1 adapter_desc_{};

		// 显示模式列表
		std::vector<D3D11VideoMode> modes_;
	};

	typedef std::shared_ptr<D3D11Adapter> D3D11AdapterPtr;
}

#endif			// _D3D11ADAPTER_HPP
