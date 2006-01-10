float4x4 model_view;
float4x4 proj;
float3 light_in_model;


void NormalDepthVS(float4 pos : POSITION,
			float3 normal : NORMAL,
			out float4 oPos : POSITION,
			out float3 oNormal : TEXCOORD0,
			out float2 oDepth : TEXCOORD1)
{
	oPos = mul(mul(pos, model_view), proj);
	oNormal = mul(normal, (float3x3)model_view);
	oDepth = oPos.zw;
}

half4 NormalDepthPS(float3 normal : TEXCOORD0, float2 depth : TEXCOORD1) : COLOR
{
	normal = normalize(normal);
	return half4(normal, depth.x / depth.y);
}

technique NormalDepth
{
	pass p0
	{
		FillMode = Solid;
		CullMode = CCW;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;

		VertexShader = compile vs_1_1 NormalDepthVS();
		PixelShader = compile ps_2_0 NormalDepthPS();
	}
}


float inv_width, inv_height;

void PostToonVS(float4 pos : POSITION,
				float3 normal : NORMAL,
				float2 tex : TEXCOORD0,
				out float4 oPos : POSITION,
				out float2 oTc0 : TEXCOORD0, // 中心
				out float4 oTc1 : TEXCOORD1, // 左上 / 右下
				out float4 oTc2 : TEXCOORD2, // 右上 / 左下
				out float4 oTc3 : TEXCOORD3, // 左 / 右
				out float4 oTc4 : TEXCOORD4, // 上 / 下
				out float oToon : TEXCOORD5)
{
	oPos = mul(mul(pos, model_view), proj);
	
	oTc0 = oPos.xy / oPos.w / 2 + 0.5f;
	oTc0.y = 1 - oTc0.y;

	oTc1 = oTc0.xyxy + float4(-inv_width, -inv_height, +inv_width, +inv_height);
	oTc2 = oTc0.xyxy + float4(+inv_width, -inv_height, -inv_width, +inv_height);
	oTc3 = oTc0.xyxy + float4(-inv_width, 0, +inv_width, 0);
	oTc4 = oTc0.xyxy + float4(0, -inv_height, 0, +inv_height);

	half3 L = normalize(light_in_model - pos.xyz);
	oToon = dot(normalize(normal), L);
}

sampler1D toonmap_sampler;
sampler2D normal_depth_sampler;

const float2 e_barrier = float2(0.8f, 0.1f); // x=norm, y=depth
const float2 e_weights = float2(0.25f, 0.5f); // x=norm, y=depth

half4 PostToonPS(float2 tc0 : TEXCOORD0,
				float4 tc1 : TEXCOORD1,
				float4 tc2 : TEXCOORD2,
				float4 tc3 : TEXCOORD3,
				float4 tc4 : TEXCOORD4,
				float toon : TEXCOORD5) : COLOR
{
	half4 s1 = tex2D(normal_depth_sampler, tc1.xy);
	half4 s2 = tex2D(normal_depth_sampler, tc1.zw);
	half4 s3 = tex2D(normal_depth_sampler, tc2.xy);
	half4 s4 = tex2D(normal_depth_sampler, tc2.zw);
	half4 s5 = tex2D(normal_depth_sampler, tc3.xy);
	half4 s6 = tex2D(normal_depth_sampler, tc3.zw);
	half4 s7 = tex2D(normal_depth_sampler, tc4.xy);
	half4 s8 = tex2D(normal_depth_sampler, tc4.zw);

	// 法线间断点过滤
	half4 ndc = tex2D(normal_depth_sampler, tc0);
	half4 nd = half4(dot(ndc.xyz, s1.xyz),
				dot(ndc.xyz, s2.xyz),
				dot(ndc.xyz, s3.xyz),
				dot(ndc.xyz, s4.xyz));
	nd -= e_barrier.x;
	nd = (nd > 0) ? 1 : 0;
	half ne = (dot(nd, e_weights.x) < 1) ? 0 : 1;

	// 深度过滤，计算梯度差距
	half4 dd = half4(s1.w + s2.w, s3.w + s4.w, s5.w + s6.w, s7.w + s8.w);
	dd = abs(2 * ndc.w - dd) - e_barrier.y;
	dd = (dd > 0) ? 1 : 0;
	half de = (dot(dd, e_weights.y) < 1) ? 1 : 0;

	return tex1D(toonmap_sampler, toon) * de * ne;
}

technique Cartoon
{
	pass p0
	{
		FillMode = Solid;
		CullMode = CCW;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;

		VertexShader = compile vs_2_0 PostToonVS();
		PixelShader = compile ps_2_0 PostToonPS();
	}
}
