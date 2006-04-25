float4 conjugate_quat(float4 rhs)
{
	return float4(-rhs.x, -rhs.y, -rhs.z, rhs.w);
}

float4 inverse_quat(float4 rhs)
{
	return conjugate_quat(rhs) / length(rhs);
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

float4 mul_quat(float4 lhs, float4 rhs)
{
	return float4(
		lhs.x * rhs.w - lhs.y * rhs.z + lhs.z * rhs.y + lhs.w * rhs.x,
		lhs.x * rhs.z + lhs.y * rhs.w - lhs.z * rhs.x + lhs.w * rhs.y,
		-lhs.x * rhs.y + lhs.y * rhs.x + lhs.z * rhs.w + lhs.w * rhs.z,
		-lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z + lhs.w * rhs.w);
}
