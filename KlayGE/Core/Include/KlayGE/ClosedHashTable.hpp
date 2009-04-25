// ClosedHashTable.hpp
// KlayGE 封闭式hash table 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2007.10.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _CLOSEDHASHTABLE_HPP
#define _CLOSEDHASHTABLE_HPP

#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <iterator>
#include <boost/assert.hpp>
#include <boost/type_traits.hpp>

namespace KlayGE
{
	template <typename Value, typename Key, typename HashFunc, typename ExtractKey, typename EqualKey, typename Alloc>
	class closed_hash_table
	{
		enum dh_tag
		{
			DHTAG_NORMAL = 0,
			DHTAG_EMPTY = 1,
			DHTAG_DELETED = 2
		};

	public:
		class iterator;
		class const_iterator;

		class local_iterator;
		class const_local_iterator;

		class iterator
		{
			friend class closed_hash_table;
			friend class const_iterator;

		public:
			typedef std::forward_iterator_tag iterator_category;
			typedef Value value_type;
			typedef ptrdiff_t difference_type;
			typedef size_t size_type;
			typedef Value& reference;
			typedef Value* pointer;

			iterator(closed_hash_table const * h,
				value_type* it, value_type* it_end, bool advance)
					: ht_(h), pos_(it), end_(it_end)
			{
				if (advance)
				{
					this->advance_past_empty_and_deleted();
				}
			}
			iterator()
			{
			}

			reference operator*() const
			{
				return *pos_;
			}
			pointer operator->() const
			{
				return &(operator*());
			}

			iterator& operator++()
			{
				BOOST_ASSERT(pos_ != end_);
				++ pos_;
				this->advance_past_empty_and_deleted();
				return *this;
			}
			iterator operator++(int)
			{
				iterator tmp(*this);
				++ *this;
				return tmp;
			}

			friend bool operator==(iterator const & lhs, iterator const & rhs)
			{
				return lhs.pos_ == rhs.pos_;
			}
			friend bool operator!=(iterator const & lhs, iterator const & rhs)
			{
				return !(lhs == rhs);
			}

		private:
			void advance_past_empty_and_deleted()
			{
				while ((pos_ != end_)
					&& (ht_->get_tag(*this) != DHTAG_NORMAL))
				{
					++ pos_;
				}
			}

		private:
			closed_hash_table const * ht_;
			value_type* pos_;
			value_type* end_;
		};

		class const_iterator
		{
			friend class closed_hash_table;
			friend class iterator;

		public:
			typedef std::forward_iterator_tag iterator_category;
			typedef Value value_type;
			typedef ptrdiff_t difference_type;
			typedef size_t size_type;
			typedef Value const & reference;
			typedef Value const * pointer;

			const_iterator(closed_hash_table const * h,
				value_type* it, value_type* it_end, bool advance)
					: ht_(h), pos_(it), end_(it_end)
			{
				if (advance)
				{
					this->advance_past_empty_and_deleted();
				}
			}
			const_iterator()
			{
			}
			const_iterator(iterator const & rhs)
				: ht_(rhs.ht_), pos_(rhs.pos_), end_(rhs.end_)
			{
			}

			reference operator*() const
			{
				return *pos_;
			}
			pointer operator->() const
			{
				return &(operator*());
			}

			const_iterator& operator++()
			{
				BOOST_ASSERT(pos_ != end_);
				++ pos_;
				this->advance_past_empty_and_deleted();
				return *this;
			}
			const_iterator operator++(int)
			{
				const_iterator tmp(*this);
				++ *this;
				return tmp;
			}

			friend bool operator==(const_iterator const & lhs, const_iterator const & rhs)
			{
				return lhs.pos_ == rhs.pos_;
			}
			friend bool operator!=(const_iterator const & lhs, const_iterator const & rhs)
			{
				return !(lhs == rhs);
			}

		private:
			void advance_past_empty_and_deleted()
			{
				while ((pos_ != end_)
					&& (ht_->get_tag(*this) != DHTAG_NORMAL))
				{
					++ pos_;
				}
			}

		private:
			closed_hash_table const * ht_;
			value_type* pos_;
			value_type* end_;
		};

		class local_iterator
		{
			friend class closed_hash_table;
			friend class const_local_iterator;

