texture diffusemap;
texture normalmap;
texture specularmap;

float4 lightPos;
float4 halfway;

sampler2D diffuseMapSampler = sampler_state
{
    Texture = <diffusemap>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = None;
};

sampler2D normalMapSampler = sampler_state
{
    Texture = <normalmap>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = None;
};

sampler2D specularMapSampler = sampler_state
{
    Texture = <specularmap>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = None;
};

float4 PerPixelLightingPS(float2 texCoord0 : TEXCOORD0,
							float2 texCoord1 : TEXCOORD1,
							float2 texCoord2 : TEXCOORD2,

							uniform sampler2D diffuseMap,
							uniform sampler2D normalMap,
							uniform sampler2D specularMap) : COLOR
{
	float3 normal = 2 * (tex2D(normalMap, texCoord0) - 0.5);
	float3 diffuse = tex2D(diffuseMap, texCoord1);
	float3 specular = tex2D(specularMap, texCoord2);

	return float4(saturate(dot(lightPos.xyz, normal)) * diffuse
					+ specular * pow(dot(halfway.xyz, normal), 8), 1);
}

technique PerPixelLighting
{
	pass p0
	{
		PixelShader = compile ps_1_1 PerPixelLightingPS(diffuseMapSampler,
										normalMapSampler, specularMapSampler);
	}
}
