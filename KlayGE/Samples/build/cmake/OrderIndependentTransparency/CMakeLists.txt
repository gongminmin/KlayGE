SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/OrderIndependentTransparency/OrderIndependentTransparency.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/OrderIndependentTransparency/OrderIndependentTransparency.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/Blend.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/DepthPeeling.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/NoOIT.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/PerPixelLinkedList.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/Blend.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/OrderIndependentTransparency/OrderIndependentTransparency.uiml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(OrderIndependentTransparency)