		public:
			typedef std::forward_iterator_tag iterator_category;
			typedef Value value_type;
			typedef ptrdiff_t difference_type;
			typedef size_t size_type;
			typedef Value& reference;
			typedef Value* pointer;

			local_iterator(closed_hash_table const * h,
				value_type* it, value_type* it_end, size_type hash, size_type num_probes, bool advance)
					: ht_(h), pos_(it), end_(it_end), hash_code_(hash), num_probes_(num_probes)
			{
				if (advance)
				{
					this->advance_past_empty_and_deleted();
				}
			}
			local_iterator()
			{
			}

			reference operator*() const
			{
				return *pos_;
			}
			pointer operator->() const
			{
				return &(operator*());
			}

			local_iterator& operator++()
			{
				BOOST_ASSERT(pos_ != end_);
				this->advance_past_empty_and_deleted();
				return *this;
			}
			local_iterator operator++(int)
			{
				local_iterator tmp(*this);
				++ *this;
				return tmp;
			}

			friend bool operator==(local_iterator const & lhs, local_iterator const & rhs)
			{
				return lhs.pos_ == rhs.pos_;
			}
			friend bool operator!=(local_iterator const & lhs, local_iterator const & rhs)
			{
				return !(lhs == rhs);
			}

		private:
			void advance_past_empty_and_deleted()
			{
				size_type const bucket_count_minus_one = ht_->bucket_count() - 1;
				size_type bucknum = pos_ - ht_->data_table_;
				while (ht_->hash_(ht_->get_key_(ht_->data_table_[bucknum])) != hash_code_)
				{
					++ num_probes_;
					bucknum = (bucknum + ht_->reprobe(num_probes_)) & bucket_count_minus_one;
					BOOST_ASSERT(num_probes_ < ht_->bucket_count());
					pos_ = ht_->data_table_ + bucknum;

					if (DHTAG_EMPTY == ht_->get_tag(bucknum))
					{
						break;
					}
				}
			}

		private:
			closed_hash_table const * ht_;
			value_type* pos_;
			value_type* end_;
			size_type hash_code_;
			size_type num_probes_;
		};

		class const_local_iterator
		{
			friend class closed_hash_table;
			friend class local_iterator;

		public:
			typedef std::forward_iterator_tag iterator_category;
			typedef Value value_type;
			typedef ptrdiff_t difference_type;
			typedef size_t size_type;
			typedef Value const & reference;
			typedef Value const * pointer;

			const_local_iterator(closed_hash_table const * h,
				value_type* it, value_type* it_end, size_type hash, size_type num_probes, bool advance)
					: ht_(h), pos_(it), end_(it_end), hash_code_(hash), num_probes_(num_probes)
			{
				if (advance)
				{
					this->advance_past_empty_and_deleted();
				}
			}
			const_local_iterator()
			{
			}
			const_local_iterator(local_iterator const & rhs)
				: ht_(rhs.ht_), pos_(rhs.pos_), end_(rhs.end_)
			{
			}

			reference operator*() const
			{
				return *pos_;
			}
			pointer operator->() const
			{
				return &(operator*());
			}

			const_local_iterator& operator++()
			{
				BOOST_ASSERT(pos_ != end_);
				this->advance_past_empty_and_deleted();
				return *this;
			}
			const_local_iterator operator++(int)
			{
				const_local_iterator tmp(*this);
				++ *this;
				return tmp;
			}

			friend bool operator==(const_local_iterator const & lhs, const_local_iterator const & rhs)
			{
				return lhs.pos_ == rhs.pos_;
			}
			friend bool operator!=(const_local_iterator const & lhs, const_local_iterator const & rhs)
			{
				return !(lhs == rhs);
			}

		private:
			void advance_past_empty_and_deleted()
			{
				size_type const bucket_count_minus_one = ht_->bucket_count() - 1;
				size_type bucknum = pos_ - ht_->data_table_;
				while (ht_->hash_(ht_->get_key_(ht_->data_table_[bucknum])) != hash_code_)
				{
					++ num_probes_;
					bucknum = (bucknum + ht_->reprobe(num_probes_)) & bucket_count_minus_one;
					BOOST_ASSERT(num_probes_ < ht_->bucket_count());
					pos_ = ht_->data_table_ + bucknum;

					if (DHTAG_EMPTY == ht_->get_tag(bucknum))
					{
						break;
					}
				}
			}

