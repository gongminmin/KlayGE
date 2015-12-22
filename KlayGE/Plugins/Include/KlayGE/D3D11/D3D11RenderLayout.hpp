// D3D11RenderLayout.hpp
// KlayGE D3D11渲染布局类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11RENDERLAYOUT_HPP
#define _D3D11RENDERLAYOUT_HPP

#pragma once

#include <vector>

#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11RenderLayout : public RenderLayout
	{
	public:
		D3D11RenderLayout();

		ID3D11InputLayout* InputLayout(size_t signature, std::vector<uint8_t> const & vs_code) const;

	private:
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertex_elems_;

		mutable std::vector<std::pair<size_t, ID3D11InputLayoutPtr>> input_layouts_;
	};
}

#endif			// _D3D11RENDERLAYOUT_HPP
