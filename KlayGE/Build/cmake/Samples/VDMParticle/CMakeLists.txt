DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_back_diffuse.dds" "f97873084a14bfb7dee9c0cc86de9c7062302ac9")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_back_normal.dds" "9505e187bb9cfa4b44516bfaa197840750986a82")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_back_specular.dds" "e432e15e1d1d85145a8617ad9da98f649ec97dc2")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_top_diffuse.dds" "28934a2cbe7f169b976899962bd4429bbec278bd")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_top_normal.dds" "9434eb8984debc8ac0c2aa9f0af2c03eaf6ab6d5")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_top_specular.dds" "25889f641377037dee04e50a415c81e3b23c7dea")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/attack_droid.meshml" "b725a810a0d6f950a6bdffab5c24250d9ce9af38")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/fire.dds" "f5037acd6159f853ee7b8848034f8f03c55f7136")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/VDMParticle/VDMParticle.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/VDMParticle/VDMParticle.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/VDMParticle.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/VDMParticle.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/Models/point_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Depth.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SMAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/ToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/VarianceDepthMap.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Depth.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Particle.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SMAA.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/VarianceDepthMap.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAAAreaTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAASearchTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/background.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/background_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/chain_texture.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/chain_texture_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/lion.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/lion_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/spnza_bricks_a_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/spnza_bricks_a_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/spnza_bricks_a_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_arch_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_arch_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_arch_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_ceiling_a_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_ceiling_a_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_column_a_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_column_a_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_column_a_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_column_b_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_column_b_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_column_b_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_column_c_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_column_c_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_column_c_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_curtain_blue_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_curtain_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_curtain_green_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_details_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_details_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_fabric_blue_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_fabric_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_fabric_green_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_fabric_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_flagpole_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_flagpole_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_floor_a_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_floor_a_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_roof_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_thorn_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_thorn_diff.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/sponza_thorn_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/vase_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/vase_dif.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/vase_hanging.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/vase_plant.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/vase_plant_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/vase_round.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/vase_round_ddn.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sponza_crytek/vase_round_spec.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_back_diffuse.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_back_normal.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_back_specular.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_top_diffuse.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_top_normal.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_top_specular.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/attack_droid.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/fire.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/Fire.psml
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/VDMParticle.kfx
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "9D6FEF0A-ACB0-4354-BDD8-0ED3CA4CF405")
ENDIF()

SETUP_SAMPLE(VDMParticle)