		private:
			closed_hash_table const * ht_;
			value_type* pos_;
			value_type* end_;
			size_type hash_code_;
			size_type num_probes_;
		};

	public:
		typedef Key key_type;
		typedef Value value_type;
		typedef HashFunc hasher;
		typedef EqualKey key_equal;

		typedef size_t				size_type;
		typedef ptrdiff_t			difference_type;
		typedef value_type*			pointer;
		typedef value_type const *	const_pointer;
		typedef value_type&			reference;
		typedef value_type const &	const_reference;

	public:
		explicit closed_hash_table(size_type n = 0, HashFunc const & hf = HashFunc(),
				EqualKey const & eql = EqualKey(), ExtractKey const & ext = ExtractKey())
			: hash_(hf), equals_(eql), get_key_(ext),
				data_table_(NULL), tag_table_(NULL),
				num_elements_(0)
		{
			num_buckets_ = this->min_size(0, n);

			this->reset_thresholds();

			data_table_ = static_cast<value_type*>(malloc(num_buckets_ * sizeof(*data_table_)));
			BOOST_ASSERT(data_table_);
			std::uninitialized_fill(data_table_, data_table_ + num_buckets_, value_type());
			tag_table_ = static_cast<char*>(malloc(num_buckets_ * sizeof(*tag_table_)));
			BOOST_ASSERT(tag_table_);
			for (size_type i = 0; i < num_buckets_; ++ i)
			{
				this->set_tag(i, DHTAG_EMPTY);
			}
		}

		closed_hash_table(closed_hash_table const & rhs, size_type min_buckets_wanted = 0)
			: hash_(rhs.hash_), equals_(rhs.equals_), get_key_(rhs.get_key_),
				data_table_(NULL), tag_table_(NULL),
				num_buckets_(0),
				num_elements_(0)
		{
			this->reset_thresholds();
			this->copy_from(rhs, min_buckets_wanted);
		}

		~closed_hash_table()
		{
			if (data_table_)
			{
				this->destroy_buckets(0, num_buckets_);
				free(data_table_);
				free(tag_table_);
			}
		}

		iterator begin()
		{
			return iterator(this, data_table_, data_table_ + num_buckets_, true);
		}
		iterator end()
		{
			return iterator(this, data_table_ + num_buckets_, data_table_ + num_buckets_, true);
		}
		const_iterator begin() const
		{
			return const_iterator(this, data_table_, data_table_ + num_buckets_, true);
		}
		const_iterator end() const
		{
			return const_iterator(this, data_table_ + num_buckets_, data_table_ + num_buckets_, true);
		}

		local_iterator begin(key_type const & key)
		{
			return local_iterator(this, data_table_ + (hash_(key) & (num_buckets_ - 1)),
				data_table_ + num_buckets_, hash_(key), 0, true);
		}
		local_iterator end(key_type const & key)
		{
			size_type num_probes = 0;
			size_type const bucket_count_minus_one = this->bucket_count() - 1;
			size_type bucknum = hash_(key) & bucket_count_minus_one;
			for (;;)
			{
				++ num_probes;
				bucknum = (bucknum + this->reprobe(num_probes)) & bucket_count_minus_one;
				BOOST_ASSERT(num_probes < this->bucket_count());

				if (DHTAG_EMPTY == this->get_tag(bucknum))
				{
					break;
				}
			}

			return local_iterator(this, data_table_ + bucknum,
				data_table_ + num_buckets_, hash_(key), num_probes, false);
		}
		const_local_iterator begin(key_type const & key) const
		{
			return const_local_iterator(this, data_table_ + (hash_(key) & (num_buckets_ - 1)),
				data_table_ + num_buckets_, hash_(key), 0, true);
		}
		const_local_iterator end(key_type const & key) const
		{
			size_type num_probes = 0;
			size_type const bucket_count_minus_one = this->bucket_count() - 1;
			size_type bucknum = hash_(key) & bucket_count_minus_one;
			for (;;)
			{
				++ num_probes;
				bucknum = (bucknum + this->reprobe(num_probes)) & bucket_count_minus_one;
				BOOST_ASSERT(num_probes < this->bucket_count());

				if (DHTAG_EMPTY == this->get_tag(bucknum))
				{
					break;
				}
			}

			return const_local_iterator(this, data_table_ + bucknum,
				data_table_ + num_buckets_, hash_(key), num_probes, false);
		}

