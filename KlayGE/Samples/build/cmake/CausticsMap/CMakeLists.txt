SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/CausticsMap/CausticsMap.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/CausticsMap/CausticsMap.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Caustics.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Scene.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/CausticsMap/Caustics.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(CausticsMap)
