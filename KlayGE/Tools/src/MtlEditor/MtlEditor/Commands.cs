using KlayGE;

namespace MtlEditor
{
	enum MtlEditorCommandCode
	{
		ECC_SetSkyboxName,
		ECC_SetCurrFrame,
		ECC_SelectMesh,
		ECC_SetAlbedoMaterial,
		ECC_SetMetalnessMaterial,
		ECC_SetGlossinessMaterial,
		ECC_SetEmissiveMaterial,
		ECC_SetOpacityMaterial,
		ECC_SetTexture,
		ECC_SetDetailMode,
		ECC_SetHeightOffset,
		ECC_SetHeightScale,
		ECC_SetEdgeTessHint,
		ECC_SetInsideTessHint,
		ECC_SetMinTess,
		ECC_SetMaxTess,
		ECC_SetTransparent,
		ECC_SetAlphaTest,
		ECC_SetSSS,
		ECC_SetTwoSided,
		ECC_AssignMaterial,
		ECC_CopyMaterial,

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

	class MtlEditorCommandSetSkyboxName : MtlEditorCommand
	{
		public MtlEditorCommandSetSkyboxName(MtlEditorCoreWrapper core, string name)
			: base(core, MtlEditorCommandCode.ECC_SetSkyboxName, "Set skybox name")
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

	class MtlEditorCommandSetAlbedoMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetAlbedoMaterial(MtlEditorCoreWrapper core, uint mtl_id, float[] value)
			: base(core, MtlEditorCommandCode.ECC_SetAlbedoMaterial, "Set albedo")
		{
			mtl_id_ = mtl_id;
			albedo_ = value;
		}

		public override object Execute()
		{
			old_albedo_ = core_.AlbedoMaterial(mtl_id_);
			core_.AlbedoMaterial(mtl_id_, albedo_);
			return null;
		}

		public override void Revoke()
		{
			core_.AlbedoMaterial(mtl_id_, old_albedo_);
		}

		private uint mtl_id_;
		private float[] albedo_;
		private float[] old_albedo_;
	};

	class MtlEditorCommandSetMetalnessMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetMetalnessMaterial(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetMetalnessMaterial, "Set metalness")
		{
			mtl_id_ = mtl_id;
			metalness_ = value;
		}

		public override object Execute()
		{
			old_metalness_ = core_.MetalnessMaterial(mtl_id_);
			core_.MetalnessMaterial(mtl_id_, metalness_);
			return null;
		}

		public override void Revoke()
		{
			core_.MetalnessMaterial(mtl_id_, old_metalness_);
		}

		private uint mtl_id_;
		private float metalness_;
		private float old_metalness_;
	};

	class MtlEditorCommandSetGlossinessMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetGlossinessMaterial(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetGlossinessMaterial, "Set glossiness")
		{
			mtl_id_ = mtl_id;
			glossiness_ = value;
		}

		public override object Execute()
		{
			old_glossiness_ = core_.GlossinessMaterial(mtl_id_);
			core_.GlossinessMaterial(mtl_id_, glossiness_);
			return null;
		}

		public override void Revoke()
		{
			core_.GlossinessMaterial(mtl_id_, old_glossiness_);
		}

