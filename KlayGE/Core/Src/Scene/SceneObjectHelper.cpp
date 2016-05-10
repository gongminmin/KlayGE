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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include <boost/assert.hpp>

#include <KlayGE/SceneObjectHelper.hpp>

namespace KlayGE
{
	SceneObjectHelper::SceneObjectHelper(uint32_t attrib)
		: SceneObject(attrib)
	{
	}

	SceneObjectHelper::SceneObjectHelper(RenderablePtr const & renderable, uint32_t attrib)
		: SceneObject(attrib)
	{
		renderable_ = renderable;
		renderable_hw_res_ready_ = renderable_->HWResourceReady();
		this->OnAttachRenderable(false);
	}

	void SceneObjectHelper::OnAttachRenderable(bool add_to_scene)
	{
		if (renderable_ && (renderable_->NumSubrenderables() > 0))
		{
			children_.resize(renderable_->NumSubrenderables());
			for (uint32_t i = 0; i < renderable_->NumSubrenderables(); ++ i)
			{
				SceneObjectHelperPtr child = MakeSharedPtr<SceneObjectHelper>(renderable_->Subrenderable(i), attrib_);
				child->Parent(this);
				children_[i] = child;

				if (add_to_scene)
				{
					child->AddToSceneManagerLocked();
				}
			}
		}
	}

	SceneObjectSkyBox::SceneObjectSkyBox(uint32_t attrib)
		: SceneObjectHelper(MakeSharedPtr<RenderableSkyBox>(), attrib | SOA_NotCastShadow)
	{
	}

	void SceneObjectSkyBox::Technique(RenderEffectPtr const & effect, RenderTechnique* tech)
	{
		checked_pointer_cast<RenderableSkyBox>(renderable_)->Technique(effect, tech);
	}

	void SceneObjectSkyBox::CubeMap(TexturePtr const & cube)
	{
		checked_pointer_cast<RenderableSkyBox>(renderable_)->CubeMap(cube);
	}

	void SceneObjectSkyBox::CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
	{
		checked_pointer_cast<RenderableSkyBox>(renderable_)->CompressedCubeMap(y_cube, c_cube);
	}


