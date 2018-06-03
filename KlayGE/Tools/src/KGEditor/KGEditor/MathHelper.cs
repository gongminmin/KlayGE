using System;
using System.Diagnostics;
using System.Windows.Media;

namespace KGEditor
{
	public static class MathHelper
	{
		public static float Deg2Rad(float degrees)
		{
			return (float)(degrees * (Math.PI / 180));
		}

		public static float Rad2Deg(float radians)
		{
			return (float)(radians * (180 / Math.PI));
		}

		public static float LinearToSRGB(float linear)
		{
			if (linear < 0.0031308f)
			{
				return 12.92f * linear;
			}
			else
			{
				const float ALPHA = 0.055f;
				return (1 + ALPHA) * (float)Math.Pow(linear, 1 / 2.4f) - ALPHA;
			}
		}

		public static float SRGBToLinear(float srgb)
		{
			if (srgb < 0.04045f)
			{
				return srgb / 12.92f;
			}
			else
			{
				const float ALPHA = 0.055f;
				return (float)Math.Pow((srgb + ALPHA) / (1 + ALPHA), 2.4f);
			}
		}

		public static float FloatPtrToMultipiler(float[] clr)
		{
			return Math.Max(Math.Max(Math.Max(clr[0], clr[1]), clr[2]), 1.0f);
		}

		public static Color FloatPtrToLDRColor(float[] clr, float multiplier)
		{
			float[] temp = new float[3];
			for (int i = 0; i < 3; ++ i)
			{
				temp[i] = LinearToSRGB(clr[i] / multiplier);
			}
			return Color.FromArgb(255,
				(byte)(Math.Max(Math.Min((int)(temp[0] * 255 + 0.5f), 255), 0)),
				(byte)(Math.Max(Math.Min((int)(temp[1] * 255 + 0.5f), 255), 0)),
				(byte)(Math.Max(Math.Min((int)(temp[2] * 255 + 0.5f), 255), 0)));
		}

		public static float[] ColorToFloatPtr(Color clr, float multiplier)
		{
			float[] ret = new float[3];
			ret[0] = clr.R / 255.0f;
			ret[1] = clr.G / 255.0f;
			ret[2] = clr.B / 255.0f;
			for (int i = 0; i < 3; ++ i)
			{
				ret[i] = SRGBToLinear(ret[i]) * multiplier;
			}
			return ret;
		}

		// From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
		public static float[] QuatToYawPitchRoll(float[] quat)
		{
			float[] yaw_pitch_roll = new float[3];

			float sqx = quat[0] * quat[0];
			float sqy = quat[1] * quat[1];
			float sqz = quat[2] * quat[2];
			float sqw = quat[3] * quat[3];
			float unit = sqx + sqy + sqz + sqw;
			float test = quat[3] * quat[0] + quat[1] * quat[2];
			if (test > 0.499f * unit)
			{
				// singularity at north pole
				yaw_pitch_roll[0] = 2 * (float)Math.Atan2(quat[2], quat[3]);
				yaw_pitch_roll[1] = (float)Math.PI / 2;
				yaw_pitch_roll[2] = 0;
			}
			else if (test < -0.499f * unit)
			{
				// singularity at south pole
				yaw_pitch_roll[0] = -2 * (float)Math.Atan2(quat[2], quat[3]);
				yaw_pitch_roll[1] = -(float)Math.PI / 2;
				yaw_pitch_roll[2] = 0;
			}
			else
			{
				yaw_pitch_roll[0] = (float)Math.Atan2(2 * (quat[1] * quat[3] - quat[0] * quat[2]), -sqx - sqy + sqz + sqw);
				yaw_pitch_roll[1] = (float)Math.Asin(2 * test / unit);
				yaw_pitch_roll[2] = (float)Math.Atan2(2 * (quat[2] * quat[3] - quat[0] * quat[1]), -sqx + sqy - sqz + sqw);
			}

			return yaw_pitch_roll;
		}

		public static float[] RotationQuatYawPitchRoll(float yaw, float pitch, float roll)
		{
			float ang_x = pitch / 2;
			float ang_y = yaw / 2;
			float ang_z = roll / 2;
			float sx = (float)Math.Sin(ang_x);
			float cx = (float)Math.Cos(ang_x);
			float sy = (float)Math.Sin(ang_y);
			float cy = (float)Math.Cos(ang_y);
			float sz = (float)Math.Sin(ang_z);
			float cz = (float)Math.Cos(ang_z);

			return new float[]
			{
				sx * cy * cz + cx * sy * sz,
				cx * sy * cz - sx * cy * sz,
				cx * cy * sz - sx * sy * cz,
				sx * sy * sz + cx * cy * cz
			};
		}

		public static bool FloatEqual(float lhs, float rhs)
		{
			return Math.Abs(lhs - rhs) < 1e-6f;
		}

		public static bool FloatArrayEqual(float[] lhs, float[] rhs)
		{
			Debug.Assert(lhs.Length == rhs.Length);

			for (uint i = 0; i < lhs.Length; ++ i)
			{
				if (!FloatEqual(lhs[i], rhs[i]))
				{
					return false;
				}
			}

			return true;
		}
	}
}
