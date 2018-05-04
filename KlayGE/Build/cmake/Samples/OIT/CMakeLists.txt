DOWNLOAD_FILE("KlayGE/Samples/media/OIT/robot_clean.meshml" "78cb4a721a19103c35be4d98843517f4b507a7a0" "bf27b026ac931cedfc4237dc453d916936d42e66")
DOWNLOAD_FILE("KlayGE/Samples/media/OIT/robot-clean_diffuse.dds" "78cb4a721a19103c35be4d98843517f4b507a7a0" "218c7dd270c056d5c4681c4cafbb00c9e8675260")
DOWNLOAD_FILE("KlayGE/Samples/media/OIT/robot-clean_normal.dds" "78cb4a721a19103c35be4d98843517f4b507a7a0" "131ef20c80fae0592ec768cb8f624a93c4b978e3")
DOWNLOAD_FILE("KlayGE/Samples/media/OIT/robot-clean_selfillumination.dds" "78cb4a721a19103c35be4d98843517f4b507a7a0" "cead9708e3350029fe597b13d26110adfab003bd")
DOWNLOAD_FILE("KlayGE/Samples/media/OIT/robot-clean_specular.dds" "78cb4a721a19103c35be4d98843517f4b507a7a0" "f3593162a63958ea2f11654fb03d24acc40178aa")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/OIT/OIT.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/OIT/OIT.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/AdaptiveTransparency.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/Blend.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/DepthPeeling.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/FragmentArray.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/FragmentList.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/NoOIT.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/PerPixelLinkedLists.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/RovAdaptiveTransparency.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/WeightedBlended.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/Blend.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/OIT.uiml
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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SkyBox.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SMAA.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAAAreaTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAASearchTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/uffizi_cross_filtered_c.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/uffizi_cross_filtered_y.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/AdaptiveTransparency.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/Blend.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/DepthPeeling.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/FragmentArray.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/FragmentList.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/NoOIT.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/PerPixelLinkedLists.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/RovAdaptiveTransparency.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/WeightedBlended.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot_clean.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot-clean_diffuse.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot-clean_normal.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot-clean_selfillumination.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot-clean_specular.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "28704547-BA31-4EA7-A1A3-1DE3EDA03BFD")
ENDIF()

SETUP_SAMPLE(OIT)
