#ifndef _KDTREE_HPP
#define _KDTREE_HPP

#include <vector>
#include <functional>
#include <algorithm>
#include <boost/assert.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/pool/pool_alloc.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace KlayGE
{
	template <typename T>
	class kdtree
	{
	private:
		typedef std::pair<size_t, typename T::value_type> neighbor_type;

		struct less_neighbor_type : public std::binary_function<neighbor_type, neighbor_type, bool>
		{
			bool operator()(neighbor_type const & lhs, neighbor_type const & rhs) const
			{
				return lhs.second < rhs.second;
			}
		};


		struct kdtree_point
		{
			T			pos;
			size_t		index;
		};

		class kd_node
		{
		public:
			union
			{
				struct
				{
					uint32_t leaf_children_dim;	// 1b for is_leaf, 27b for children offset, 4b for dim
					typename T::value_type cut_val;
				} node_data;

				struct
				{
					uint32_t leaf_points;		// 1b for is_leaf, 31b for point index
					uint32_t num_elements;
				} leaf_data;
			};

			void query_node(kd_node const * node_pool, kdtree_point const * points,
				typename T::value_type rd, std::vector<neighbor_type>& neighbors, size_t num_neighbors, T const & query_position, T& query_offsets) const
			{
				if (!(node_data.leaf_children_dim & 0x80000000UL))
				{
					int const dim = node_data.leaf_children_dim & 0xF;
					typename T::value_type const old_off = query_offsets[dim];
					typename T::value_type const new_off = query_position[dim] - node_data.cut_val;
					int const branch = new_off < 0 ? 0 : 1;
					int const children = node_data.leaf_children_dim >> 4;
					node_pool[children + branch].query_node(node_pool, points, rd, neighbors, num_neighbors, query_position, query_offsets);
					rd += new_off * new_off - old_off * old_off;
					if (rd < neighbors.back().second)
					{
		  				query_offsets[dim] = new_off;
		  				node_pool[children + !branch].query_node(node_pool, points, rd, neighbors, num_neighbors, query_position, query_offsets);
		  				query_offsets[dim] = old_off;
					}
				}
  				else
				{
					kdtree_point const * p = points + (leaf_data.leaf_points & 0x7FFFFFFFUL);
					kdtree_point const * p_end = p + leaf_data.num_elements;
					for (; p < p_end; ++ p)
					{
						typename T::value_type sqr_dist = KlayGE::MathLib::length_sq(p->pos - query_position);
						if (sqr_dist < neighbors.back().second)
						{
							if (neighbors.size() >= num_neighbors)
							{
								neighbors.pop_back();
							}

							neighbor_type new_neighbor(p->index, sqr_dist);
							typename std::vector<neighbor_type>::iterator iter = std::lower_bound(neighbors.begin(), neighbors.end(),
								new_neighbor, less_neighbor_type());
							neighbors.insert(iter, new_neighbor);
						}
					}
				}
			}

			void query_node(kd_node const * node_pool, kdtree_point const * points,
				typename T::value_type rd, neighbor_type& neighbor, T const & query_position, T& query_offsets) const
			{
				if (!(node_data.leaf_children_dim & 0x80000000UL))
				{
					int const dim = node_data.leaf_children_dim & 0xF;
					typename T::value_type const old_off = query_offsets[dim];
					typename T::value_type const new_off = query_position[dim] - node_data.cut_val;
					int const branch = new_off < 0 ? 0 : 1;
					int const children = node_data.leaf_children_dim >> 4;
					node_pool[children + branch].query_node(node_pool, points, rd, neighbor, query_position, query_offsets);
					rd += new_off * new_off - old_off * old_off;
					if (rd < neighbor.second)
					{
		  				query_offsets[dim] = new_off;
		  				node_pool[children + !branch].query_node(node_pool, points, rd, neighbor, query_position, query_offsets);
		  				query_offsets[dim] = old_off;
					}
				}
  				else
				{
					kdtree_point const * p = points + (leaf_data.leaf_points & 0x7FFFFFFFUL);
					kdtree_point const * p_end = p + leaf_data.num_elements;
					for (; p < p_end; ++ p)
					{
						typename T::value_type sqr_dist = KlayGE::MathLib::length_sq(p->pos - query_position);
						if (sqr_dist < neighbor.second)
						{
							neighbor = neighbor_type(p->index, sqr_dist);
						}
					}
				}
			}
		};

	public:
		kdtree(T const * positions, size_t num_positions, uint32_t max_bucket_size = 640)
			: points_(num_positions),
				bucket_size_(max_bucket_size),
				node_pool_(1)
		{
			for (size_t i = 0; i < num_positions; ++ i)
			{
				points_[i].pos = positions[i];
				points_[i].index = i;
			}
			this->compute_enclosing_bounding_box(bbox_low_corner_, bbox_high_corner_);
			T maximum = bbox_high_corner_;
			T minimum = bbox_low_corner_;
			this->create_tree(0, 0, static_cast<int>(num_positions), maximum, minimum);
		}

		size_t query_position(T const & position, size_t num_neighbors)
		{
			BOOST_ASSERT(num_neighbors != 0);

			T query_offsets;
			for (size_t dim = 0; dim < query_offsets.size(); ++ dim)
			{
				query_offsets[dim] = 0;
			}
			neighbors_.assign(1, std::make_pair(-1, std::numeric_limits<typename T::value_type>::max()));
			typename T::value_type sqr_dist = this->compute_box_sqr_distance(position, bbox_low_corner_, bbox_high_corner_, query_offsets);
			node_pool_[0].query_node(&node_pool_[0], &points_[0], sqr_dist, neighbors_, num_neighbors, position, query_offsets);

			if (static_cast<size_t>(-1) == neighbors_.back().first)
			{
				neighbors_.pop_back();
			}

			return neighbors_.size();
		}

		size_t query_position(T const & position)
		{
			T query_offsets;
			for (size_t dim = 0; dim < query_offsets.size(); ++ dim)
			{
				query_offsets[dim] = 0;
			}
			neighbors_.assign(1, std::make_pair(-1, std::numeric_limits<typename T::value_type>::max()));
			typename T::value_type sqr_dist = this->compute_box_sqr_distance(position, bbox_low_corner_, bbox_high_corner_, query_offsets);
			node_pool_[0].query_node(&node_pool_[0], &points_[0], sqr_dist, neighbors_[0], position, query_offsets);

			if (static_cast<size_t>(-1) == neighbors_.back().first)
			{
				neighbors_.pop_back();
			}

			return neighbors_.size();
		}

		size_t neighbor_position_index(size_t i) const
		{
			return neighbors_[i].first;
		}

		typename T::value_type squared_distance(size_t i) const
		{
			return neighbors_[i].second;
		}

	private:
		typename T::value_type compute_box_sqr_distance(T const & q, T const & low, T const & high, T& offset) const
		{
			typename T::value_type dist = 0;
			for (size_t dim = 0; dim < high.size(); ++ dim)
			{
				if (q[dim] < low[dim])
				{
					typename T::value_type t = low[dim] - q[dim];
					offset[dim] = t;
					dist += t * t;
				}
				else
				{
					if (q[dim] > high[dim])
					{
						typename T::value_type t = q[dim] - high[dim];
						offset[dim] = t;
						dist += t * t;
					}
				}
			}

			return dist;
		}

		void compute_enclosing_bounding_box(T& low_corner, T& hi_corner) const
		{
			hi_corner = low_corner = points_[0].pos;
			for (size_t i = 1; i < points_.size(); ++ i)
			{
				T const & tmp = points_[i].pos;
				for (size_t dim = 0; dim < hi_corner.size(); ++ dim)
				{
					if (hi_corner[dim] < tmp[dim])
					{
						hi_corner[dim] = tmp[dim];
					}
					else
					{
						if (low_corner[dim] > tmp[dim])
						{
							low_corner[dim] = tmp[dim];
						}
					}
				}
			}
		}

		void create_tree(uint32_t node_offset, int32_t start, int32_t end, T& maximum, T& minimum)
		{
			int32_t n = end - start;
			T diff = maximum - minimum;
			unsigned char dim = 0;
			// get longest axis
			for (size_t d = 1; d < diff.size(); ++ d)
			{
				if (diff[d] > diff[dim])
				{
					dim = static_cast<unsigned char>(d);
				}
			}

			uint32_t const cur_children_offset = static_cast<uint32_t>(node_pool_.size());
			node_pool_.resize(cur_children_offset + 2);
			node_pool_[node_offset].node_data.leaf_children_dim |= (cur_children_offset << 4) | dim;

			typename T::value_type best_cut = (maximum[dim] + minimum[dim]) / typename T::value_type(2);
			typename T::value_type mmin, mmax;
			this->get_min_max(&points_[start], n, dim, mmin, mmax);	// find min/max coordinates
			if (best_cut < mmin)		// slide to min or max as needed
			{
				node_pool_[node_offset].node_data.cut_val = mmin;
			}
			else
			{
				if (best_cut > mmax)
				{
					node_pool_[node_offset].node_data.cut_val = mmax;
				}
				else
				{
					node_pool_[node_offset].node_data.cut_val = best_cut;
				}
			}

			int br1, br2;
			// permute points accordingly
			this->split_at_mid(&points_[start], n, dim, node_pool_[node_offset].node_data.cut_val, br1, br2);

			int	mid;
			if (best_cut < mmin)
			{
				mid = start + 1;
			}
			else
			{
				if (best_cut > mmax)
				{
					mid = end - 1;
				}
				else
				{
					if (br1 > n / 2)
					{
						mid = start + br1;
					}
					else
					{
						if (br2 < n / 2)
						{
							mid = start + br2;
						}
						else
						{
							mid = start + n / 2;
						}
					}
				}
			}

			if (mid - start <= bucket_size_)
			{
				// new leaf
				kd_node& leaf = node_pool_[cur_children_offset + 0];
				leaf.leaf_data.leaf_points = 0x80000000UL | start;
				leaf.leaf_data.num_elements = mid - start;
			}
			else
			{
				// new node
				node_pool_[cur_children_offset + 0].node_data.leaf_children_dim = 0;
				typename T::value_type old_max = maximum[dim];
				maximum[dim] = node_pool_[node_offset].node_data.cut_val;
				this->create_tree(cur_children_offset + 0, start, mid, maximum, minimum);
				maximum[dim] = old_max;
			}

			if (end - mid <= bucket_size_)
			{
				// new leaf
				kd_node& leaf = node_pool_[cur_children_offset + 1];
				leaf.leaf_data.leaf_points = 0x80000000UL | mid;
				leaf.leaf_data.num_elements = end - mid;
			}
			else
			{
				// new node
				node_pool_[cur_children_offset + 1].node_data.leaf_children_dim = 0;
				minimum[dim] = node_pool_[node_offset].node_data.cut_val;
				this->create_tree(cur_children_offset + 1, mid, end, maximum, minimum);
			}
		}

		void get_min_max(kdtree_point const * points, size_t num_points, int dim,
			typename T::value_type& mmin, typename T::value_type& mmax) const
		{
			mmax = mmin = points[0].pos[dim];
			for (size_t i = 1; i < num_points; ++ i)
			{
				typename T::value_type f = points[i].pos[dim];
				if (f < mmin)
				{
					mmin = f;
				}
				else
				{
					if (f > mmax)
					{
						mmax = f;
					}
				}
			}
		}

		// splits the points such that on return for all points:
		//		points[0..br1-1] < cutVal
		//		points[br1-1..br2-1] == cutVal
		//		points[br2..num_points-1] > cutVal
		void split_at_mid(kdtree_point* points, size_t num_points, int dim, typename T::value_type cut_val, int& br1, int& br2)
		{
			int l = 0;
			int r = static_cast<int>(num_points - 1);
			for (;;)
			{
				// partition points[0..n-1] about the cut value
				while ((l < static_cast<int>(num_points)) && (points[l].pos[dim] < cut_val))
				{
					++ l;
				}
				while ((r >= 0) && (points[r].pos[dim] >= cut_val))
				{
					-- r;
				}
				if (l > r)
				{
					break;
				}
				std::swap(points[l], points[r]);
				++ l;
				-- r;
			}
			br1 = l;			// now: points[0..br1-1] < cutVal <= points[br1..n-1]
			r = static_cast<int>(num_points - 1);
			for (;;)
			{
				// partition points[br1..n-1] about cutVal
				while ((l < static_cast<int>(num_points)) && (points[l].pos[dim] <= cut_val))
				{
					++ l;
				}
				while ((r >= br1) && (points[r].pos[dim] > cut_val))
				{
					-- r;
				}
				if (l > r)
				{
					break;
				}
				std::swap(points[l], points[r]);
				++ l;
				-- r;
			}
			br2 = l;			// now: points[br1..br2-1] == cutVal < points[br2..n-1]
		}

	private:
		std::vector<kdtree_point>		points_;
		std::vector<neighbor_type>  	neighbors_;
		int								bucket_size_;
		T								bbox_low_corner_;
		T								bbox_high_corner_;

		std::vector<kd_node, boost::pool_allocator<kd_node> >	node_pool_;
	};
}

#endif
