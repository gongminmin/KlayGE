DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/OIT/robot_clean.glb" "46CC7A2C06A6CFFF529744CC09E21C946DC2890A")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/OIT/robot-clean_diffuse.jpg" "1C29E58BC8C9AA64D976CAB5464BF6CDE352D68E")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/OIT/robot-clean_normal.jpg" "4E90E7073DAEC57536A03D17B10DE2FC9FB0345D")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/OIT/robot-clean_selfillumination.jpg" "097B975852004CA2B0BCB8C35E9D78FD80335B10")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/OIT/robot-clean_specular.jpg" "77F47F64AD4E1ABC87E8AAEFF0701DD4409E4A8D")

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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PredefinedCBuffers.kfx
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
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot_clean.glb.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot-clean_diffuse.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot-clean_normal.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot-clean_selfillumination.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/OIT/robot-clean_specular.jpg.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "28704547-BA31-4EA7-A1A3-1DE3EDA03BFD")
ENDIF()

SETUP_SAMPLE(OIT)
