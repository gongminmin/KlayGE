#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>

#include <cassert>
#include <vector>

#include <KlayGE/D3D9/D3D9AdapterList.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D9AdapterList::D3D9AdapterList()
						: currentAdapter_(0)
	{
	}

	// 获取系统显卡数目
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D9AdapterList::NumAdapter() const
	{
		return adapters_.size();
	}

	// 获取显卡
	/////////////////////////////////////////////////////////////////////////////////
	const D3D9Adapter& D3D9AdapterList::Adapter(size_t index) const
	{
		assert(index < adapters_.size());

		return adapters_[index];
	}

	// 获取当前显卡索引
	/////////////////////////////////////////////////////////////////////////////////
	U32 D3D9AdapterList::CurrentAdapterIndex() const
	{
		return currentAdapter_;
	}

	// 设置当前显卡索引
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9AdapterList::CurrentAdapterIndex(U32 index)
	{
		currentAdapter_ = index;
	}

	// 枚举系统显卡
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9AdapterList::Enumerate(const COMPtr<IDirect3D9>& d3d)
	{
		// 枚举系统中的适配器 (通常只有一个，除非有几块显卡)
		for (U32 i = 0; i < d3d->GetAdapterCount(); ++ i)
		{
			D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier;
			D3DDISPLAYMODE d3ddmDesktop;

			// 填充适配器信息
			d3d->GetAdapterIdentifier(i, 0, &d3dAdapterIdentifier);
			d3d->GetAdapterDisplayMode(i, &d3ddmDesktop);

			D3D9Adapter adapter(i, d3dAdapterIdentifier, d3ddmDesktop);
			adapter.Enumerate(d3d);

			// 如果发现有效的设备则使用该适配器
			if (adapter.NumVideoMode() != 0)
			{
				adapters_.push_back(adapter);
			}
		}

		// 如果没有找到兼容的设备则抛出错误
		if (adapters_.empty())
		{
			THR(E_FAIL);
		}
	}
}