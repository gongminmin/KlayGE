SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/HWCollect/HWCollect.cpp
)

SETUP_TOOL(HWCollect)

IF(KLAYGE_PLATFORM_WINDOWS)
	if(KLAYGE_COMPILER_GCC)
		# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
		target_compile_options(HWCollect
			PRIVATE
				-Wno-unknown-pragmas
		)
	endif()

	target_link_libraries(HWCollect
		PRIVATE
			dxsdk
	)
endif()
