#ifndef _D3D9ADAPTERINFO_HPP
#define _D3D9ADAPTERINFO_HPP

#include <d3d9.h>
#include <vector>
#include <KlayGE/D3D9/D3D9VideoMode.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	// 保存适配器的信息，包含该适配器支持的设备列表
	/////////////////////////////////////////////////////////////////////////////////
	class D3D9Adapter
	{
	public:
		D3D9Adapter();
		D3D9Adapter(U32 adapterNo,
			const D3DADAPTER_IDENTIFIER9& d3dadapterIdentifer,
			const D3DDISPLAYMODE& d3ddmDesktop);

		void Enumerate(const COMPtr<IDirect3D9>& d3d);

		// 访问设备描述字符串
		const String Description() const;

		U32 AdapterNo() const
			{ return adapterNo_; }

		const D3DADAPTER_IDENTIFIER9& AdapterIdentifier() const
			{ return d3dAdapterIdentifier_; }

		const D3DDISPLAYMODE& DesktopMode() const
			{ return d3ddmDesktop_; }

		size_t VideoModeNum() const;
		const D3D9VideoMode& VideoMode(size_t index) const;

	private:
		// 这是第几块适配器
		U32			adapterNo_;

		// 适配器信息
		D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier_;
		D3DDISPLAYMODE d3ddmDesktop_;		// 该适配器所用的桌面模式

		// 显示模式列表
		typedef std::vector<D3D9VideoMode, alloc<D3D9VideoMode> > ModeType;
		ModeType modes_;
	};
}

#endif			// _D3D9ADAPTERINFO_HPP