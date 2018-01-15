SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/PrefilterCube/PrefilterCube.cpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Tools/media/PrefilterCube/PrefilterCube.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Tools/media/PrefilterCube/PrefilterCube.ppml
)

SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
	${KLAYGE_FILESYSTEM_LIBRARY})

SETUP_TOOL(PrefilterCube)
