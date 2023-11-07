if (NOT KLAYGE_PLATFORM_WINDOWS)
	return()
endif()

ADD_LIBRARY(KlayGE_ShowEngine_MFShow SHARED
	Source/MFShow.hpp
	Source/MFShowEngine.cpp
	Source/MFShowFactory.cpp
	Source/TinyMFMediaEngine.hpp
)

if(KLAYGE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(KlayGE_ShowEngine_MFShow
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

SET_TARGET_PROPERTIES(KlayGE_ShowEngine_MFShow PROPERTIES
	OUTPUT_NAME KlayGE_ShowEngine_MFShow${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Show"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_ShowEngine_MFShow PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_ShowEngine_MFShow
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_ShowEngine_MFShow
	PRIVATE
		KlayGE_Core
		DirectX-Headers
		dxsdk
)

ADD_DEPENDENCIES(AllInEngine KlayGE_ShowEngine_MFShow)
