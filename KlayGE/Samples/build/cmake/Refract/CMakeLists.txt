SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Refract/Refract.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Refract/Refract.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Refract/Refract.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Refract/Refract.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(Refract)
