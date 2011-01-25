SET(SOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/FFT.cpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/Ocean.cpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/OceanSimulator.cpp
)

SET(HEADER_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/FFT.hpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/Ocean.hpp
    ${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/OceanSimulator.hpp
)

SET(EFFECT_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/FFT.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/Ocean.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/OceanSimulator.fxml
)

SET(UI_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/Ocean.uiml
)

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(Ocean)
