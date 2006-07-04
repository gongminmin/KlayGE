float3 DecompressNormal(sampler2D normal_map, float2 uv)
{
	float3 normal;
	normal.xy = tex2D(normal_map, uv).ag * 2 - 1;
	normal.z = sqrt(1 - dot(normal.xy, normal.xy));
	return normal;
}
