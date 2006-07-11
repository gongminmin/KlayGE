#include "../Common/util.fx"

float4x4 inv_mvp;

void SkyBoxVS(float4 pos : POSITION,
			out float4 oPos : POSITION,
			out float3 texcoord0 : TEXCOORD0)
{
	oPos = pos;
	texcoord0 = mul(pos, inv_mvp);
}


sampler skybox_YcubeMapSampler;
sampler skybox_CcubeMapSampler;
float exposure_level;

float4 HDRSkyBoxPS(float3 texCoord0 : TEXCOORD0) : COLOR
{
	float3 rgb = decode_hdr_yc(texCUBE(skybox_YcubeMapSampler, texCoord0).r,
					texCUBE(skybox_CcubeMapSampler, texCoord0).ga);
	rgb *= exposure_level;
	
	return float4(rgb, 1);
}

technique HDRSkyBoxTec
{
	pass p0
	{
		CullMode = None;
		VertexShader = compile vs_1_1 SkyBoxVS();
		PixelShader = compile ps_2_0 HDRSkyBoxPS();
	}
}
