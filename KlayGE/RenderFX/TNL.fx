int MaxLights = 8;
int MaxClipPlanes = 6;
int MaxTextureUnits = 8;
int MaxTextureCoords = 8;
int MaxDrawBuffers = 1;

float4x4 global_model_matrix;
float4x4 global_view_matrix;
float4x4 global_projection_matrix;
float4x4 global_model_view_matrix;
float4x4 global_model_view_projection_matrix;
float4x4 global_texture_matrix[8];

float4x4 global_model_matrix_inverse;
float4x4 global_view_matrix_inverse;
float4x4 global_projection_matrix_inverse;
float4x4 global_model_view_matrix_inverse;
float4x4 global_model_view_projection_matrix_inverse;
float4x4 global_texture_matrix_inverse[8];

float4x4 global_model_matrix_transpose;
float4x4 global_view_matrix_transpose;
float4x4 global_projection_matrix_transpose;
float4x4 global_model_view_matrix_transpose;
float4x4 global_model_view_projection_matrix_transpose;
float4x4 global_texture_matrix_transpose[8];

float4x4 global_model_matrix_inverse_transpose;
float4x4 global_view_matrix_inverse_transpose;
float4x4 global_projection_matrix_inverse_transpose;
float4x4 global_model_view_matrix_inverse_transpose;
float4x4 global_model_view_projection_matrix_inverse_transpose;
float4x4 global_texture_matrix_inverse_transpose[8];

void UpdateMatrices()
{
	global_model_view_matrix = global_model_matrix * global_view_matrix;
	global_model_view_projection_matrix = global_model_view_matrix * global_projection_matrix;

	global_model_view_matrix_inverse = global_view_matrix_inverse * global_model_matrix_inverse;
	global_model_view_projection_matrix_inverse = global_projection_matrix_transpose * global_model_view_matrix_inverse;

	global_model_matrix_transpose = transpose(global_model_matrix);
	global_model_view_matrix_transpose = transpose(global_model_view_matrix);
	global_model_view_projection_matrix_transpose = transpose(global_model_view_projection_matrix);

	global_model_matrix_inverse_transpose = transpose(global_model_matrix_inverse);
	global_model_view_matrix_inverse_transpose = transpose(global_model_view_matrix_inverse);
	global_model_view_projection_matrix_inverse_transpose = transpose(global_model_view_projection_matrix_inverse);
	
	for (int i = 0; i < 8; ++ i)
	{
		global_texture_matrix_transpose[i] = transpose(global_texture_matrix[i]);
		global_texture_matrix_inverse_transpose[i] = transpose(global_texture_matrix_inverse[i]);
	}
}

void InitMatrices()
{
	float4x4 identity = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

	global_model_matrix = identity;
	global_view_matrix = identity;
	global_projection_matrix = identity;

	global_model_matrix_inverse = identity;
	global_view_matrix_inverse = identity;
	global_projection_matrix_inverse = identity;

	for (int i = 0; i < 8; ++ i)
	{
		global_texture_matrix[i] = identity;
		global_texture_matrix_inverse[i] = identity;
	}
	
	UpdateMatrices();
}

void ModelMatrix(float4x4 model, float4x4 model_inverse)
{
	global_model_matrix = model;
	global_model_matrix_inverse = model_inverse;

	UpdateMatrices();
}

void ViewMatrix(float4x4 view, float4x4 view_inverse)
{
	global_view_matrix = view;
	global_view_matrix_inverse = view_inverse;

	UpdateMatrices();
}

void ProjectionMatrix(float4x4 proj, float4x4 proj_inverse)
{
	global_projection_matrix = proj;
	global_projection_matrix_inverse = proj_inverse;
	
	UpdateMatrices();
}

float DirectionalLighting(float3 lightDir, float3 normal)
{
	return dot(-lightDir, normal);
}

float PointLighting(float3 lightPos, float3 pos, float3 normal)
{
	return dot(lightPos - pos, normal);
}

float4 TransformPos(float4 pos)
{
	return mul(pos, global_model_view_projection_matrix);
}

float3 TransformNormal(float3 normal)
{
	return mul(normal, (float3x3)global_model_view_matrix_inverse_transpose);
}
