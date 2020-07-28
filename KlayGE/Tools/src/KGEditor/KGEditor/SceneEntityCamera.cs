using System.Diagnostics;

namespace KGEditor
{
	public class SceneEntityCamera : SceneEntity
	{
		public float[] CameraLookAt
		{
			get
			{
				return look_at_;
			}
			set
			{
				Debug.Assert(value.Length == 3);
				if (!MathHelper.FloatArrayEqual(look_at_, value))
				{
					value.CopyTo(look_at_, 0);
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.CameraLookAt(Id, look_at_);
					}
				}
			}
		}

		public float[] CameraUpVec
		{
			get
			{
				return up_vec_;
			}
			set
			{
				Debug.Assert(value.Length == 3);
				if (!MathHelper.FloatArrayEqual(up_vec_, value))
				{
					value.CopyTo(up_vec_, 0);
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.CameraUpVec(Id, up_vec_);
					}
				}
			}
		}

		public float CameraFoV
		{
			get
			{
				return fov_;
			}
			set
			{
				if (!MathHelper.FloatEqual(fov_, value))
				{
					fov_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.CameraFoV(Id, fov_);
					}
				}
			}
		}

		public float CameraAspect
		{
			get
			{
				return aspect_;
			}
			set
			{
				if (!MathHelper.FloatEqual(aspect_, value))
				{
					aspect_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.CameraAspect(Id, aspect_);
					}
				}
			}
		}

		public float CameraNearPlane
		{
			get
			{
				return near_plane_;
			}
			set
			{
				if (!MathHelper.FloatEqual(near_plane_, value))
				{
					near_plane_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.CameraNearPlane(Id, near_plane_);
					}
				}
			}
		}

		public float CameraFarPlane
		{
			get
			{
				return far_plane_;
			}
			set
			{
				if (!MathHelper.FloatEqual(far_plane_, value))
				{
					far_plane_ = value;
					if (Id > 0)
					{
						MainWindow.KGEditNativeCore.CameraFarPlane(Id, far_plane_);
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

				core.CameraLookAt(Id).CopyTo(look_at_, 0);
				core.CameraUpVec(Id).CopyTo(up_vec_, 0);
				fov_ = core.CameraFoV(Id);
				aspect_ = core.CameraAspect(Id);
				near_plane_ = core.CameraNearPlane(Id);
				far_plane_ = core.CameraFarPlane(Id);
			}
		}

		private float[] look_at_ = new float[3];
		private float[] up_vec_ = new float[3];
		private float fov_;
		private float aspect_;
		private float near_plane_;
		private float far_plane_;
	}
}