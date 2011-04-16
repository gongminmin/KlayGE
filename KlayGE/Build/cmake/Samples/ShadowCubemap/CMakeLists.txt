SET(SOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/ShadowCubemap/ShadowCubemap.cpp
)

SET(HEADER_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/ShadowCubemap/ShadowCubemap.hpp
)

SET(EFFECT_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/ShadowCubemap/ShadowCubemap.fxml
)

SET(UI_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/ShadowCubemap/ShadowCubemap.uiml
)

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(ShadowCubemap)
