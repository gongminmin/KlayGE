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
#include <KlayGE/AABBox.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneObjectHelper : public SceneObject
	{
	public:
		explicit SceneObjectHelper(uint32_t attrib);
		SceneObjectHelper(RenderablePtr const & renderable, uint32_t attrib);
		virtual ~SceneObjectHelper()
		{
		}
	};

	class KLAYGE_CORE_API SceneObjectSkyBox : public SceneObjectHelper
	{
	public:
		SceneObjectSkyBox(uint32_t attrib = 0);
		virtual ~SceneObjectSkyBox()
		{
		}

		void Technique(RenderTechniquePtr const & tech);
		void CubeMap(TexturePtr const & cube);

		void Pass(PassType type);
	};

	class KLAYGE_CORE_API SceneObjectHDRSkyBox : public SceneObjectSkyBox
	{
	public:
		SceneObjectHDRSkyBox(uint32_t attrib = 0);
		virtual ~SceneObjectHDRSkyBox()
		{
		}

		void Technique(RenderTechniquePtr const & tech);
		void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube);
	};

	class KLAYGE_CORE_API SceneObjectLightSourceProxy : public SceneObjectHelper
	{
	public:
		explicit SceneObjectLightSourceProxy(LightSourcePtr const & light);

		void Update();

		void Scaling(float x, float y, float z);
		void Scaling(float3 const & s);
		void Translation(float x, float y, float z);
		void Translation(float3 const & t);

		void Pass(PassType type);

	protected:
		float4x4 model_scaling_;
		float4x4 model_translation_;

		LightSourcePtr light_;
	};

	class KLAYGE_CORE_API SceneObjectCameraProxy : public SceneObjectHelper
	{
	public:
		explicit SceneObjectCameraProxy(CameraPtr const & camera);

		void Update();

		void EyePos(float x, float y, float z);
		void EyePos(float3 const & t);
		void LookAt(float x, float y, float z);
		void LookAt(float3 const & t);
		void UpVec(float x, float y, float z);
		void UpVec(float3 const & t);

	protected:
		float3 eye_pos_;
		float3 look_at_;
		float3 up_vec_;

		CameraPtr camera_;
	};
}

#endif		// _RENDERABLEHELPER_HPP
