// Frustum.hpp
// KlayGE 视锥类 头文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.2
// 初次建立 (2004.6.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _FRUSTUM_HPP
#define _FRUSTUM_HPP

#include <KlayGE/Math.hpp>
#include <boost/array.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Scene_OCTree_d.lib")
#else
	#pragma comment(lib, "KlayGE_Scene_OCTree.lib")
#endif

namespace KlayGE
{
	class Frustum
	{
	public:
		void CalculateFrustum(Matrix4 const & clip);
		bool Visiable(Vector3 const & v) const;

	private:
		typedef boost::array<Plane, 6> PlanesType;
		PlanesType planes_;
	};
}

#endif			// _FRUSTUM_HPP
