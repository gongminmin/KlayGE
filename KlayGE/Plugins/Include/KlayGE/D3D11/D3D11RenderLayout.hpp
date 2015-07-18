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

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <D3D11Shader.h>

#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11RenderLayout : public RenderLayout
	{
	public:
		D3D11RenderLayout();

		ID3D11InputLayoutPtr const & InputLayout(size_t signature, std::vector<uint8_t> const & vs_code) const;

	private:
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertex_elems_;

		mutable std::vector<std::pair<size_t, ID3D11InputLayoutPtr>> input_layouts_;
	};
}

#endif			// _D3D11RENDERLAYOUT_HPP
