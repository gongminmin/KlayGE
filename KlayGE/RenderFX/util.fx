float3 decompress_normal(float4 comp_normal)
{
	float3 normal;
	normal.xy = comp_normal.ag * 2 - 1;
	normal.z = sqrt(1 - dot(normal.xy, normal.xy));
	return normal;
}
