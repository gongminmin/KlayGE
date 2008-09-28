// D3D10Adapter.cpp
// KlayGE D3D10适配器 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <boost/smart_ptr.hpp>

#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/D3D10/D3D10Adapter.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D10Adapter::D3D10Adapter()
					: adapter_no_(0)
	{
	}

	D3D10Adapter::D3D10Adapter(uint32_t adapter_no, IDXGIAdapterPtr const & adapter)
					: adapter_no_(adapter_no),
						adapter_(adapter)
	{
		adapter_->GetDesc(&adapter_desc_);
	}

	// 获取设备描述字符串
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const D3D10Adapter::Description() const
	{
		return std::wstring(adapter_desc_.Description);
	}

	// 获取支持的显示模式数目
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D10Adapter::NumVideoMode() const
	{
		return modes_.size();
	}

	// 获取显示模式
	/////////////////////////////////////////////////////////////////////////////////
	D3D10VideoMode const & D3D10Adapter::VideoMode(size_t index) const
	{
		BOOST_ASSERT(index < modes_.size());

		return modes_[index];
	}

	// 枚举显示模式
	/////////////////////////////////////////////////////////////////////////////////
	void D3D10Adapter::Enumerate()
	{
		std::vector<DXGI_FORMAT> formats;
		formats.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
		formats.push_back(DXGI_FORMAT_R10G10B10A2_UNORM);

		UINT i = 0;
		IDXGIOutput* output;
		while (adapter_->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			if (output != NULL)
			{
				BOOST_FOREACH(BOOST_TYPEOF(formats)::reference format, formats)
				{
					UINT num;
					output->GetDisplayModeList(format, 0, &num, 0);

					std::vector<DXGI_MODE_DESC> mode_descs(num);
					output->GetDisplayModeList(format, 0, &num, &mode_descs[0]);

					BOOST_FOREACH(BOOST_TYPEOF(mode_descs)::reference mode_desc, mode_descs)
					{
						D3D10VideoMode const video_mode(mode_desc.Width, mode_desc.Height,
							mode_desc.Format);

						// 如果找到一个新模式, 加入模式列表
						if (std::find(modes_.begin(), modes_.end(), video_mode) == modes_.end())
						{
							modes_.push_back(video_mode);
						}
					}
				}

				output->Release();
			}

			++ i;
		}

		std::sort(modes_.begin(), modes_.end());
	}
}
