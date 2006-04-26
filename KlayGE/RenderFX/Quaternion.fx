float4 conjugate_quat(float4 rhs)
{
	return float4(-rhs.x, -rhs.y, -rhs.z, rhs.w);
}

float4 exp_quat(float4 rhs)
{
	float theta = length(rhs.xyz);
	return float4(normalize(rhs.xyz) * sin(theta), cos(theta));
}

float4 ln_quat(float4 rhs)
{
	float theta_2 = acos(rhs.w);
	return float4(normalize(rhs.xyz) * (theta_2 + theta_2), 0);
}

float4 inverse_quat(float4 rhs)
{
	return conjugate_quat(rhs) / length(rhs);
}

float4 mul_quat(float4 lhs, float4 rhs)
{
	return float4(
		lhs.x * rhs.w - lhs.y * rhs.z + lhs.z * rhs.y + lhs.w * rhs.x,
		lhs.x * rhs.z + lhs.y * rhs.w - lhs.z * rhs.x + lhs.w * rhs.y,
		-lhs.x * rhs.y + lhs.y * rhs.x + lhs.z * rhs.w + lhs.w * rhs.z,
		-lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z + lhs.w * rhs.w);
}

float4 rotation_quat(float3 yaw_pitch_roll)
{
	float3 ang = yaw_pitch_roll / 2;
	float3 s;
	float3 c;
	sincos(ang, s, c);

	return float4(
		s.x * c.y * c.z + c.x * s.y * s.z,
		c.x * s.y * c.z - s.x * c.y * s.z,
		c.x * c.y * s.z - s.x * s.y * c.z,
		s.x * s.y * s.z + c.x * c.y * c.z);
}

void quat_to_axis_angle(out float3 vec, out float ang, float4 quat)
{
	float tw = acos(quat.w);
	float scale = 1.0 / sin(tw);

	ang = tw + tw;
	vec = quat.xyz * scale;
}

float4 axis_angle_to_quat(float3 v, float angle)
{
	float ang(angle / 2);
	float sa, ca;
	sincos(ang, sa, ca);

	return float4(sa * normalize(v), ca);
}

float4 slerp(float4 lhs, float4 rhs, float s)
{
	float scale0, scale1;
	float4 q2;

	// DOT the quats to get the cosine of the angle between them
	float cosom = dot(lhs, rhs);

	// Two special cases:
	// Quats are exactly opposite?
	if (cosom > -1)
	{
		// make sure they are different enough to avoid a divide by 0
		if (cosom < 1)
		{
			// SLERP away
			float omega = acos(cosom);
			float isinom = 1.0 / sin(omega);
			scale0 = sin((1 - s) * omega) * isinom;
			scale1 = sin(s * omega) * isinom;
		}
		else
		{
			// LERP is good enough at this distance
			scale0 = 1 - s;
			scale1 = s;
		}

		q2 = rhs * scale1;
	}
	else
	{
		// SLERP towards a perpendicular quat
		// Set slerp parameters
		scale0 = sin(1 - slerp) * 3.1415926 / 2);
		scale1 = sin(slerp * 3.1415926 / 2);

		q2 = float4(-rhs.y, rhs.x, -rhs.w, rhs.z) * scale1;
	}

	// Compute the result
	return scale0 * lhs + q2;
}

float4 mat4_to_quat(float4x4 mat)
{
	float4 quat;
	float s;
	float const tr = mat[0][0] + mat[1][1] + mat[2][2];

	// check the diagonal
	if (tr > 0)
	{
		s = Sqrt(tr + 1);
		quat.w = s / 2;
		s = 0.5 / s;
		quat.x = (mat[1][2] - mat[2][1]) * s;
		quat.y = (mat[2][0] - mat[0][2]) * s;
		quat.z = (mat[0][1] - mat[1][0]) * s;
	}
	else
	{
		if ((mat[1][1] > mat[0][0]) && (mat[2][2] <= mat[1][1]))
		{
			s = sqrt((mat[1][1] - (mat[2][2] + mat[0][0])) + 1);

			quat.y = s / 2;

			if (s != 0)
			{
				s = 0.5 / s;
			}

			quat.w = (mat[2][0] - mat[0][2]) * s;
			quat.z = (mat[2][1] + mat[1][2]) * s;
			quat.x = (mat[0][1] + mat[1][0]) * s;
		}
		else
		{
			if (((mat[1][1] <= mat[0][0]) && (mat[2][2] > mat[0][0])) || (mat[2][2] > mat[1][1]))
			{
				s = sqrt((mat[2][2] - (mat[0][0] + mat[1][1])) + 1);

				quat.z = s / 2;

				if (s != 0)
				{
					s = 0.5 / s;
				}

				quat.w = (mat[0][1] - mat[1][0]) * s;
				quat.x = (mat[0][2] + mat[2][0]) * s;
				quat.y = (mat[1][2] + mat[2][1]) * s;
			}
			else
			{
				s = sqrt((mat[0][0] - (mat[1][1] + mat[2][2])) + 1);

				quat.x = s / 2;

				if (s != 0)
				{
					s = 0.5 / s;
				}

				quat.w = (mat[1][2] - mat[2][1]) * s;
				quat.y = (mat[1][0] + mat[0][1]) * s;
				quat.z = (mat[2][0] + mat[0][2]) * s;
			}
		}
	}

	return normalize(quat);
}

float4x4 quat_to_mat(float4 quat)
{
	float3 xyz2 = quat.xyz + quat.xyz;
			  
	float3 c0 = quat.x * xyz2;
	float3 c1 = quat.yyz * xyz2.yzz;
	float3 c2 = quat.w * xyz2;

	return float4x4(
		1 - c1.x - c1.z,	c0.y + c2.z,		c0.z - c2.y,		0,
		c0.y - c2.z,		1 - c0.x - c1.z,	c1.y + c2.x,		0,
		c0.z + c2.y,		c1.y - c2.x,		1 - c0.x - c1.x,	0,
		0,				0,				0,				1);
}
