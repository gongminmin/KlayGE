DOWNLOAD_FILE("KlayGE/Samples/media/SSSSS/Infinite-Level_02.meshml" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Samples/media/SSSSS/Infinite-Level_02_Tangent_SmoothUV.dds" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Samples/media/SSSSS/Map-COL.dds" "cbda47a1678ce70b6720856736100979d469e159")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SSSSS.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SSSSS.hpp
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/SSSSS.uiml
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
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Translucency.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Depth.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSS.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/Infinite-Level_02.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/Infinite-Level_02_Tangent_SmoothUV.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/Map-COL.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "B0CE0295-2665-474E-9DEA-9BC15AB32047")
ENDIF()

SETUP_SAMPLE(SSSSS)
