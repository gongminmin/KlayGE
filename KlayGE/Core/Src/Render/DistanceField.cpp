/**
 * @file DistanceField.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/DistanceField.hpp>

namespace KlayGE
{
	float EdgeDistance(float2 const & grad, float val)
	{
		float df;
		if ((0 == grad.x()) || (0 == grad.y()))
		{
			df = 0.5f - val;
		}
		else
		{
			float2 n_grad = MathLib::abs(MathLib::normalize(grad));
			if (n_grad.x() < n_grad.y())
			{
				std::swap(n_grad.x(), n_grad.y());
			}

			float v1 = 0.5f * n_grad.y() / n_grad.x();
			if (val < v1)
			{
				df = 0.5f * (n_grad.x() + n_grad.y()) - MathLib::sqrt(2 * n_grad.x() * n_grad.y() * val);
			}
			else if (val < 1 - v1)
			{
				df = (0.5f - val) * n_grad.x();
			}
			else
			{
				df = -0.5f * (n_grad.x() + n_grad.y()) + MathLib::sqrt(2 * n_grad.x() * n_grad.y() * (1 - val));
			}
		}
		return df;
	}

	float AADist(std::vector<float> const & img, std::vector<float2> const & grad,
		int width, int offset_addr, int2 const & offset_dist_xy, float2 const & new_dist)
	{
		int closest = offset_addr - offset_dist_xy.y() * width - offset_dist_xy.x(); // Index to the edge pixel pointed to from c
		float val = MathLib::clamp(img[closest], 0.0f, 1.0f);
		if (0 == val)
		{
			return 1e10f;
		}

		float di = MathLib::length(new_dist);
		float df;
		if (0 == di)
		{
			df = EdgeDistance(grad[closest], val);
		}
		else
		{
			df = EdgeDistance(new_dist, val);
		}
		return di + df;
	}

	bool UpdateDistance(int x, int y, int dx, int dy, std::vector<float> const & img, int width,
		std::vector<float2> const & grad, std::vector<int2>& dist_xy, std::vector<float>& dist)
	{
		float const EPSILON = 1e-3f;

		bool changed = false;
		int addr = y * width + x;
		float old_dist = dist[addr];
		if (old_dist > 0)
		{
			int offset_addr = (y + dy) * width + (x + dx);
			int2 new_dist_xy = dist_xy[offset_addr] - int2(dx, dy);
			float new_dist = AADist(img, grad, width, offset_addr, dist_xy[offset_addr], new_dist_xy);
			if (new_dist < old_dist - EPSILON)
			{
				dist_xy[addr] = new_dist_xy;
				dist[addr] = new_dist;
				changed = true;
			}
		}

		return changed;
	}

	void AAEuclideanDistance(std::vector<float> const & img, std::vector<float2> const & grad,
		int width, int height, std::vector<float>& dist)
	{
		std::vector<int2> dist_xy(img.size(), int2(0, 0));

		for (size_t i = 0; i < img.size(); ++ i)
		{
			if (img[i] <= 0)
			{
				dist[i] = 1e10f;
			}
			else if (img[i] < 1)
			{
				dist[i] = EdgeDistance(grad[i], img[i]);
			}
			else
			{
				dist[i] = 0;
			}
		}

		bool changed;
		do
		{
			changed = false;

			for (int y = 1; y < height; ++ y)
			{
				// Scan right, propagate distances from above & left
				for (int x = 0; x < width; ++ x)
				{
					if (x > 0)
					{
						changed |= UpdateDistance(x, y, -1, +0, img, width, grad, dist_xy, dist);
						changed |= UpdateDistance(x, y, -1, -1, img, width, grad, dist_xy, dist);
					}
					changed |= UpdateDistance(x, y, +0, -1, img, width, grad, dist_xy, dist);
					if (x < width - 1)
					{
						changed |= UpdateDistance(x, y, +1, -1, img, width, grad, dist_xy, dist);
					}
				}

				// Scan left, propagate distance from right
				for (int x = width - 2; x >= 0; -- x)
				{
					changed |= UpdateDistance(x, y, +1, +0, img, width, grad, dist_xy, dist);
				}
			}

			for (int y = height - 2; y >= 0; -- y)
			{
				// Scan left, propagate distances from below & right
				for (int x = width - 1; x >= 0; -- x)
				{
					if (x < width - 1)
					{
						changed |= UpdateDistance(x, y, +1, +0, img, width, grad, dist_xy, dist);
						changed |= UpdateDistance(x, y, +1, +1, img, width, grad, dist_xy, dist);
					}
					changed |= UpdateDistance(x, y, +0, +1, img, width, grad, dist_xy, dist);
					if (x > 0)
					{
						changed |= UpdateDistance(x, y, -1, +1, img, width, grad, dist_xy, dist);
					}
				}

				// Scan right, propagate distance from left
				for (int x = 1; x < width; ++ x)
				{
					changed |= UpdateDistance(x, y, -1, +0, img, width, grad, dist_xy, dist);
				}
			}
		} while (changed);
	}

	template KLAYGE_CORE_API void Downsample2x(std::vector<float> const & input_data, uint32_t input_width, uint32_t input_height,
		std::vector<float>& output_data);
	template KLAYGE_CORE_API void Downsample2x(std::vector<float2> const & input_data, uint32_t input_width, uint32_t input_height,
		std::vector<float2>& output_data);

	template <typename T>
	void Downsample2x(std::vector<T> const & input_data, uint32_t input_width, uint32_t input_height,
		std::vector<T>& output_data)
	{
		uint32_t output_width = input_width / 2;
		uint32_t output_height = input_height / 2;

		output_data.resize(output_width * output_height);

		for (uint32_t y = 0; y < output_height; ++ y)
		{
			for (uint32_t x = 0; x < output_width; ++ x)
			{
				output_data[y * output_width + x] = (input_data[(y * 2 + 0) * input_width + (x * 2 + 0)]
					+ input_data[(y * 2 + 0) * input_width + (x * 2 + 1)]
					+ input_data[(y * 2 + 1) * input_width + (x * 2 + 0)]
					+ input_data[(y * 2 + 1) * input_width + (x * 2 + 1)]) * 0.25f;
			}
		}
	}

	void ComputeGradient(std::vector<float> const & img, int w, int h, std::vector<float2>& grad)
	{
		BOOST_ASSERT(img.size() == static_cast<size_t>(w * h));
		BOOST_ASSERT(grad.size() == static_cast<size_t>(w * h));

		grad.assign(w * h, float2(0, 0));
		for (int y = 1; y < h - 1; ++ y)
		{
			for (int x = 1; x < w - 1; ++ x)
			{
				int addr = y * w + x;
				if ((img[addr] > 0) && (img[addr] < 1))
				{
					float s0 = -img[addr - w - 1] + img[addr + w + 1];
					float s1 = -img[addr + w - 1] + img[addr - w + 1];
					grad[addr] = MathLib::normalize(float2(s0 + s1 - SQRT2 * (img[addr - 1] - img[addr + 1]),
						s0 - s1 - SQRT2 * (img[addr - w] - img[addr + w])));
				}
			}
		}
	}

	void ComputeDistance(std::vector<float> const & aa_2x_data, uint32_t input_width, uint32_t input_height,
		std::vector<float>& dist_data)
	{
		BOOST_ASSERT((input_width & 0x1) == 0);
		BOOST_ASSERT((input_height & 0x1) == 0);

		std::vector<float> aa_data(aa_2x_data.size() / 4);
		Downsample2x(aa_2x_data, input_width, input_height, aa_data);

		std::vector<float2> grad_2x_data(aa_2x_data.size());
		ComputeGradient(aa_2x_data, input_width, input_height, grad_2x_data);

		std::vector<float2> grad_data(aa_data.size());
		Downsample2x(grad_2x_data, input_width, input_height, grad_data);

		std::vector<float> outside(grad_data.size());
		AAEuclideanDistance(aa_data, grad_data, input_width / 2, input_height / 2, outside);

		for (size_t i = 0; i < grad_data.size(); ++ i)
		{
			aa_data[i] = 1 - aa_data[i];
			grad_data[i] = -grad_data[i];
		}

		std::vector<float> inside(grad_data.size());
		AAEuclideanDistance(aa_data, grad_data, input_width / 2, input_height / 2, inside);

		dist_data.resize(outside.size());
		for (uint32_t i = 0; i < outside.size(); ++ i)
		{
			if (inside[i] < 0)
			{
				inside[i] = 0;
			}
			if (outside[i] < 0)
			{
				outside[i] = 0;
			}

			dist_data[i] = inside[i] - outside[i];
		}
	}
}
