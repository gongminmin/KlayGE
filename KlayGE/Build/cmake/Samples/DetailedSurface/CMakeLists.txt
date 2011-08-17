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

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(DetailedSurface)
