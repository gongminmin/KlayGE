using System;
using System.Collections.Generic;
using KlayGE;

namespace KGEditor
{
	enum KGEditorCommandCode
	{
		ECC_SetSceneName,
		ECC_SetSkyboxName,
		ECC_SetControlMode,
		ECC_AddModel,
		ECC_AddLight,
		ECC_AddCamera,
		ECC_RemoveEntity,
		ECC_ClearEntities,
		ECC_SelectEntity,
		ECC_SetEntityName,
		ECC_HideEntity,
		ECC_SetEntityScaling,
		ECC_SetEntityRotation,
		ECC_SetEntityTranslation,
		ECC_SetActiveCamera,
		ECC_SetLightEnabled,
		ECC_SetLightAttrib,
		ECC_SetLightColor,
		ECC_SetLightFalloff,
		ECC_SetLightInnerAngle,
		ECC_SetLightOuterAngle,
		ECC_SetCameraLookAt,
		ECC_SetCameraUpVec,
		ECC_SetCameraFoV,
		ECC_SetCameraAspect,
		ECC_SetCameraNearPlane,
		ECC_SetCameraFarPlane,

		ECC_NumCommands
	}

	abstract class KGEditorCommand
	{
		public KGEditorCommand(KGEditorCoreWrapper core, KGEditorCommandCode code, string cmd_name)
		{
			core_ = core;
			code_ = code;
			cmd_name_ = cmd_name;
		}

		public string Name()
		{
			return cmd_name_;
		}
		public KGEditorCommandCode Code()
		{
			return code_;
		}

		public abstract object Execute();
		public abstract void Revoke();

		protected KGEditorCoreWrapper core_;
		protected KGEditorCommandCode code_;
		protected string cmd_name_;
	};

	class KGEditorCommandSetSceneName : KGEditorCommand
	{
		public KGEditorCommandSetSceneName(KGEditorCoreWrapper core, string name)
			: base(core, KGEditorCommandCode.ECC_SetSceneName, "Set scene name")
		{
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.SceneName();
			core_.SceneName(name_);
			return null;
		}

		public override void Revoke()
		{
			core_.SceneName(old_name_);
		}

		private string name_;
		private string old_name_;
	};

	class KGEditorCommandSetSkyboxName : KGEditorCommand
	{
		public KGEditorCommandSetSkyboxName(KGEditorCoreWrapper core, string name)
			: base(core, KGEditorCommandCode.ECC_SetSkyboxName, "Set skybox name")
		{
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.SkyboxName();
			core_.SkyboxName(name_);
			return null;
		}

		public override void Revoke()
		{
			core_.SkyboxName(old_name_);
		}

		private string name_;
		private string old_name_;
	};

	class KGEditorCommandSetControlMode : KGEditorCommand
	{
		public KGEditorCommandSetControlMode(KGEditorCoreWrapper core, KGEditorCoreWrapper.ControlMode mode)
			: base(core, KGEditorCommandCode.ECC_SetControlMode, "Set control mode")
		{
			mode_ = mode;
		}

		public override object Execute()
		{
			old_mode_ = core_.GetControlMode();
			core_.SetControlMode(mode_);
			return null;
		}

		public override void Revoke()
		{
			core_.SetControlMode(old_mode_);
		}

		private KGEditorCoreWrapper.ControlMode mode_;
		private KGEditorCoreWrapper.ControlMode old_mode_;
	};

	class KGEditorCommandAddModel : KGEditorCommand
	{
		public KGEditorCommandAddModel(KGEditorCoreWrapper core, string meshml_name)
			: base(core, KGEditorCommandCode.ECC_AddModel, "Add model")
		{
			name_ = meshml_name;
		}

		public override object Execute()
		{
			old_selected_entity_id_ = core_.SelectedEntity();
			entity_id_ = core_.AddModel(name_);
			core_.SelectEntity(entity_id_);
			return entity_id_;
		}

		public override void Revoke()
		{
			core_.RemoveEntity(entity_id_);
			core_.SelectEntity(old_selected_entity_id_);
		}

		private string name_;
		private uint entity_id_;
		private uint old_selected_entity_id_;
	};

