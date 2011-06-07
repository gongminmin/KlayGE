#ifndef _MODEL_HPP
#define _MODEL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>

class DetailedSkinnedMesh;

class DetailedSkinnedModel : public KlayGE::SkinnedModel
{
	friend class DetailedSkinnedMesh;

public:
	explicit DetailedSkinnedModel(std::wstring const & name);

	void BuildModelInfo();

	void SetTime(float time);
	void SetLightPos(KlayGE::float3 const & light_pos);
	void SetEyePos(KlayGE::float3 const & eye_pos);

	void VisualizeLighting();
	void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index);
	void VisualizeTexture(int slot);

	void LineMode(bool line_mode);
	void SmoothMesh(bool smooth);
	void SetTessFactor(int32_t tess_factor);

	KlayGE::RenderEffectPtr const & Effect() const
	{
		return effect_;
	}

	std::map<std::string, KlayGE::TexturePtr>& TexPool()
	{
		return tex_pool_;
	}

	KlayGE::TexturePtr const & EmptyNormalMap() const
	{
		return empty_normal_map_;
	}

private:
	KlayGE::RenderEffectPtr effect_;
	std::map<std::string, KlayGE::TexturePtr> tex_pool_;

	KlayGE::TexturePtr empty_normal_map_;
};

class DetailedSkinnedMesh : public KlayGE::SkinnedMesh
{
public:
	DetailedSkinnedMesh(KlayGE::RenderModelPtr const & model, std::wstring const & name);

	void BuildMeshInfo();

	void OnRenderBegin();
	void Render();

	void SetWorld(KlayGE::float4x4 const & mat);
	void SetLightPos(KlayGE::float3 const & light_pos);
	void SetEyePos(KlayGE::float3 const & eye_pos);

	void VisualizeLighting();
	void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index);
	void VisualizeTexture(int slot);

	void LineMode(bool line_mode);
	void SmoothMesh(bool smooth);
	void SetTessFactor(int32_t tess_factor);

	bool HasOpacityMap() const
	{
		return has_opacity_map_;
	}

private:
	void UpdateTech();

private:
	KlayGE::float4x4 world_;
	KlayGE::float4x4 inv_world_;
	KlayGE::RenderEffectPtr effect_;
	KlayGE::float3 light_pos_;

	bool line_mode_;
	bool smooth_mesh_;
	float tess_factor_;
	std::string visualize_;
	bool has_opacity_map_;
	bool has_skinned_;

	KlayGE::RenderLayoutPtr mesh_rl_;
	KlayGE::RenderLayoutPtr point_rl_;
	KlayGE::RenderLayoutPtr skinned_rl_;
	KlayGE::RenderLayoutPtr tess_pattern_rl_;
	KlayGE::GraphicsBufferPtr skinned_pos_vb_;
	KlayGE::GraphicsBufferPtr skinned_tex_vb_;
	KlayGE::GraphicsBufferPtr skinned_normal_vb_;
	KlayGE::GraphicsBufferPtr skinned_tangent_vb_;
	KlayGE::GraphicsBufferPtr bindable_ib_;

	KlayGE::TexturePtr diffuse_map_;
	KlayGE::TexturePtr specular_map_;
	KlayGE::TexturePtr emit_map_;
	KlayGE::TexturePtr opacity_map_;
	KlayGE::TexturePtr normal_map_;

	KlayGE::float4 ambient_clr_;
	KlayGE::float4 diffuse_clr_;
	KlayGE::float4 specular_clr_;
	KlayGE::float4 emit_clr_;
	KlayGE::float4 opacity_clr_;
	float specular_level_;
	float shininess_;
};

#endif		// _MODEL_HPP
