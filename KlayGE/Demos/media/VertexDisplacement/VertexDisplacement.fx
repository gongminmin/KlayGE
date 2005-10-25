float4x4 modelview : WORLDVIEW;
float4x4 proj : PROJECTION;
float4x4 modelviewIT;
float currentAngle;
float3 lightDir = float3(0, 0, 1);

float half_length;
float half_width;

struct VS_INPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float4 clr			: COLOR0;
	float2 texcoord0	: TEXCOORD0;
};

VS_OUTPUT VertexDisplacementVS(VS_INPUT input,
					uniform float4x4 modelview,
					uniform float4x4 modelviewIT,
					uniform float4x4 proj,
					uniform float currentAngle)
{
	VS_OUTPUT output;

	float4 v = input.pos;
	float2 offset = input.pos.xy + float2(half_length, -half_width);
	v.z = (cos(offset.x + currentAngle) + sin(offset.y + currentAngle)) * offset.x * 0.2f;

	half3 x_dir = half3(0.5f, 0, -sin(input.pos.x + currentAngle) * 0.2f);
	half3 y_dir = half3(0, 0.5f, cos(input.pos.y + currentAngle) * input.pos.x * 0.2f);
	half3 normal = mul(normalize(cross(x_dir, y_dir)), (float3x3)modelviewIT);

	output.pos = mul(mul(v, modelview), proj);
	output.clr = float4(max(0, dot(lightDir, normal)).xxx, 1);
	output.texcoord0 = input.texcoord0;

	return output;
}

sampler2D flagSampler;

float4 VertexDisplacementPS(float4 clr : COLOR0,
							float2 texCoord : TEXCOORD0,
		uniform sampler2D flagSampler) : COLOR0
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

		VertexShader = compile vs_1_1 VertexDisplacementVS(modelview, modelviewIT, proj, currentAngle);
		PixelShader = compile ps_1_1 VertexDisplacementPS(flagSampler);
	}
}
