using System.Diagnostics;

namespace KGEditor
{
	public class SceneEntityLight : SceneEntity
	{
		public uint LightType { get; set; }

		public bool LightEnabled
		{
			get
			{
				return enabled_;
			}
			set
			{
				if (enabled_ != value)
				{
					enabled_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.LightEnabled(Id, enabled_);
					}
				}
			}
		}

		public int LightAttrib
		{
			get
			{
				return attrib_;
			}
			set
			{
				if (attrib_ != value)
				{
					attrib_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.LightAttrib(Id, attrib_);
					}
				}
			}
		}

		public float[] LightColor
		{
			get
			{
				return color_;
			}
			set
			{
				Debug.Assert(value.Length == 3);
				if (!MathHelper.FloatArrayEqual(color_, value))
				{
					value.CopyTo(color_, 0);
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.LightColor(Id, color_);
					}
				}
			}
		}

		public float[] LightFalloff
		{
			get
			{
				return falloff_;
			}
			set
			{
				Debug.Assert(value.Length == 3);
				if (!MathHelper.FloatArrayEqual(falloff_, value))
				{
					value.CopyTo(falloff_, 0);
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.LightFalloff(Id, falloff_);
					}
				}
			}
		}

		public float LightInnerAngle
		{
			get
			{
				return inner_angle_;
			}
			set
			{
				if (!MathHelper.FloatEqual(inner_angle_, value))
				{
					inner_angle_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.LightInnerAngle(Id, inner_angle_);
					}
				}
			}
		}
		public float LightOuterAngle
		{
			get
			{
				return outer_angle_;
			}
			set
			{
				if (!MathHelper.FloatEqual(outer_angle_, value))
				{
					outer_angle_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.LightInnerAngle(Id, outer_angle_);
					}
				}
			}
		}

		public string LightProjectiveTex
		{
			get
			{
				return projective_tex_;
			}
			set
			{
				if (projective_tex_ != value)
				{
					projective_tex_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.LightProjectiveTex(Id, projective_tex_);
					}
				}
			}
		}

		public override void RetrieveProperties()
		{
			if (Id > 0)
			{
				base.RetrieveProperties();

				var core = MainWindow.KGEditNativeCore;

				this.LightType = (uint)core.GetLightType(Id);

				enabled_ = core.LightEnabled(Id);
				attrib_ = core.LightAttrib(Id);
				core.LightColor(Id).CopyTo(color_, 0);
				core.LightFalloff(Id).CopyTo(falloff_, 0);
				inner_angle_ = core.LightInnerAngle(Id);
				outer_angle_ = core.LightOuterAngle(Id);
				projective_tex_ = core.LightProjectiveTex(Id);
			}
		}

		private bool enabled_;

		private int attrib_;

		private float[] color_ = new float[3];
		private float[] falloff_ = new float[3];

		private float inner_angle_;
		private float outer_angle_;

		private string projective_tex_;
	}
}