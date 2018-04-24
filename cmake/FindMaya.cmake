# - Find Maya
# Find the path of Maya, and its SDK
#
# Defines following variables:
#  KLAYGE_MAYA_FOUND - True if Maya found.
#  KLAYGE_MAYA_PATH  - Location of Maya.
#

SET(ProgramFilesX86 "ProgramFiles(x86)")

IF(NOT KLAYGE_MAYA_PATH)
	FIND_PATH(KLAYGE_MAYA_PATH bin/maya.exe
		PATHS
		"$ENV{MAYA_ROOT}"
		"$ENV{ProgramFiles}/Maya*/"
		"$ENV{${ProgramFilesX86}}/Maya*/"
		"$ENV{ProgramFiles}/Autodesk/Maya*/"
		"$ENV{${ProgramFilesX86}}/Autodesk/Maya*/"
	)
ENDIF()

IF(KLAYGE_MAYA_PATH)
	SET(KLAYGE_MAYA_FOUND TRUE)
	MESSAGE(STATUS "Found Maya: ${KLAYGE_MAYA_PATH}")

	SET(KLAYGE_MAYA_INCLUDE_DIR "${KLAYGE_MAYA_PATH}/include")
	SET(KLAYGE_MAYA_LIBRARY_DIR "${KLAYGE_MAYA_PATH}/lib")
	SET(KLAYGE_MAYA_FOUNDATION_LIBRARY "${KLAYGE_MAYA_LIBRARY_DIR}/Foundation.lib")
	SET(KLAYGE_MAYA_OPENMAYA_LIBRARY "${KLAYGE_MAYA_LIBRARY_DIR}/OpenMaya.lib")
	SET(KLAYGE_MAYA_OPENMAYAANIM_LIBRARY "${KLAYGE_MAYA_LIBRARY_DIR}/OpenMayaAnim.lib")
ELSE()
	MESSAGE(STATUS "Could NOT find Maya.")
ENDIF()
