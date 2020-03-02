#ifndef _MTL_EDITOR_CORE_MODEL_HPP
#define _MTL_EDITOR_CORE_MODEL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>

class DetailedSkinnedMesh;

class DetailedSkinnedModel : public KlayGE::SkinnedModel
{
	friend class DetailedSkinnedMesh;

public:
	explicit DetailedSkinnedModel(std::wstring_view name, uint32_t node_attrib);

	void DoBuildModelInfo() override;

	bool IsSkinned() const override
	{
		return is_skinned_;
	}

	void SetTime(float time);

	KlayGE::uint32_t CopyMaterial(KlayGE::uint32_t mtl_index);
	KlayGE::uint32_t ImportMaterial(std::string const & name);

	KlayGE::RenderEffectPtr const & Effect() const
	{
		return effect_;
	}

private:
	KlayGE::RenderEffectPtr effect_;

	KlayGE::RenderTechnique* visualize_gbuffer_techs_[2];

	bool is_skinned_;
};

class DetailedSkinnedMesh : public KlayGE::SkinnedMesh
{
public:
	explicit DetailedSkinnedMesh(std::wstring_view name);

	void VisualizeLighting();
	void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index);
	void VisualizeTexture(int slot);

	void UpdateEffectAttrib();
	void UpdateMaterial();
	virtual void UpdateTechniques() override;

protected:
	void DoBuildMeshInfo(KlayGE::RenderModel const & model) override;

private:
	int visualize_;

	DetailedSkinnedModel const * model_;
};

class SkeletonMesh : public KlayGE::SkinnedMesh
{
public:
	explicit SkeletonMesh(KlayGE::RenderModel const & model);

private:
	DetailedSkinnedModel const * model_;
};

#endif		// _MTL_EDITOR_CORE_MODEL_HPP
