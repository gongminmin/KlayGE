SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/OrderIndependentTransparency/OrderIndependentTransparency.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/OrderIndependentTransparency/OrderIndependentTransparency.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/AdaptiveTransparency.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/Blend.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/DepthPeeling.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/FragmentList.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/NoOIT.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/PerPixelLinkedLists.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/Blend.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/OrderIndependentTransparency.uiml
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
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/uffizi_cross_filtered_c.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/uffizi_cross_filtered_y.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/AdaptiveTransparency.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/Blend.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/DepthPeeling.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/FragmentList.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/NoOIT.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/PerPixelLinkedLists.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/robot_clean.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/robot-clean_diffuse.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/robot-clean_normal.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/robot-clean_selfillumination.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/robot-clean_specular.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "28704547-BA31-4EA7-A1A3-1DE3EDA03BFD")
ENDIF()

SETUP_SAMPLE(OrderIndependentTransparency)
