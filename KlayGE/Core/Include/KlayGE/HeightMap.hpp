// HeightMap.hpp
// KlayGE HeightMap���������� ͷ�ļ�
// Ver 3.11.0
// ��Ȩ����(C) ������, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// �ϲ���Core (2010.8.21)
//
// 3.4.0
// ʹ��function (2006.7.23)
//
// 2.0.0
// ���ν��� (2003.10.5)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _HEIGHTMAP_HPP
#define _HEIGHTMAP_HPP

#pragma once

#include <vector>

namespace KlayGE
{
	// �߶�ͼ��������
	/////////////////////////////////////////////////////////////////////////////////
	class HeightMap final : boost::noncopyable
	{
	public:
		void BuildTerrain(float start_x, float start_y, float end_x, float end_y, float span_x, float span_y,
			std::vector<float3>& vertices, std::vector<uint16_t>& indices,
			std::function<float(float, float)> HeightFunc);
	};
}

#endif		// _HEIGHTMAP_HPP