	class KGEditorCommandAddLight : KGEditorCommand
	{
		public KGEditorCommandAddLight(KGEditorCoreWrapper core, KGEditorCoreWrapper.LightType type, string name)
			: base(core, KGEditorCommandCode.ECC_AddLight, "Add light")
		{
			type_ = type;
			name_ = name;
		}

		public override object Execute()
		{
			old_selected_entity_id_ = core_.SelectedEntity();
			entity_id_ = core_.AddLight(type_, name_);
			core_.SelectEntity(entity_id_);
			return entity_id_;
		}

		public override void Revoke()
		{
			core_.RemoveEntity(entity_id_);
			core_.SelectEntity(old_selected_entity_id_);
		}

		private KGEditorCoreWrapper.LightType type_;
		private string name_;
		private uint entity_id_;
		private uint old_selected_entity_id_;
	};

	class KGEditorCommandAddCamera : KGEditorCommand
	{
		public KGEditorCommandAddCamera(KGEditorCoreWrapper core, string name)
			: base(core, KGEditorCommandCode.ECC_AddCamera, "Add camera")
		{
			name_ = name;
		}

		public override object Execute()
		{
			old_selected_entity_id_ = core_.SelectedEntity();
			entity_id_ = core_.AddCamera(name_);
			core_.SelectEntity(entity_id_);
			return entity_id_;
		}

		public override void Revoke()
		{
			core_.RemoveEntity(entity_id_);
			core_.SelectEntity(old_selected_entity_id_);
		}

		private string name_;
		private uint entity_id_;
		private uint old_selected_entity_id_;
	};

	class KGEditorCommandRemoveEntity : KGEditorCommand
	{
		public KGEditorCommandRemoveEntity(KGEditorCoreWrapper core, uint entity_id)
			: base(core, KGEditorCommandCode.ECC_RemoveEntity, "Remove entity")
		{
			entity_id_ = entity_id;
		}

		// TODO: Destory the backup entity when dispose

		public override object Execute()
		{
			old_selected_entity_id_ = core_.SelectedEntity();
			if (old_selected_entity_id_ == entity_id_)
			{
				core_.SelectEntity(0);
			}

			backup_entity_id_ = core_.BackupEntityInfo(entity_id_);
			return null;
		}

		public override void Revoke()
		{
			core_.RestoreEntityInfo(entity_id_, backup_entity_id_);
			core_.SelectEntity(old_selected_entity_id_);
		}

		private uint entity_id_;
		private uint backup_entity_id_;
		private uint old_selected_entity_id_;
	};

	class KGEditorCommandClearEntities : KGEditorCommand
	{
		public KGEditorCommandClearEntities(KGEditorCoreWrapper core, KGEditorCoreWrapper.EntityType type)
			: base(core, KGEditorCommandCode.ECC_ClearEntities, "Clear entities")
		{
			type_ = type;
		}

		// TODO: Destory the backup entities when dispose

		public override object Execute()
		{
			uint n = core_.NumEntities();
			for (uint i = 0; i < n; ++ i)
			{
				uint id = core_.EntityIDByIndex(i);
				if (core_.GetEntityType(id) == type_)
				{
					backup_entity_ids_.Add(id, core_.BackupEntityInfo(id));
				}
			}

			switch (type_)
			{
				case KGEditorCoreWrapper.EntityType.ET_Model:
					core_.ClearModels();
					break;

				case KGEditorCoreWrapper.EntityType.ET_Light:
					core_.ClearLights();
					break;

				case KGEditorCoreWrapper.EntityType.ET_Camera:
				default:
					core_.ClearCameras();
					break;
			}

			return null;
		}

		public override void Revoke()
		{
			foreach (var item in backup_entity_ids_)
			{
				core_.RestoreEntityInfo(item.Key, item.Value);
			}
		}

		private KGEditorCoreWrapper.EntityType type_;
		private Dictionary<uint, uint> backup_entity_ids_ = new Dictionary<uint, uint>();
	};

	class KGEditorCommandSelectEntity : KGEditorCommand
	{
		public KGEditorCommandSelectEntity(KGEditorCoreWrapper core, uint id)
			: base(core, KGEditorCommandCode.ECC_SelectEntity, "Select entity")
		{
			id_ = id;
		}

		public override object Execute()
		{
			old_id_ = core_.SelectedEntity();
			core_.SelectEntity(id_);
			return null;
		}

