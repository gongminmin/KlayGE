DOWNLOAD_FILE("KlayGE/Samples/media/VDMParticle/At_Droid_back_diffuse.dds" "bb080b8e949760116f22ca09d4b2cf63002c3847" "9b5d31bc237efb590d8170a14b264d0b40d1f07c")
DOWNLOAD_FILE("KlayGE/Samples/media/VDMParticle/At_Droid_back_normal.dds" "bb080b8e949760116f22ca09d4b2cf63002c3847" "453717b09ff9c3bba47ac3fc98b318568ab78aa6")
DOWNLOAD_FILE("KlayGE/Samples/media/VDMParticle/At_Droid_back_specular.dds" "bb080b8e949760116f22ca09d4b2cf63002c3847" "1485ff55d5c45f467e3e4dcaae0c75b4df83d4cc")
DOWNLOAD_FILE("KlayGE/Samples/media/VDMParticle/At_Droid_top_diffuse.dds" "bb080b8e949760116f22ca09d4b2cf63002c3847" "8a4533a7935a7bef1ad87a0192691ef82c6380a9")
DOWNLOAD_FILE("KlayGE/Samples/media/VDMParticle/At_Droid_top_normal.dds" "bb080b8e949760116f22ca09d4b2cf63002c3847" "34403bff235cece4196d037c2c4f7665497c93ad")
DOWNLOAD_FILE("KlayGE/Samples/media/VDMParticle/At_Droid_top_specular.dds" "bb080b8e949760116f22ca09d4b2cf63002c3847" "34afd65711732abf8b4a23862be4a1313798048d")
DOWNLOAD_FILE("KlayGE/Samples/media/VDMParticle/attack_droid.meshml" "bb080b8e949760116f22ca09d4b2cf63002c3847" "b725a810a0d6f950a6bdffab5c24250d9ce9af38")
DOWNLOAD_FILE("KlayGE/Samples/media/VDMParticle/fire.dds" "bb080b8e949760116f22ca09d4b2cf63002c3847" "f5037acd6159f853ee7b8848034f8f03c55f7136")

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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/VarianceDepthMap.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
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

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "9D6FEF0A-ACB0-4354-BDD8-0ED3CA4CF405")
ENDIF()

SETUP_SAMPLE(VDMParticle)
