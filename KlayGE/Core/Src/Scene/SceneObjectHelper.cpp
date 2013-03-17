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
		if (renderable_)
		{
			renderable_->ModelMatrix(model_);
		}
	}

	SceneObjectSkyBox::SceneObjectSkyBox(uint32_t attrib)
		: SceneObjectHelper(MakeSharedPtr<RenderableSkyBox>(), attrib | SOA_NotCastShadow)
	{
	}

	void SceneObjectSkyBox::Technique(RenderTechniquePtr const & tech)
	{
		checked_pointer_cast<RenderableSkyBox>(renderable_)->Technique(tech);
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
			function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
		: SceneObjectHelper(SOA_Cullable | SOA_Moveable | SOA_NotCastShadow),
			light_(light)
	{
		this->Init(light, CreateMeshFactoryFunc);
	}

	void SceneObjectLightSourceProxy::Update(float /*app_time*/, float /*elapsed_time*/)
	{
		model_ = model_scaling_ * MathLib::to_matrix(light_->Rotation()) * MathLib::translation(light_->Position())
			* model_translation_;
		if (LT_Spot == light_->Type())
		{
			float radius = light_->CosOuterInner().w();
			model_ = MathLib::scaling(radius, radius, 1.0f) * model_;
		}

		RenderModelPtr light_model = checked_pointer_cast<RenderModel>(renderable_);
		for (uint32_t i = 0; i < light_model->NumMeshes(); ++ i)
		{
			RenderableLightSourceProxyPtr light_mesh = checked_pointer_cast<RenderableLightSourceProxy>(light_model->Mesh(i));
			light_mesh->ModelMatrix(model_);
			light_mesh->Update();
		}
	}

	void SceneObjectLightSourceProxy::Scaling(float x, float y, float z)
	{
		model_scaling_ = MathLib::scaling(x, y, z);
	}

	void SceneObjectLightSourceProxy::Scaling(float3 const & s)
	{
		model_scaling_ = MathLib::scaling(s);
	}
	
	void SceneObjectLightSourceProxy::Translation(float x, float y, float z)
	{
		model_translation_ = MathLib::translation(x, y, z);
	}

	void SceneObjectLightSourceProxy::Translation(float3 const & t)
	{
		model_translation_ = MathLib::translation(t);
	}

	void SceneObjectLightSourceProxy::Init(LightSourcePtr const & light, RenderModelPtr const & light_model)
	{
		renderable_ = light_model;
		model_scaling_ = model_translation_ = float4x4::Identity();

		for (uint32_t i = 0; i < light_model->NumMeshes(); ++ i)
		{
			checked_pointer_cast<RenderableLightSourceProxy>(light_model->Mesh(i))->AttachLightSrc(light);
		}
	}

	void SceneObjectLightSourceProxy::Init(LightSourcePtr const & light,
			function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc)
	{
		std::string mesh_name;
		switch (light->Type())
		{
		case LT_Ambient:
			mesh_name = "ambient_light_proxy.meshml";
			break;

		case LT_Point:
			mesh_name = "point_light_proxy.meshml";
			break;
		
		case LT_Directional:
			mesh_name = "directional_light_proxy.meshml";
			break;

		case LT_Spot:
			mesh_name = "spot_light_proxy.meshml";
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
		eye_pos_ = camera_->EyePos();
		look_at_ = camera->LookAt();
		up_vec_ = camera->UpVec();;
	}

	void SceneObjectCameraProxy::Update(float /*app_time*/, float /*elapsed_time*/)
	{
		camera_->ViewParams(eye_pos_, look_at_, up_vec_);
		model_ = camera_->InverseViewMatrix();
	}

	void SceneObjectCameraProxy::EyePos(float x, float y, float z)
	{
		eye_pos_ = float3(x, y, z);
	}

	void SceneObjectCameraProxy::EyePos(float3 const & t)
	{
		eye_pos_ = t;
	}

	void SceneObjectCameraProxy::LookAt(float x, float y, float z)
	{
		look_at_ = float3(x, y, z);
	}

	void SceneObjectCameraProxy::LookAt(float3 const & t)
	{
		look_at_ = t;
	}

	void SceneObjectCameraProxy::UpVec(float x, float y, float z)
	{
		up_vec_ = float3(x, y, z);
	}

	void SceneObjectCameraProxy::UpVec(float3 const & t)
	{
		up_vec_ = t;
	}
}
