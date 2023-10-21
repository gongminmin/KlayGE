DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/SubSurface/Dragon.glb" "52F7E1993268B2DFE29181E4E3CB1B67A6B022AD")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/SubSurface/Eye_022green.jpg" "726200238A2E325BF84220A31C84F9AAB7B8CC5F")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/SubSurface/REDRUST2.jpg" "3588B4C50C8CC92945431C5C4845568FB4A33654")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/SubSurface/SCALE_B_W.png" "981BA9B95513C5D4F8316C70DE164CB8BB3DBEBF")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/SubSurface/Stucco8.jpg" "F41675E07DF2151D1CE96D3134410BD3FD98B672")

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
	${KLAYGE_PROJECT_DIR}/media/Models/PointLightProxy.glb.model_bin
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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PredefinedCBuffers.kfx
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
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/Dragon.glb.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/Eye_022green.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/REDRUST2.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/SCALE_B_W.png.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/Stucco8.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/SubSurface.kfx
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "C5FF3F49-5A2D-430A-AB71-1919D1A50215")
ENDIF()

SETUP_SAMPLE(SubSurface)
