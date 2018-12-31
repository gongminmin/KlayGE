FIND_PACKAGE(PythonInterp)

SET(EXE_NAME "GLCompatibility")
SET(KLAYGE_ROOT_DIR "${KLAYGE_ROOT_DIR}")
SET(MEDIA_DIR "${KLAYGE_PROJECT_DIR}/Tools/media/GLCompatibility/")
SET(PY_SRCS "\"GLCompatibility\"")
if(KLAYGE_PLATFORM_WINDOWS_STORE OR KLAYGE_PLATFORM_ANDROID)
	set(TARGET_DIR "\"\"")
else()
	set(TARGET_DIR "\"${KLAYGE_BIN_DIR}\"")
endif()
CONFIGURE_FILE(
	${KLAYGE_CMAKE_MODULE_DIR}/MakePyZip.py.in
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
