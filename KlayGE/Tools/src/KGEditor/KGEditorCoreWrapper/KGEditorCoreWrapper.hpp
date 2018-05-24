#ifndef _KGEDITOR_CORE_WRAPPER_HPP
#define _KGEDITOR_CORE_WRAPPER_HPP

#pragma once

#include "../KGEditorCore/Common.hpp"

namespace KlayGE
{
	public ref class KGEditorCoreWrapper
	{
	public:
		enum class ControlMode
		{
			CM_EntitySelection = 0,
			CM_EntityPosition,
			CM_EntityRotation,
			CM_EntityScale
		};

		enum class LightType
		{
			LT_Ambient = LightSource::LT_Ambient,
			LT_Directional,
			LT_Point,
			LT_Spot,
			LT_SphereArea,
			LT_TubeArea,
		};

		enum class LightSrcAttrib
		{
			LSA_NoShadow = LightSource::LSA_NoShadow,
			LSA_NoDiffuse = LightSource::LSA_NoDiffuse,
			LSA_NoSpecular = LightSource::LSA_NoSpecular,
			LSA_IndirectLighting = LightSource::LSA_IndirectLighting,
			LSA_Temporary = LightSource::LSA_Temporary
		};

	public:
		explicit KGEditorCoreWrapper(System::IntPtr native_wnd);
		~KGEditorCoreWrapper();

		void Refresh();
		void Resize(uint32_t width, uint32_t height);

		void LoadScene(System::String^ file_name);
		void SaveScene(System::String^ file_name);
		void CloseScene();

		System::String^ SceneName();
		void SceneName(System::String^ name);

		System::String^ SkyboxName();
		void SkyboxName(System::String^ name);

		void DisplaySSVO(bool ssvo);
		void DisplayHDR(bool hdr);
		void DisplayAA(bool aa);
		void DisplayGamma(bool gamma);
		void DisplayColorGrading(bool cg);

		void SetControlMode(ControlMode mode);

		uint32_t AddModel(System::String^ meshml_name);
		void ClearModels();

		uint32_t AddLight(LightType type, System::String^ name);
		void ClearLights();

		uint32_t AddCamera(System::String^ name);
		void ClearCameras();

		void RemoveEntity(uint32_t id);
		void SelectEntity(uint32_t id);

		System::String^ EntityName(uint32_t id);
		void EntityName(uint32_t id, System::String^ name);
		bool EntityVisible(uint32_t id);
		void EntityVisible(uint32_t id, bool visible);
		array<float>^ EntityScale(uint32_t id);
		void EntityScale(uint32_t id, array<float>^ s);
		array<float>^ EntityRotation(uint32_t id);
		void EntityRotation(uint32_t id, array<float>^ r);
		array<float>^ EntityPosition(uint32_t id);
		void EntityPosition(uint32_t id, array<float>^ t);
		array<float>^ EntityPivot(uint32_t id);
		void EntityPivot(uint32_t id, array<float>^ t);

		LightType GetLightType(uint32_t id);
		bool LightEnabled(uint32_t id);
		void LightEnabled(uint32_t id, bool enabled);
		int32_t LightAttrib(uint32_t id);
		void LightAttrib(uint32_t id, int32_t attrib);
		array<float>^ LightColor(uint32_t id);
		void LightColor(uint32_t id, array<float>^ color);
		array<float>^ LightFalloff(uint32_t id);
		void LightFalloff(uint32_t id, array<float>^ falloff);
		float LightInnerAngle(uint32_t id);
		void LightInnerAngle(uint32_t id, float angle);
		float LightOuterAngle(uint32_t id);
		void LightOuterAngle(uint32_t id, float angle);
		System::String^ LightProjectiveTex(uint32_t id);
		void LightProjectiveTex(uint32_t id, System::String^ name);

		array<float>^ CameraLookAt(uint32_t id);
		void CameraLookAt(uint32_t id, array<float>^ look_at);
		array<float>^ CameraUpVec(uint32_t id);
		void CameraUpVec(uint32_t id, array<float>^ up_vec);
		float CameraFoV(uint32_t id);
		void CameraFoV(uint32_t id, float fov);
		float CameraAspect(uint32_t id);
		void CameraAspect(uint32_t id, float aspect);
		float CameraNearPlane(uint32_t id);
		void CameraNearPlane(uint32_t id, float near_plane);
		float CameraFarPlane(uint32_t id);
		void CameraFarPlane(uint32_t id, float far_plane);

		uint32_t ActiveCameraId();
		void ActiveCameraId(uint32_t id);

		void MouseMove(int x, int y, uint32_t button);
		void MouseDown(int x, int y, uint32_t button);
		void MouseUp(int x, int y, uint32_t button);

	public:
		delegate void UpdatePropertyDelegate();
		delegate void UpdateSelectEntityDelegate(uint32_t obj_id);
		delegate void AddModelDelegate(System::String^ name);
		delegate void AddLightDelegate(LightType type);
		delegate void AddCameraDelegate();

		void UpdatePropertyCallback(UpdatePropertyDelegate^ callback);
		void UpdateSelectEntityCallback(UpdateSelectEntityDelegate^ callback);
		void AddModelCallback(AddModelDelegate^ callback);
		void AddLightCallback(AddLightDelegate^ callback);
		void AddCameraCallback(AddCameraDelegate^ callback);

	private:
		delegate void AddModelDelegateWithCString(char const * name);

		void AddModelDelegateWithCStringFunc(char const * name);

	private:
		KGEditorCore* core_;

		UpdatePropertyDelegate^ update_property_delegate_;
		UpdateSelectEntityDelegate^ update_select_entity_delegate_;
		AddModelDelegate^ add_model_delegate_;
		AddModelDelegateWithCString^ add_model_delegate_with_c_string_;
		AddLightDelegate^ add_light_delegate_;
		AddCameraDelegate^ add_camera_delegate_;
	};
}

#endif		// _KGEDITOR_CORE_WRAPPER_HPP
