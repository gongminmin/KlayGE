SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/GLESCompatibility/GLESCompatibility.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/GLESCompatibility/GLESCompatibility.py
)

IF(CMAKE_GENERATOR MATCHES "Win64")
	SET(EXTRA_LINKED_DIRS
		${KLAYGE_PROJECT_DIR}/../External/Python/libs/amd64
	)
ELSE()
	SET(EXTRA_LINKED_DIRS
		${KLAYGE_PROJECT_DIR}/../External/Python/libs
	)
ENDIF()

SETUP_TOOL(GLESCompatibility)

INSTALL(FILES ${KLAYGE_PROJECT_DIR}/Tools/src/GLESCompatibility/GLESCompatibility.py
	DESTINATION ${KLAYGE_BIN_DIR}
)
