using System;
using System.Windows.Media;
using KlayGE;

namespace MtlEditor
{
	enum MtlEditorCommandCode
	{
		ECC_SetCurrFrame,
		ECC_SelectMesh,
		ECC_SetAmbientMaterial,
		ECC_SetDiffuseMaterial,
		ECC_SetSpecularMaterial,
		ECC_SetShininessMaterial,
		ECC_SetEmitMaterial,
		ECC_SetOpacityMaterial,
		ECC_SetDiffuseTexture,
		ECC_SetSpecularTexture,
		ECC_SetShininessTexture,
		ECC_SetNormalTexture,
		ECC_SetHeightTexture,
		ECC_SetEmitTexture,
		ECC_SetOpacityTexture,
		ECC_SetDetailMode,
		ECC_SetHeightOffset,
		ECC_SetHeightScale,
		ECC_SetEdgeTessHint,
		ECC_SetInsideTessHint,
		ECC_SetMinTess,
		ECC_SetMaxTess,

		ECC_NumCommands
	}

	abstract class MtlEditorCommand
	{
		public MtlEditorCommand(MtlEditorCoreWrapper core, MtlEditorCommandCode code, string cmd_name)
		{
			core_ = core;
			code_ = code;
			cmd_name_ = cmd_name;
		}

		public string Name()
		{
			return cmd_name_;
		}
		public MtlEditorCommandCode Code()
		{
			return code_;
		}

		public abstract object Execute();
		public abstract void Revoke();

		protected MtlEditorCoreWrapper core_;
		protected MtlEditorCommandCode code_;
		protected string cmd_name_;
	};

	class MtlEditorCommandSetCurrFrame : MtlEditorCommand
	{
		public MtlEditorCommandSetCurrFrame(MtlEditorCoreWrapper core, float frame)
			: base(core, MtlEditorCommandCode.ECC_SetCurrFrame, "Set current frame")
		{
			frame_ = frame;
		}

		public override object Execute()
		{
			old_frame_ = core_.CurrFrame();
			core_.CurrFrame(frame_);
			return null;
		}

		public override void Revoke()
		{
			core_.CurrFrame(old_frame_);
		}

		private float frame_;
		private float old_frame_;
	};

	class MtlEditorCommandSelectMesh : MtlEditorCommand
	{
		public MtlEditorCommandSelectMesh(MtlEditorCoreWrapper core, uint mesh_id)
			: base(core, MtlEditorCommandCode.ECC_SelectMesh, "Select mesh")
		{
			mesh_id_ = mesh_id;
		}

		public override object Execute()
		{
			old_mesh_id_ = core_.SelectedMesh();
			core_.SelectMesh(mesh_id_);
			return null;
		}

		public override void Revoke()
		{
			core_.SelectMesh(old_mesh_id_);
		}

		private uint mesh_id_;
		private uint old_mesh_id_;
	};

	class MtlEditorCommandSetAmbientMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetAmbientMaterial(MtlEditorCoreWrapper core, uint mtl_id, float[] value)
			: base(core, MtlEditorCommandCode.ECC_SetAmbientMaterial, "Set ambient")
		{
			mtl_id_ = mtl_id;
			ambient_ = value;
		}

		public override object Execute()
		{
			old_ambient_ = core_.AmbientMaterial(mtl_id_);
			core_.AmbientMaterial(mtl_id_, ambient_);
			return null;
		}

		public override void Revoke()
		{
			core_.AmbientMaterial(mtl_id_, old_ambient_);
		}

