# - Find Maya
# Find the path of Maya, and its SDK
#
# Defines following variables:
#  KLAYGE_MAYA_FOUND - True if Maya found.
#  KLAYGE_MAYA_PATH  - Location of Maya.
#

IF(NOT KLAYGE_MAYA_PATH)
	FIND_PATH(KLAYGE_MAYA_PATH bin/maya.exe
		PATHS
		"$ENV{MAYA_ROOT}"
		"$ENV{ProgramFiles}/Maya*/"
		"$ENV{ProgramFiles(x86)}/Maya*/"
		"$ENV{ProgramFiles}/Autodesk/Maya*/"
		"$ENV{ProgramFiles(x86)}/Autodesk/Maya*/"
	)
ENDIF()

IF(KLAYGE_MAYA_PATH)
	SET(KLAYGE_MAYA_FOUND TRUE)
	MESSAGE(STATUS "Found Maya: ${KLAYGE_MAYA_PATH}")
ELSE()
	MESSAGE(STATUS "Could NOT find Maya.")
ENDIF()
