DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/AtmosphericScattering/earth_day.dds" "23347ac909cbaf9aed03e8228d4eb64639e785f7")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/AtmosphericScattering/earth_night.dds" "1d44faa87e412a4d675aa4de5d4a1cf4bb0b68e7")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/AtmosphericScattering/geosphere.glb" "189E62FD09C6CE0C44E7483B900095DDD0A415F9")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/AtmosphericScattering/ocean_mask.dds" "e006d8f16187e53c34f49c29d092a8f53f74824b")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/AtmosphericScattering/AtmosphericScattering.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/AtmosphericScattering/AtmosphericScattering.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/AtmosphericScattering.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/AtmosphericScattering.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/Models/DirectionalLightProxy.glb.model_bin
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
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/AtmosphericScattering.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/earth_day.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/earth_night.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/geosphere.glb.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/ocean_mask.dds
)
	
IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "be19c46c-d114-4f14-b908-71599c66f7c7")
ENDIF()

SETUP_SAMPLE(AtmosphericScattering)
