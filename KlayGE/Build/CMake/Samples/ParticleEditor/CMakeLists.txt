DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/ParticleEditor/fire.dds" "f5037acd6159f853ee7b8848034f8f03c55f7136")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/ParticleEditor/grass.dds" "99497d6d0617e14eecd8c25e16e2f31eb9011195")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/ParticleEditor/particle.dds" "a1377fd4f1435b8b15077c569afb4b088ab6b23b")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ParticleEditor/ParticleEditor.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ParticleEditor/ParticleEditor.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/ParticleEditor/ParticleEditor.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/ParticleEditor/ParticleEditor.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Depth.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SMAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/ToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Depth.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Particle.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PredefinedCBuffers.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SMAA.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/klayge_logo_sm.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAAAreaTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAASearchTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/ParticleEditor/fire.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/ParticleEditor/grass.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/ParticleEditor/particle.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/ParticleEditor/Fire.psml
	${KLAYGE_PROJECT_DIR}/Samples/media/ParticleEditor/ParticleEditor.kfx
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "262E8799-E0EE-437E-A60C-60802E739E56")
ENDIF()

SETUP_SAMPLE(ParticleEditor)
