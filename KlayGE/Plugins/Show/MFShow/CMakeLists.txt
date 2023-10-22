if (NOT KLAYGE_PLATFORM_WINDOWS)
	return()
endif()

SET(LIB_NAME KlayGE_ShowEngine_MFShow)

ADD_LIBRARY(${LIB_NAME} SHARED
	Source/MFShow.hpp
	Source/MFShowEngine.cpp
	Source/MFShowFactory.cpp
	Source/TinyMFMediaEngine.hpp
)

if(KLAYGE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(${LIB_NAME}
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	PROJECT_LABEL ${LIB_NAME}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Show"
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
		DirectX-Headers
		dxsdk
)

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
