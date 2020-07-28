DOWNLOAD_DEPENDENCY("KlayGE/Tutorials/media/DistanceMapping/diffuse.dds" "b68b543509b380eb1a65755107f0fcdcf8f4d9e6")
DOWNLOAD_DEPENDENCY("KlayGE/Tutorials/media/DistanceMapping/distance.dds" "60aaa3599993f631aa266b8ad8cf8ac3f8858090")
DOWNLOAD_DEPENDENCY("KlayGE/Tutorials/media/DistanceMapping/normal.dds" "3b6fc91238c77d42efa091e8ec047f97a33c3840")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/src/DistanceMapping/DistanceMapping.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/src/DistanceMapping/DistanceMapping.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/media/DistanceMapping/DistanceMapping.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/media/DistanceMapping/DistanceMapping.uiml
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
	${KLAYGE_PROJECT_DIR}/Tutorials/media/DistanceMapping/diffuse.dds
	${KLAYGE_PROJECT_DIR}/Tutorials/media/DistanceMapping/distance.dds
	${KLAYGE_PROJECT_DIR}/Tutorials/media/DistanceMapping/normal.dds
	${KLAYGE_PROJECT_DIR}/Tutorials/media/DistanceMapping/DistanceMapping.kfx
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "D31DB9CE-8757-4316-8FE2-239014DF5B97")
ENDIF()

SETUP_TUTORIAL(DistanceMapping)
