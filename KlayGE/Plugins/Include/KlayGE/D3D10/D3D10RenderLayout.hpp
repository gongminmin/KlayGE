// D3D10RenderLayout.hpp
// KlayGE D3D10渲染布局类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10RENDERLAYOUT_HPP
#define _D3D10RENDERLAYOUT_HPP

#pragma once

#include <boost/smart_ptr.hpp>

#include <vector>
#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>

#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/D3D10/D3D10Typedefs.hpp>

namespace KlayGE
{
	class D3D10RenderLayout : public RenderLayout
	{
	public:
		D3D10RenderLayout();

		ID3D10InputLayoutPtr const & InputLayout(ID3D10BlobPtr const & vs_code) const;

	private:
		typedef std::vector<D3D10_INPUT_ELEMENT_DESC> input_elems_type;
		input_elems_type vertex_elems_;

		mutable bool dirty_decl_;
		mutable ID3D10InputLayoutPtr input_layout_;
	};
}

#endif			// _D3D10RENDERLAYOUT_HPP
