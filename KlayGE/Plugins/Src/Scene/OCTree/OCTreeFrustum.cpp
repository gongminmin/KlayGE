// OCTreeFrustum.cpp
// KlayGE 视锥类 实现文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.2
// 初次建立 (2004.6.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/OCTree/OCTreeFrustum.hpp>

namespace KlayGE
{
	OCTreeFrustum::OCTreeFrustum(Matrix4 const & clip)
	{
		Vector4 column1(clip(0, 0), clip(1, 0), clip(2, 0), clip(3, 0));
		Vector4 column2(clip(0, 1), clip(1, 1), clip(2, 1), clip(3, 1));
		Vector4 column3(clip(0, 2), clip(1, 2), clip(2, 2), clip(3, 2));
		Vector4 column4(clip(0, 3), clip(1, 3), clip(2, 3), clip(3, 3));

		planes_[0] = column4 - column1;  // left
		planes_[1] = column4 + column1;  // right
		planes_[2] = column4 - column2;  // bottom
		planes_[3] = column4 + column2;  // top
		planes_[4] = column4 - column3;  // near
		planes_[5] = column4 + column3;  // far

		// Loop through each side of the frustum and normalize it.
		for (planes_t::iterator iter = planes_.begin(); iter != planes_.end(); ++ iter)
		{
			MathLib::Normalize(*iter, *iter);
		}

		boost::array<Vector3, 8> frustumVertex;
		frustumVertex[0] = Vector3(-1, +1, 0);
		frustumVertex[1] = Vector3(+1, +1, 0);
		frustumVertex[2] = Vector3(+1, -1, 0);
		frustumVertex[3] = Vector3(-1, -1, 0);
		frustumVertex[4] = Vector3(-1, +1, 1);
		frustumVertex[5] = Vector3(+1, +1, 1);
		frustumVertex[6] = Vector3(+1, -1, 1);
		frustumVertex[7] = Vector3(-1, -1, 1);

		Matrix4 inverse_clip;
		MathLib::Inverse(inverse_clip, clip);
		for (size_t i = 0; i < frustumVertex.size(); ++ i)
		{
			MathLib::TransformCoord(frustumVertex[i], frustumVertex[i], inverse_clip);
		}

		frustumLines_[0].first = frustumVertex[4] - frustumVertex[0];
		frustumLines_[0].second = frustumVertex[0];
		frustumLines_[1].first = frustumVertex[5] - frustumVertex[1];
		frustumLines_[1].second = frustumVertex[1];
		frustumLines_[2].first = frustumVertex[6] - frustumVertex[2];
		frustumLines_[2].second = frustumVertex[2];
		frustumLines_[3].first = frustumVertex[7] - frustumVertex[3];
		frustumLines_[3].second = frustumVertex[3];

		for (size_t i = 0; i < frustumLines_.size(); ++ i)
		{
			MathLib::Normalize(frustumLines_[i].first, frustumLines_[i].first);
		}
	}

	bool OCTreeFrustum::Visiable(Box const & box, Matrix4 const & model) const
	{
		boost::array<Vector3, 8> vecs;
		for (size_t i = 0; i < 8; ++ i)
		{
			MathLib::TransformCoord(vecs[i], box[i], model);

			if (this->Visiable(vecs[i]))
			{
				return true;
			}
		}

		boost::array<boost::array<Vector3, 3>, 6> boxVertices;
		boxVertices[0][0] = vecs[0];
		boxVertices[0][1] = vecs[1];
		boxVertices[0][2] = vecs[2];
		boxVertices[1][0] = vecs[3];
		boxVertices[1][1] = vecs[2];
		boxVertices[1][2] = vecs[6];
		boxVertices[2][0] = vecs[7];
		boxVertices[2][1] = vecs[6];
		boxVertices[2][2] = vecs[5];
		boxVertices[3][0] = vecs[4];
		boxVertices[3][1] = vecs[5];
		boxVertices[3][2] = vecs[1];
		boxVertices[4][0] = vecs[1];
		boxVertices[4][1] = vecs[5];
		boxVertices[4][2] = vecs[6];
		boxVertices[5][0] = vecs[4];
		boxVertices[5][1] = vecs[0];
		boxVertices[5][2] = vecs[3];

		boost::array<Plane, 6> boxPlanes;
		for (size_t i = 0; i < 6; ++ i)
		{
			MathLib::FromPoints(boxPlanes[i], boxVertices[i][0], boxVertices[i][1], boxVertices[i][2]);
		}

		for (size_t i = 0; i < 6; ++ i)
		{
			Vector3 min;
			MathLib::Minimize(min, boxVertices[i][0], boxVertices[i][2]);
			Vector3 max;
			MathLib::Maximize(max, boxVertices[i][0], boxVertices[i][2]);

			for (size_t j = 0; j < 4; ++ j)
			{
				Vector3 v;
				float const t = MathLib::IntersectLine(v, boxPlanes[i], frustumLines_[j].second, frustumLines_[j].first);

				if (t >= 0)
				{
					if (((v.x() >= min.x()) && (v.x() <= max.x()))
						&& ((v.y() >= min.y()) && (v.y() <= max.y()))
						&& ((v.z() >= min.z()) && (v.z() <= max.z())))
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool OCTreeFrustum::Visiable(Vector3 const & v) const
	{
		for (size_t i = 0; i < planes_.size(); ++ i)
		{
			if (MathLib::DotCoord(planes_[i], v) < 0)
			{
				return false;
			}
		}

		return true;
	}
}
