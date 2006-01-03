float4x4 model_view;
float4x4 proj;
float3 light_in_model;


void PositionVS(float4 pos : POSITION,
			out float4 oPos : POSITION,
			out float4 oPosOut : TEXCOORD0)
{
	oPos = mul(mul(pos, model_view), proj);
	oPosOut = oPos;
}

half4 PositionPS(float4 position : TEXCOORD0) : COLOR
{
	return position / position.w;
}

technique Position
{
	pass p0
	{
		FillMode = Solid;
		CullMode = CCW;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;

		VertexShader = compile vs_1_1 PositionVS();
		PixelShader = compile ps_2_0 PositionPS();
	}
}


void NormalVS(float4 pos : POSITION,
			float3 normal : NORMAL,
			out float4 oPos : POSITION,
			out float3 oNormal : TEXCOORD0)
{
	oPos = mul(mul(pos, model_view), proj);
	oNormal = normalize(mul(normal, (float3x3)model_view));
}

half4 NormalPS(float3 normal : TEXCOORD0) : COLOR
{
	normal = normalize(normal);
	return half4(normal, 0);
}

technique Normal
{
	pass p0
	{
		FillMode = Solid;
		CullMode = CCW;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;

		VertexShader = compile vs_1_1 NormalVS();
		PixelShader = compile ps_2_0 NormalPS();
	}
}


void PositionNormalVS(float4 pos : POSITION,
			float3 normal : NORMAL,
			out float4 oPos : POSITION,
			out float4 oPosOut : TEXCOORD0,
			out float3 oNormal : TEXCOORD1)
{
	oPos = mul(mul(pos, model_view), proj);
	oPosOut = oPos;
	oNormal = normalize(mul(normal, (float3x3)model_view));
}

void PositionNormalPS(float4 position : TEXCOORD0,
						float3 normal : TEXCOORD1,
					out float4 oPos : COLOR0,
					out float4 oNormal : COLOR1)
{
	oPos = position / position.w;
	normal = normalize(normal);
	oNormal = float4(normal, 0);
}

technique PositionNormal
{
	pass p0
	{
		FillMode = Solid;
		CullMode = CCW;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;

		VertexShader = compile vs_1_1 PositionNormalVS();
		PixelShader = compile ps_2_0 PositionNormalPS();
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

sampler1D toonMapSampler;
sampler2D posSampler;
sampler2D normalSampler;

const float2 e_barrier = float2(0.8f, 0.1f); // x=norm, y=depth
const float2 e_weights = float2(0.25f, 0.5f); // x=norm, y=depth

half4 PostToonPS(float2 tc0 : TEXCOORD0,
				float4 tc1 : TEXCOORD1,
				float4 tc2 : TEXCOORD2,
				float4 tc3 : TEXCOORD3,
				float4 tc4 : TEXCOORD4,
				float toon : TEXCOORD5) : COLOR
{
	// 法线间断点过滤
	half3 nc = tex2D(normalSampler, tc0);
	half4 nd;
	nd.x = dot(nc.xyz, tex2D(normalSampler, tc1.xy).xyz);
	nd.y = dot(nc.xyz, tex2D(normalSampler, tc1.zw).xyz);
	nd.z = dot(nc.xyz, tex2D(normalSampler, tc2.xy).xyz);
	nd.w = dot(nc.xyz, tex2D(normalSampler, tc2.zw).xyz);
	nd -= e_barrier.x;
	nd = (nd > 0) ? 1 : 0;
	half ne = (dot(nd, e_weights.x) < 1) ? 0 : 1;

	// 深度过滤，计算梯度差距
	half3 dc = tex2D(posSampler, tc0.xy);
	half4 dd;
	dd.x = tex2D(posSampler, tc1.xy).z + tex2D(posSampler, tc1.zw).z;
	dd.y = tex2D(posSampler, tc2.xy).z + tex2D(posSampler, tc2.zw).z;
	dd.z = tex2D(posSampler, tc3.xy).z + tex2D(posSampler, tc3.zw).z;
	dd.w = tex2D(posSampler, tc4.xy).z + tex2D(posSampler, tc4.zw).z;
	dd = abs(2 * dc.z - dd) - e_barrier.y;
	dd = (dd > 0) ? 1 : 0;
	half de = (dot(dd, e_weights.y) < 1) ? 1 : 0;
	
	return tex1D(toonMapSampler, toon) * de * ne;
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
