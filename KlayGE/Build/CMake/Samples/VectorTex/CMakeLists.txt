DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/VectorTex/Drawing.dds" "118c6b3bb377fa2b0ea6ab1dfe3fc5a776257647")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/VectorTex/VectorTex.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/VectorTex/VectorTex.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/VectorTex/VectorTex.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/VectorTex/VectorTex.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
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
	${KLAYGE_PROJECT_DIR}/Samples/media/VectorTex/Drawing.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/VectorTex/VectorTex.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/teapot.glb.model_bin
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "B0EB60AD-C50E-464D-9D86-5B00B5A16E54")
ENDIF()

SETUP_SAMPLE(VectorTex)
