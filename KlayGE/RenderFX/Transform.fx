float4 TransformPos(float4 pos, float4x4 modelview, float4x4 proj)
{
	return mul(mul(pos, modelview), proj);
}

float4 TransformPos(float4 pos, float4x4 modelviewproj)
{
	return mul(pos, modelviewproj);
}

float3 TransformNormal(float3 normal, float3x3 modelviewIT)
{
	return mul(normal, modelviewIT);
}

float3 TransformNormal(float3 normal, float4x4 modelviewIT)
{
	return TransformNormal(normal, (float3x3)modelviewIT);
}
