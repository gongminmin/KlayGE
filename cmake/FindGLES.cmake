# - Try to find OpenGL ES
# Once done this will define
#  
#  KLAYGE_GLES_FOUND       - system has OpenGL ES
#  KLAYGE_GLES_SDK_PATH    - Path to the OpenGL ES SDK
#   

IF(NOT KLAYGE_GLES_SDK_PATH)
	IF(WIN32)
		FIND_PATH(KLAYGE_GLES_SDK_PATH include/KHR/khrplatform.h
			PATHS
				"$ENV{ProgramFiles}/NVIDIA Corporation/win_x86_es2emu"
				"$ENV{ProgramFiles(x86)}/NVIDIA Corporation/win_x86_es2emu"
				"$ENV{SystemDrive}/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds"
			)
	ENDIF()
	IF(UNIX)
		FIND_PATH(KLAYGE_GLES_SDK_PATH include/KHR/khrplatform.h
			PATHS
				/usr/local
				/usr
				/usr/local/X11R6
				/usr/X11R6
		)
	ENDIF()
ENDIF()

IF(KLAYGE_GLES_SDK_PATH)
	SET(KLAYGE_GLES_FOUND TRUE)
	MESSAGE(STATUS "Found GLES SDK: ${KLAYGE_GLES_SDK_PATH}")
ENDIF()
