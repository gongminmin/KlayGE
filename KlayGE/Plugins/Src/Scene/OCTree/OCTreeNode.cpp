#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

namespace KlayGE
{
	OCTreeNode::OCTreeNode(const Box& box)
		: box_(box)
	{
	}

	void OCTreeNode::Clear()
	{
		renderables_.clear();
		for (int i = 0; i < 8; ++ i)
		{
			children_[i].reset();
		}
	}

	bool OCTreeNode::InsideNode(const RenderablePtr& renderable)
	{
		Box box(renderable->GetBound());

		if (MathLib::VecInBox(box_, box.LeftBottomNear())
			&& MathLib::VecInBox(box_, box.LeftTopNear())
			&& MathLib::VecInBox(box_, box.RightBottomNear())
			&& MathLib::VecInBox(box_, box.RightTopNear())
			&& MathLib::VecInBox(box_, box.LeftBottomFar())
			&& MathLib::VecInBox(box_, box.LeftTopFar())
			&& MathLib::VecInBox(box_, box.RightBottomFar())
			&& MathLib::VecInBox(box_, box.RightTopFar()))
		{
			return true;
		}

		return false;
	}

	void OCTreeNode::AddRenderable(const RenderablePtr& renderable)
	{
		if (!renderable->CanBeCulled())
		{
			SceneNode::AddRenderable(renderable);
		}
		else
		{
			if (this->InsideNode(renderable))
			{
				if (renderables_.size() > 1)
				{
					Vector3 center((box_.LeftBottomNear() + box_.RightTopFar()) / 2);

					children_[0] = OCTreeNodePtr(new OCTreeNode(Box(box_.LeftBottomNear(), center)));
					children_[1] = OCTreeNodePtr(new OCTreeNode(Box(box_.LeftTopNear(), center)));
					children_[2] = OCTreeNodePtr(new OCTreeNode(Box(box_.RightBottomNear(), center)));
					children_[3] = OCTreeNodePtr(new OCTreeNode(Box(box_.RightTopNear(), center)));
					children_[4] = OCTreeNodePtr(new OCTreeNode(Box(box_.LeftBottomFar(), center)));
					children_[5] = OCTreeNodePtr(new OCTreeNode(Box(box_.LeftTopFar(), center)));
					children_[6] = OCTreeNodePtr(new OCTreeNode(Box(box_.RightBottomFar(), center)));
					children_[7] = OCTreeNodePtr(new OCTreeNode(Box(box_.RightTopFar(), center)));

					bool inserted(false);
					for (int i = 0; i < 8; ++ i)
					{
						if (children_[i]->InsideNode(renderable))
						{
							children_[i]->AddRenderable(renderable);
							inserted = true;

							break;
						}
					}

					if (!inserted)
					{
						SceneNode::AddRenderable(renderable);
					}
				}
				else
				{
					SceneNode::AddRenderable(renderable);
				}
			}
		}
	}

	void OCTreeNode::Clip(const Frustum& frustum)
	{
		for (RenderablesType::iterator iter = renderables_.begin();
			iter != renderables_.end();)
		{
			if ((*iter)->CanBeCulled())
			{
				Box box((*iter)->GetBound());
				if (!frustum.Visiable(box.LeftBottomNear())
						|| !frustum.Visiable(box.RightTopFar()))
				{
					iter = renderables_.erase(iter);
				}
				else
				{
					++ iter;
				}
			}
			else
			{
				++ iter;
			}
		}

		for (int i = 0; i < 8; ++ i)
		{
			if ((children_[i].get() != NULL) && (!children_[i]->renderables_.empty()))
			{
				if (frustum.Visiable(children_[i]->box_.LeftBottomNear())
					&& frustum.Visiable(children_[i]->box_.RightTopFar()))
				{
					children_[i] = OCTreeNodePtr();
				}
				else
				{
					children_[i]->Clip(frustum);
				}
			}
		}
	}

	void OCTreeNode::GetRenderables(std::vector<RenderablePtr>& renderables)
	{
		renderables.insert(renderables.end(), renderables_.begin(), renderables_.end());
		for (int i = 0; i < 8; ++ i)
		{
			if (children_[i].get() != NULL)
			{
				renderables.insert(renderables.end(),
					children_[i]->renderables_.begin(), children_[i]->renderables_.end());
			}
		}
	}
}
