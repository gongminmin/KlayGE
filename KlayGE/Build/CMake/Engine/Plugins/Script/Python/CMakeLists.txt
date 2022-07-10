SET(LIB_NAME KlayGE_ScriptEngine_Python)

SET(PYTHON_SE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Script/Python/PythonScript.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Script/Python/PythonScriptFactory.cpp
)

SET(PYTHON_SE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/Python/PythonScript.hpp
)

SOURCE_GROUP("Source Files" FILES ${PYTHON_SE_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${PYTHON_SE_HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
	${PYTHON_SE_SOURCE_FILES} ${PYTHON_SE_HEADER_FILES}
)

target_compile_definitions(${LIB_NAME}
	PRIVATE
		-DPy_NO_ENABLE_SHARED
)

target_include_directories(${LIB_NAME}
	PRIVATE
		${KLAYGE_PROJECT_DIR}/Plugins/Include
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	PROJECT_LABEL ${LIB_NAME}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Script"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(${LIB_NAME} PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

KLAYGE_ADD_PRECOMPILED_HEADER(${LIB_NAME} "${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp")

target_link_libraries(${LIB_NAME}
	PRIVATE
		${KLAYGE_CORELIB_NAME}
		libpython-static
)
if(KLAYGE_PLATFORM_WINDOWS)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			ws2_32 # Required by signalmodule
			version # Required by sysmodule
			shlwapi # Required by PC/getpathp
	)

	if(KLAYGE_PLATFORM_WINDOWS_STORE)
		target_link_libraries(${LIB_NAME}
			PRIVATE
				advapi32 # Required by RegKey*
		)
	endif()
endif()

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
