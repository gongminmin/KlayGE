SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/CausticsMap/CausticsMap.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/CausticsMap/CausticsMap.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Caustics.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Scene.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Caustics.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/Models/spot_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Depth.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Depth.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SkyBox.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/uffizi_cross_filtered_c.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/uffizi_cross_filtered_y.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sphere_high.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/bunny.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/point.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Caustics.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Scene.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/ShadowCubeMap/ShadowCubeMap.kfx
	${KLAYGE_PROJECT_DIR}/Tutorials/media/DistanceMapping/diffuse.dds
	${KLAYGE_PROJECT_DIR}/Tutorials/media/DistanceMapping/distance.dds
	${KLAYGE_PROJECT_DIR}/Tutorials/media/DistanceMapping/normal.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "D6CC245C-8560-457E-8F9D-5AFB2C912169")
ENDIF()

SETUP_SAMPLE(CausticsMap)
