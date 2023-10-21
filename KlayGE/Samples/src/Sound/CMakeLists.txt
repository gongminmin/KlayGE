DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Sound/Carl_Douglas_-_Kung_Fu_Fighting.ogg" "3ffdfd65a6c3ce480f129f850e10690a22924cb0")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Sound/Cash_register.ogg" "c789a1a76e78af18370df374f4274a55e6fc9d5c")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Sound/Metallica_-_Enter_Sandman.ogg" "51e40e4911580e3c82dbd8a507f82260f2a080f0")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Sound/Sound.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Sound/Sound.hpp
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Sound/Sound.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Sound/Carl_Douglas_-_Kung_Fu_Fighting.ogg
	${KLAYGE_PROJECT_DIR}/Samples/media/Sound/Cash_register.ogg
	${KLAYGE_PROJECT_DIR}/Samples/media/Sound/Metallica_-_Enter_Sandman.ogg
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
	${KLAYGE_PROJECT_DIR}/Samples/media/Sound/Carl_Douglas_-_Kung_Fu_Fighting.ogg
	${KLAYGE_PROJECT_DIR}/Samples/media/Sound/Cash_register.ogg
	${KLAYGE_PROJECT_DIR}/Samples/media/Sound/Metallica_-_Enter_Sandman.ogg
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "3BE8502D-CF73-434D-A968-3E46F3B55436")
ENDIF()

SETUP_SAMPLE(Sound)
