// OCTreeNode.cpp
// KlayGE 八叉树结点类 实现文件
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

#include <KlayGE/OCTree/OCTree.hpp>

namespace KlayGE
{
	OCTreeNode::OCTreeNode(Box const & box)
		: box_(box)
	{
	}

	OCTreeNode::~OCTreeNode()
	{
	}

	void OCTreeNode::Clear()
	{
		renderables_.clear();
		for (ChildrenType::iterator iter = children_.begin(); iter != children_.end(); ++ iter)
		{
			iter->reset();
		}
	}

	bool OCTreeNode::InsideNode(RenderablePtr const & renderable)
	{
		Box const & box(renderable->GetBound());

		boost::array<Vector3, 8> vecs;
		vecs[0] = box.LeftBottomNear();
		vecs[1] = box.LeftTopNear();
		vecs[2] = box.RightBottomNear();
		vecs[3] = box.RightTopNear();
		vecs[4] = box.LeftBottomFar();
		vecs[5] = box.LeftTopFar();
		vecs[6] = box.RightBottomFar();
		vecs[7] = box.RightTopFar();

		for (size_t i = 0; i < vecs.size(); ++ i)
		{
			MathLib::TransformCoord(vecs[i], vecs[i], renderable->GetWorld());
			if (!MathLib::VecInBox(box_, vecs[i]))
			{
				return false;
			}
		}
		return true;
	}

	void OCTreeNode::AddRenderable(RenderablePtr const & renderable)
	{
		
	}

	void OCTreeNode::Clip(OCTreeFrustum const & frustum)
	{
		for (ChildrenType::iterator iter = children_.begin(); iter != children_.end(); ++ iter)
		{
			OCTreeNodePtr& child(*iter);

			if (child && (!child->renderables_.empty()))
			{
				Box& box(child->box_);
				if (frustum.Visiable(box.LeftBottomNear())
						|| frustum.Visiable(box.LeftTopNear())
						|| frustum.Visiable(box.RightBottomNear())
						|| frustum.Visiable(box.RightTopNear())
						|| frustum.Visiable(box.LeftBottomFar())
						|| frustum.Visiable(box.LeftTopFar())
						|| frustum.Visiable(box.RightBottomFar())
						|| frustum.Visiable(box.RightTopFar()))
				{
					child->Clip(frustum);
				}
				else
				{
					child.reset();
				}
			}
		}
	}

	void OCTreeNode::GetRenderables(std::vector<RenderablePtr>& renderables)
	{
		renderables.insert(renderables.end(), renderables_.begin(), renderables_.end());

		for (ChildrenType::iterator iter = children_.begin(); iter != children_.end(); ++ iter)
		{
			if (*iter)
			{
				(*iter)->GetRenderables(renderables);
			}
		}
	}
}
