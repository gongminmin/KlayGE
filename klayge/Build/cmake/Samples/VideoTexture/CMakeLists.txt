SET(SOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/VideoTexture/VideoTexture.cpp
)

SET(HEADER_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/VideoTexture/VideoTexture.hpp
)

SET(EFFECT_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/VideoTexture/VideoTexture.fxml
)

SET(UI_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/VideoTexture/VideoTexture.uiml
)

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(VideoTexture)
