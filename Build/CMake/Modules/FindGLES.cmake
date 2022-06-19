# - Try to find OpenGL ES
# Once done this will define
#  
#  KLAYGE_GLES_FOUND       - system has OpenGL ES
#  KLAYGE_GLES_INCLUDE_DIR - Directory to the OpenGL ES SDK's include
#   

SET(ProgramFilesX86 "ProgramFiles(x86)")

IF(NOT KLAYGE_GLES_INCLUDE_DIR)
	IF(WIN32)
		FIND_PATH(KLAYGE_GLES_INCLUDE_DIR GLES3/gl3platform.h
			PATHS
				"$ENV{SystemDrive}/AdrenoSDK/Development/Inc"
				"$ENV{SystemDrive}/Imagination/PowerVR/GraphicsSDK/SDK_*/Builds/include"
				"$ENV{SystemDrive}/Imagination/PowerVR_Graphics/PowerVR_SDK/SDK_*/Builds/include"
				"$ENV{KHRONOS_HEADERS}"
				"$ENV{ProgramFiles}/ARM/Mali Developer Tools/Mali OpenGL ES Emulator*/include"
				"$ENV{${ProgramFilesX86}}/ARM/Mali Developer Tools/Mali OpenGL ES Emulator*/include"
			)
	ELSEIF(ANDROID)
		FIND_PATH(KLAYGE_GLES_INCLUDE_DIR GLES3/gl3platform.h
			PATHS
				"${CMAKE_SYSROOT}/usr/include"
				"${ANDROID_NDK}/platforms/${ANDROID_PLATFORM}/arch-${ANDROID_ARCH_NAME}/usr/include"
		)
	ELSEIF(IOS)
		FIND_PATH(KLAYGE_GLES_INCLUDE_DIR EAGL.h
			PATHS
				/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/System/Library/Frameworks/OpenGLES.framework/Headers
		)
	ELSEIF(APPLE)
		FIND_PATH(KLAYGE_GLES_INCLUDE_DIR GLES3/gl3platform.h
			PATHS
				/Users/Shared/Imagination/PowerVR_Graphics/PowerVR_SDK/SDK_3.4/Builds/Include
		)
	ELSEIF(UNIX)
		FIND_PATH(KLAYGE_GLES_INCLUDE_DIR GLES3/gl3platform.h
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
