SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/GPUParticleSystem/GPUParticleSystem.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/GPUParticleSystem/GPUParticleSystem.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/GPUParticleSystem.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/Terrain.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/Blend.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/GPUParticleSystem.uiml
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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/GPUParticleSystem.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/grass.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/particle.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/Terrain.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/terrain_height.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/GPUParticleSystem/terrain_normal.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "181315CA-EE52-48FE-8F53-A5BFCA013F3F")
ENDIF()

SETUP_SAMPLE(GPUParticleSystem)
