#ifndef _FRUSTUM_HPP
#define _FRUSTUM_HPP

#include <KlayGE/Math.hpp>

#pragma comment(lib, "KlayGE_Scene_OCTree.lib")

namespace KlayGE
{
	class Frustum
	{
	public:
		void CalculateFrustum(const Matrix4& view, const Matrix4& proj);
		bool Visiable(const Vector3& v) const;

	private:
		Plane frustum_[6];
	};
}

#endif			// _FRUSTUM_HPP
