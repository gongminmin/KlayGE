#ifndef _MTL_EDITOR_CORE_MODEL_HPP
#define _MTL_EDITOR_CORE_MODEL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>

void InitInstancedTessBuffs();
void DeinitInstancedTessBuffs();

class DetailedSkinnedMesh;

class DetailedSkinnedModel : public KlayGE::SkinnedModel
{
	friend class DetailedSkinnedMesh;

public:
	explicit DetailedSkinnedModel(std::wstring const & name);

	virtual void DoBuildModelInfo() override;

	virtual bool IsSkinned() const override
	{
		return is_skinned_;
	}

	void SetTime(float time);

	void VisualizeLighting();
	void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index);
	void VisualizeTexture(int slot);

	void UpdateEffectAttrib(KlayGE::uint32_t mtl_index);
	void UpdateMaterial(KlayGE::uint32_t mtl_index);

	KlayGE::RenderEffectPtr const & Effect() const
	{
		return effect_;
	}

private:
	KlayGE::RenderEffectPtr effect_;

	KlayGE::RenderTechniquePtr depth_techs_[3];	
	KlayGE::RenderTechniquePtr depth_alpha_test_techs_[3];	
	KlayGE::RenderTechniquePtr depth_alpha_blend_back_techs_[3];	
	KlayGE::RenderTechniquePtr depth_alpha_blend_front_techs_[3];	
	KlayGE::RenderTechniquePtr gbuffer_rt0_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_alpha_test_rt0_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_back_rt0_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_front_rt0_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_rt1_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_alpha_test_rt1_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_back_rt1_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_front_rt1_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_mrt_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_alpha_test_mrt_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_back_mrt_techs_[3];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_front_mrt_techs_[3];
	KlayGE::RenderTechniquePtr special_shading_techs_[3];
	KlayGE::RenderTechniquePtr special_shading_alpha_blend_back_techs_[3];
	KlayGE::RenderTechniquePtr special_shading_alpha_blend_front_techs_[3];

	bool is_skinned_;
};

class DetailedSkinnedMesh : public KlayGE::SkinnedMesh
{
public:
	DetailedSkinnedMesh(KlayGE::RenderModelPtr const & model, std::wstring const & name);

	virtual void DoBuildMeshInfo() override;

	void OnRenderBegin();

	void VisualizeLighting();
	void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index);
	void VisualizeTexture(int slot);

	void UpdateEffectAttrib();
	void UpdateMaterial();

private:
	virtual void UpdateTechniques() override;

private:
	int visualize_;
};

#endif		// _MTL_EDITOR_CORE_MODEL_HPP