		public override void Revoke()
		{
			core_.SelectEntity(old_id_);
		}

		private uint id_;
		private uint old_id_;
	};

	class KGEditorCommandSetEntityName : KGEditorCommand
	{
		public KGEditorCommandSetEntityName(KGEditorCoreWrapper core, uint id, string name)
			: base(core, KGEditorCommandCode.ECC_SetEntityName, "Set entity name")
		{
			id_ = id;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.EntityName(id_);
			core_.EntityName(id_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.EntityName(id_, old_name_);
		}

		private uint id_;
		private string name_;
		private string old_name_;
	};

	class KGEditorCommandHideEntity : KGEditorCommand
	{
		public KGEditorCommandHideEntity(KGEditorCoreWrapper core, uint id, bool hide)
			: base(core, KGEditorCommandCode.ECC_HideEntity, "Hide entity")
		{
			id_ = id;
			hide_ = hide;
		}

		public override object Execute()
		{
			old_hide_ = core_.HideEntity(id_);
			core_.HideEntity(id_, hide_);
			return null;
		}

		public override void Revoke()
		{
			core_.HideEntity(id_, old_hide_);
		}

		private uint id_;
		private bool hide_;
		private bool old_hide_;
	};

	class KGEditorCommandSetEntityScaling : KGEditorCommand
	{
		public KGEditorCommandSetEntityScaling(KGEditorCoreWrapper core, uint id, float[] scaling)
			: base(core, KGEditorCommandCode.ECC_SetEntityScaling, "Set entity scaling")
		{
			id_ = id;
			scaling_ = scaling;
		}

		public override object Execute()
		{
			old_scaling_ = core_.EntityScaling(id_);
			core_.EntityScaling(id_, scaling_);
			return null;
		}

		public override void Revoke()
		{
			core_.EntityScaling(id_, old_scaling_);
		}

		private uint id_;
		private float[] scaling_;
		private float[] old_scaling_;
	};

	class KGEditorCommandSetEntityRotation : KGEditorCommand
	{
		public KGEditorCommandSetEntityRotation(KGEditorCoreWrapper core, uint id, float[] rot_quat)
			: base(core, KGEditorCommandCode.ECC_SetEntityRotation, "Set entity rotation")
		{
			id_ = id;
			rot_ = rot_quat;
		}

		public override object Execute()
		{
			old_rot_ = core_.EntityRotation(id_);
			core_.EntityRotation(id_, rot_);
			return null;
		}

		public override void Revoke()
		{
			core_.EntityRotation(id_, old_rot_);
		}

		private uint id_;
		private float[] rot_;
		private float[] old_rot_;
	};

	class KGEditorCommandSetEntityTranslation : KGEditorCommand
	{
		public KGEditorCommandSetEntityTranslation(KGEditorCoreWrapper core, uint id, float[] trans)
			: base(core, KGEditorCommandCode.ECC_SetEntityTranslation, "Set entity translation")
		{
			id_ = id;
			trans_ = trans;
		}

		public override object Execute()
		{
			old_trans_ = core_.EntityTranslation(id_);
			core_.EntityTranslation(id_, trans_);
			return null;
		}

		public override void Revoke()
		{
			core_.EntityTranslation(id_, old_trans_);
		}

		private uint id_;
		private float[] trans_;
		private float[] old_trans_;
	};

	class KGEditorCommandSetActiveCamera : KGEditorCommand
	{
		public KGEditorCommandSetActiveCamera(KGEditorCoreWrapper core, uint id)
			: base(core, KGEditorCommandCode.ECC_SetActiveCamera, "Switch camera")
		{
			id_ = id;
		}

		public override object Execute()
		{
			old_id_ = core_.ActiveCamera();
			core_.ActiveCamera(id_);
			return null;
		}

		public override void Revoke()
		{
			core_.ActiveCamera(old_id_);
		}

		private uint id_;
		private uint old_id_;
	};

	class KGEditorCommandSetLightEnabled : KGEditorCommand
	{
		public KGEditorCommandSetLightEnabled(KGEditorCoreWrapper core, uint id, bool enabled)
			: base(core, KGEditorCommandCode.ECC_SetLightEnabled, "Set light enabled")
		{
			id_ = id;
			enabled_ = enabled;
		}

