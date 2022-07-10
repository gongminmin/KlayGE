// D3D11RenderLayout.hpp
// KlayGE D3D11��Ⱦ������ ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11RENDERLAYOUT_HPP
#define _D3D11RENDERLAYOUT_HPP

#pragma once

#include <vector>

#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11Util.hpp>

namespace KlayGE
{
	class D3D11RenderLayout final : public RenderLayout
	{
	public:
		D3D11RenderLayout();

		void Active() const;
		ID3D11InputLayout* InputLayout(ShaderObject const * so) const;

		std::vector<ID3D11Buffer*> const & VBs() const
		{
			return vbs_;
		}
		std::vector<UINT> const & Strides() const
		{
			return strides_;
		}
		std::vector<UINT> const & Offsets() const
		{
			return offsets_;
		}

	private:
		mutable std::vector<D3D11_INPUT_ELEMENT_DESC> vertex_elems_;
		mutable std::vector<std::pair<uint32_t, ID3D11InputLayoutPtr>> input_layouts_;

		mutable std::vector<ID3D11Buffer*> vbs_;
		mutable std::vector<UINT> strides_;
		mutable std::vector<UINT> offsets_;
	};
}

#endif			// _D3D11RENDERLAYOUT_HPP
