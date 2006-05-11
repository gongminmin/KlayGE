float4 joint_rots[64];
float4 joint_poss[64];

float4x4 worldviewproj : WORLDVIEWPROJECTION;
float3 light_pos = float3(300, 100, 100);
float3 eye_pos;

float3 mul_quat(float3 v, float4 quat)
{
	float3 abc = float3(quat.w * quat.w - dot(quat.xyz, quat.xyz),
					2 * dot(quat.xyz, v), quat.w + quat.w);

	return abc.x * v + abc.y * quat.xyz + abc.z * cross(quat.xyz, v);
}

void SkinnedMeshVS(float4 pos : POSITION,
				float2 tex0 : TEXCOORD0,
				float3 T	: TEXCOORD1,	// in object space
				float3 B	: TEXCOORD2,	// in object space
				float4 blend_weights : BLENDWEIGHT,
				float4 blend_indices : BLENDINDICES,
				
				out float4 oPos : POSITION,
				out float2 oTex0 : TEXCOORD0,
				out float4 oL	: TEXCOORD1,	// in tangent space
				out float4 oH	: TEXCOORD2)	// in tangent space
{
	oTex0 = tex0;

	// Compensate for lack of UBYTE4 on Geforce3
	int4 index_vec = D3DCOLORtoUBYTE4(blend_indices);

	float3 result_pos = 0;
	float3x3 obj2tan = 0;
	for (int i = 0; i < 4; ++ i)
	{
		float4 joint_rot = joint_rots[index_vec[i]];
		float4 joint_pos = joint_poss[index_vec[i]];
		float weight = blend_weights[i];

		result_pos += (mul_quat(pos.xyz, joint_rot) + joint_pos.xyz) * weight;
		obj2tan[0] += mul_quat(T, joint_rot) * weight;
		obj2tan[1] += mul_quat(B, joint_rot) * weight;
	}
	obj2tan[0] = normalize(obj2tan[0]);
	obj2tan[1] = normalize(obj2tan[1]);
	obj2tan[2] = cross(obj2tan[0], obj2tan[1]);

	result_pos.zy = result_pos.yz;
	oPos = mul(float4(result_pos, 1), worldviewproj);

	float3 L = light_pos - result_pos;
	float3 V = eye_pos - result_pos;
	float3 H = normalize(L + V);

	oH = float4(mul(obj2tan, H), 1);
	oL = float4(mul(obj2tan, L), 1);
}


sampler diffuse_map;
sampler normal_map;
sampler specular_map;

float4 SkinnedMeshPS(float2 uv : TEXCOORD0,
				float3 L	: TEXCOORD1,
				float3 H	: TEXCOORD2) : COLOR
{
	float3 bump_normal = 2 * tex2D(normal_map, uv) - 1;
	float3 diffuse = tex2D(diffuse_map, uv);
	float3 specular = tex2D(specular_map, uv);
	
	float3 light_vec = normalize(L);
	float3 half_way = normalize(H);

	float diffuse_factor = saturate(dot(light_vec, bump_normal));
	float specular_factor = pow(dot(half_way, bump_normal), 8);

	return float4(diffuse * diffuse_factor + specular * specular_factor, 1);
}

technique SkinnedMeshTech
{
	pass p0
	{
		FillMode = Solid;
		CullMode = CW;

		VertexShader = compile vs_2_0 SkinnedMeshVS();
		PixelShader = compile ps_2_0 SkinnedMeshPS();
	}
}
