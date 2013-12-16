# - Try to find OpenGL ES
# Once done this will define
#  
#  KLAYGE_GLES_FOUND       - system has OpenGL ES
#  KLAYGE_GLES_INCLUDE_DIR - Directory to the OpenGL ES SDK's include
#   

IF(NOT KLAYGE_GLES_INCLUDE_DIR)
	IF(WIN32)
		FIND_PATH(KLAYGE_GLES_INCLUDE_DIR KHR/khrplatform.h
			PATHS
				"$ENV{ProgramFiles}/NVIDIA Corporation/win_x86_es2emu/include"
				"$ENV{ProgramFiles(x86)}/NVIDIA Corporation/win_x86_es2emu/include"
				"$ENV{SystemDrive}/Imagination/PowerVR/GraphicsSDK/SDK_*/Builds/include"
				"$ENV{ProgramFiles}/ARM/Mali Developer Tools/Mali OpenGL ES Emulator*/include"
				"$ENV{ProgramFiles(x86)}/ARM/Mali Developer Tools/Mali OpenGL ES Emulator*/include"
			)
	ELSEIF(UNIX)
		FIND_PATH(KLAYGE_GLES_INCLUDE_DIR KHR/khrplatform.h
			PATHS
				/usr/local/include
				/usr/include
				/usr/local/X11R6/include
				/usr/X11R6/include
		)
	ENDIF()
ENDIF()

IF(KLAYGE_GLES_INCLUDE_DIR)
	SET(KLAYGE_GLES_FOUND TRUE)
	MESSAGE(STATUS "Found GLES SDK's include: ${KLAYGE_GLES_INCLUDE_DIR}")
ELSE()
	MESSAGE(STATUS "Could NOT find GLES SDK.")
ENDIF()
