FIND_PACKAGE(PythonInterp)

SET(EXE_NAME "GLESCompatibility")
SET(KLAYGE_ROOT_DIR "${KLAYGE_ROOT_DIR}")
SET(MEDIA_DIR "${KLAYGE_PROJECT_DIR}/Tools/media/GLESCompatibility/")
SET(PY_SRCS "\"GLESCompatibility\"")
CONFIGURE_FILE(
	${KLAYGE_ROOT_DIR}/cmake/MakePyZip.py.in
	${KLAYGE_PROJECT_DIR}/Tools/media/GLESCompatibility/MakePyZip.py
	@ONLY
)

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/GLESCompatibility/GLESCompatibility.cpp
	${KLAYGE_PROJECT_DIR}/Tools/media/GLESCompatibility/GLESCompatibility.py
)

SETUP_TOOL(GLESCompatibility)

ADD_CUSTOM_COMMAND(TARGET GLESCompatibility
	PRE_BUILD
	COMMAND "${PYTHON_EXECUTABLE}" "${KLAYGE_PROJECT_DIR}/Tools/media/GLESCompatibility/MakePyZip.py")

ADD_CUSTOM_COMMAND(TARGET GLESCompatibility
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${KLAYGE_PROJECT_DIR}/Tools/media/GLESCompatibility/GLESCompatibilityPy.zip ${KLAYGE_BIN_DIR})

INSTALL(FILES ${KLAYGE_PROJECT_DIR}/Tools/media/GLESCompatibility/GLESCompatibilityPy.zip
	DESTINATION ${KLAYGE_BIN_DIR}
)
