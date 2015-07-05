DOWNLOAD_FILE("KlayGE/Samples/media/ShadowCubeMap/lamp.dds" "cbda47a1678ce70b6720856736100979d469e159" "454abe1e0e71f7c56f97822bae184a9661f20cb8")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ShadowCubeMap/ShadowCubeMap.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ShadowCubeMap/ShadowCubeMap.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/ShadowCubeMap/ShadowCubeMap.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/ShadowCubeMap/ShadowCubeMap.uiml
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
	${KLAYGE_PROJECT_DIR}/Samples/media/ShadowCubeMap/lamp.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/ShadowCubeMap/ShadowCubeMap.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/teapot.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci1.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci2.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci3.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci4.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci5.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci6.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci8.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci9.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "60321328-2F5A-4546-B410-B952F924DABE")
ENDIF()

SETUP_SAMPLE(ShadowCubeMap)
