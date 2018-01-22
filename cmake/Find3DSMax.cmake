# - Find 3DSMax
# Find the path of 3DSMax, and its SDK
#
#  KLAYGE_3DSMAX_FOUND       - True if 3DSMax found.
#  KLAYGE_3DSMAX_SDK_FOUND    - True if 3DSMax SDK found.
#  KLAYGE_3DSMAX_PATH     - Location of 3DSMax.
#  KLAYGE_3DSMAX_SDK_PATH - Location of 3DSMax SDK.
#  KLAYGE_3DSMAX_UNICODE  - True if the 3DSMax is unicode version (>=2013).
#

IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0053 OLD)
ENDIF()

IF(NOT KLAYGE_3DSMAX_PATH)
	FIND_PATH(KLAYGE_3DSMAX_PATH 3dsmax.exe
		PATHS
		"$ENV{ADSK_3DSMAX_2014}"
		"$ENV{ADSK_3DSMAX_2013}"
		"$ENV{ADSK_3DSMAX_2012}"
		"$ENV{3DSMAX_2011_PATH}"
		"$ENV{ProgramFiles}/Autodesk/3ds Max 2010"
		"$ENV{ProgramFiles(x86)}/Autodesk/3ds Max 2010"
		"$ENV{ProgramFiles}/Autodesk/3ds Max 2009 SDK"
		"$ENV{ProgramFiles(x86)}/Autodesk/3ds Max 2009"
		"$ENV{ProgramFiles}/Autodesk/3ds Max 2008"
		"$ENV{ProgramFiles(x86)}/Autodesk/3ds Max 2008"
		"$ENV{ProgramFiles}/Autodesk/3ds Max 9"
		"$ENV{ProgramFiles(x86)}/Autodesk/3ds Max 9"
	)
ENDIF()

IF(NOT KLAYGE_3DSMAX_SDK_PATH)
	FIND_PATH(KLAYGE_3DSMAX_SDK_PATH include/max.h 
		PATHS
		"$ENV{ADSK_3DSMAX_SDK_2014}/maxsdk"
		"$ENV{ADSK_3DSMAX_SDK_2013}/maxsdk"
		"$ENV{ADSK_3DSMAX_SDK_2012}/maxsdk"
		"$ENV{3DSMAX_2011_SDK_PATH}/maxsdk"
		"$ENV{ProgramFiles}/Autodesk/3ds Max 2010 SDK/maxsdk"
		"$ENV{ProgramFiles(x86)}/Autodesk/3ds Max 2010 SDK/maxsdk"
		"$ENV{ProgramFiles}/Autodesk/3ds Max 2009 SDK/maxsdk"
		"$ENV{ProgramFiles(x86)}/Autodesk/3ds Max 2009 SDK/maxsdk"
		"$ENV{ProgramFiles}/Autodesk/3ds Max 2008 SDK/maxsdk"
		"$ENV{ProgramFiles(x86)}/Autodesk/3ds Max 2008 SDK/maxsdk"
		"$ENV{ProgramFiles}/Autodesk/3ds Max 9 SDK/maxsdk"
		"$ENV{ProgramFiles(x86)}/Autodesk/3ds Max 9 SDK/maxsdk"
	)
ENDIF()

IF(KLAYGE_3DSMAX_PATH)
	SET(KLAYGE_3DSMAX_FOUND TRUE)
	MESSAGE(STATUS "Found 3DSMax: ${KLAYGE_3DSMAX_PATH}")
ELSE()
	MESSAGE(STATUS "Could NOT find 3DSMax.")
ENDIF()

IF(KLAYGE_3DSMAX_SDK_PATH)
	SET(KLAYGE_3DSMAX_SDK_FOUND TRUE)
	MESSAGE(STATUS "Found 3DSMax SDK: ${KLAYGE_3DSMAX_SDK_PATH}")

	SET(KLAYGE_3DSMAX_INCLUDE_DIR "${KLAYGE_3DSMAX_SDK_PATH}/maxsdk/include")
	SET(KLAYGE_3DSMAX_LIBRARY_DIR "${KLAYGE_3DSMAX_SDK_PATH}/maxsdk/${KLAYGE_ARCH_NAME}/lib")
	SET(KLAYGE_3DSMAX_CORE_LIBRARY "${KLAYGE_3DSMAX_LIBRARY_DIR}/core.lib")
	SET(KLAYGE_3DSMAX_GEOM_LIBRARY "${KLAYGE_3DSMAX_LIBRARY_DIR}/geom.lib")
	SET(KLAYGE_3DSMAX_MAXUTIL_LIBRARY "${KLAYGE_3DSMAX_LIBRARY_DIR}/maxutil.lib")
	SET(KLAYGE_3DSMAX_MESH_LIBRARY "${KLAYGE_3DSMAX_LIBRARY_DIR}/mesh.lib")
ELSE()
	MESSAGE(STATUS "Could NOT find 3DSMax SDK.")
ENDIF()

IF(NOT KLAYGE_3DSMAX_UNICODE)
	IF((EXISTS "$ENV{ADSK_3DSMAX_2013}") OR (EXISTS "$ENV{ADSK_3DSMAX_2014}"))
		SET(KLAYGE_3DSMAX_UNICODE TRUE)
	ELSE()
		SET(KLAYGE_3DSMAX_UNICODE FALSE)
	ENDIF()
ENDIF()
MESSAGE(STATUS "3DSMax in unicode: ${KLAYGE_3DSMAX_UNICODE}")
		