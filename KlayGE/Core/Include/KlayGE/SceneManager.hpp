#ifndef _SCENEMANAGER_HPP
#define _SCENEMANAGER_HPP

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/SharePtr.hpp>
#include <KlayGE/alloc.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/MathTypes.hpp>
#include <KlayGE/MapVector.hpp>

#include <vector>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	struct ViewPoint
	{
		Vector3		pos;
		Quaternion	quat;
	};

	typedef SharePtr<ViewPoint> ViewPointPtr;

	class SceneManager
	{
	protected:
		typedef std::pair<VertexBufferPtr, Matrix4>					RenderItemType;
		typedef std::vector<RenderItemType, alloc<RenderItemType> >	RenderItemsType;
		typedef MapVector<RenderEffectPtr, RenderItemsType>			RenderQueueType;

	public:
		SceneManager();
		virtual ~SceneManager()
			{ }

		void PushRenderable(const RenderablePtr& obj, const Matrix4& worldMat = Matrix4::Identity());

		virtual void ClipScene(const ViewPoint& viewPoint)
			{ }

		virtual void Update(const ViewPointPtr& viewPoint = ViewPointPtr());

	protected:
		RenderQueueType renderQueue_;

	private:
		SceneManager(const SceneManager&);
		SceneManager& operator=(const SceneManager&);
	};
}

#endif			// _SCENEMANAGER_HPP