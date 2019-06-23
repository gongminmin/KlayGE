#ifndef _KGEDITOR_CORE_HPP
#define _KGEDITOR_CORE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/SceneNode.hpp>

#include "PreDeclare.hpp"
#include "Common.hpp"

#include <map>

namespace KlayGE
{
	class KLAYGE_KGEDITOR_CORE_API KGEditorCore : public App3DFramework
	{
	public:
		enum ControlMode
		{
			CM_EntitySelection,
			CM_EntityTranslation,
			CM_EntityRotation,
			CM_EntityScale
		};

		enum EntityType
		{
			ET_Model,
			ET_Light,
			ET_Camera
		};

		struct EntityInfo
		{
			std::string name;
			EntityType type;
			RenderModelPtr model;
			std::string meshml_name;
			LightSourcePtr light;
			std::string projective_tex_name;
			CameraPtr camera;
			OBBox obb;
			float3 trf_pivot;
			float3 trf_pos;
			float3 trf_scale;
			Quaternion trf_rotate;
			SceneNodePtr scene_node;
		};

	private:
		enum SelectedAxis
		{
			SA_None = 0,
			SA_X = 1UL << 0,
			SA_Y = 1UL << 1,
			SA_Z = 1UL << 2,
			SA_XY = SA_X | SA_Y,
			SA_YZ = SA_Y | SA_Z,
			SA_XZ = SA_X | SA_Z,
			SA_XYZ = SA_X | SA_Y | SA_Z
		};

	public:
		explicit KGEditorCore(void* native_wnd);

		void Resize(uint32_t width, uint32_t height);

		void LoadScene(std::string const & file_name);
		void SaveScene(std::string const & file_name);
		void CloseScene();

		std::string const & SceneName() const;
		void SceneName(std::string const & name);

		std::string const & SkyboxName() const;
		void SkyboxName(std::string const & name);

		void DisplaySSVO(bool ssvo);
		void DisplayHDR(bool hdr);
		void DisplayAA(bool aa);
		void DisplayGamma(bool gamma);
		void DisplayColorGrading(bool cg);

		void SetControlMode(ControlMode mode);

		uint32_t AddModel(std::string const & meshml_name);
		void ClearModels();

		uint32_t AddLight(LightSource::LightType type, std::string const & name);
		void ClearLights();

		uint32_t AddCamera(std::string const & name);
		void ClearCameras();

		void RemoveEntity(uint32_t id);
		void SelectEntity(uint32_t id);

		std::string const & EntityName(uint32_t id);
		void EntityName(uint32_t id, std::string const & name);
		bool EntityVisible(uint32_t id);
		void EntityVisible(uint32_t id, bool visible);
		LightSourcePtr const & GetLight(uint32_t id) const;
		std::string const & LightProjectiveTexName(uint32_t id) const;
		void LightProjectiveTexName(uint32_t id, std::string const & name);
		CameraPtr const & GetCamera(uint32_t id) const;
		float3 const & EntityScale(uint32_t id) const;
		void EntityScale(uint32_t id, float3 const & s);
		Quaternion const & EntityRotation(uint32_t id) const;
		void EntityRotation(uint32_t id, Quaternion const & r);
		float3 const & EntityPosition(uint32_t id) const;
		void EntityPosition(uint32_t id, float3 const & t);
		float3 const & EntityPivot(uint32_t id) const;
		void EntityPivot(uint32_t id, float3 const & t);

		uint32_t ActiveCameraId() const;
		void ActiveCameraId(uint32_t id);

		void MouseMove(int x, int y, uint32_t button);
		void MouseDown(int x, int y, uint32_t button);
		void MouseUp(int x, int y, uint32_t button);

	// Callbacks
	public:
		typedef void (__stdcall *UpdatePropertyEvent)();
		typedef void (__stdcall *UpdateSelectEntityEvent)(uint32_t obj_id);
		typedef void (__stdcall *AddModelEvent)(char const * name);
		typedef void (__stdcall *AddLightEvent)(LightSource::LightType type);
		typedef void (__stdcall *AddCameraEvent)();

		void UpdatePropertyCallback(UpdatePropertyEvent callback)
		{
			update_property_event_ = callback;
		}
		void UpdateSelectEntityCallback(UpdateSelectEntityEvent callback)
		{
			update_select_entity_event_ = callback;
		}
		void AddModelCallback(AddModelEvent callback)
		{
			add_model_event_ = callback;
		}
		void AddLightCallback(AddLightEvent callback)
		{
			add_light_event_ = callback;
		}
		void AddCameraCallback(AddCameraEvent callback)
		{
			add_camera_event_ = callback;
		}

	private:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnResize(uint32_t width, uint32_t height) override;
		virtual void DoUpdateOverlay() override;
		virtual uint32_t DoUpdate(uint32_t pass) override;

		void UpdateSelectedEntity(bool callback);
		void UpdateEntityAxis();
		void UpdateHelperObjs();
		void UpdateSceneAABB();
		void UpdateProxyScaling();
		float4x4 CalcAdaptiveScaling(EntityInfo const & ei, uint32_t pixels, float3& scaling);
		void LoadTransformNodes(XMLNodePtr const & node, EntityInfo& ei);
		void SaveTransformNodes(std::ostream& os, EntityInfo const & ei);

		EntityInfo& GetEntityInfo(uint32_t id);
		EntityInfo const & GetEntityInfo(uint32_t id) const;

	private:
		FontPtr font_;

		DeferredRenderingLayer* deferred_rendering_;

		std::string scene_name_;
		std::string skybox_name_;
		TexturePtr skybox_y_cube_;
		TexturePtr skybox_c_cube_;

		LightSourcePtr ambient_light_;

		std::map<uint32_t, EntityInfo> entities_;
		std::map<uint32_t, EntityInfo> backup_entities_;
		uint32_t selected_entity_;
		SceneNodePtr axis_;
		SceneNodePtr grid_;
		SceneNodePtr skybox_;
		SceneNodePtr selected_bb_;
		SceneNodePtr translation_axis_;
		SceneNodePtr rotation_axis_;
		SceneNodePtr scaling_axis_;
		float3 display_scaling_for_axis_;
		CameraPtr system_camera_;
		uint32_t active_camera_id_;

		ControlMode ctrl_mode_;
		SelectedAxis selected_axis_;
		bool mouse_down_in_wnd_;
		bool mouse_tracking_mode_;
		int2 last_mouse_pt_;
		TrackballCameraController tb_controller_;

		bool update_selective_buffer_;
		FrameBufferPtr selective_fb_;
		TexturePtr selective_tex_;
		TexturePtr selective_cpu_tex_;

		uint32_t last_entity_id_;

		AABBox scene_aabb_;

		UpdatePropertyEvent update_property_event_;
		UpdateSelectEntityEvent update_select_entity_event_;
		AddModelEvent add_model_event_;
		AddLightEvent add_light_event_;
		AddCameraEvent add_camera_event_;
	};
}

#endif		// _KGEDITOR_CORE_HPP
