// SceneObjectHelper.hpp
// KlayGE 一些常用的场景对象 头文件
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
// 3.1.0
// 初次建立 (2005.10.31)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SCENEOBJECTHELPER_HPP
#define _SCENEOBJECTHELPER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KFL/AABBox.hpp>
#include <KlayGE/Mesh.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneObjectSkyBox : public SceneObject
	{
	public:
		explicit SceneObjectSkyBox(uint32_t attrib = 0);
		virtual ~SceneObjectSkyBox()
		{
		}

		void Technique(RenderEffectPtr const & effect, RenderTechnique* tech);
		void CubeMap(TexturePtr const & cube);
		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube);
	};

	class KLAYGE_CORE_API SceneObjectLightSourceProxy : public SceneObject
	{
	public:
		explicit SceneObjectLightSourceProxy(LightSourcePtr const & light);
		SceneObjectLightSourceProxy(LightSourcePtr const & light, RenderModelPtr const & light_model);
		SceneObjectLightSourceProxy(LightSourcePtr const & light,
			std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc);

		virtual bool MainThreadUpdate(float app_time, float elapsed_time) override;

		void Scaling(float x, float y, float z);
		void Scaling(float3 const & s);

		RenderModelPtr const & LightModel() const
		{
			return light_model_;
		}

	private:
		RenderModelPtr LoadModel(LightSourcePtr const & light,
			std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc);

	protected:
		float4x4 model_scaling_;

		LightSourcePtr light_;
		RenderModelPtr light_model_;
	};

	class KLAYGE_CORE_API SceneObjectCameraProxy : public SceneObject
	{
	public:
		explicit SceneObjectCameraProxy(CameraPtr const & camera);
		SceneObjectCameraProxy(CameraPtr const & camera, RenderModelPtr const & camera_model);
		SceneObjectCameraProxy(CameraPtr const & camera,
			std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc);

		virtual void SubThreadUpdate(float app_time, float elapsed_time) override;

		void Scaling(float x, float y, float z);
		void Scaling(float3 const & s);

		RenderModelPtr const & CameraModel() const
		{
			return camera_model_;
		}

	private:
		RenderModelPtr LoadModel(std::function<StaticMeshPtr(RenderModelPtr const &, std::wstring const &)> CreateMeshFactoryFunc);

	protected:
		float4x4 model_scaling_;

		CameraPtr camera_;
		RenderModelPtr camera_model_;
	};
}

#endif		// _RENDERABLEHELPER_HPP
