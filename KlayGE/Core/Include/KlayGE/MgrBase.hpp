// MgrBase.hpp
// KlayGE 管理器基类 头文件
// Ver 1.2.8.11
// 版权所有(C) 龚敏敏, 2002
// Homepage: http://www.enginedev.com
//
// 1.2.8.11
// 初次建立 (2002.11.20)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _MGRBASE_HPP
#define _MGRBASE_HPP

#include <KlayGE/alloc.hpp>

namespace KlayGE
{
	template <typename T, 
		typename container = std::map<size_t, T, std::less<size_t>, alloc<std::pair<size_t, T> > > >
	class ManagerBase
	{
	public:
		typedef typename container::iterator		iterator;
		typedef typename container::const_iterator	const_iterator;

		typedef typename container::size_type		size_type;

		typedef typename container::allocator_type	allocator_type;

	public:
		ManagerBase()
			{ }

		const_iterator begin() const
			{ return container_.begin(); }
		iterator begin()
			{ return container_.begin(); }
		const_iterator end() const
			{ return container_.end(); }
		iterator end()
			{ return container_.end(); }

		const_iterator find(size_t id) const
			{ return container_.find(id); }
		iterator find(size_t id)
			{ return container_.find(id); }
		bool exist(size_t id) const
			{ return (container_.count(id) != 0); }

		const T& data(const_iterator iter) const
			{ return iter->second; }
		T& data(iterator iter)
			{ return iter->second; }

		void insert(size_t id, const T& data)
			{ container_.insert(std::pair<size_t, T>(id, data)); }

		size_type erase(size_t id)
			{ return container_.erase(id); }

		void clear()
			{ container_.clear(); }

		bool empty() const
			{ return container_.empty(); }

		size_type size() const
			{ return container_.size(); }

	private:
		container container_;
	};
}

#endif		// _MGRBASE_HPP