		public override object Execute()
		{
			old_enabled_ = core_.LightEnabled(id_);
			core_.LightEnabled(id_, enabled_);
			return null;
		}

		public override void Revoke()
		{
			core_.LightEnabled(id_, old_enabled_);
		}

		private uint id_;
		private bool enabled_;
		private bool old_enabled_;
	};

	class KGEditorCommandSetLightAttrib : KGEditorCommand
	{
		public KGEditorCommandSetLightAttrib(KGEditorCoreWrapper core, uint id, int attrib)
			: base(core, KGEditorCommandCode.ECC_SetLightAttrib, "Set light attribute")
		{
			id_ = id;
			attrib_ = attrib;
		}

		public override object Execute()
		{
			old_attrib_ = core_.LightAttrib(id_);
			core_.LightAttrib(id_, attrib_);
			return null;
		}

		public override void Revoke()
		{
			core_.LightAttrib(id_, old_attrib_);
		}

		private uint id_;
		private int attrib_;
		private int old_attrib_;
	};

	class KGEditorCommandSetLightColor : KGEditorCommand
	{
		public KGEditorCommandSetLightColor(KGEditorCoreWrapper core, uint id, float[] color)
			: base(core, KGEditorCommandCode.ECC_SetLightColor, "Set light color")
		{
			id_ = id;
			color_ = color;
		}

		public override object Execute()
		{
			old_color_ = core_.LightColor(id_);
			core_.LightColor(id_, color_);
			return null;
		}

		public override void Revoke()
		{
			core_.LightColor(id_, old_color_);
		}

		private uint id_;
		private float[] color_;
		private float[] old_color_;
	};

	class KGEditorCommandSetLightFalloff : KGEditorCommand
	{
		public KGEditorCommandSetLightFalloff(KGEditorCoreWrapper core, uint id, float[] falloff)
			: base(core, KGEditorCommandCode.ECC_SetLightFalloff, "Set light falloff")
		{
			id_ = id;
			falloff_ = falloff;
		}

		public override object Execute()
		{
			old_falloff_ = core_.LightFalloff(id_);
			core_.LightFalloff(id_, falloff_);
			return null;
		}

		public override void Revoke()
		{
			core_.LightFalloff(id_, old_falloff_);
		}

		private uint id_;
		private float[] falloff_;
		private float[] old_falloff_;
	};

	class KGEditorCommandSetLightInnerAngle : KGEditorCommand
	{
		public KGEditorCommandSetLightInnerAngle(KGEditorCoreWrapper core, uint id, float angle)
			: base(core, KGEditorCommandCode.ECC_SetLightInnerAngle, "Set light inner angle")
		{
			id_ = id;
			angle_ = angle;
		}

		public override object Execute()
		{
			old_angle_ = core_.LightInnerAngle(id_);
			core_.LightInnerAngle(id_, angle_);
			return null;
		}

		public override void Revoke()
		{
			core_.LightInnerAngle(id_, old_angle_);
		}

		private uint id_;
		private float angle_;
		private float old_angle_;
	};

	class KGEditorCommandSetLightOuterAngle : KGEditorCommand
	{
		public KGEditorCommandSetLightOuterAngle(KGEditorCoreWrapper core, uint id, float angle)
			: base(core, KGEditorCommandCode.ECC_SetLightOuterAngle, "Set light outer angle")
		{
			id_ = id;
			angle_ = angle;
		}

		public override object Execute()
		{
			old_angle_ = core_.LightOuterAngle(id_);
			core_.LightOuterAngle(id_, angle_);
			return null;
		}

		public override void Revoke()
		{
			core_.LightOuterAngle(id_, old_angle_);
		}

		private uint id_;
		private float angle_;
		private float old_angle_;
	};

	class KGEditorCommandSetProjectiveTex : KGEditorCommand
	{
		public KGEditorCommandSetProjectiveTex(KGEditorCoreWrapper core, uint id, string name)
			: base(core, KGEditorCommandCode.ECC_SetLightOuterAngle, "Set projective texture")
		{
			id_ = id;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.LightProjectiveTex(id_);
			core_.LightProjectiveTex(id_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.LightProjectiveTex(id_, old_name_);
		}

		private uint id_;
		private string name_;
		private string old_name_;
	};

