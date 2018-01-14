SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/PrefilterCube/PrefilterCube.cpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Tools/media/PrefilterCube/PrefilterCube.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Tools/media/PrefilterCube/PrefilterCube.ppml
)

IF(NOT KLAYGE_COMPILER_MSVC)
	IF(KLAYGE_COMPILER_GCC)
		SET(FS_LIB "stdc++fs")
	ELSE()
		SET(FS_LIB ${Boost_FILESYSTEM_LIBRARY})
	ENDIF()
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
		${FS_LIB})
ENDIF()

SETUP_TOOL(PrefilterCube)
