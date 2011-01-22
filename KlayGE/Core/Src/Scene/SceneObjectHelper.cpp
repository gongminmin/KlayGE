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
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Light.hpp>

#include <boost/assert.hpp>
#include <boost/tuple/tuple.hpp>

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
	}

	SceneObjectSkyBox::SceneObjectSkyBox(uint32_t attrib)
		: SceneObjectHelper(MakeSharedPtr<RenderableSkyBox>(), attrib)
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

	SceneObjectHDRSkyBox::SceneObjectHDRSkyBox(uint32_t attrib)
		: SceneObjectSkyBox(attrib)
	{
		renderable_ = MakeSharedPtr<RenderableHDRSkyBox>();
	}

	void SceneObjectHDRSkyBox::Technique(RenderTechniquePtr const & tech)
	{
		checked_pointer_cast<RenderableHDRSkyBox>(renderable_)->Technique(tech);
	}

	void SceneObjectHDRSkyBox::CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
	{
		checked_pointer_cast<RenderableHDRSkyBox>(renderable_)->CompressedCubeMap(y_cube, c_cube);
	}


	SceneObjectLightSourceProxy::SceneObjectLightSourceProxy(LightSourcePtr const & light, float scale)
		: SceneObjectHelper(SOA_Cullable | SOA_Moveable),
			light_(light)
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
		renderable_ = LoadModel(mesh_name.c_str(), EAH_GPU_Read, CreateModelFactory<RenderModel>(), CreateMeshFactory<RenderableLightSourceProxy>())()->Mesh(0);
		model_org_ = MathLib::scaling(scale, scale, scale);

		checked_pointer_cast<RenderableLightSourceProxy>(renderable_)->AttachLightSrc(light);
	}

	void SceneObjectLightSourceProxy::Update()
	{
		model_ = model_org_ * MathLib::to_matrix(light_->Rotation()) * MathLib::translation(light_->Position());
		checked_pointer_cast<RenderableLightSourceProxy>(renderable_)->SetModelMatrix(model_);

		checked_pointer_cast<RenderableLightSourceProxy>(renderable_)->Update();
	}

	float4x4 const & SceneObjectLightSourceProxy::GetModelMatrix() const
	{
		return model_;
	}
}
