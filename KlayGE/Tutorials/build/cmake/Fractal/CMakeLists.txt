SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/src/Fractal/Fractal.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/src/Fractal/Fractal.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/media/Fractal/Fractal.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/media/Fractal/Fractal.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_TUTORIAL(Fractal)
