float4x4 worldviewproj : WORLDVIEWPROJECTION;
float4 lightPos;
float4 eyePos;

struct VS_INPUT
{
	float3 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;
	float3 T			: TEXCOORD1;	// in object space
	float3 B			: TEXCOORD2;	// in object space
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;

	float3 L			: TEXCOORD1;	// in tangent space
	float3 V			: TEXCOORD2;	// in tangent space
};

VS_OUTPUT ParallaxVS(VS_INPUT input,
						uniform float4x4 worldviewproj,
						uniform float4 lightPos,
						uniform float4 eyePos)
{
	VS_OUTPUT output;

	output.pos = mul(float4(input.pos, 1), worldviewproj);
	output.texcoord0 = input.texcoord0;

	float3x3 objToTangentSpace;
	objToTangentSpace[0] = input.T;
	objToTangentSpace[1] = input.B;
	objToTangentSpace[2] = cross(input.T, input.B);

	float3 lightVec = normalize(lightPos.xyz - input.pos);
	float3 viewVec = normalize(eyePos.xyz - input.pos);

	output.L = normalize(mul(objToTangentSpace, lightVec)) * 0.5 + 0.5;
	output.V = normalize(mul(objToTangentSpace, viewVec)) * 0.5 + 0.5;

	return output;
}


texture diffusemap;
texture normalmap;
texture heightmap;

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

float4 ParallaxPS(float2 texCoord0	: TEXCOORD0,
					float3 L		: TEXCOORD1,
					float3 V		: TEXCOORD2,

					uniform sampler2D diffuseMap,
					uniform sampler2D normalMap,
					uniform sampler2D heightMap) : COLOR
{
	float height = 2 * tex2D(heightMap, texCoord0).r - 1;
	float2 texUV = texCoord0 + ((2 * V - 1).xy * (height * 0.04));

	float3 bumpNormal = 2 * tex2D(normalMap, texUV) - 1;
	float3 diffuse = tex2D(diffuseMap, texUV);

	float3 lightVec = 2 * L - 1;
	float diffuseFactor = saturate(dot(lightVec, bumpNormal));

	return float4(diffuse * diffuseFactor, 1);
}

technique Parallax
{
	pass p0
	{
		VertexShader = compile vs_1_1 ParallaxVS(worldviewproj, lightPos, eyePos);
		PixelShader = compile ps_1_4 ParallaxPS(diffuseMapSampler,
										normalMapSampler, heightMapSampler);
	}
}
