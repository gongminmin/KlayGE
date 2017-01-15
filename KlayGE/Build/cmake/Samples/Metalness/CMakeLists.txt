DOWNLOAD_FILE("KlayGE/Samples/media/Metalness/helmet_armet_2.meshml" "dc5b074b1a178a2f1755e2eb47c821913dd293e2" "29a8d99c824be86098ee830d9c8dcfc32705cfde")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Metalness/Metalness.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Metalness/Metalness.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Metalness/Metalness.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Metalness/Metalness.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SkyBox.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/rnl_cross_filtered_c.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/rnl_cross_filtered_y.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/sphere_high.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Metalness/helmet_armet_2.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Metalness/Metalness.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Metalness/Metalness.uiml
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "411CDD88-8CE6-4601-9C66-1E22B32CF8E6")
ENDIF()

SETUP_SAMPLE(Metalness)
