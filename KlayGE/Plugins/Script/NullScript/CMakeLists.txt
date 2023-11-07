ADD_LIBRARY(KlayGE_ScriptEngine_NullScript ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/NullScript.cpp
	Source/NullScript.hpp
	Source/NullScriptFactory.cpp
)

SET_TARGET_PROPERTIES(KlayGE_ScriptEngine_NullScript PROPERTIES
	OUTPUT_NAME KlayGE_ScriptEngine_NullScript${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Script"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_ScriptEngine_NullScript PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_ScriptEngine_NullScript
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_ScriptEngine_NullScript
	PRIVATE
		KlayGE_Core
)

ADD_DEPENDENCIES(AllInEngine KlayGE_ScriptEngine_NullScript)
