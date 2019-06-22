DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/DetailedSurface.jdt" "CD25A1C1095F05FECA3B518A49E867EF63F76F30")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/diffuse.dds" "95430e9a0e2100f0d4c721cc6a49b4a39df8de34")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/height.dds" "8a5475aa5e5e5c64dc06804c63325c82e6ea7866")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/na_length.dds" "cd9a5826b056bf8d37a9d3ecb210d17a8b1607b3")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/normal.dds" "226e1bff32280d0a598d537c9547940bd07fd506")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/occlusion.dds" "FC495C94B1A161EDC9DFE37E5E10E8C4E3E9B317")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DetailedSurface/DetailedSurface.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DetailedSurface/DetailedSurface.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/DetailedSurface.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/DetailedSurface.uiml
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
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/teapot.glb.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/DetailedSurface.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/DetailedSurface.jdt
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/na_length.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "BF549728-1DBA-48B7-B973-4CAB423B57F2")
ENDIF()

SETUP_SAMPLE(DetailedSurface)
