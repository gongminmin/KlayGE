float4x4 worldviewproj : WORLDVIEWPROJECTION;
float4 lightPos;
float4 eyePos;

struct VS_INPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;
	float3 T			: TEXCOORD1;	// in object space
	float3 B			: TEXCOORD2;	// in object space
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;

	float4 L			: TEXCOORD1;	// in tangent space
	float4 V			: TEXCOORD2;	// in tangent space
};

VS_OUTPUT ParallaxVS(VS_INPUT input,
						uniform float4x4 worldviewproj,
						uniform float4 lightPos,
						uniform float4 eyePos)
{
	VS_OUTPUT output;

	output.pos = mul(input.pos, worldviewproj);
	output.texcoord0 = input.texcoord0;

	float3x3 objToTangentSpace;
	objToTangentSpace[0] = input.T;
	objToTangentSpace[1] = input.B;
	objToTangentSpace[2] = cross(input.T, input.B);

	float3 lightVec = lightPos.xyz - input.pos;
	float3 viewVec = eyePos.xyz - input.pos;

	output.L = float4(mul(objToTangentSpace, lightVec), 1);
	output.V = float4(mul(objToTangentSpace, viewVec), 1);

	return output;
}


texture diffusemap;
texture normalmap;
texture heightmap;
texture normalizermap;

sampler2D diffuseMapSampler = sampler_state
{
	Texture = <diffusemap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Wrap;
	AddressV  = Wrap;
};

sampler2D normalMapSampler = sampler_state
{
	Texture = <normalmap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Wrap;
	AddressV  = Wrap;
};

sampler2D heightMapSampler = sampler_state
{
	Texture = <heightmap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Wrap;
	AddressV  = Wrap;
};

samplerCUBE normalizerMapSampler = sampler_state
{
	Texture = <normalizermap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Wrap;
	AddressV  = Wrap;
	AddressW  = Wrap;
};

float4 ParallaxPS(float2 texCoord0	: TEXCOORD0,
					float3 L		: TEXCOORD1,
					float3 V		: TEXCOORD2,

					uniform sampler2D diffuseMap,
					uniform sampler2D normalMap,
					uniform sampler2D heightMap,
					uniform samplerCUBE normalizerMap) : COLOR
{
	V = 2 * texCUBE(normalizerMap, V) - 1;
	
	float height = tex2D(heightMap, texCoord0);
	float2 texUV = texCoord0 + (V.xy * (height * 0.04 - 0.02));

	float3 diffuse = tex2D(diffuseMap, texUV);

	float3 bumpNormal = 2 * texCUBE(normalizerMap, 2 * tex2D(normalMap, texUV) - 1) - 1;
	float3 lightVec = 2 * texCUBE(normalizerMap, L) - 1;
	float diffuseFactor = dot(lightVec, bumpNormal);

	return float4(diffuse * diffuseFactor, 1);
}

technique Parallax
{
	pass p0
	{
		VertexShader = compile vs_1_1 ParallaxVS(worldviewproj, lightPos, eyePos);
		PixelShader = compile ps_2_0 ParallaxPS(diffuseMapSampler,
										normalMapSampler, heightMapSampler,
										normalizerMapSampler);
	}
}
