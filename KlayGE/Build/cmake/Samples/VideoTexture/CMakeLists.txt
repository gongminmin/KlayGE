DOWNLOAD_FILE("KlayGE/Samples/media/VideoTexture/big_buck_bunny.avi" "cbda47a1678ce70b6720856736100979d469e159" "5074a181b6f246d1eaa7879010f2c2319a3cd857")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/VideoTexture/VideoTexture.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/VideoTexture/VideoTexture.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/VideoTexture/VideoTexture.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/VideoTexture/VideoTexture.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/Models/point_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SMAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/ToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SMAA.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAAAreaTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAASearchTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VideoTexture/big_buck_bunny.avi
	${KLAYGE_PROJECT_DIR}/Samples/media/VideoTexture/VideoTexture.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/teapot.meshml.model_bin
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "93BE4B67-5093-4D00-8CF1-C6C3F0C5F268")
ENDIF()

SETUP_SAMPLE(VideoTexture)
