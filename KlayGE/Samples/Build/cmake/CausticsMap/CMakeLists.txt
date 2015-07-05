DOWNLOAD_FILE("KlayGE/Samples/media/CausticsMap/bunny.7z" "cbda47a1678ce70b6720856736100979d469e159" "5d62c4cb2f1036ba22b1ae35a845b978efab2cd3")
DOWNLOAD_FILE("KlayGE/Samples/media/CausticsMap/diffuse.dds" "55df1241283c25a5174c96090d7918060aef9966" "b68b543509b380eb1a65755107f0fcdcf8f4d9e6")
DOWNLOAD_FILE("KlayGE/Samples/media/CausticsMap/distance.dds" "55df1241283c25a5174c96090d7918060aef9966" "60aaa3599993f631aa266b8ad8cf8ac3f8858090")
DOWNLOAD_FILE("KlayGE/Samples/media/CausticsMap/normal.dds" "55df1241283c25a5174c96090d7918060aef9966" "3b6fc91238c77d42efa091e8ec047f97a33c3840")
DOWNLOAD_FILE("KlayGE/Samples/media/CausticsMap/point.dds" "cbda47a1678ce70b6720856736100979d469e159" "551e042821897c6338be6063ad18bbbc7fa5b5af")

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
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/diffuse.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/distance.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/normal.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/point.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Caustics.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Scene.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/ShadowCubeMap/ShadowCubeMap.kfx
)

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "D6CC245C-8560-457E-8F9D-5AFB2C912169")
ENDIF()

SETUP_SAMPLE(CausticsMap)
