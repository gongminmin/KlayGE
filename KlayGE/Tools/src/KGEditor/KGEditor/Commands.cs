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
		ECC_SetEntityVisible,
		ECC_SetEntityScale,
		ECC_SetEntityRotation,
		ECC_SetEntityTranslation,
		ECC_SetActiveCamera,
		ECC_SetLightEnabled,
		ECC_SetLightAttrib,
		ECC_SetLightColor,
		ECC_SetLightFalloff,
		ECC_SetLightInnerAngle,
		ECC_SetLightOuterAngle,
		ECC_SetLightProjectiveTex,
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
		public KGEditorCommand(KGEditorCommandCode code, string cmd_name)
		{
			code_ = code;
			cmd_name_ = cmd_name;
		}

		public string Name
		{
			get
			{
				return cmd_name_;
			}
		}
		public KGEditorCommandCode Code
		{
			get
			{
				return code_;
			}
		}

		public abstract object Execute();
		public abstract void Revoke();

		protected readonly KGEditorCommandCode code_;
		protected readonly string cmd_name_;
	};

	class KGEditorCommandSetSceneName : KGEditorCommand
	{
		public KGEditorCommandSetSceneName(Scene scene, string name)
			: base(KGEditorCommandCode.ECC_SetSceneName, "Set scene name")
		{
			scene_ = scene;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = scene_.Name;
			scene_.Name = name_;
			return null;
		}

		public override void Revoke()
		{
			scene_.Name = old_name_;
		}

		private readonly Scene scene_;
		private readonly string name_;
		private string old_name_;
	};

	class KGEditorCommandSetSkyboxName : KGEditorCommand
	{
		public KGEditorCommandSetSkyboxName(Scene scene, string name)
			: base(KGEditorCommandCode.ECC_SetSkyboxName, "Set skybox name")
		{
			scene_ = scene;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = scene_.SkyboxName;
			scene_.SkyboxName = name_;
			return null;
		}

		public override void Revoke()
		{
			scene_.SkyboxName = old_name_;
		}

		private readonly Scene scene_;
		private readonly string name_;
		private string old_name_;
	};

	class KGEditorCommandSetControlMode : KGEditorCommand
	{
		public KGEditorCommandSetControlMode(MainWindow wnd, KGEditorCoreWrapper.ControlMode mode)
			: base(KGEditorCommandCode.ECC_SetControlMode, "Set control mode")
		{
			wnd_ = wnd;
			mode_ = mode;
		}

		public override object Execute()
		{
			old_mode_ = (KGEditorCoreWrapper.ControlMode)wnd_.ControlModeProperty;
			wnd_.DoSetControlMode(mode_);
			return null;
		}

		public override void Revoke()
		{
			wnd_.DoSetControlMode(old_mode_);
		}

		private readonly MainWindow wnd_;
		private readonly KGEditorCoreWrapper.ControlMode mode_;
		private KGEditorCoreWrapper.ControlMode old_mode_;
	};

	class KGEditorCommandAddModel : KGEditorCommand
	{
		public KGEditorCommandAddModel(KGEditorCoreWrapper core, MainWindow wnd, Scene scene, string meshml_name)
			: base(KGEditorCommandCode.ECC_AddModel, "Add model")
		{
			core_ = core;
			wnd_ = wnd;
			scene_ = scene;
			model_name_ = meshml_name;
		}

		public override object Execute()
		{
			old_selected_entity_id_ = wnd_.SelectedEntityId;
			entity_id_ = core_.AddModel(model_name_);
			scene_.AddEntity(entity_id_, SceneEntityType.ET_Model);
			wnd_.SelectedEntityId = entity_id_;
			return entity_id_;
		}

		public override void Revoke()
		{
			scene_.RemoveEntity(entity_id_);
			wnd_.SelectedEntityId = old_selected_entity_id_;
		}

		private readonly KGEditorCoreWrapper core_;
		private readonly MainWindow wnd_;
		private readonly Scene scene_;
		private readonly string model_name_;
		private uint entity_id_;
		private uint old_selected_entity_id_;
	};

	class KGEditorCommandAddLight : KGEditorCommand
	{
		public KGEditorCommandAddLight(KGEditorCoreWrapper core, MainWindow wnd, Scene scene, KGEditorCoreWrapper.LightType type, string name)
			: base(KGEditorCommandCode.ECC_AddLight, "Add light")
		{
			core_ = core;
			wnd_ = wnd;
			scene_ = scene;
			type_ = type;
			name_ = name;
		}

		public override object Execute()
		{
			old_selected_entity_id_ = wnd_.SelectedEntityId;
			entity_id_ = core_.AddLight(type_, name_);
			scene_.AddEntity(entity_id_, SceneEntityType.ET_Light);
			wnd_.SelectedEntityId = entity_id_;
			return entity_id_;
		}

		public override void Revoke()
		{
			scene_.RemoveEntity(entity_id_);
			wnd_.SelectedEntityId = old_selected_entity_id_;
		}

		private readonly KGEditorCoreWrapper core_;
		private readonly MainWindow wnd_;
		private readonly Scene scene_;
		private readonly KGEditorCoreWrapper.LightType type_;
		private readonly string name_;
		private uint entity_id_;
		private uint old_selected_entity_id_;
	};

	class KGEditorCommandAddCamera : KGEditorCommand
	{
		public KGEditorCommandAddCamera(KGEditorCoreWrapper core, MainWindow wnd, Scene scene, string name)
			: base(KGEditorCommandCode.ECC_AddCamera, "Add camera")
		{
			core_ = core;
			wnd_ = wnd;
			scene_ = scene;
			name_ = name;
		}

		public override object Execute()
		{
			old_selected_entity_id_ = wnd_.SelectedEntityId;
			entity_id_ = core_.AddCamera(name_);
			scene_.AddEntity(entity_id_, SceneEntityType.ET_Camera);
			wnd_.SelectedEntityId = entity_id_;
			return entity_id_;
		}

		public override void Revoke()
		{
			scene_.RemoveEntity(entity_id_);
			wnd_.SelectedEntityId = old_selected_entity_id_;
		}

		private readonly KGEditorCoreWrapper core_;
		private readonly MainWindow wnd_;
		private readonly Scene scene_;
		private readonly string name_;
		private uint entity_id_;
		private uint old_selected_entity_id_;
	};

	class KGEditorCommandRemoveEntity : KGEditorCommand
	{
		public KGEditorCommandRemoveEntity(KGEditorCoreWrapper core, MainWindow wnd, SceneEntity entity)
			: base(KGEditorCommandCode.ECC_RemoveEntity, "Remove entity")
		{
			core_ = core;
			wnd_ = wnd;
			entity_ = entity;
		}

		public override object Execute()
		{
			old_selected_entity_id_ = wnd_.SelectedEntityId;
			if (old_selected_entity_id_ == entity_.Id)
			{
				wnd_.SelectedEntityId = 0;
			}

			core_.EntityVisible(entity_.Id, false);

			return null;
		}

		public override void Revoke()
		{
			wnd_.RestoreEntityInfo(entity_);
			wnd_.SelectedEntityId = old_selected_entity_id_;
			core_.EntityVisible(entity_.Id, true);
		}

		private readonly KGEditorCoreWrapper core_;
		private readonly MainWindow wnd_;
		private readonly SceneEntity entity_;
		private uint old_selected_entity_id_;
	};

	class KGEditorCommandSelectEntity : KGEditorCommand
	{
		public KGEditorCommandSelectEntity(MainWindow wnd, uint id)
			: base(KGEditorCommandCode.ECC_SelectEntity, "Select entity")
		{
			wnd_ = wnd;
			id_ = id;
		}

		public override object Execute()
		{
			old_id_ = wnd_.SelectedEntityId;
			wnd_.SelectedEntityId = id_;
			return null;
		}

		public override void Revoke()
		{
			wnd_.SelectedEntityId = old_id_;
		}

		private readonly MainWindow wnd_;
		private readonly uint id_;
		private uint old_id_;
	};

	class KGEditorCommandSetEntityName : KGEditorCommand
	{
		public KGEditorCommandSetEntityName(SceneEntity entity, string name)
			: base(KGEditorCommandCode.ECC_SetEntityName, "Set entity name")
		{
			entity_ = entity;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = entity_.Name;
			entity_.Name = name_;
			return null;
		}

		public override void Revoke()
		{
			entity_.Name = old_name_;
		}

		private readonly SceneEntity entity_;
		private readonly string name_;
		private string old_name_;
	};

	class KGEditorCommandEntityVisible : KGEditorCommand
	{
		public KGEditorCommandEntityVisible(SceneEntity entity, bool visible)
			: base(KGEditorCommandCode.ECC_SetEntityVisible, "Set entity visible")
		{
			entity_ = entity;
			visible_ = visible;
		}

		public override object Execute()
		{
			old_hide_ = entity_.Visible;
			entity_.Visible = visible_;
			return null;
		}

		public override void Revoke()
		{
			entity_.Visible = old_hide_;
		}

		private readonly SceneEntity entity_;
		private readonly bool visible_;
		private bool old_hide_;
	};

	class KGEditorCommandSetEntityScale : KGEditorCommand
	{
		public KGEditorCommandSetEntityScale(SceneEntity entity, float[] scale)
			: base(KGEditorCommandCode.ECC_SetEntityScale, "Set entity scale")
		{
			entity_ = entity;
			scale_ = scale;
		}

		public override object Execute()
		{
			old_scale_ = entity_.TransformScale;
			entity_.TransformScale = scale_;
			return null;
		}

		public override void Revoke()
		{
			entity_.TransformScale = old_scale_;
		}

		private readonly SceneEntity entity_;
		private readonly float[] scale_;
		private float[] old_scale_;
	};

	class KGEditorCommandSetEntityRotation : KGEditorCommand
	{
		public KGEditorCommandSetEntityRotation(SceneEntity entity, float[] rot_quat)
			: base(KGEditorCommandCode.ECC_SetEntityRotation, "Set entity rotation")
		{
			entity_ = entity;
			rot_ = rot_quat;
		}

		public override object Execute()
		{
			old_rot_ = entity_.TransformRotation;
			entity_.TransformRotation = rot_;
			return null;
		}

		public override void Revoke()
		{
			entity_.TransformRotation = old_rot_;
		}

		private readonly SceneEntity entity_;
		private readonly float[] rot_;
		private float[] old_rot_;
	};

	class KGEditorCommandSetEntityPosition : KGEditorCommand
	{
		public KGEditorCommandSetEntityPosition(SceneEntity entity, float[] trans)
			: base(KGEditorCommandCode.ECC_SetEntityTranslation, "Set entity translation")
		{
			entity_ = entity;
			trans_ = trans;
		}

		public override object Execute()
		{
			old_trans_ = entity_.TransformPosition;
			entity_.TransformPosition = trans_;
			return null;
		}

		public override void Revoke()
		{
			entity_.TransformPosition = old_trans_;
		}

		private readonly SceneEntity entity_;
		private readonly float[] trans_;
		private float[] old_trans_;
	};

	class KGEditorCommandSetActiveCamera : KGEditorCommand
	{
		public KGEditorCommandSetActiveCamera(Scene scene, uint id)
			: base(KGEditorCommandCode.ECC_SetActiveCamera, "Switch camera")
		{
			scene_ = scene;
			id_ = id;
		}

		public override object Execute()
		{
			old_id_ = scene_.ActiveCameraId;
			scene_.ActiveCameraId = id_;
			return null;
		}

		public override void Revoke()
		{
			scene_.ActiveCameraId = old_id_;
		}

		private readonly Scene scene_;
		private readonly uint id_;
		private uint old_id_;
	};

	class KGEditorCommandSetLightEnabled : KGEditorCommand
	{
		public KGEditorCommandSetLightEnabled(SceneEntityLight light, bool enabled)
			: base(KGEditorCommandCode.ECC_SetLightEnabled, "Set light enabled")
		{
			light_ = light;
			enabled_ = enabled;
		}

		public override object Execute()
		{
			old_enabled_ = light_.LightEnabled;
			light_.LightEnabled = enabled_;
			return null;
		}

		public override void Revoke()
		{
			light_.LightEnabled = old_enabled_;
		}

		private readonly SceneEntityLight light_;
		private readonly bool enabled_;
		private bool old_enabled_;
	};

	class KGEditorCommandSetLightAttrib : KGEditorCommand
	{
		public KGEditorCommandSetLightAttrib(SceneEntityLight light, int attrib)
			: base(KGEditorCommandCode.ECC_SetLightAttrib, "Set light attribute")
		{
			light_ = light;
			attrib_ = attrib;
		}

		public override object Execute()
		{
			old_attrib_ = light_.LightAttrib;
			light_.LightAttrib = attrib_;
			return null;
		}

		public override void Revoke()
		{
			light_.LightAttrib = old_attrib_;
		}

		private readonly SceneEntityLight light_;
		private readonly int attrib_;
		private int old_attrib_;
	};

	class KGEditorCommandSetLightColor : KGEditorCommand
	{
		public KGEditorCommandSetLightColor(SceneEntityLight light, float[] color)
			: base(KGEditorCommandCode.ECC_SetLightColor, "Set light color")
		{
			light_ = light;
			color_ = color;
		}

		public override object Execute()
		{
			old_color_ = light_.LightColor;
			light_.LightColor = color_;
			return null;
		}

		public override void Revoke()
		{
			light_.LightColor = old_color_;
		}

		private readonly SceneEntityLight light_;
		private readonly float[] color_;
		private float[] old_color_;
	};

	class KGEditorCommandSetLightFalloff : KGEditorCommand
	{
		public KGEditorCommandSetLightFalloff(SceneEntityLight light, float[] falloff)
			: base(KGEditorCommandCode.ECC_SetLightFalloff, "Set light falloff")
		{
			light_ = light;
			falloff_ = falloff;
		}

		public override object Execute()
		{
			old_falloff_ = light_.LightFalloff;
			light_.LightFalloff = falloff_;
			return null;
		}

		public override void Revoke()
		{
			light_.LightFalloff = old_falloff_;
		}

		private readonly SceneEntityLight light_;
		private readonly float[] falloff_;
		private float[] old_falloff_;
	};

	class KGEditorCommandSetLightInnerAngle : KGEditorCommand
	{
		public KGEditorCommandSetLightInnerAngle(SceneEntityLight light, float angle)
			: base(KGEditorCommandCode.ECC_SetLightInnerAngle, "Set light inner angle")
		{
			light_ = light;
			angle_ = angle;
		}

		public override object Execute()
		{
			old_angle_ = light_.LightInnerAngle;
			light_.LightInnerAngle = angle_;
			return null;
		}

		public override void Revoke()
		{
			light_.LightInnerAngle = old_angle_;
		}

		private readonly SceneEntityLight light_;
		private readonly float angle_;
		private float old_angle_;
	};

	class KGEditorCommandSetLightOuterAngle : KGEditorCommand
	{
		public KGEditorCommandSetLightOuterAngle(SceneEntityLight light, float angle)
			: base(KGEditorCommandCode.ECC_SetLightOuterAngle, "Set light outer angle")
		{
			light_ = light;
			angle_ = angle;
		}

		public override object Execute()
		{
			old_angle_ = light_.LightOuterAngle;
			light_.LightOuterAngle = angle_;
			return null;
		}

		public override void Revoke()
		{
			light_.LightOuterAngle = old_angle_;
		}

		private readonly SceneEntityLight light_;
		private readonly float angle_;
		private float old_angle_;
	};

	class KGEditorCommandSetProjectiveTex : KGEditorCommand
	{
		public KGEditorCommandSetProjectiveTex(SceneEntityLight light, string name)
			: base(KGEditorCommandCode.ECC_SetLightProjectiveTex, "Set projective texture")
		{
			light_ = light;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = light_.LightProjectiveTex;
			light_.LightProjectiveTex = name_;
			return null;
		}

		public override void Revoke()
		{
			light_.LightProjectiveTex = old_name_;
		}

		private readonly SceneEntityLight light_;
		private readonly string name_;
		private string old_name_;
	};

	class KGEditorCommandSetCameraLookAt : KGEditorCommand
	{
		public KGEditorCommandSetCameraLookAt(SceneEntityCamera camera, float[] look_at)
			: base(KGEditorCommandCode.ECC_SetCameraLookAt, "Set camera look at")
		{
			camera_ = camera;
			look_at_ = look_at;
		}

		public override object Execute()
		{
			old_look_at_ = camera_.CameraLookAt;
			camera_.CameraLookAt = look_at_;
			return null;
		}

		public override void Revoke()
		{
			camera_.CameraLookAt = old_look_at_;
		}

		private readonly SceneEntityCamera camera_;
		private readonly float[] look_at_;
		private float[] old_look_at_;
	};

	class KGEditorCommandSetCameraUpVec : KGEditorCommand
	{
		public KGEditorCommandSetCameraUpVec(SceneEntityCamera camera, float[] up_vec)
			: base(KGEditorCommandCode.ECC_SetCameraUpVec, "Set camera up vec")
		{
			camera_ = camera;
			up_vec_ = up_vec;
		}

		public override object Execute()
		{
			old_up_vec_ = camera_.CameraUpVec;
			camera_.CameraUpVec = up_vec_;
			return null;
		}

		public override void Revoke()
		{
			camera_.CameraUpVec = old_up_vec_;
		}

		private readonly SceneEntityCamera camera_;
		private readonly float[] up_vec_;
		private float[] old_up_vec_;
	};

	class KGEditorCommandSetCameraFoV : KGEditorCommand
	{
		public KGEditorCommandSetCameraFoV(SceneEntityCamera camera, float fov)
			: base(KGEditorCommandCode.ECC_SetCameraFoV, "Set camera fov")
		{
			camera_ = camera;
			fov_ = fov;
		}

		public override object Execute()
		{
			old_fov_ = camera_.CameraFoV;
			camera_.CameraFoV = fov_;
			return null;
		}

		public override void Revoke()
		{
			camera_.CameraFoV = old_fov_;
		}

		private readonly SceneEntityCamera camera_;
		private readonly float fov_;
		private float old_fov_;
	};

	class KGEditorCommandSetCameraAspect : KGEditorCommand
	{
		public KGEditorCommandSetCameraAspect(SceneEntityCamera camera, float aspect)
			: base(KGEditorCommandCode.ECC_SetCameraAspect, "Set camera aspect")
		{
			camera_ = camera;
			aspect_ = aspect;
		}

		public override object Execute()
		{
			old_aspect_ = camera_.CameraAspect;
			camera_.CameraAspect = aspect_;
			return null;
		}

		public override void Revoke()
		{
			camera_.CameraAspect = old_aspect_;
		}

		private readonly SceneEntityCamera camera_;
		private readonly float aspect_;
		private float old_aspect_;
	};

	class KGEditorCommandSetCameraNearPlane : KGEditorCommand
	{
		public KGEditorCommandSetCameraNearPlane(SceneEntityCamera camera, float near_plane)
			: base(KGEditorCommandCode.ECC_SetCameraNearPlane, "Set camera near plane")
		{
			camera_ = camera;
			near_plane_ = near_plane;
		}

		public override object Execute()
		{
			old_near_plane_ = camera_.CameraNearPlane;
			camera_.CameraNearPlane = near_plane_;
			return null;
		}

		public override void Revoke()
		{
			camera_.CameraNearPlane = old_near_plane_;
		}

		private readonly SceneEntityCamera camera_;
		private readonly float near_plane_;
		private float old_near_plane_;
	};

	class KGEditorCommandSetCameraFarPlane : KGEditorCommand
	{
		public KGEditorCommandSetCameraFarPlane(SceneEntityCamera camera, float far_plane)
			: base(KGEditorCommandCode.ECC_SetCameraFarPlane, "Set camera far plane")
		{
			camera_ = camera;
			far_plane_ = far_plane;
		}

		public override object Execute()
		{
			old_far_plane_ = camera_.CameraFarPlane;
			camera_.CameraFarPlane = far_plane_;
			return null;
		}

		public override void Revoke()
		{
			camera_.CameraFarPlane = old_far_plane_;
		}

		private readonly SceneEntityCamera camera_;
		private readonly float far_plane_;
		private float old_far_plane_;
	};
}