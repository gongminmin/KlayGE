SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SSSBlur.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SSSSS.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SubsurfaceMesh.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SSSBlur.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SSSSS.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SubsurfaceMesh.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/SubsurfaceMesh.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/SSS.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/Translucency.ppml
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SETUP_SAMPLE(SSSSS)