		hasher hash_function() const
		{
			return hash_;
		}
		key_equal key_eq() const
		{
			return equals_;
		}

		size_type size() const
		{
			return num_elements_;
		}
		size_type max_size() const
		{
			return (size_type(-1) >> 1UL) + 1;
		}
		bool empty() const
		{
			return 0 == this->size();
		}
		size_type bucket_count() const
		{
			return num_buckets_;
		}
		size_type max_bucket_count() const
		{
			return this->max_size();
		}
		size_type nonempty_bucket_count() const
		{
			return num_elements_;
		}

		void resize(size_type req_elements)
		{
			if (consider_shrink_ || (0 == req_elements))
			{
				this->shrink();
			}
			if (req_elements > num_elements_)
			{
				return this->resize_delta(req_elements - num_elements_, 0);
			}
		}

		closed_hash_table& operator=(closed_hash_table const & rhs)
		{
			if (&rhs != this)
			{
				this->clear();
				hash_ = rhs.hash_;
				equals_ = rhs.equals_;
				get_key_ = rhs.get_key_;
				this->copy_from(rhs);
			}

			return *this;
		}

		void swap(closed_hash_table& rhs)
		{
			std::swap(hash_, rhs.hash_);
			std::swap(equals_, rhs.equals_);
			std::swap(get_key_, rhs.get_key_);
			std::swap(data_table_, rhs.data_table_);
			std::swap(tag_table_, rhs.tag_table_);
			std::swap(num_buckets_, rhs.num_buckets_);
			std::swap(num_elements_, rhs.num_elements_);
			this->reset_thresholds();
			rhs.reset_thresholds();
		}

		void clear()
		{
			if (data_table_)
			{
				this->destroy_buckets(0, num_buckets_);
			}
			num_buckets_ = this->min_size(0, 0);
			this->reset_thresholds();
			data_table_ = static_cast<value_type*>(realloc(data_table_, num_buckets_ * sizeof(*data_table_)));
			BOOST_ASSERT(data_table_);
			std::uninitialized_fill(data_table_, data_table_ + num_buckets_, value_type());
			tag_table_ = static_cast<char*>(realloc(tag_table_, num_buckets_ * sizeof(*tag_table_)));
			BOOST_ASSERT(tag_table_);
			for (size_type i = 0; i < num_buckets_; ++ i)
			{
				this->set_tag(i, DHTAG_EMPTY);
			}
			num_elements_ = 0;
		}

		void clear_no_resize()
		{
			if (data_table_)
			{
				this->destroy_buckets(0, num_buckets_);
				for (size_type i = 0; i < num_buckets_; ++ i)
				{
					this->set_tag(i, DHTAG_EMPTY);
				}
			}
			this->reset_thresholds();
			num_elements_ = 0;
		}

		iterator find(key_type const & key)
		{
			if (this->empty())
			{
				return this->end();
			}
			std::pair<size_type, size_type> pos = this->find_position(key);
			if (ILLEGAL_BUCKET == pos.first)
			{
				return this->end();
			}
			else
			{
				return iterator(this, data_table_ + pos.first, data_table_ + num_buckets_, false);
			}
		}

		const_iterator find(key_type const & key) const
		{
			if (this->empty())
			{
				return this->end();
			}
			std::pair<size_type, size_type> pos = this->find_position(key);
			if (ILLEGAL_BUCKET == pos.first)
			{
				return this->end();
			}
			else
			{
				return const_iterator(this, data_table_ + pos.first, data_table_ + num_buckets_, false);
			}
		}

		size_type count(key_type const & key) const
		{
			std::pair<size_type, size_type> pos = this->find_position(key);
			return (ILLEGAL_BUCKET == pos.first) ? 0 : 1;
		}

		std::pair<iterator, iterator> equal_range(key_type const & key)
		{
			iterator const pos = this->find(key);
			return std::make_pair(pos, pos);
		}
		std::pair<const_iterator, const_iterator> equal_range(key_type const & key) const
		{
			const_iterator const pos = this->find(key);
			return std::make_pair(pos, pos);
		}

		std::pair<iterator, bool> insert(value_type const & obj)
		{
			this->resize_delta(1);
			return insert_noresize(obj);
		}

