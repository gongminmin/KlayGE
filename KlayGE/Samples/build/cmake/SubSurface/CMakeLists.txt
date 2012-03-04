SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SubSurface/Model.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/SubSurface/SubSurface.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SubSurface/Model.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/SubSurface/SubSurface.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/SubSurface.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/SubSurface/SubSurface.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(SubSurface)