	SceneObjectLightSourceProxy::SceneObjectLightSourceProxy(LightSourcePtr const & light)
		: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_NotCastShadow),
			light_(light)
	{
		this->Init(light, CreateMeshFactory<RenderableLightSourceProxy>());
	}

	SceneObjectLightSourceProxy::SceneObjectLightSourceProxy(LightSourcePtr const & light, RenderModelPtr const & light_model)
		: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_NotCastShadow),
			light_(light)
	{
		this->Init(light, light_model);
	}

	SceneObjectLightSourceProxy::SceneObjectLightSourceProxy(LightSourcePtr const & light,
			std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
		: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_NotCastShadow),
			light_(light)
	{
		this->Init(light, CreateMeshFactoryFunc);
	}

	bool SceneObjectLightSourceProxy::MainThreadUpdate(float /*app_time*/, float /*elapsed_time*/)
	{
		model_ = model_scaling_ * MathLib::to_matrix(light_->Rotation()) * MathLib::translation(light_->Position());
		if (LightSource::LT_Spot == light_->Type())
		{
			float radius = light_->CosOuterInner().w();
			model_ = MathLib::scaling(radius, radius, 1.0f) * model_;
		}

		RenderModelPtr light_model = checked_pointer_cast<RenderModel>(renderable_);
		for (uint32_t i = 0; i < light_model->NumSubrenderables(); ++ i)
		{
			RenderableLightSourceProxyPtr light_mesh = checked_pointer_cast<RenderableLightSourceProxy>(light_model->Subrenderable(i));
			light_mesh->Update();
		}

		return false;
	}

	void SceneObjectLightSourceProxy::Scaling(float x, float y, float z)
	{
		model_scaling_ = MathLib::scaling(x, y, z);
	}

	void SceneObjectLightSourceProxy::Scaling(float3 const & s)
	{
		model_scaling_ = MathLib::scaling(s);
	}

	void SceneObjectLightSourceProxy::Init(LightSourcePtr const & light, RenderModelPtr const & light_model)
	{
		renderable_ = light_model;
		model_scaling_ = float4x4::Identity();

		children_.resize(light_model->NumSubrenderables());
		for (uint32_t i = 0; i < light_model->NumSubrenderables(); ++ i)
		{
			checked_pointer_cast<RenderableLightSourceProxy>(light_model->Subrenderable(i))->AttachLightSrc(light);

			SceneObjectHelperPtr child = MakeSharedPtr<SceneObjectHelper>(light_model->Subrenderable(i), attrib_);
			child->Parent(this);
			children_[i] = child;
		}
	}

	void SceneObjectLightSourceProxy::Init(LightSourcePtr const & light,
			std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
	{
		std::string mesh_name;
		switch (light->Type())
		{
		case LightSource::LT_Ambient:
			mesh_name = "ambient_light_proxy.meshml";
			break;

		case LightSource::LT_Point:
		case LightSource::LT_SphereArea:
			mesh_name = "point_light_proxy.meshml";
			break;

		case LightSource::LT_Directional:
		case LightSource::LT_Sun:
			mesh_name = "directional_light_proxy.meshml";
			break;

		case LightSource::LT_Spot:
			mesh_name = "spot_light_proxy.meshml";
			break;

		case LightSource::LT_TubeArea:
			mesh_name = "tube_light_proxy.meshml";
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
		RenderModelPtr light_model = SyncLoadModel(mesh_name.c_str(), EAH_GPU_Read | EAH_Immutable,
			CreateModelFactory<RenderModel>(), CreateMeshFactoryFunc);
		this->Init(light, light_model);
	}


	SceneObjectCameraProxy::SceneObjectCameraProxy(CameraPtr const & camera)
		: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_NotCastShadow),
			camera_(camera)
	{
		this->Init(camera, CreateMeshFactory<RenderableCameraProxy>());
	}

	SceneObjectCameraProxy::SceneObjectCameraProxy(CameraPtr const & camera, RenderModelPtr const & camera_model)
		: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_NotCastShadow),
			camera_(camera)
	{
		this->Init(camera, camera_model);
	}

	SceneObjectCameraProxy::SceneObjectCameraProxy(CameraPtr const & camera,
			std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
		: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_NotCastShadow),
			camera_(camera)
	{
		this->Init(camera, CreateMeshFactoryFunc);
	}

	void SceneObjectCameraProxy::SubThreadUpdate(float /*app_time*/, float /*elapsed_time*/)
	{
		model_ = model_scaling_ * camera_->InverseViewMatrix();
	}

	void SceneObjectCameraProxy::Scaling(float x, float y, float z)
	{
		model_scaling_ = MathLib::scaling(x, y, z);
	}

	void SceneObjectCameraProxy::Scaling(float3 const & s)
	{
		model_scaling_ = MathLib::scaling(s);
	}

	void SceneObjectCameraProxy::Init(CameraPtr const & camera, RenderModelPtr const & camera_model)
	{
		renderable_ = camera_model;
		model_scaling_ = float4x4::Identity();

		children_.resize(camera_model->NumSubrenderables());
		for (uint32_t i = 0; i < camera_model->NumSubrenderables(); ++ i)
		{
			checked_pointer_cast<RenderableCameraProxy>(camera_model->Subrenderable(i))->AttachCamera(camera);

			SceneObjectHelperPtr child = MakeSharedPtr<SceneObjectHelper>(camera_model->Subrenderable(i), attrib_);
			child->Parent(this);
			children_[i] = child;
		}
	}

	void SceneObjectCameraProxy::Init(CameraPtr const & camera,
			std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
	{
		RenderModelPtr camera_model = SyncLoadModel("camera_proxy.meshml", EAH_GPU_Read | EAH_Immutable,
			CreateModelFactory<RenderModel>(), CreateMeshFactoryFunc);
		this->Init(camera, camera_model);
	}
}
