float4x4 modelview : WORLDVIEW;
float4x4 proj : PROJECTION;
float4x4 modelviewIT;
float currentAngle;
float3 lightDir = float3(0, 0, 1);

float half_length;
float half_width;


void VertexDisplacementVS(float4 pos			: POSITION,
								float2 texcoord0	: TEXCOORD0,
								out float4 oPos			: POSITION,
								out float4 oClr			: COLOR0,
								out float2 oTexcoord0	: TEXCOORD0)
{
	float4 v = pos;
	float2 offset = pos.xy + float2(half_length, -half_width);
	v.z = (cos(offset.x + currentAngle) + sin(offset.y + currentAngle)) * offset.x * 0.2f;

	half3 x_dir = half3(0.5f, 0, -sin(pos.x + currentAngle) * 0.2f);
	half3 y_dir = half3(0, 0.5f, cos(pos.y + currentAngle) * pos.x * 0.2f);
	half3 normal = mul(normalize(cross(x_dir, y_dir)), (float3x3)modelviewIT);

	oPos = mul(mul(v, modelview), proj);
	oClr = float4(max(0, dot(lightDir, normal)).xxx, 1);
	oTexcoord0 = texcoord0;
}

sampler2D flagSampler;

float4 VertexDisplacementPS(float4 clr : COLOR0,
							float2 texCoord : TEXCOORD0) : COLOR0
{
	return clr * tex2D(flagSampler, texCoord);
}

technique VertexDisplacement
{
	pass p0
	{
		FillMode = Solid;
		CullMode = CCW;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;

		VertexShader = compile vs_1_1 VertexDisplacementVS();
		PixelShader = compile ps_1_1 VertexDisplacementPS();
	}
}
