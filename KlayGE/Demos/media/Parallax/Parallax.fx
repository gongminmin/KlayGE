float3 decompress_normal(float4 comp_normal)
{
	float3 normal;
	normal.xy = comp_normal.ag * 2 - 1;
	normal.z = sqrt(1 - dot(normal.xy, normal.xy));
	return normal;
}

float4x4 mvp : WORLDVIEWPROJECTION;
float3 lightPos;
float3 eyePos;

struct VS_INPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;
	float3 T			: TANGENT;	// in object space
	float3 B			: BINORMAL;	// in object space
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;

	float3 L			: TEXCOORD1;	// in tangent space
	float3 V			: TEXCOORD2;	// in tangent space
};

VS_OUTPUT ParallaxVS(VS_INPUT input,
						uniform float4x4 mvp,
						uniform float3 lightPos,
						uniform float3 eyePos)
{
	VS_OUTPUT output;

	output.pos = mul(input.pos, mvp);
	output.texcoord0 = input.texcoord0;

	float3x3 objToTangentSpace;
	objToTangentSpace[0] = input.T;
	objToTangentSpace[1] = input.B;
	objToTangentSpace[2] = cross(input.T, input.B);

	float3 lightVec = lightPos - input.pos;
	float3 viewVec = eyePos - input.pos;

	output.L = mul(lightVec, objToTangentSpace);
	output.V = mul(viewVec, objToTangentSpace);

	return output;
}


sampler2D diffuseMapSampler;
sampler2D normalMapSampler;
sampler2D heightMapSampler;
samplerCUBE normalizerMapSampler;

half3 NormalizeByCube(half3 v)
{
	return texCUBE(normalizerMapSampler, v).rgb * 2 - 1;
}

half4 ParallaxPS(float2 texCoord0	: TEXCOORD0,
					float3 L		: TEXCOORD1,
					float3 V		: TEXCOORD2,

					uniform sampler2D diffuseMap,
					uniform sampler2D normalMap,
					uniform sampler2D heightMap,
					uniform samplerCUBE normalizerMap) : COLOR
{
	half3 view = NormalizeByCube(V);

	half height = tex2D(heightMap, texCoord0).r * 0.06 - 0.02;
	half2 texUV = texCoord0 + (view.xy * height);

	half3 diffuse = tex2D(diffuseMap, texUV).rgb;

	half3 bumpNormal = decompress_normal(tex2D(normalMap, texUV));
	half3 lightVec = NormalizeByCube(L);
	half diffuseFactor = dot(lightVec, bumpNormal);

	return half4(diffuse * diffuseFactor, 1);
}

technique Parallax
{
	pass p0
	{
		CullMode = CCW;
		
		VertexShader = compile vs_1_1 ParallaxVS(mvp, lightPos, eyePos);
		PixelShader = compile ps_2_0 ParallaxPS(diffuseMapSampler,
										normalMapSampler, heightMapSampler,
										normalizerMapSampler);
	}
}