		private uint mtl_id_;
		private float glossiness_;
		private float old_glossiness_;
	};

	class MtlEditorCommandSetEmissiveMaterial : MtlEditorCommand
	{
		public MtlEditorCommandSetEmissiveMaterial(MtlEditorCoreWrapper core, uint mtl_id, float[] value)
			: base(core, MtlEditorCommandCode.ECC_SetEmissiveMaterial, "Set emissive")
		{
			mtl_id_ = mtl_id;
			emissive_ = value;
		}

		public override object Execute()
		{
			old_emissive_ = core_.EmissiveMaterial(mtl_id_);
			core_.EmissiveMaterial(mtl_id_, emissive_);
			return null;
		}

		public override void Revoke()
		{
			core_.EmissiveMaterial(mtl_id_, old_emissive_);
		}

		private uint mtl_id_;
		private float[] emissive_;
		private float[] old_emissive_;
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

	class MtlEditorCommandSetTexture : MtlEditorCommand
	{
		public MtlEditorCommandSetTexture(MtlEditorCoreWrapper core, uint mtl_id, KlayGE.MtlEditorCoreWrapper.TextureSlot slot, string name)
			: base(core, MtlEditorCommandCode.ECC_SetTexture, "Set texture")
		{
			mtl_id_ = mtl_id;
			slot_ = slot;
			name_ = name;
		}

		public override object Execute()
		{
			old_name_ = core_.Texture(mtl_id_, slot_);
			core_.Texture(mtl_id_, slot_, name_);
			return null;
		}

		public override void Revoke()
		{
			core_.Texture(mtl_id_, slot_, old_name_);
		}

		private uint mtl_id_;
		private KlayGE.MtlEditorCoreWrapper.TextureSlot slot_;
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

	class MtlEditorCommandSetTransparent : MtlEditorCommand
	{
		public MtlEditorCommandSetTransparent(MtlEditorCoreWrapper core, uint mtl_id, bool value)
			: base(core, MtlEditorCommandCode.ECC_SetTransparent, "Set transparent")
		{
			mtl_id_ = mtl_id;
			transparent_ = value;
		}

		public override object Execute()
		{
			old_transparent_ = core_.TransparentMaterial(mtl_id_);
			core_.TransparentMaterial(mtl_id_, transparent_);
			return null;
		}

		public override void Revoke()
		{
			core_.TransparentMaterial(mtl_id_, old_transparent_);
		}

		private uint mtl_id_;
		private bool transparent_;
		private bool old_transparent_;
	};

	class MtlEditorCommandSetAlphaTest : MtlEditorCommand
	{
		public MtlEditorCommandSetAlphaTest(MtlEditorCoreWrapper core, uint mtl_id, float value)
			: base(core, MtlEditorCommandCode.ECC_SetAlphaTest, "Set alpha test")
		{
			mtl_id_ = mtl_id;
			alpha_test_ = value;
		}

		public override object Execute()
		{
			old_alpha_test_ = core_.AlphaTestMaterial(mtl_id_);
			core_.AlphaTestMaterial(mtl_id_, alpha_test_);
			return null;
		}

		public override void Revoke()
		{
			core_.AlphaTestMaterial(mtl_id_, old_alpha_test_);
		}

		private uint mtl_id_;
		private float alpha_test_;
		private float old_alpha_test_;
	};

	class MtlEditorCommandSetSSS : MtlEditorCommand
	{
		public MtlEditorCommandSetSSS(MtlEditorCoreWrapper core, uint mtl_id, bool value)
			: base(core, MtlEditorCommandCode.ECC_SetSSS, "Set SSS")
		{
			mtl_id_ = mtl_id;
			sss_ = value;
		}

		public override object Execute()
		{
			old_sss_ = core_.SSSMaterial(mtl_id_);
			core_.SSSMaterial(mtl_id_, sss_);
			return null;
		}

		public override void Revoke()
		{
			core_.SSSMaterial(mtl_id_, old_sss_);
		}

		private uint mtl_id_;
		private bool sss_;
		private bool old_sss_;
	};

	class MtlEditorCommandSetTwoSided : MtlEditorCommand
	{
		public MtlEditorCommandSetTwoSided(MtlEditorCoreWrapper core, uint mtl_id, bool value)
			: base(core, MtlEditorCommandCode.ECC_SetTwoSided, "Set two sided")
		{
			mtl_id_ = mtl_id;
			two_sided_ = value;
		}

		public override object Execute()
		{
			old_two_sided_ = core_.TwoSidedMaterial(mtl_id_);
			core_.TwoSidedMaterial(mtl_id_, two_sided_);
			return null;
		}

		public override void Revoke()
		{
			core_.TwoSidedMaterial(mtl_id_, old_two_sided_);
		}

		private uint mtl_id_;
		private bool two_sided_;
		private bool old_two_sided_;
	};

	class MtlEditorCommandAssignMaterial : MtlEditorCommand
	{
		public MtlEditorCommandAssignMaterial(MtlEditorCoreWrapper core, uint mesh_id, uint mtl_id)
			: base(core, MtlEditorCommandCode.ECC_AssignMaterial, "Assign material")
		{
			mesh_id_ = mesh_id;
			mtl_id_ = mtl_id;
		}

		public override object Execute()
		{
			old_mtl_id_ = core_.MaterialID(mesh_id_);
			core_.MaterialID(mesh_id_, mtl_id_);
			return null;
		}

		public override void Revoke()
		{
			core_.MaterialID(mesh_id_, old_mtl_id_);
		}

		private uint mesh_id_;
		private uint mtl_id_;
		private uint old_mtl_id_;
	};

	class MtlEditorCommandCopyMaterial : MtlEditorCommand
	{
		public MtlEditorCommandCopyMaterial(MtlEditorCoreWrapper core, uint mtl_id)
			: base(core, MtlEditorCommandCode.ECC_CopyMaterial, "Copy material")
		{
			mtl_id_ = mtl_id;
		}

		public override object Execute()
		{
			new_mtl_id_ = core_.CopyMaterial(mtl_id_);
			return new_mtl_id_;
		}

		public override void Revoke()
		{
			// TODO
		}

		private uint mtl_id_;
		private uint new_mtl_id_;
	};
}
