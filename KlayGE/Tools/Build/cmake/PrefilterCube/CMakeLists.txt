SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/PrefilterCube/PrefilterCube.cpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Tools/media/PrefilterCube/PrefilterCube.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Tools/media/PrefilterCube/PrefilterCube.ppml
)

IF(MSVC)
	SET(EXTRA_LINKED_LIBRARIES "")
ELSE()
	SET(EXTRA_LINKED_LIBRARIES 
		${Boost_FILESYSTEM_LIBRARY})
ENDIF()

SETUP_TOOL(PrefilterCube)
