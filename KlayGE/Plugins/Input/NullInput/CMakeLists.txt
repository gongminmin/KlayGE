option(KLAYGE_BUILD_PLUGIN_NULL_INPUT_ENGINE "Build Null input engine plugin" ON)
if(NOT KLAYGE_BUILD_PLUGIN_NULL_INPUT_ENGINE)
	return()
endif()

SET(LIB_NAME KlayGE_InputEngine_NullInput)

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/NullInput.hpp
	Source/NullInputEngine.cpp
	Source/NullInputFactory.cpp
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Input"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(${LIB_NAME} PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(${LIB_NAME}
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(${LIB_NAME}
	PRIVATE
		KlayGE_Core
)

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
