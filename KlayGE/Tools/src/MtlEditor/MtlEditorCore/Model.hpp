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
	void UpdateTechniques(KlayGE::uint32_t mtl_index);

	KlayGE::uint32_t CopyMaterial(KlayGE::uint32_t mtl_index);
	KlayGE::uint32_t ImportMaterial(std::string const & name);

	KlayGE::RenderEffectPtr const & Effect() const
	{
		return effect_;
	}

private:
	KlayGE::RenderEffectPtr effect_;

	KlayGE::RenderTechnique* visualize_gbuffer_mrt_techs_[2];

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
	virtual void UpdateTechniques() override;

private:
	int visualize_;
};

#endif		// _MTL_EDITOR_CORE_MODEL_HPP
