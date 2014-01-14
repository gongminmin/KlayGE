SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ModelViewer/Model.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/ModelViewer/ModelViewer.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ModelViewer/Model.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/ModelViewer/ModelViewer.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/ModelViewer/ModelViewer.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/ModelViewer/ModelViewer64.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/ModelViewer/ModelViewer128.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/ModelViewer/MVUtil.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/ModelViewer/ModelViewer.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(ModelViewer)
