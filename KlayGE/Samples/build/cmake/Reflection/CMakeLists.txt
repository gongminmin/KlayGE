SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Reflection/Reflection.cpp
	${KLAYGE_PROJECT_DIR}/Samples/media/Reflection/Reflection.cam_path
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Reflection/Reflection.hpp
)


SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Reflection/Reflection.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Reflection/Reflection.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(Reflection)
