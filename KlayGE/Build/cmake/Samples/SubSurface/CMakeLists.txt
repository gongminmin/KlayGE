DOWNLOAD_FILE("KlayGE/Samples/media/SubSurface/Dragon.7z" "cbda47a1678ce70b6720856736100979d469e159" "61d3b946e7e092373727896be0caebda289b2e24")
DOWNLOAD_FILE("KlayGE/Samples/media/SubSurface/Eye_022green.dds" "cbda47a1678ce70b6720856736100979d469e159" "d8245f1bda730b9b3168a27b7517f8d5f108b5c1")
DOWNLOAD_FILE("KlayGE/Samples/media/SubSurface/REDRUST2.dds" "cbda47a1678ce70b6720856736100979d469e159" "65fc44d0bda4eca4b66172f9e9fb35e51b81b4ac")
DOWNLOAD_FILE("KlayGE/Samples/media/SubSurface/SCALE_B_W.dds" "cbda47a1678ce70b6720856736100979d469e159" "1a1d1b7d156225c3f23865f89bea6667ff3a21ad")
DOWNLOAD_FILE("KlayGE/Samples/media/SubSurface/Stucco8.dds" "cbda47a1678ce70b6720856736100979d469e159" "ce96a9c35bc84f02a3a2a07214b127dd375a748d")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SubSurface/Model.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/SubSurface/SubSurface.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SubSurface/Model.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/SubSurface/SubSurface.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/SubSurface.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/SubSurface.uiml
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
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/Dragon.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/Eye_022green.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/REDRUST2.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/SCALE_B_W.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/Stucco8.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/SubSurface.kfx
)

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "C5FF3F49-5A2D-430A-AB71-1919D1A50215")
ENDIF()

SETUP_SAMPLE(SubSurface)
