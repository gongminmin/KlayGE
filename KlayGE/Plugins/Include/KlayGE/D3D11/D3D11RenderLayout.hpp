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

#include <boost/smart_ptr.hpp>

#include <vector>
#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <d3d11.h>
#include <D3D11Shader.h>

#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11RenderLayout : public RenderLayout
	{
	public:
		D3D11RenderLayout();

		ID3D11InputLayoutPtr const & InputLayout(std::vector<D3D11_SIGNATURE_PARAMETER_DESC> const & signature, ID3D10BlobPtr const & vs_code) const;

	private:
		typedef std::vector<D3D11_INPUT_ELEMENT_DESC> input_elems_type;
		input_elems_type vertex_elems_;

		mutable bool dirty_decl_;
		mutable ID3D11InputLayoutPtr input_layout_;
	};
}

#endif			// _D3D11RENDERLAYOUT_HPP
