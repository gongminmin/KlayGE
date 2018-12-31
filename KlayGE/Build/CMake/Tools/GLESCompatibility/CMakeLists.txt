FIND_PACKAGE(PythonInterp)

SET(EXE_NAME "GLESCompatibility")
SET(KLAYGE_ROOT_DIR "${KLAYGE_ROOT_DIR}")
SET(MEDIA_DIR "${KLAYGE_PROJECT_DIR}/Tools/media/GLESCompatibility/")
SET(PY_SRCS "\"GLESCompatibility\"")
if(KLAYGE_PLATFORM_WINDOWS_STORE OR KLAYGE_PLATFORM_ANDROID)
	set(TARGET_DIR "\"\"")
else()
	set(TARGET_DIR "\"${KLAYGE_BIN_DIR}\"")
endif()
CONFIGURE_FILE(
	${KLAYGE_CMAKE_MODULE_DIR}/MakePyZip.py.in
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
