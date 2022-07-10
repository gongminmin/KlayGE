using System.Collections.Generic;
using System.Diagnostics;

namespace KGEditor
{
	public enum SceneEntityType
	{
		ET_Model = 0,
		ET_Light,
		ET_Camera
	}

	public class SceneEntity
	{
		public uint Id { get; set; }

		public string Name
		{
			get
			{
				return name_;
			}
			set
			{
				if (name_ != value)
				{
					name_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.EntityName(Id, name_);
					}
				}
			}
		}

		public SceneEntityType Type { get; set; }

		public bool Visible
		{
			get
			{
				return visible_;
			}
			set
			{
				if (visible_ != value)
				{
					visible_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.EntityVisible(Id, visible_);
					}
				}
			}
		}

		public float[] TransformPivot
		{
			get
			{
				return trf_pivot_;
			}
			set
			{
				Debug.Assert(value.Length == 3);
				if (!MathHelper.FloatArrayEqual(trf_pivot_, value))
				{
					value.CopyTo(trf_pivot_, 0);
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.EntityPivot(Id, trf_pivot_);
					}
				}
			}
		}
		public float[] TransformPosition
		{
			get
			{
				return trf_pos_;
			}
			set
			{
				Debug.Assert(value.Length == 3);
				if (!MathHelper.FloatArrayEqual(trf_pos_, value))
				{
					value.CopyTo(trf_pos_, 0);
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.EntityPosition(Id, trf_pos_);
					}
				}
			}
		}

		public float[] TransformScale
		{
			get
			{
				return trf_scale_;
			}
			set
			{
				Debug.Assert(value.Length == 3);
				if (!MathHelper.FloatArrayEqual(trf_scale_, value))
				{
					value.CopyTo(trf_scale_, 0);
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.EntityScale(Id, trf_scale_);
					}
				}
			}
		}

		public float[] TransformRotation
		{
			get
			{
				return trf_rotation_;
			}
			set
			{
				Debug.Assert(value.Length == 4);
				if (!MathHelper.FloatArrayEqual(trf_rotation_, value))
				{
					value.CopyTo(trf_rotation_, 0);
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.EntityRotation(Id, trf_rotation_);
					}
				}
			}
		}

		public IList<SceneEntity> Children
		{
			get
			{
				return children_;
			}
		}

		public virtual void RetrieveProperties()
		{
			if (Id > 0)
			{
				var core = MainWindow.KGEditNativeCore;

				name_ = core.EntityName(Id);
				visible_ = core.EntityVisible(Id);
				core.EntityPivot(Id).CopyTo(trf_pivot_, 0);
				core.EntityPosition(Id).CopyTo(trf_pos_, 0);
				core.EntityScale(Id).CopyTo(trf_scale_, 0);
				core.EntityRotation(Id).CopyTo(trf_rotation_, 0);
			}
		}

		protected string name_;
		protected bool visible_;

		protected float[] trf_pivot_ = new float[3];
		protected float[] trf_pos_ = new float[3];
		protected float[] trf_scale_ = new float[3];
		protected float[] trf_rotation_ = new float[4];

		protected readonly IList<SceneEntity> children_ = new List<SceneEntity>();
	}
}