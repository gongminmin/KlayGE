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

	void VisualizeLighting();
	void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index);
	void VisualizeTexture(int slot);

	void LineMode(bool line_mode);
	void SmoothMesh(bool smooth);
	void SetTessFactor(KlayGE::int32_t tess_factor);

	KlayGE::RenderEffectPtr const & Effect() const
	{
		return effect_;
	}

private:
	KlayGE::RenderEffectPtr effect_;
};

class DetailedSkinnedMesh : public KlayGE::SkinnedMesh
{
public:
	DetailedSkinnedMesh(KlayGE::RenderModelPtr const & model, std::wstring const & name);

	void BuildMeshInfo();

	void OnRenderBegin();
	void Render();

	void SetLightPos(KlayGE::float3 const & light_pos);

	void VisualizeLighting();
	void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index);
	void VisualizeTexture(int slot);

	void LineMode(bool line_mode);
	void SmoothMesh(bool smooth);
	void SetTessFactor(KlayGE::int32_t tess_factor);

	bool HasOpacityMap() const
	{
		return opacity_map_enabled_;
	}

private:
	void UpdateTech();

private:
	KlayGE::RenderEffectPtr effect_;

	bool line_mode_;
	bool smooth_mesh_;
	float tess_factor_;
	std::string visualize_;
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
};

#endif		// _MODEL_HPP
