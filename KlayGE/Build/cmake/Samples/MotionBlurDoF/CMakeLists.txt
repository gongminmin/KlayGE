SET(SOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/MotionBlurDoF/MotionBlurDoF.cpp
    ${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF_init.py
)

SET(HEADER_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/src/MotionBlurDoF/MotionBlurDoF.hpp
)

SET(EFFECT_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/DepthOfFieldPP.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF.fxml
    ${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurPP.fxml
)

SET(UI_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF.uiml
)

SET(RESOURCE_FILES
    ${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
    ${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SET(EXTRA_LINKED_LIBRARIES
    debug ${PYTHON_DEBUG_LIBRARIES} optimized ${PYTHON_LIBRARIES}
)

SETUP_SAMPLE(MotionBlurDoF)
