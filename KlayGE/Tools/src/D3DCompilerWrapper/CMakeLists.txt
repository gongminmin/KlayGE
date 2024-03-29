IF(KLAYGE_PLATFORM_WINDOWS)
	SET(SOURCE_FILES
		${KLAYGE_PROJECT_DIR}/Tools/src/D3DCompilerWrapper/D3DCompilerWrapper.cpp
		${KLAYGE_PROJECT_DIR}/Tools/src/D3DCompilerWrapper/D3DCompilerWrapper.py
	)

	SETUP_TOOL(D3DCompilerWrapper)

	target_link_libraries(D3DCompilerWrapper
		PRIVATE
			dxsdk
	)
ELSE()
	if(KLAYGE_PLATFORM_DARWIN)
		set(winegcc "x86_64-w64-mingw32-gcc") # Use MinGW-w64 as a workaround
	elseif(KLAYGE_PLATFORM_LINUX)
		set(winegcc "winegcc")
	endif()

	FIND_PATH(WINEGCC_PATH ${winegcc}
		PATHS /usr/local/bin opt/local/sbin /usr/bin /bin /usr/sbin /sbin /usr/local/bin)

	IF(NOT WINEGCC_PATH)
		MESSAGE(FATAL_ERROR "Could NOT find winegcc")
	ENDIF()

	ADD_CUSTOM_TARGET(D3DCompilerWrapper ALL
		COMMAND "${Python3_EXECUTABLE}" "${KLAYGE_PROJECT_DIR}/Tools/src/D3DCompilerWrapper/D3DCompilerWrapper.py" "${KLAYGE_PROJECT_DIR}/Tools/src/D3DCompilerWrapper/D3DCompilerWrapper.cpp" "${KLAYGE_BIN_DIR}/D3DCompilerWrapper$<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>" "-I${KLAYGE_PROJECT_DIR}/Core/Include"
	)

	SET_TARGET_PROPERTIES(D3DCompilerWrapper PROPERTIES FOLDER "KlayGE/Tools")
ENDIF()
