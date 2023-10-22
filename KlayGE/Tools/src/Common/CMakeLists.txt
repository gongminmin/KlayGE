SET(LIB_NAME ToolCommon)

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/Common/ToolCommon.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Tools/Include/KlayGE/ToolCommon.hpp
)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} SHARED
	${SOURCE_FILES} ${HEADER_FILES} ${PLAT_CONF_FILES})

target_include_directories(${LIB_NAME}
	PUBLIC
		${KLAYGE_PROJECT_DIR}/Tools/Include
)

SET(LIB_OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX})

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	PROJECT_LABEL ${LIB_NAME}
	OUTPUT_NAME ${LIB_OUTPUT_NAME}
	FOLDER "KlayGE/Tools"
)

target_precompile_headers(${LIB_NAME}
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(${LIB_NAME}
	PUBLIC
		KlayGE_Core
)
if(KLAYGE_PLATFORM_LINUX)
	target_link_libraries(${LIB_NAME}
		PUBLIC
			dl pthread)
endif()

ADD_DEPENDENCIES(${LIB_NAME} AllInEngine)
