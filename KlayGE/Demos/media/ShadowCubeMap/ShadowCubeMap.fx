float4x4 model_view_proj;

float4x4 model;
float4x4 obj_model_to_light_model;

void GenShadowMapVS(float4 Position : POSITION,
						out float4 oPos : POSITION,
						out float4 oLightWorldPos : TEXCOORD0)
{
    oPos = mul(Position, model_view_proj);
    oLightWorldPos = mul(Position, obj_model_to_light_model);
}

half4 GenShadowMapPS(half4 LightWorldPos : TEXCOORD0) : COLOR
{
	LightWorldPos /= LightWorldPos.w;
	half dist = length(LightWorldPos.xyz);
	return half4(dist, dist * dist, 0, 1);
}

technique GenShadowMap
{
	Pass P0
	{
		CullMode = None;
		
		VertexShader = compile vs_1_1 GenShadowMapVS();
		PixelShader = compile ps_2_0 GenShadowMapPS();
	}
}


float3 light_pos;

sampler lamp_sampler;
sampler shadow_map_sampler;

void RenderSceneVS(float4 Position : POSITION,
					float3 Normal   : NORMAL,
					out float4 oPos : POSITION,
					out float3 oDiffuse : COLOR0,
					out float4 oLightWorldPos : TEXCOORD0)
{
	float4 world_pos = mul(Position, model);
	world_pos /= world_pos.w;
	float3 world_normal = normalize(mul(Normal, (float3x3)model));

	oPos = mul(Position, model_view_proj);
	oLightWorldPos = mul(Position, obj_model_to_light_model);
    oDiffuse = dot(normalize(light_pos - world_pos.xyz), world_normal);
}

float4 RenderScenePS(half3 diffuse : COLOR0,
				float3 LightWorldPos : TEXCOORD0) : COLOR 
{
	half2 moments = texCUBE(shadow_map_sampler, LightWorldPos);
	
	half dist = length(LightWorldPos);
	half3 color = diffuse * texCUBE(lamp_sampler, LightWorldPos).rgb / (0.001f + 0.4 * moments.y);
	if (dist > moments.x + 0.1f)
	{
		// Variance shadow mapping
		half variance = moments.y - moments.x * moments.x;
		half m_d = moments.x - dist;
		half p_max = variance / (variance + m_d * m_d);

		color *= p_max;
	}

	return float4(color, 1.0);
}

technique RenderScene
{
	Pass P0
	{
		CullMode = CCW;
		
		VertexShader = compile vs_1_1 RenderSceneVS();
		PixelShader = compile ps_2_0 RenderScenePS();
	}
}
