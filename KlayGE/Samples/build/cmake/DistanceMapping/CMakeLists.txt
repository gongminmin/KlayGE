SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DistanceMapping/DistanceMapping.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DistanceMapping/DistanceMapping.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/DistanceMapping/DistanceMapping.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/DistanceMapping/DistanceMapping.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(DistanceMapping)
