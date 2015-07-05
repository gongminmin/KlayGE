DOWNLOAD_FILE("KlayGE/Samples/media/DetailedSurface/DetailedSurface.jdt" "cbda47a1678ce70b6720856736100979d469e159" "9fd82c4b5528d181e0c40e29e6d20d9e7427e8be")
DOWNLOAD_FILE("KlayGE/Samples/media/DetailedSurface/diffuse.dds" "cbda47a1678ce70b6720856736100979d469e159" "95430e9a0e2100f0d4c721cc6a49b4a39df8de34")
DOWNLOAD_FILE("KlayGE/Samples/media/DetailedSurface/height.dds" "cbda47a1678ce70b6720856736100979d469e159" "8a5475aa5e5e5c64dc06804c63325c82e6ea7866")
DOWNLOAD_FILE("KlayGE/Samples/media/DetailedSurface/na_length.dds" "cbda47a1678ce70b6720856736100979d469e159" "cd9a5826b056bf8d37a9d3ecb210d17a8b1607b3")
DOWNLOAD_FILE("KlayGE/Samples/media/DetailedSurface/normal.dds" "cbda47a1678ce70b6720856736100979d469e159" "226e1bff32280d0a598d537c9547940bd07fd506")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DetailedSurface/DetailedSurface.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DetailedSurface/DetailedSurface.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/DetailedSurface.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/DetailedSurface.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/Models/point_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/teapot.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/DetailedSurface.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/DetailedSurface.jdt
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/na_length.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "BF549728-1DBA-48B7-B973-4CAB423B57F2")
ENDIF()

SETUP_SAMPLE(DetailedSurface)
