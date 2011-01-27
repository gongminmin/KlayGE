SET(SOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/PostProcessing.cpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/AsciiArtsPP.cpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/CartoonPP.cpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/NightVisionPP.cpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/PostProcessing.cpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/TilingPP.cpp
)

SET(HEADER_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/PostProcessing.hpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/AsciiArtsPP.hpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/CartoonPP.hpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/NightVisionPP.hpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/PostProcessing.hpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/PostProcessing/TilingPP.hpp
)

SET(EFFECT_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/PostProcessing/AdaptiveAntiAliasPP.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/PostProcessing/AsciiArtsPP.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/PostProcessing/CartoonPP.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/PostProcessing/NightVisionPP.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/PostProcessing/OldFashion.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/PostProcessing/PostProcessing.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/PostProcessing/TilingPP.fxml
)

SET(POST_PROCESSORS
    ${KLAYGE_PROJECT_DIR}/Samples/media/PostProcessing/OldFashion.ppml
)

SET(UI_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/PostProcessing/PostProcessing.uiml
)

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(PostProcessing)
