float4x4 ViewProj : VIEWPROJECTION;
float4 lightPos;

struct VS_INPUT
{
	float4 pos			: POSITION;
	float3 normal		: NORMAL;

	float4 row0			: TEXCOORD1;
	float4 row1			: TEXCOORD2;
	float4 row2			: TEXCOORD3;
	float4 clr			: COLOR0;
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float4 clr			: COLOR;
};

VS_OUTPUT InstanceVS(VS_INPUT input,
						uniform float4x4 ViewProj,
						uniform float4 lightPos)
{
	VS_OUTPUT output;
	
	float4x4 model = { input.row0, input.row1, input.row2, float4(0, 0, 0, 1) };
	model = transpose(model);

	output.pos = mul(mul(input.pos, model), ViewProj);
	
	float3 normal = mul(input.normal, (float3x3)model);
	output.clr = float4(input.clr.rgb * dot(normal, lightPos.xyz - input.pos.xyz), input.clr.a);

	return output;
}


float4 PS(float4 clr : COLOR) : COLOR
{
	return clr;
}

technique Instance
{
	pass p0
	{
		CullMode = CCW;
		
		VertexShader = compile vs_1_1 InstanceVS(ViewProj, lightPos);
		PixelShader = compile ps_1_1 PS();
	}
}

float4x4 modelmat : WORLD;
float4 color;

VS_OUTPUT NormalMeshVS(VS_INPUT input,
						uniform float4x4 ViewProj,
						uniform float4 lightPos)
{
	VS_OUTPUT output;
	
	output.pos = mul(mul(input.pos, modelmat), ViewProj);
	
	float3 normal = mul(input.normal, (float3x3)modelmat);
	output.clr = float4(color.rgb * dot(normal, lightPos.xyz - input.pos.xyz), color.a);

	return output;
}

technique NormalMesh
{
	pass p0
	{
		CullMode = CCW;

		VertexShader = compile vs_1_1 NormalMeshVS(ViewProj, lightPos);
		PixelShader = compile ps_1_1 PS();
	}
}
