SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/GLCompatibility/GLCompatibility.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/GLCompatibility/GLCompatibility.py
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

SET(EXTRA_LINKED_LIBRARIES
    debug python32_d optimized python32
)

SETUP_TOOL(GLCompatibility)

INSTALL(FILES ${KLAYGE_PROJECT_DIR}/Tools/src/GLCompatibility/GLCompatibility.py
	DESTINATION ${KLAYGE_BIN_DIR}
)