		private uint mtl_id_;
		private float[] ambient_;
		private float[] old_ambient_;
	};

	class MtlEditorCommandSetDiffuseMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetDiffuseMaterial(MtlEditorCoreWrapper core, uint mtl_id, float[] value)
			: base(core, MtlEditorCommandCode.ECC_SetDiffuseMaterial, "Set diffuse")
		{
			mtl_id_ = mtl_id;
			diffuse_ = value;
		}

		public override object Execute()
		{
			old_diffuse_ = core_.DiffuseMaterial(mtl_id_);
			core_.DiffuseMaterial(mtl_id_, diffuse_);
			return null;
		}

		public override void Revoke()
		{
			core_.DiffuseMaterial(mtl_id_, old_diffuse_);
		}

		private uint mtl_id_;
		private float[] diffuse_;
		private float[] old_diffuse_;
	};

	class MtlEditorCommandSetSpecularMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetSpecularMaterial(MtlEditorCoreWrapper core, uint mtl_id, float[] value)
			: base(core, MtlEditorCommandCode.ECC_SetSpecularMaterial, "Set specular")
		{
			mtl_id_ = mtl_id;
			specular_ = value;
		}

		public override object Execute()
		{
			old_specular_ = core_.SpecularMaterial(mtl_id_);
			core_.SpecularMaterial(mtl_id_, specular_);
			return null;
		}

		public override void Revoke()
		{
			core_.SpecularMaterial(mtl_id_, old_specular_);
		}

		private uint mtl_id_;
		private float[] specular_;
		private float[] old_specular_;
	};

	class MtlEditorCommandSetShininessMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetShininessMaterial(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetShininessMaterial, "Set shininess")
		{
			mtl_id_ = mtl_id;
			shininess_ = value;
		}

		public override object Execute()
		{
			old_shininess_ = core_.ShininessMaterial(mtl_id_);
			core_.ShininessMaterial(mtl_id_, shininess_);
			return null;
		}

		public override void Revoke()
		{
			core_.ShininessMaterial(mtl_id_, old_shininess_);
		}

		private uint mtl_id_;
		private float shininess_;
		private float old_shininess_;
	};

	class MtlEditorCommandSetEmitMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetEmitMaterial(MtlEditorCoreWrapper core, uint mtl_id, float[] value)
			: base(core, MtlEditorCommandCode.ECC_SetEmitMaterial, "Set emit")
		{
			mtl_id_ = mtl_id;
			emit_ = value;
		}

		public override object Execute()
		{
			old_emit_ = core_.EmitMaterial(mtl_id_);
			core_.EmitMaterial(mtl_id_, emit_);
			return null;
		}

		public override void Revoke()
		{
			core_.EmitMaterial(mtl_id_, old_emit_);
		}

		private uint mtl_id_;
		private float[] emit_;
		private float[] old_emit_;
	};

	class MtlEditorCommandSetOpacityMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetOpacityMaterial(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetOpacityMaterial, "Set opacity")
		{
			mtl_id_ = mtl_id;
			opacity_ = value;
		}

		public override object Execute()
		{
			old_opacity_ = core_.OpacityMaterial(mtl_id_);
			core_.OpacityMaterial(mtl_id_, opacity_);
			return null;
		}

		public override void Revoke()
		{
			core_.OpacityMaterial(mtl_id_, old_opacity_);
		}

		private uint mtl_id_;
		private float opacity_;
		private float old_opacity_;
	};

	class MtlEditorCommandSetDiffuseTexture : MtlEditorCommand
	{
		public MtlEditorCommandSetDiffuseTexture(MtlEditorCoreWrapper core, uint mtl_id, string name)
			: base(core, MtlEditorCommandCode.ECC_SetDiffuseTexture, "Set diffuse texture")
		{
			mtl_id_ = mtl_id;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.DiffuseTexture(mtl_id_);
			core_.DiffuseTexture(mtl_id_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.DiffuseTexture(mtl_id_, old_name_);
		}

		private uint mtl_id_;
		private string name_;
		private string old_name_;
	};

	class MtlEditorCommandSetSpecularTexture : MtlEditorCommand
	{
		public MtlEditorCommandSetSpecularTexture(MtlEditorCoreWrapper core, uint mtl_id, string name)
			: base(core, MtlEditorCommandCode.ECC_SetSpecularTexture, "Set specular texture")
		{
			mtl_id_ = mtl_id;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.SpecularTexture(mtl_id_);
			core_.SpecularTexture(mtl_id_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.SpecularTexture(mtl_id_, old_name_);
		}

		private uint mtl_id_;
		private string name_;
		private string old_name_;
	};

	class MtlEditorCommandSetShininessTexture : MtlEditorCommand
	{
		public MtlEditorCommandSetShininessTexture(MtlEditorCoreWrapper core, uint mtl_id, string name)
			: base(core, MtlEditorCommandCode.ECC_SetShininessTexture, "Set shininess texture")
		{
			mtl_id_ = mtl_id;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.ShininessTexture(mtl_id_);
			core_.ShininessTexture(mtl_id_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.ShininessTexture(mtl_id_, old_name_);
		}

		private uint mtl_id_;
		private string name_;
		private string old_name_;
	};

	class MtlEditorCommandSetNormalTexture : MtlEditorCommand
	{
		public MtlEditorCommandSetNormalTexture(MtlEditorCoreWrapper core, uint mtl_id, string name)
			: base(core, MtlEditorCommandCode.ECC_SetNormalTexture, "Set normal texture")
		{
			mtl_id_ = mtl_id;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.NormalTexture(mtl_id_);
			core_.NormalTexture(mtl_id_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.NormalTexture(mtl_id_, old_name_);
		}

		private uint mtl_id_;
		private string name_;
		private string old_name_;
	};

	class MtlEditorCommandSetHeightTexture : MtlEditorCommand
	{
		public MtlEditorCommandSetHeightTexture(MtlEditorCoreWrapper core, uint mtl_id, string name)
			: base(core, MtlEditorCommandCode.ECC_SetHeightTexture, "Set height texture")
		{
			mtl_id_ = mtl_id;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.HeightTexture(mtl_id_);
			core_.HeightTexture(mtl_id_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.HeightTexture(mtl_id_, old_name_);
		}

		private uint mtl_id_;
		private string name_;
		private string old_name_;
	};

	class MtlEditorCommandSetEmitTexture : MtlEditorCommand
	{
		public MtlEditorCommandSetEmitTexture(MtlEditorCoreWrapper core, uint mtl_id, string name)
			: base(core, MtlEditorCommandCode.ECC_SetEmitTexture, "Set emit texture")
		{
			mtl_id_ = mtl_id;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.EmitTexture(mtl_id_);
			core_.EmitTexture(mtl_id_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.EmitTexture(mtl_id_, old_name_);
		}

		private uint mtl_id_;
		private string name_;
		private string old_name_;
	};

	class MtlEditorCommandSetOpacityTexture : MtlEditorCommand
	{
		public MtlEditorCommandSetOpacityTexture(MtlEditorCoreWrapper core, uint mtl_id, string name)
			: base(core, MtlEditorCommandCode.ECC_SetOpacityTexture, "Set opacity texture")
		{
			mtl_id_ = mtl_id;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.OpacityTexture(mtl_id_);
			core_.OpacityTexture(mtl_id_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.OpacityTexture(mtl_id_, old_name_);
		}

		private uint mtl_id_;
		private string name_;
		private string old_name_;
	};

	class MtlEditorCommandSetDetailMode : MtlEditorCommand
	{
		public MtlEditorCommandSetDetailMode(MtlEditorCoreWrapper core, uint mtl_id, uint value)
			: base(core, MtlEditorCommandCode.ECC_SetDetailMode, "Set detail mode")
		{
			mtl_id_ = mtl_id;
			mode_ = value;
		}

		public override object Execute()
		{
			old_mode_ = core_.DetailMode(mtl_id_);
			core_.DetailMode(mtl_id_, mode_);
			return null;
		}

		public override void Revoke()
		{
			core_.DetailMode(mtl_id_, old_mode_);
		}

		private uint mtl_id_;
		private uint mode_;
		private uint old_mode_;
	};

	class MtlEditorCommandSetHeightOffset : MtlEditorCommand
	{
		public MtlEditorCommandSetHeightOffset(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetHeightOffset, "Set height offset")
		{
			mtl_id_ = mtl_id;
			offset_ = value;
		}

		public override object Execute()
		{
			old_offset_ = core_.HeightOffset(mtl_id_);
			core_.HeightOffset(mtl_id_, offset_);
			return null;
		}

		public override void Revoke()
		{
			core_.HeightOffset(mtl_id_, old_offset_);
		}

		private uint mtl_id_;
		private float offset_;
		private float old_offset_;
	};

	class MtlEditorCommandSetHeightScale : MtlEditorCommand
	{
		public MtlEditorCommandSetHeightScale(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetHeightScale, "Set height scale")
		{
			mtl_id_ = mtl_id;
			scale_ = value;
		}

		public override object Execute()
		{
			old_scale_ = core_.HeightScale(mtl_id_);
			core_.HeightScale(mtl_id_, scale_);
			return null;
		}

		public override void Revoke()
		{
			core_.HeightScale(mtl_id_, old_scale_);
		}

		private uint mtl_id_;
		private float scale_;
		private float old_scale_;
	};

	class MtlEditorCommandSetEdgeTessHint : MtlEditorCommand
	{
		public MtlEditorCommandSetEdgeTessHint(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetEdgeTessHint, "Set edge tessellation hint")
		{
			mtl_id_ = mtl_id;
			edge_ = value;
		}

		public override object Execute()
		{
			old_edge_ = core_.EdgeTessHint(mtl_id_);
			core_.EdgeTessHint(mtl_id_, edge_);
			return null;
		}

		public override void Revoke()
		{
			core_.EdgeTessHint(mtl_id_, old_edge_);
		}

		private uint mtl_id_;
		private float edge_;
		private float old_edge_;
	};

	class MtlEditorCommandSetInsideTessHint : MtlEditorCommand
	{
		public MtlEditorCommandSetInsideTessHint(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetInsideTessHint, "Set inside tessellation hint")
		{
			mtl_id_ = mtl_id;
			edge_ = value;
		}

		public override object Execute()
		{
			old_edge_ = core_.InsideTessHint(mtl_id_);
			core_.InsideTessHint(mtl_id_, edge_);
			return null;
		}

		public override void Revoke()
		{
			core_.InsideTessHint(mtl_id_, old_edge_);
		}

		private uint mtl_id_;
		private float edge_;
		private float old_edge_;
	};

	class MtlEditorCommandSetMinTess : MtlEditorCommand
	{
		public MtlEditorCommandSetMinTess(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetMinTess, "Set min tessellation")
		{
			mtl_id_ = mtl_id;
			min_ = value;
		}

		public override object Execute()
		{
			old_min_ = core_.MinTess(mtl_id_);
			core_.MinTess(mtl_id_, min_);
			return null;
		}

		public override void Revoke()
		{
			core_.MinTess(mtl_id_, old_min_);
		}

		private uint mtl_id_;
		private float min_;
		private float old_min_;
	};

	class MtlEditorCommandSetMaxTess : MtlEditorCommand
	{
		public MtlEditorCommandSetMaxTess(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetMaxTess, "Set max tessellation")
		{
			mtl_id_ = mtl_id;
			max_ = value;
		}

		public override object Execute()
		{
			old_max_ = core_.MaxTess(mtl_id_);
			core_.MaxTess(mtl_id_, max_);
			return null;
		}

		public override void Revoke()
		{
			core_.MaxTess(mtl_id_, old_max_);
		}

		private uint mtl_id_;
		private float max_;
		private float old_max_;
	};
}
