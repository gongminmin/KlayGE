SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DepthPeeling/DepthPeeling.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DepthPeeling/DepthPeeling.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/DepthPeeling/DepthPeeling.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/DepthPeeling/Blend.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/DepthPeeling/DepthPeeling.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(DepthPeeling)
