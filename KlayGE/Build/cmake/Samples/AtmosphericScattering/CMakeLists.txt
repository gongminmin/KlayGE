DOWNLOAD_FILE("KlayGE/Samples/media/AtmosphericScattering/earth_color.dds" "cbda47a1678ce70b6720856736100979d469e159" "d7403dbcf242b65198c4d96816a655bee3f07622")
DOWNLOAD_FILE("KlayGE/Samples/media/AtmosphericScattering/geosphere.7z" "cbda47a1678ce70b6720856736100979d469e159" "7caf25a9bc2fa910faec90bbfc53e8df7d1d8c95")
DOWNLOAD_FILE("KlayGE/Samples/media/AtmosphericScattering/ocean_mask.dds" "cbda47a1678ce70b6720856736100979d469e159" "e006d8f16187e53c34f49c29d092a8f53f74824b")

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
	${KLAYGE_PROJECT_DIR}/media/Models/directional_light_proxy.meshml.model_bin
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
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/AtmosphericScattering.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/earth_color.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/geosphere.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/ocean_mask.dds
)
	
IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "be19c46c-d114-4f14-b908-71599c66f7c7")
ENDIF()

SETUP_SAMPLE(AtmosphericScattering)