		template <typename InputIterator>
		void insert(InputIterator f, InputIterator l)
		{
			this->insert(f, l, typename std::iterator_traits<InputIterator>::iterator_category());
		}

		size_type erase(key_type const & key)
		{
			iterator pos = this->find(key);
			if (pos != this->end())
			{
				BOOST_ASSERT(this->get_tag(pos) != DHTAG_DELETED);
				this->set_value(&(*pos), value_type());
				this->set_tag(pos, DHTAG_DELETED);
				-- num_elements_;
				consider_shrink_ = true;
				return 1;
			}
			else
			{
				return 0;
			}
		}

		iterator erase(iterator pos)
		{
			if (pos == this->end())
			{
				return pos;
			}
			if (this->get_tag(pos) != DHTAG_DELETED)
			{
				this->set_value(&(*pos), value_type());
				this->set_tag(pos, DHTAG_DELETED);

				-- num_elements_;
				consider_shrink_ = true;
			}
			++ pos;
			return pos;
		}

		iterator erase(iterator f, iterator l)
		{
			for (; f != l; ++ f)
			{
				if (this->get_tag(f) != DHTAG_DELETED)
				{
					this->set_value(&(*f), value_type());
					this->set_tag(f, DHTAG_DELETED);
					-- num_elements_;
				}
			}
			consider_shrink_ = true;
			++ l;
			return l;
		}

		friend bool operator==(closed_hash_table const & lhs, closed_hash_table const & rhs)
		{
			if (lhs.size() != rhs.size())
			{
				return false;
			}
			else if (&lhs == &rhs)
			{
				return true;
			}
			else
			{
				for (typename closed_hash_table::const_iterator it = lhs.begin(); it != lhs.end(); ++ it)
				{
					const_iterator it2 = rhs.find(lhs.get_key_(*it));
					if ((it2 == rhs.end()) || (*it != *it2))
					{
						return false;
					}
				}
				return true;
			}

		}
		friend bool operator!=(closed_hash_table const & lhs, closed_hash_table const & rhs)
		{
			return !(lhs == rhs);
		}

	private:
		size_type reprobe(size_type num_probes) const
		{
			return num_probes;
		}

		void set_value(value_type* dst, value_type const & src)
		{
			dst->~value_type();
			new (dst) value_type(src);
		}

		void destroy_buckets(size_type first, size_type last)
		{
			for (; first != last; ++ first)
			{
				data_table_[first].~value_type();
			}
		}

		size_type min_size(size_type num_elts, size_type min_buckets_wanted)
		{
			size_type sz = HT_MIN_BUCKETS;
			while ((sz < min_buckets_wanted) || (num_elts >= sz * HT_OCCUPANCY_FLT))
			{
				sz *= 2;
			}
			return sz;
		}

		void shrink()
		{
			BOOST_ASSERT(num_elements_ >= 0);
			BOOST_ASSERT(0 == (this->bucket_count() & (this->bucket_count() - 1)));
			BOOST_ASSERT(this->bucket_count() >= HT_MIN_BUCKETS);

			if ((num_elements_ < shrink_threshold_)
				&& (this->bucket_count() > HT_MIN_BUCKETS))
			{
				size_type sz = this->bucket_count() / 2;
				while ((sz > HT_MIN_BUCKETS)
					&& (num_elements_ < sz * HT_EMPTY_FLT))
				{
					sz /= 2;
				}
				closed_hash_table tmp(*this, sz);
				this->swap(tmp);
			}
			consider_shrink_ = false;
		}

		void resize_delta(size_type delta, size_type min_buckets_wanted = 0)
		{
			if (consider_shrink_)
			{
				this->shrink();
			}
			if ((this->bucket_count() > min_buckets_wanted)
				&& (num_elements_ + delta <= enlarge_threshold_))
			{
				return;
			}

			size_type const resize_to = this->min_size(num_elements_ + delta,
				min_buckets_wanted);
			if (resize_to > this->bucket_count())
			{
				closed_hash_table tmp(*this, resize_to);
				this->swap(tmp);
			}
		}

		void expand_array(size_t resize_to, boost::true_type)
		{
			data_table_ = static_cast<value_type*>(realloc(data_table_, resize_to * sizeof(*data_table_)));
			BOOST_ASSERT(data_table_);
			tag_table_ = static_cast<char*>(realloc(tag_table_, resize_to * sizeof(*tag_table_)));
			BOOST_ASSERT(tag_table_);
			for (size_type i = num_buckets_; i < resize_to; ++ i)
			{
				this->set_tag(i, DHTAG_EMPTY);
			}
		}

