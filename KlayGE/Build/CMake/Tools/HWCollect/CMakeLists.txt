SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/HWCollect/HWCollect.cpp
)

SETUP_TOOL(HWCollect)

IF(KLAYGE_PLATFORM_WINDOWS)
	target_include_directories(HWCollect
		PRIVATE
			${KLAYGE_PROJECT_DIR}/../External/UniversalDXSDK/Include
	)
endif()
