if (NOT KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	return()
endif()

IF(KLAYGE_COMPILER_MSVC)
	SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
ENDIF()

ADD_LIBRARY(KlayGE_ShowEngine_DShow SHARED
	Source/DShow.hpp
	Source/DShowEngine.cpp
	Source/DShowFactory.cpp
	Source/DShowVMR9Allocator.cpp
	Source/DShowVMR9Allocator.hpp
)

if(KLAYGE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(KlayGE_ShowEngine_DShow
		PRIVATE
			-Wno-unknown-pragmas
			-Wno-comment
	)
endif()

SET_TARGET_PROPERTIES(KlayGE_ShowEngine_DShow PROPERTIES
	OUTPUT_NAME KlayGE_ShowEngine_DShow${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Show"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_ShowEngine_DShow PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_ShowEngine_DShow
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_ShowEngine_DShow
	PRIVATE
		KlayGE_Core
		dxsdk
		strmiids ole32 uuid
)

ADD_DEPENDENCIES(AllInEngine KlayGE_ShowEngine_DShow)
