SET(SOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/DeferredRendering/DeferredRendering.cpp
)

SET(HEADER_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/DeferredRendering/DeferredRendering.hpp
)

SET(EFFECT_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/DeferredRendering/DeferredRenderingDebug.fxml
)

SET(UI_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/DeferredRendering/DeferredRendering.uiml
)

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(DeferredRendering)
