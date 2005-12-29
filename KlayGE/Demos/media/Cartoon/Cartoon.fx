float4x4 model_view;
float4x4 proj;
float3 lightPos;


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

float inv_width, inv_height;

void PostToonVS(float4 pos : POSITION,
				float2 tex : TEXCOORD0,
				out float4 oPos : POSITION,
				out float2 oTc0: TEXCOORD0, // 中心
				out float2 oTc1: TEXCOORD1, // 左上
				out float2 oTc2: TEXCOORD2, // 右下
				out float2 oTc3: TEXCOORD3, // 右上
				out float2 oTc4: TEXCOORD4, // 左下
				out float4 oTc5: TEXCOORD5, // 左 / 右
				out float4 oTc6: TEXCOORD6) // 上 / 下
{
	oPos = pos;
	oPos.z = 0.9f;
	oTc0 = tex;
	oTc1 = tex + float2(-inv_width, -inv_height);
	oTc2 = tex + float2(+inv_width, +inv_height);
	oTc3 = tex + float2(+inv_width, -inv_height);
	oTc4 = tex + float2(-inv_width, +inv_height);
	oTc5.xy = tex + float2(-inv_width, 0);
	oTc5.zw = tex + float2(+inv_width, 0);
	oTc6.xy = tex + float2(0, -inv_height);
	oTc6.zw = tex + float2(0, +inv_height);
}

sampler1D toonMapSampler;
sampler2D posSampler;
sampler2D normalSampler;

const float2 e_barrier = float2(0.8f, 0.1f); // x=norm(~.8f), y=depth(~.5f)
const float2 e_weights = float2(0.25f, 0.5f); // x=norm, y=depth

half4 PostToonPS(float2 tc0: TEXCOORD0,
				float2 tc1: TEXCOORD1,
				float2 tc2: TEXCOORD2,
				float2 tc3: TEXCOORD3,
				float2 tc4: TEXCOORD4,
				float4 tc5: TEXCOORD5,
				float4 tc6: TEXCOORD6,
				uniform sampler1D toonMap) : COLOR
{
	half4 ret = 0;
	
	// 法线间断点过滤器
	half4 nc = tex2D(normalSampler, tc0);
	if (nc.w < 0.1)
	{
		half4 nd;
		nd.x = dot(nc.xyz, tex2D(normalSampler, tc1).xyz);
		nd.y = dot(nc.xyz, tex2D(normalSampler, tc2).xyz);
		nd.z = dot(nc.xyz, tex2D(normalSampler, tc3).xyz);
		nd.w = dot(nc.xyz, tex2D(normalSampler, tc4).xyz);
		nd -= e_barrier.x;
		nd = (nd > 0) ? 1 : 0;
		half ne = (dot(nd, e_weights.x) < 1) ? 0 : 1;

		// 深度过滤器：计算梯度差距：
		// (c-sample1)+(c-sample1_opposite)
		half4 dc = tex2D(posSampler, tc0.xy);
		half4 dd;
		dd.x = tex2D(posSampler, tc1.xy).z + tex2D(posSampler, tc2.xy).z;
		dd.y = tex2D(posSampler, tc3.xy).z + tex2D(posSampler, tc4.xy).z;
		dd.z = tex2D(posSampler, tc5.xy).z + tex2D(posSampler, tc5.zw).z;
		dd.w = tex2D(posSampler, tc6.xy).z + tex2D(posSampler, tc6.zw).z;
		dd = abs(2 * dc.z - dd) - e_barrier.y;
		dd = (dd > 0) ? 1 : 0;
		half de = (dot(dd, e_weights.y) < 1) ? 1 : 0;

		half3 L = normalize(lightPos - dc.xyz);
		half toon = dot(nc.xyz, L);

		ret = tex1D(toonMap, toon) * de * ne;
		ret.a = 1;
	}

	return ret;
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

		AlphaTestEnable = true;
		AlphaFunc = Greater;
		AlphaRef = 8;

		VertexShader = compile vs_1_1 PostToonVS();
		PixelShader = compile ps_2_0 PostToonPS(toonMapSampler);
	}
}
