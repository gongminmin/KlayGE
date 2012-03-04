SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Fractal/Fractal.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Fractal/Fractal.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Fractal/Fractal.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Fractal/Fractal.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(Fractal)
