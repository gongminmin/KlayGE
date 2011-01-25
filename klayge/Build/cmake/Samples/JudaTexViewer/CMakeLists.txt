SET(SOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/JudaTexViewer/JudaTexViewer.cpp
)

SET(HEADER_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/JudaTexViewer/JudaTexViewer.hpp
)

SET(EFFECT_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/JudaTexViewer/JudaTexViewer.fxml
)

SET(UI_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/JudaTexViewer/JudaTexViewer.uiml
)

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(JudaTexViewer)
