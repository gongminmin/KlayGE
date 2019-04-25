// SceneObjectHelper.cpp
// KlayGE 一些常用的可渲染对象 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2005-2011
// Homepage: http://www.klayge.org
//
// 3.10.0
// SceneObjectSkyBox和SceneObjectHDRSkyBox增加了Technique() (2010.1.4)
//
// 3.9.0
// 增加了SceneObjectHDRSkyBox (2009.5.4)
//
// 2.7.1
// 增加了RenderableHelper基类 (2005.7.10)
//
// 2.6.0
// 增加了RenderableSkyBox (2005.5.26)
//
// 2.5.0
// 增加了RenderablePoint，RenderableLine和RenderableTriangle (2005.4.13)
//
// 2.4.0
// 初次建立 (2005.3.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include <boost/assert.hpp>

#include <KlayGE/SceneNodeHelper.hpp>

namespace KlayGE
{
	RenderModelPtr LoadLightSourceProxyModel(
		LightSourcePtr const& light, std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc)
	{
		char const* mesh_name;
		switch (light->Type())
		{
		case LightSource::LT_Ambient:
			mesh_name = "AmbientLightProxy.glb";
			break;

		case LightSource::LT_Point:
		case LightSource::LT_SphereArea:
			mesh_name = "PointLightProxy.glb";
			break;

		case LightSource::LT_Directional:
			mesh_name = "DirectionalLightProxy.glb";
			break;

		case LightSource::LT_Spot:
			mesh_name = "SpotLightProxy.glb";
			break;

		case LightSource::LT_TubeArea:
			mesh_name = "TubeLightProxy.glb";
			break;

		default:
			KFL_UNREACHABLE("Invalid light type");
		}

		auto light_model = SyncLoadModel(mesh_name, EAH_GPU_Read | EAH_Immutable,
			SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow, nullptr, CreateModelFactory<RenderModel>,
			CreateMeshFactoryFunc);

		for (uint32_t i = 0; i < light_model->NumMeshes(); ++i)
		{
			checked_pointer_cast<RenderableLightSourceProxy>(light_model->Mesh(i))->AttachLightSrc(light);
		}

		if (light->Type() == LightSource::LT_Spot)
		{
			float const radius = light->CosOuterInner().w();
			light_model->RootNode()->TransformToParent(
				MathLib::scaling(radius, radius, 1.0f) * light_model->RootNode()->TransformToParent());
		}

		light_model->RootNode()->OnMainThreadUpdate().Connect([](SceneNode& root_node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(app_time);
			KFL_UNUSED(elapsed_time);

			root_node.Traverse([](SceneNode& node)
			{
				node.ForEachComponentOfType<RenderableComponent>([](RenderableComponent& renderable_comp)
				{
					renderable_comp.BoundRenderableOfType<RenderableLightSourceProxy>().Update();
				});

				return true;
			});
		});

		return light_model;
	}

	RenderModelPtr LoadCameraProxyModel(CameraPtr const& camera, std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc)
	{
		auto camera_model = SyncLoadModel("CameraProxy.glb", EAH_GPU_Read | EAH_Immutable,
			SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow,
			nullptr, CreateModelFactory<RenderModel>, CreateMeshFactoryFunc);

		for (uint32_t i = 0; i < camera_model->NumMeshes(); ++i)
		{
			checked_pointer_cast<RenderableCameraProxy>(camera_model->Mesh(i))->AttachCamera(camera);
		}

		camera_model->RootNode()->OnMainThreadUpdate().Connect([&camera](SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(app_time);
			KFL_UNUSED(elapsed_time);

			node.TransformToParent(camera->InverseViewMatrix());
		});

		return camera_model;
	}
}
