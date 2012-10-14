SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/src/VertexDisplacement/VertexDisplacement.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/src/VertexDisplacement/VertexDisplacement.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/media/VertexDisplacement/VertexDisplacement.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Tutorials/media/VertexDisplacement/VertexDisplacement.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_TUTORIAL(VertexDisplacement)
