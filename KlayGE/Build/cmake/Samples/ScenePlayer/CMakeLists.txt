FIND_PACKAGE(PythonInterp)

SET(EXE_NAME "ScenePlayer")
SET(KLAYGE_ROOT_DIR "${KLAYGE_ROOT_DIR}")
SET(MEDIA_DIR "${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/")
SET(PY_SRCS "\"ScenePlayer\"")
CONFIGURE_FILE(
	${KLAYGE_ROOT_DIR}/cmake/MakePyZip.py.in
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/MakePyZip.py
	@ONLY
)

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ScenePlayer/ScenePlayer.cpp
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayer.py
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/DeferredRendering.kges
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/GlobalIllumination.kges
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ShadowCubeMap.kges
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ScenePlayer/ScenePlayer.hpp
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayer.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/Models/indirect_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/Models/point_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/Models/spot_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy2LightBuffer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Depth.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/FFT.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/MultiRes.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/RSM2VPLs.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SMAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/TAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/ToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Translucency.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/VarianceDepthMap.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/CascadedShadow.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ClusteredDeferredRendering.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy2LightBuffer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DeferredRendering.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DeferredRenderingDebug.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Depth.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBuffer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/FFT.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightIndexedDeferredRendering.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/MultiRes.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Particle.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/RSM2VPLs.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SkyBox.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SMAA.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSGI.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSR.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSS.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSVO.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/TAA.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/VarianceDepthMap.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/VPLsLighting.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/lens_effects_imag.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/lens_effects_real.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAAAreaTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAASearchTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/white_noise_tex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/Lake_CraterLake03_filtered_c.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/Lake_CraterLake03_filtered_y.dds
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
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/DeferredRendering.kges
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/GlobalIllumination.kges
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ShadowCubeMap.kges
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayerPy.zip
	${KLAYGE_PROJECT_DIR}/Samples/media/ShadowCubeMap/lamp.dds
)

SET_SOURCE_FILES_PROPERTIES(${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayerPy.zip PROPERTIES GENERATED 1)

SET(DEBUG_CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Script/KlayGE_ScriptEngine_Python${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
)

SET(RELEASE_CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Script/KlayGE_ScriptEngine_Python${KLAYGE_OUTPUT_SUFFIX}.dll
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "ED655B57-0284-4088-8180-01EED2DF4CA2")
ENDIF()

SETUP_SAMPLE(ScenePlayer)

ADD_CUSTOM_COMMAND(TARGET ScenePlayer
	PRE_BUILD
	COMMAND "${PYTHON_EXECUTABLE}" "${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/MakePyZip.py")

IF(NOT KLAYGE_PLATFORM_WINDOWS_STORE AND NOT KLAYGE_PLATFORM_ANDROID)
	ADD_CUSTOM_COMMAND(TARGET ScenePlayer
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayerPy.zip ${KLAYGE_BIN_DIR})

	INSTALL(FILES ${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayerPy.zip
		DESTINATION ${KLAYGE_BIN_DIR}
	)
ENDIF()
