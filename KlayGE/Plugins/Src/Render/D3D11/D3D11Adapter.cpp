// D3D11Adapter.cpp
// KlayGE D3D11������ ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/D3D11/D3D11Adapter.hpp>

namespace KlayGE
{
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	D3D11Adapter::D3D11Adapter(uint32_t adapter_no, IDXGIAdapter2* adapter)
					: adapter_no_(adapter_no)
	{
		this->ResetAdapter(adapter);
	}

	// ��ȡ�豸�����ַ���
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const D3D11Adapter::Description() const
	{
		return std::wstring(adapter_desc_.Description);
	}

	// ��ȡ֧�ֵ���ʾģʽ��Ŀ
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D11Adapter::NumVideoMode() const noexcept
	{
		return modes_.size();
	}

	// ��ȡ��ʾģʽ
	/////////////////////////////////////////////////////////////////////////////////
	D3D11VideoMode const & D3D11Adapter::VideoMode(size_t index) const
	{
		BOOST_ASSERT(index < modes_.size());

		return modes_[index];
	}

	// ö����ʾģʽ
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11Adapter::Enumerate()
	{
		static DXGI_FORMAT constexpr formats[] =
		{
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
			DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
			DXGI_FORMAT_R10G10B10A2_UNORM
		};

		UINT i = 0;
		com_ptr<IDXGIOutput> output;
		while (adapter_->EnumOutputs(i, output.release_and_put()) != DXGI_ERROR_NOT_FOUND)
		{
			if (output != nullptr)
			{
				for (auto const & format : formats)
				{
					UINT num = 0;
					output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, nullptr);
					if (num > 0)
					{
						std::vector<DXGI_MODE_DESC> mode_descs(num);
						output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, &mode_descs[0]);

						for (auto const & mode_desc : mode_descs)
						{
							D3D11VideoMode video_mode(mode_desc.Width, mode_desc.Height, mode_desc.Format);

							// ����ҵ�һ����ģʽ, ����ģʽ�б�
							if (std::find(modes_.begin(), modes_.end(), video_mode) == modes_.end())
							{
								modes_.emplace_back(std::move(video_mode));
							}
						}
					}
				}
			}

			++ i;
		}

		std::sort(modes_.begin(), modes_.end());
	}

	void D3D11Adapter::ResetAdapter(IDXGIAdapter2* adapter)
	{
		adapter_.reset(adapter);
		adapter_->GetDesc2(&adapter_desc_);
		modes_.clear();
	}
}
