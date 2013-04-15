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

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(GPUParticleSystem)
