if(KLAYGE_PLATFORM_WINDOWS_STORE)
	return()
endif()

SET(GLLOADER_PROJECT_DIR "${KLAYGE_ROOT_DIR}/glloader")

SET(CodeGenOutput "${GLLOADER_PROJECT_DIR}/include/glloader/glloader_gl.h")
ADD_CUSTOM_TARGET(autogen ALL
	COMMAND "${Python3_EXECUTABLE}" "autogen.py" "-q"
	WORKING_DIRECTORY "${GLLOADER_PROJECT_DIR}")
SET_TARGET_PROPERTIES(autogen PROPERTIES FOLDER "glloader")

MACRO(SETUP_GLLOADER LIB_NAME)
	SET(GLLOADER_SOURCE_FILES
		${GLLOADER_PROJECT_DIR}/src/glloader_egl.c
		${GLLOADER_PROJECT_DIR}/src/glloader_gl.c
		${GLLOADER_PROJECT_DIR}/src/glloader_gles.c
		${GLLOADER_PROJECT_DIR}/src/glloader_glx.c
		${GLLOADER_PROJECT_DIR}/src/glloader_wgl.c
		${GLLOADER_PROJECT_DIR}/src/utils.cpp
	)
	SET(GLLOADER_INCLUDE_FILES
		${GLLOADER_PROJECT_DIR}/include/glloader/glloader.h
		${GLLOADER_PROJECT_DIR}/include/glloader/glloader_egl.h
		${GLLOADER_PROJECT_DIR}/include/glloader/glloader_gl.h
		${GLLOADER_PROJECT_DIR}/include/glloader/glloader_gles.h
		${GLLOADER_PROJECT_DIR}/include/glloader/glloader_glx.h
		${GLLOADER_PROJECT_DIR}/include/glloader/glloader_wgl.h
	)
	SET(GLLOADER_HEADER_FILES
		${GLLOADER_PROJECT_DIR}/src/utils.h
	)
	SOURCE_GROUP("Source Files" FILES ${GLLOADER_SOURCE_FILES})
	SOURCE_GROUP("Header Files" FILES ${GLLOADER_HEADER_FILES})
	SOURCE_GROUP("Include Files" FILES ${GLLOADER_INCLUDE_FILES})

	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/src/glloader_egl.c PROPERTIES GENERATED 1)
	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/src/glloader_gl.c PROPERTIES GENERATED 1)
	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/src/glloader_gles.c PROPERTIES GENERATED 1)
	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/src/glloader_glx.c PROPERTIES GENERATED 1)
	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/src/glloader_wgl.c PROPERTIES GENERATED 1)
	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/include/glloader/glloader_egl.h PROPERTIES GENERATED 1)
	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/include/glloader/glloader_gl.h PROPERTIES GENERATED 1)
	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/include/glloader/glloader_gles.h PROPERTIES GENERATED 1)
	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/include/glloader/glloader_glx.h PROPERTIES GENERATED 1)
	SET_SOURCE_FILES_PROPERTIES(${GLLOADER_PROJECT_DIR}/include/glloader/glloader_wgl.h PROPERTIES GENERATED 1)

	ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
		${GLLOADER_SOURCE_FILES} ${GLLOADER_HEADER_FILES} ${GLLOADER_INCLUDE_FILES}
	)

	target_include_directories(${LIB_NAME}
		PUBLIC
			${GLLOADER_PROJECT_DIR}/include
	)

	if(KLAYGE_COMPILER_CLANGCL AND (KLAYGE_COMPILER_VERSION GREATER_EQUAL 160))
		target_compile_options(${LIB_NAME}
			PRIVATE
				-Wno-reserved-identifier
		)
	endif()

	SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
		OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
		FOLDER "glloader"
	)
	if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
		set_target_properties(${LIB_NAME} PROPERTIES
			C_VISIBILITY_PRESET hidden
			CXX_VISIBILITY_PRESET hidden
			VISIBILITY_INLINES_HIDDEN ON
		)
	endif()

	ADD_DEPENDENCIES(${LIB_NAME} autogen)

	if(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_IOS)
		find_library(CORE_FOUNDATION CoreFoundation "/")
		target_link_libraries(${LIB_NAME}
			PRIVATE
				${CORE_FOUNDATION}
		)
	else()
		target_link_libraries(${LIB_NAME})
	endif()
ENDMACRO(SETUP_GLLOADER)

IF(NOT (ANDROID OR IOS))
	SETUP_GLLOADER(glloader)
ENDIF()

FIND_PACKAGE(GLES)
IF(KLAYGE_GLES_FOUND)
	SETUP_GLLOADER(glloader_es)

	target_compile_definitions(glloader_es
		PRIVATE
			-DGLLOADER_GLES_SUPPORT
	)

	target_include_directories(glloader_es
		PUBLIC
			${KLAYGE_GLES_INCLUDE_DIR}
	)
	if(APPLE AND NOT IOS)
		target_include_directories(glloader_es
			PUBLIC
				/opt/X11/include
		)
	endif()
ENDIF()
