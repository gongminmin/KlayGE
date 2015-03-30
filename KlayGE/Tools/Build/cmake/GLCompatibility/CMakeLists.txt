FIND_PACKAGE(PythonInterp)

SET(EXE_NAME "GLCompatibility")
SET(KLAYGE_ROOT_DIR "${KLAYGE_ROOT_DIR}")
SET(MEDIA_DIR "${KLAYGE_PROJECT_DIR}/Tools/media/GLCompatibility/")
SET(PY_SRCS "\"GLCompatibility\"")
CONFIGURE_FILE(
	${KLAYGE_ROOT_DIR}/cmake/MakePyZip.py.in
	${KLAYGE_PROJECT_DIR}/Tools/media/GLCompatibility/MakePyZip.py
	@ONLY
)

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/GLCompatibility/GLCompatibility.cpp
	${KLAYGE_PROJECT_DIR}/Tools/media/GLCompatibility/GLCompatibility.py
)

SETUP_TOOL(GLCompatibility)

ADD_CUSTOM_COMMAND(TARGET GLCompatibility
	PRE_BUILD
	COMMAND "${PYTHON_EXECUTABLE}" "${KLAYGE_PROJECT_DIR}/Tools/media/GLCompatibility/MakePyZip.py")

ADD_CUSTOM_COMMAND(TARGET GLCompatibility
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${KLAYGE_PROJECT_DIR}/Tools/media/GLCompatibility/GLCompatibilityPy.zip ${KLAYGE_BIN_DIR})

INSTALL(FILES ${KLAYGE_PROJECT_DIR}/Tools/media/GLCompatibility/GLCompatibilityPy.zip
	DESTINATION ${KLAYGE_BIN_DIR}
)
