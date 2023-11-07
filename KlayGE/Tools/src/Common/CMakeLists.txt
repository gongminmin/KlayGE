SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/Common/ToolCommon.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Tools/Include/KlayGE/ToolCommon.hpp
)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})

ADD_LIBRARY(ToolCommon SHARED
	${SOURCE_FILES} ${HEADER_FILES} ${PLAT_CONF_FILES})

target_include_directories(ToolCommon
	PUBLIC
		${KLAYGE_PROJECT_DIR}/Tools/Include
)

SET_TARGET_PROPERTIES(ToolCommon PROPERTIES
	OUTPUT_NAME ToolCommon${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Tools"
)

target_precompile_headers(ToolCommon
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(ToolCommon
	PUBLIC
		KlayGE_Core
)
if(KLAYGE_PLATFORM_LINUX)
	target_link_libraries(ToolCommon
		PUBLIC
			dl pthread)
endif()

ADD_DEPENDENCIES(ToolCommon AllInEngine)
