SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/Ocean.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/OceanSimulator.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/ProceduralTerrain.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/Ocean.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/OceanSimulator.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/ProceduralTerrain.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/Ocean.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/OceanSimulator.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/ProceduralTerrain.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/ProceduralTerrain.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/Ocean.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(Ocean)