		void expand_array(size_t resize_to, boost::false_type)
		{
			value_type* new_data_table = static_cast<value_type*>(malloc(resize_to * sizeof(*data_table_)));
			BOOST_ASSERT(new_data_table);
			std::uninitialized_fill(new_data_table, new_data_table + resize_to, value_type());
			for (size_type i = 0; i < num_buckets_; ++ i)
			{
				if (DHTAG_NORMAL == this->get_tag(i))
				{
					std::uninitialized_copy(data_table_ + i, data_table_ + i + 1, new_data_table);
				}
			}
			tag_table_ = static_cast<char*>(realloc(tag_table_, resize_to * sizeof(*tag_table_)));
			BOOST_ASSERT(tag_table_);
			for (size_type i = num_buckets_; i < resize_to; ++ i)
			{
				this->set_tag(i, DHTAG_EMPTY);
			}
			this->destroy_buckets(0, num_buckets_);
			free(data_table_);
			data_table_ = new_data_table;
		}

		void copy_from(closed_hash_table const & rhs, size_type min_buckets_wanted = 0)
		{
			this->clear();

			size_type const resize_to = this->min_size(rhs.size(), min_buckets_wanted);
			if (resize_to > this->bucket_count())
			{
				typedef boost::integral_constant<bool,
					(boost::has_trivial_copy<value_type>::value
					&& boost::has_trivial_destructor<value_type>::value)>
					realloc_ok;
				this->expand_array(resize_to, realloc_ok());
				num_buckets_ = resize_to;
				this->reset_thresholds();
			}

			BOOST_ASSERT(0 == (this->bucket_count() & (this->bucket_count() - 1)));
			for (const_iterator it = rhs.begin(); it != rhs.end(); ++ it)
			{
				size_type num_probes = 0;
				size_type bucknum;
				size_type const bucket_count_minus_one = this->bucket_count() - 1;
				for (bucknum = hash_(get_key_(*it)) & bucket_count_minus_one;
					this->get_tag(bucknum) != DHTAG_EMPTY;
					bucknum = (bucknum + this->reprobe(num_probes)) & bucket_count_minus_one)
				{
					++ num_probes;
					BOOST_ASSERT(num_probes < this->bucket_count());
				}
				this->set_value(&data_table_[bucknum], *it);
				this->set_tag(bucknum, it.ht_->get_tag(it));
				++ num_elements_;
			}
		}

		std::pair<size_type, size_type> find_position(key_type const & key) const
		{
			size_type num_probes = 0;
			size_type const bucket_count_minus_one = this->bucket_count() - 1;
			size_type bucknum = hash_(key) & bucket_count_minus_one;
			size_type insert_pos = ILLEGAL_BUCKET;
			for (;;)
			{
				if (DHTAG_EMPTY == this->get_tag(bucknum))
				{
					if (ILLEGAL_BUCKET == insert_pos)
					{
						return std::make_pair(ILLEGAL_BUCKET, bucknum);
					}
					else
					{
						return std::make_pair(ILLEGAL_BUCKET, insert_pos);
					}
				}
				else
				{
					if (DHTAG_DELETED == this->get_tag(bucknum))
					{
						if (ILLEGAL_BUCKET == insert_pos)
						{
							insert_pos = bucknum;
						}
					}
					else
					{
						if (equals_(key, get_key_(data_table_[bucknum])))
						{
							return std::make_pair(bucknum, ILLEGAL_BUCKET);
						}
					}
				}
				++ num_probes;
				bucknum = (bucknum + this->reprobe(num_probes)) & bucket_count_minus_one;
				BOOST_ASSERT(num_probes < this->bucket_count());
			}
		}

		std::pair<iterator, bool> insert_noresize(value_type const & obj)
		{
			std::pair<size_type, size_type> const pos = this->find_position(get_key_(obj));
			if (pos.first != ILLEGAL_BUCKET)
			{
				return std::make_pair(iterator(this, data_table_ + pos.first,
					data_table_ + num_buckets_, false), false);
			}
			else
			{
				++ num_elements_;
				this->set_value(&data_table_[pos.second], obj);
				this->set_tag(pos.second, DHTAG_NORMAL);
				return std::make_pair(iterator(this, data_table_ + pos.second,
					data_table_ + num_buckets_, false), true);
			}
		}

