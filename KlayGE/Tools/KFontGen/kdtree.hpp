#ifndef _KDTREE_HPP
#define _KDTREE_HPP

#include <vector>
#include <queue>
#include <functional>

template <typename T>
class kdtree
{
private:
	typedef std::pair<size_t, typename T::value_type> neighbor_type;

	struct greater_neighbor_type : public std::binary_function<neighbor_type, neighbor_type, bool>
	{
		bool operator()(const neighbor_type& lhs, const neighbor_type& rhs) const
		{
			return lhs.second > rhs.second;
		}
	};

	typedef std::priority_queue<neighbor_type, std::vector<neighbor_type>, greater_neighbor_type> pqueue;

	
	struct kdtree_point
	{
		T			pos;
		size_t		index;
	};

	class kd_node
	{
	public:
		bool is_leaf;

		union
		{
			struct
			{
				kd_node* children[2];
				typename T::value_type cut_val;
				unsigned char dim;			
			} node_data;

			struct
			{
				kdtree_point const * points;
				unsigned int num_elements;
			} leaf_data;
		};

		explicit kd_node(bool leaf)
		{
			is_leaf = leaf;
			if (!is_leaf)
			{
				node_data.children[0] = NULL;
				node_data.children[1] = NULL;
			}
		}

		~kd_node()
		{
			if (!is_leaf)
			{
				delete node_data.children[0];
				delete node_data.children[1];
			}
		}

		void query_node(typename T::value_type rd, pqueue& query_priority_queue, size_t num_neighbors, T const & query_position, T& query_offsets)
		{
			if (!is_leaf)
			{
				typename T::value_type old_off = query_offsets[node_data.dim];
				typename T::value_type new_off = query_position[node_data.dim] - node_data.cut_val;
				if (new_off < 0)
				{
					node_data.children[0]->query_node(rd, query_priority_queue, num_neighbors, query_position, query_offsets);
					rd += new_off * new_off - old_off * old_off;
					if (rd < query_priority_queue.top().second)
					{
		  				query_offsets[node_data.dim] = new_off;
		  				node_data.children[1]->query_node(rd, query_priority_queue, num_neighbors, query_position, query_offsets);
		  				query_offsets[node_data.dim] = old_off;
					}
				}
				else
				{
					node_data.children[1]->query_node(rd, query_priority_queue, num_neighbors, query_position, query_offsets);
					rd += new_off * new_off - old_off * old_off;
					if (rd < query_priority_queue.top().second)
					{
		  				query_offsets[node_data.dim] = new_off;
		  				node_data.children[0]->query_node(rd, query_priority_queue, num_neighbors, query_position, query_offsets);
		  				query_offsets[node_data.dim] = old_off;
					}
				}
			}
  			else
			{
				kdtree_point const * point = leaf_data.points;
				for (unsigned int i = 0; i < leaf_data.num_elements; ++ i)
				{
					typename T::value_type sqr_dist = MathLib::length_sq(point[i].pos - query_position);
					if (sqr_dist < query_priority_queue.top().second)
					{
						query_priority_queue.push(neighbor_type(point[i].index, sqr_dist));
					}
				}
			}
		}
	};

public:
	kdtree(T const * positions, size_t num_positions, unsigned int max_bucket_size = 10)
		: points_(num_positions),
			bucket_size_(max_bucket_size),
			root_(false)
	{
		for (size_t i = 0; i < num_positions; ++ i)
		{
			points_[i].pos = positions[i];
			points_[i].index = i;
		}
		this->compute_enclosing_bounding_box(bbox_low_corner_, bbox_high_corner_);
		T maximum = bbox_high_corner_;
		T minimum = bbox_low_corner_;
		this->create_tree(root_, 0, static_cast<int>(num_positions), maximum, minimum);
	}

	size_t query_position(T const & position, size_t num_neighbors)
	{
		assert(num_neighbors != 0);

		T query_offsets = T::Zero();
		pqueue query_priority_queue;
		neighbors_.resize(num_neighbors);
		query_priority_queue.push(std::make_pair(-1, std::numeric_limits<typename T::value_type>::max()));
		typename T::value_type sqr_dist = this->compute_box_sqr_distance(position, bbox_low_corner_, bbox_high_corner_, query_offsets);
		root_.query_node(sqr_dist, query_priority_queue, num_neighbors, position, query_offsets);

		if (-1 == query_priority_queue.top().first)
		{
			query_priority_queue.pop();
		}

		size_t num_found_neighbors = std::min(query_priority_queue.size(), num_neighbors);
		for (int i = static_cast<int>(num_found_neighbors - 1); i >= 0; -- i)
		{
			neighbors_[i] = query_priority_queue.top();
			query_priority_queue.pop();
		}

		return num_found_neighbors;
	}

	size_t neighbor_position_index(unsigned int i) const
	{
		return neighbors_[i].first;
	}

	typename T::value_type squared_distance(unsigned int i) const
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

	void create_tree(kd_node& node, int start, int end, T& maximum, T& minimum)
	{
		int n = end - start;
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

		node.node_data.dim = dim;
		typename T::value_type best_cut = (maximum[dim] + minimum[dim]) / typename T::value_type(2);
		typename T::value_type mmin, mmax;
		this->get_min_max(&points_[start], n, dim, mmin, mmax);	// find min/max coordinates
		if (best_cut < mmin)		// slide to min or max as needed
		{
			node.node_data.cut_val = mmin;
		}
		else
		{
			if (best_cut > mmax)
			{
				node.node_data.cut_val = mmax;
			}
			else
			{
				node.node_data.cut_val = best_cut;
			}
		}

		int br1, br2;
		// permute points accordingly
		this->split_at_mid(&points_[start], n, dim, node.node_data.cut_val, br1, br2);	

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
			kd_node* leaf = new kd_node(true);
			node.node_data.children[0] = leaf;
			leaf->leaf_data.points = &points_[start];
			leaf->leaf_data.num_elements = mid - start;
		}
		else
		{
			// new node
			kd_node* child = new kd_node(false);
			node.node_data.children[0] = child;
			typename T::value_type old_max = maximum[dim];
			maximum[dim] = node.node_data.cut_val;
			this->create_tree(*child, start, mid, maximum, minimum);
			maximum[dim] = old_max;
		}
		
		if (end - mid <= bucket_size_)
		{
			// new leaf
			kd_node* leaf = new kd_node(true);
			node.node_data.children[1] = leaf;
			leaf->leaf_data.points = &points_[mid];
			leaf->leaf_data.num_elements = end - mid;
		}
		else
		{
			// new node
			minimum[dim] = node.node_data.cut_val;
			kd_node* child = new kd_node(false);
			node.node_data.children[1] = child;
			this->create_tree(*child, mid, end, maximum, minimum);
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
	kd_node							root_;
	T								bbox_low_corner_;
	T								bbox_high_corner_;
};

#endif
