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
#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11VideoMode.hpp>

namespace KlayGE
{
	// ��������������Ϣ��������������֧�ֵ��豸�б�
	/////////////////////////////////////////////////////////////////////////////////
	class D3D11Adapter
	{
	public:
		D3D11Adapter();
		D3D11Adapter(uint32_t adapter_no, IDXGIAdapter1Ptr const & adapter);

		void Enumerate();

		// �����豸�����ַ���
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
		// ���ǵڼ���������
		uint32_t			adapter_no_;

		// ��������Ϣ
		IDXGIAdapter1Ptr	adapter_;
		DXGI_ADAPTER_DESC1	adapter_desc_;

		// ��ʾģʽ�б�
		std::vector<D3D11VideoMode> modes_;
	};

	typedef std::shared_ptr<D3D11Adapter> D3D11AdapterPtr;
}

#endif			// _D3D11ADAPTER_HPP