		template <typename ForwardIterator>
		void insert(ForwardIterator f, ForwardIterator l, std::forward_iterator_tag)
		{
			size_type n = std::distance(f, l);
			this->resize_delta(n);
			for (; n > 0; -- n, ++ f)
			{
				this->insert_noresize(*f);
			}
		}

		template <typename InputIterator>
		void insert(InputIterator f, InputIterator l, std::input_iterator_tag)
		{
			for (; f != l; ++ f)
			{
				this->insert(*f);
			}
		}

		void reset_thresholds()
		{
			enlarge_threshold_ = static_cast<size_type>(num_buckets_ * HT_OCCUPANCY_FLT);
			shrink_threshold_ = static_cast<size_type>(num_buckets_ * HT_EMPTY_FLT);
			consider_shrink_ = false;
		}

		void set_tag(size_type bucknum, dh_tag tag)
		{
			tag_table_[bucknum] = static_cast<char>(tag);
		}
		void set_tag(iterator const & iter, dh_tag tag)
		{
			this->set_tag(iter.pos_ - data_table_, tag);
		}
		void set_tag(const_iterator const & iter, dh_tag tag)
		{
			this->set_tag(iter.pos_ - data_table_, tag);
		}

		dh_tag get_tag(size_type bucknum) const
		{
			return static_cast<dh_tag>(tag_table_[bucknum]);
		}
		dh_tag get_tag(iterator const & iter) const
		{
			return this->get_tag(iter.pos_ - data_table_);
		}
		dh_tag get_tag(const_iterator const & iter) const
		{
			return this->get_tag(iter.pos_ - data_table_);
		}

	private:
		// How full we let the table get before we resize.  Knuth says .8 is
		// good -- higher causes us to probe too much, though saves memory
		static const float HT_OCCUPANCY_FLT;	// = 0.8;

		// How empty we let the table get before we resize lower.
		// It should be less than OCCUPANCY_FLT / 2 or we thrash resizing
		static const float HT_EMPTY_FLT;		// = 0.4 * HT_OCCUPANCY_FLT

		// Minimum size we're willing to let hashtables be.
		// Must be a power of two, and at least 4.
		// Note, however, that for a given hashtable, the initial size is a
		// function of the first constructor arg, and may be >HT_MIN_BUCKETS.
		static const size_t HT_MIN_BUCKETS = 32;

		// Because of the above, size_type(-1) is never legal; use it for errors
		static const size_type ILLEGAL_BUCKET = size_type(-1);

	private:
		hasher hash_;
		key_equal equals_;
		ExtractKey get_key_;

		value_type* data_table_;
		char* tag_table_;

		size_type num_buckets_;
		size_type num_elements_;
		size_type shrink_threshold_;			// num_buckets_ * HT_EMPTY_FLT
		size_type enlarge_threshold_;			// num_buckets_ * HT_OCCUPANCY_FLT
		bool consider_shrink_;					// true if we should try to shrink before next insert
	};

	// How full we let the table get before we resize.  Knuth says .8 is
	// good -- higher causes us to probe too much, though saves memory
	template <typename V, typename K, typename HF, typename ExK, typename EqK, typename A>
	const float closed_hash_table<V, K, HF, ExK, EqK, A>::HT_OCCUPANCY_FLT = 0.8f;

	// How empty we let the table get before we resize lower.
	// It should be less than OCCUPANCY_FLT / 2 or we thrash resizing
	template <typename V, typename K, typename HF, typename ExK, typename EqK, typename A>
	const float closed_hash_table<V, K, HF, ExK, EqK, A>::HT_EMPTY_FLT = 0.4f *
		closed_hash_table<V, K, HF, ExK, EqK, A>::HT_OCCUPANCY_FLT;
}

namespace std
{
	template <typename V, typename K, typename HF, typename ExK, typename EqK, typename A>
	inline void
	swap(KlayGE::closed_hash_table<V, K, HF, ExK, EqK, A>& lhs, KlayGE::closed_hash_table<V, K, HF, ExK, EqK, A>& rhs)
	{
		lhs.swap(rhs);
	}
}

#endif		// _CLOSEDHASHTABLE_HPP
