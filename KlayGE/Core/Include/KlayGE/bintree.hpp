// bintree.hpp
// KlayGE 二叉树模板 头文件
// Ver 1.2.8.10
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.enginedev.com
//
// 1.2.8.10
// 修正了SetLChild和SetRChild的bug (2002.10.26)
//
// 1.2.8.8
// 去掉了~bintree的virtual (2002.10.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _BINTREE_HPP
#define _BINTREE_HPP

#include <cstddef>
#include <KlayGE/SharedPtr.hpp>

namespace KlayGE
{
	template <typename T>
	class bintree
	{
	public:
		typedef T				value_type;
		typedef T*				pointer;
		typedef const T*		const_pointer;
		typedef T&				reference;
		typedef const T&		const_reference;
		typedef std::size_t		size_type;
		typedef std::ptrdiff_t	difference_type;

	private:
		T				root_;

		SharedPtr<bintree<T> >	lChild_;
		SharedPtr<bintree<T> >	rChild_;
		bintree<T>*				parent_;

	public:
		bintree(const T& root = T(),
					SharedPtr<bintree<T> > lChild = SharedPtr<bintree<T> >(),
					SharedPtr<bintree<T> > rChild = SharedPtr<bintree<T> >(),
					bintree<T>* parent = NULL)
			: root_(root), 
				lChild_(lChild), rChild_(rChild),
				parent_(parent)
			{ }
		~bintree()
		{
			this->parent_ = NULL;
		}

		void RootData(const_reference data)
			{ this->root_ = data; }

		reference RootData()
			{ return this->root_; }
		const_reference RootData() const
			{ return this->root_; }

		SharedPtr<bintree<T> >& LChild()
			{ return this->lChild_; }
		const SharedPtr<bintree<T> >& LChild() const
			{ return this->lChild_; }
		SharedPtr<bintree<T> >& RChild()
			{ return this->rChild_; }
		const SharedPtr<bintree<T> >& RChild() const
			{ return this->rChild_; }
		bintree<T>*& Parent()
			{ return this->parent_; }
		const bintree<T>* Parent() const
			{ return this->parent_; }

		void LChild(const SharedPtr<bintree<T> >& lChild)
		{
			this->lChild_ = lChild;
			this->lChild_->parent_ = this;
		}
		void RChild(const SharedPtr<bintree<T> >& rChild)
		{
			this->rChild_ = rChild;
			this->rChild_->parent_ = this;
		}

		void LChildData(const_reference data)
		{
			if (this->LChild() != NULL)
			{
				this->LChild()->RootData(data);
			}
			else
			{
				this->LChild = SharedPtr<bintree<T> >(new bintree<T>(data, NULL, NULL, this));
			}
		}
		void RChildData(const_reference Data)
		{
			if (this->RChild() != NULL)
			{
				this->RChild()->RootData(data);
			}
			else
			{
				this->RChild = SharedPtr<bintree<T> >(new bintree<T>(data, NULL, NULL, this));
			}
		}

		// 先序遍历
		template <typename Function>
		void PreOrder(Function op)
		{
			op(this->RootData());
			if (this->LChild() != NULL)
			{
				this->LChild()->PreOrder(op);
			}
			if (this->RChild() != NULL)
			{
				this->RChild()->PreOrder(op);
			}
		}

		// 中序遍历
		template <typename Function>
		void InOrder(Function op)
		{
			if (this->LChild() != NULL)
			{
				this->LChild()->InOrder(op);
			}
			op(this->RootData());
			if (this->RChild() != NULL)
			{
				this->RChild()->InOrder(op);
			}
		}

		// 后序遍历
		template <typename Function>
		void PostOrder(Function op)
		{
			if (this->LChild() != NULL)
			{
				this->LChild->PostOrder(op);
			}
			if (this->RChild() != NULL)
			{
				this->RChild->PostOrder(op);
			}
			op(this->RootData());
		}
	};
}

#endif		// _BINTREE_HPP