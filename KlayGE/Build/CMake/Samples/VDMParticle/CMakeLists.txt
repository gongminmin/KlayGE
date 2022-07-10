DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_back_diffuse.jpg" "4E2D10CF7385A3DA1B0DA0D5FCF76A3EB66D6A38")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_back_normal.jpg" "286EA8170335D88E06A3FA0A1623990738B0133A")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_back_specular.jpg" "809516113FDC0F0C2429754DC79E018796A73999")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_top_diffuse.jpg" "4E802E3DC8B64E61A8A9E68DE59B003E694612BB")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_top_normal.jpg" "82B1A0DED59CB8497A411D2B4574E297E9330B80")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/At_Droid_top_specular.jpg" "463534C478C47E62C48C0E4F75B73C3D55B85091")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VDMParticle/attack_droid.glb" "5D4936455E2B38B8821A000D352961B85110FA1D")
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
	${KLAYGE_PROJECT_DIR}/media/Models/PointLightProxy.glb.model_bin
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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PredefinedCBuffers.kfx
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
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/background.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/background_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/chain_texture.png.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/chain_texture_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/lion.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/lion_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/spnza_bricks_a_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/spnza_bricks_a_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/spnza_bricks_a_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza.glb.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_arch_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_arch_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_arch_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_ceiling_a_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_ceiling_a_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_column_a_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_column_a_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_column_a_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_column_b_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_column_b_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_column_b_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_column_c_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_column_c_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_column_c_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_curtain_blue_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_curtain_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_curtain_green_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_details_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_details_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_fabric_blue_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_fabric_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_fabric_green_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_fabric_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_flagpole_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_flagpole_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_floor_a_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_floor_a_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_roof_diff.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_thorn_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_thorn_diff.png.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/sponza_thorn_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/vase_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/vase_dif.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/vase_hanging.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/vase_plant.png.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/vase_plant_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/vase_round.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/vase_round_ddn.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/Sponza/vase_round_spec.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_back_diffuse.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_back_normal.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_back_specular.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_top_diffuse.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_top_normal.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/At_Droid_top_specular.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/attack_droid.glb.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/fire.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/Fire.psml
	${KLAYGE_PROJECT_DIR}/Samples/media/VDMParticle/VDMParticle.kfx
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "9D6FEF0A-ACB0-4354-BDD8-0ED3CA4CF405")
ENDIF()

SETUP_SAMPLE(VDMParticle)
