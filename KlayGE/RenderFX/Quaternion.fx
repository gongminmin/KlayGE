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
	return float4(dot(lhs, float4(rhs.w, -rhs.z, rhs.yx)),
		dot(lhs, float4(rhs.zw, -rhs.x, rhs.y)),
		dot(lhs, float4(-rhs.y, rhs.xwz)),
		dot(lhs, float4(-rhs.xyz, rhs.w)));
}

float3 mul_quat(float3 v, float4 quat)
{
	float3 abc = float3(quat.w * quat.w - dot(quat.xyz, quat.xyz),
					2 * dot(quat.xyz, v), quat.w + quat.w);

	return abc.x * v + abc.y * quat.xyz + abc.z * cross(quat.xyz, v);
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

	ang = tw + tw;
	vec = quat.xyz;
	
	float stw = sin(tw);
	if (stw != 0)
	{
		vec /= stw;
	}
}

float4 axis_angle_to_quat(float3 v, float angle)
{
	float sa, ca;
	sincos(angle / 2, sa, ca);

	float4 ret = float4(sa.xxx, ca);
	if (dot(v, v) != 0)
	{
		ret.xyz *= normalize(v);
	}

	return ret;
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
		scale0 = sin((1 - s) * 3.1415926 / 2);
		scale1 = sin(s * 3.1415926 / 2);

		q2 = float4(-rhs.y, rhs.x, -rhs.w, rhs.z) * scale1;
	}

	// Compute the result
	return scale0 * lhs + q2;
}

float4 mat4_to_quat(float4x4 mat)
{
	float3 diag = float3(mat._m00, mat._m11, mat._m22);
	float tr = diag.x + diag.y + diag.z;
	float4 s = sqrt(float4(tr, diag.y - (diag.z + diag.x),
				diag.z - (diag.x + diag.y), diag.x - (diag.y + diag.z)) + 1);
	float4 quat = s.wyzx / 2;
	float4 s2 = 0.5 / s;

	s.x = s2.x;
	if (s.y != 0)
	{
		s.y = s2.y;
	}
	if (s.z != 0)
	{
		s.z = s2.z;
	}
	if (s.w != 0)
	{
		s.w = s2.w;
	}

	// check the diagonal
	if (tr > 0)
	{
		quat.xyz = (mat._m12_m20_m01 - mat._m21_m02_m10) * s.x;
	}
	else
	{
		if ((diag.y > diag.x) && (diag.z <= diag.y))
		{
			quat.xzw = (mat._m01_m21_m20 - mat._m10_m12_m02) * s.y;
		}
		else
		{
			if (((diag.y <= diag.x) && (diag.z > diag.x)) || (diag.z > diag.y))
			{
				quat.xyw = (mat._m02_m12_m01 - mat._m20_m21_m10) * s.z;
			}
			else
			{
				quat.yzw = (mat._m10_m20_m12 - mat._m01_m02_m21) * s.w;
			}
		}
	}

	return normalize(quat);
}

float4x4 quat_to_mat4(float4 quat)
{
	float3 xyz2 = quat.xyz + quat.xyz;
			  
	float3 c0 = quat.x * xyz2;
	float3 c1 = quat.yyz * xyz2.yzz;
	float3 c2 = quat.w * xyz2;

	return float4x4(
		1 - c1.x - c1.z,	c0.y + c2.z,		c0.z - c2.y,		0,
		c0.y - c2.z,		1 - c0.x - c1.z,	c1.y + c2.x,		0,
		c0.z + c2.y,		c1.y - c2.x,		1 - c0.x - c1.x,	0,
		0,					0,					0,					1);
}