	class KGEditorCommandSetCameraLookAt : KGEditorCommand
	{
		public KGEditorCommandSetCameraLookAt(KGEditorCoreWrapper core, uint id, float[] look_at)
			: base(core, KGEditorCommandCode.ECC_SetCameraLookAt, "Set camera look at")
		{
			id_ = id;
			look_at_ = look_at;
		}

		public override object Execute()
		{
			old_look_at_ = core_.CameraLookAt(id_);
			core_.CameraLookAt(id_, look_at_);
			return null;
		}

		public override void Revoke()
		{
			core_.CameraLookAt(id_, old_look_at_);
		}

		private uint id_;
		private float[] look_at_;
		private float[] old_look_at_;
	};

	class KGEditorCommandSetCameraUpVec : KGEditorCommand
	{
		public KGEditorCommandSetCameraUpVec(KGEditorCoreWrapper core, uint id, float[] up_vec)
			: base(core, KGEditorCommandCode.ECC_SetCameraUpVec, "Set camera up vec")
		{
			id_ = id;
			up_vec_ = up_vec;
		}

		public override object Execute()
		{
			old_up_vec_ = core_.CameraUpVec(id_);
			core_.CameraUpVec(id_, up_vec_);
			return null;
		}

		public override void Revoke()
		{
			core_.CameraUpVec(id_, old_up_vec_);
		}

		private uint id_;
		private float[] up_vec_;
		private float[] old_up_vec_;
	};

	class KGEditorCommandSetCameraFoV : KGEditorCommand
	{
		public KGEditorCommandSetCameraFoV(KGEditorCoreWrapper core, uint id, float fov)
			: base(core, KGEditorCommandCode.ECC_SetCameraFoV, "Set camera fov")
		{
			id_ = id;
			fov_ = fov;
		}

		public override object Execute()
		{
			old_fov_ = core_.CameraFoV(id_);
			core_.CameraFoV(id_, fov_);
			return null;
		}

		public override void Revoke()
		{
			core_.CameraFoV(id_, old_fov_);
		}

		private uint id_;
		private float fov_;
		private float old_fov_;
	};

	class KGEditorCommandSetCameraAspect : KGEditorCommand
	{
		public KGEditorCommandSetCameraAspect(KGEditorCoreWrapper core, uint id, float aspect)
			: base(core, KGEditorCommandCode.ECC_SetCameraAspect, "Set camera aspect")
		{
			id_ = id;
			aspect_ = aspect;
		}

		public override object Execute()
		{
			old_aspect_ = core_.CameraAspect(id_);
			core_.CameraAspect(id_, aspect_);
			return null;
		}

		public override void Revoke()
		{
			core_.CameraAspect(id_, old_aspect_);
		}

		private uint id_;
		private float aspect_;
		private float old_aspect_;
	};

	class KGEditorCommandSetCameraNearPlane : KGEditorCommand
	{
		public KGEditorCommandSetCameraNearPlane(KGEditorCoreWrapper core, uint id, float near_plane)
			: base(core, KGEditorCommandCode.ECC_SetCameraNearPlane, "Set camera near plane")
		{
			id_ = id;
			near_plane_ = near_plane;
		}

		public override object Execute()
		{
			old_near_plane_ = core_.CameraNearPlane(id_);
			core_.CameraNearPlane(id_, near_plane_);
			return null;
		}

		public override void Revoke()
		{
			core_.CameraNearPlane(id_, old_near_plane_);
		}

		private uint id_;
		private float near_plane_;
		private float old_near_plane_;
	};

	class KGEditorCommandSetCameraFarPlane : KGEditorCommand
	{
		public KGEditorCommandSetCameraFarPlane(KGEditorCoreWrapper core, uint id, float far_plane)
			: base(core, KGEditorCommandCode.ECC_SetCameraFarPlane, "Set camera far plane")
		{
			id_ = id;
			far_plane_ = far_plane;
		}

		public override object Execute()
		{
			old_far_plane_ = core_.CameraFarPlane(id_);
			core_.CameraFarPlane(id_, far_plane_);
			return null;
		}

		public override void Revoke()
		{
			core_.CameraFarPlane(id_, old_far_plane_);
		}

		private uint id_;
		private float far_plane_;
		private float old_far_plane_;
	};
}