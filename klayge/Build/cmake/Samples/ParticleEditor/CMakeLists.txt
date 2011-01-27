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

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(ParticleEditor)
