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

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/DepthOfField.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(MotionBlurDoF)

ADD_CUSTOM_COMMAND(TARGET MotionBlurDoF
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF_init.py ${KLAYGE_BIN_DIR})


INSTALL(FILES ${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF_init.py
	DESTINATION ${KLAYGE_BIN_DIR}
)
