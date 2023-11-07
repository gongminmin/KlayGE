ADD_LIBRARY(KlayGE_ShowEngine_NullShow ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/NullShow.hpp
	Source/NullShowEngine.cpp
	Source/NullShowFactory.cpp
)

SET_TARGET_PROPERTIES(KlayGE_ShowEngine_NullShow PROPERTIES
	OUTPUT_NAME KlayGE_ShowEngine_NullShow${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Show"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_ShowEngine_NullShow PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_ShowEngine_NullShow
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_ShowEngine_NullShow
	PRIVATE
		KlayGE_Core
)

ADD_DEPENDENCIES(AllInEngine KlayGE_ShowEngine_NullShow)
