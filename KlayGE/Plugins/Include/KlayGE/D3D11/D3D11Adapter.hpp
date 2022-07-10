// D3D11Adapter.hpp
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

#ifndef _D3D11ADAPTER_HPP
#define _D3D11ADAPTER_HPP

#pragma once

#include <vector>
#include <string>
#include <KlayGE/D3D11/D3D11Util.hpp>
#include <KlayGE/D3D11/D3D11VideoMode.hpp>

namespace KlayGE
{
	// ��������������Ϣ��������������֧�ֵ��豸�б�
	/////////////////////////////////////////////////////////////////////////////////
	class D3D11Adapter final
	{
	public:
		D3D11Adapter(uint32_t adapter_no, IDXGIAdapter2* adapter);

		void Enumerate();

		// �����豸�����ַ���
		std::wstring const Description() const;
		void ResetAdapter(IDXGIAdapter2* adapter);

		uint32_t AdapterNo() const noexcept
		{
			return adapter_no_;
		}

		IDXGIAdapter2* DXGIAdapter() const noexcept
		{
			return adapter_.get();
		}

		DXGI_FORMAT DesktopFormat() const noexcept
		{
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		size_t NumVideoMode() const noexcept;
		D3D11VideoMode const & VideoMode(size_t index) const;

	private:
		// ���ǵڼ���������
		uint32_t			adapter_no_;

		// ��������Ϣ
		IDXGIAdapter2Ptr adapter_;
		DXGI_ADAPTER_DESC2 adapter_desc_{};

		// ��ʾģʽ�б�
		std::vector<D3D11VideoMode> modes_;
	};

	typedef std::shared_ptr<D3D11Adapter> D3D11AdapterPtr;
}

#endif			// _D3D11ADAPTER_HPP
