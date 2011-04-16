SET(SOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/PNTriangles/PNTriangles.cpp
)

SET(HEADER_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/PNTriangles/PNTriangles.hpp
)

SET(EFFECT_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/PNTriangles/PNTriangles.fxml
)

SET(UI_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/PNTriangles/PNTriangles.uiml
)

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(PNTriangles)
