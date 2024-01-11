option(KLAYGE_BUILD_PLUGIN_PYTHON_SCRIPT_ENGINE "Build Python script engine plugin" ON)
if(NOT KLAYGE_BUILD_PLUGIN_PYTHON_SCRIPT_ENGINE)
	return()
endif()

ADD_LIBRARY(KlayGE_ScriptEngine_Python ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/PythonScript.cpp
	Source/PythonScript.hpp
	Source/PythonScriptFactory.cpp
)

target_compile_definitions(KlayGE_ScriptEngine_Python
	PRIVATE
		-DPy_NO_ENABLE_SHARED
)

SET_TARGET_PROPERTIES(KlayGE_ScriptEngine_Python PROPERTIES
	OUTPUT_NAME KlayGE_ScriptEngine_Python${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Script"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_ScriptEngine_Python PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_ScriptEngine_Python
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_ScriptEngine_Python
	PRIVATE
		KlayGE_Core
		libpython-static
)
if(KLAYGE_PLATFORM_WINDOWS)
	target_link_libraries(KlayGE_ScriptEngine_Python
		PRIVATE
			ws2_32 # Required by signalmodule
			version # Required by sysmodule
			shlwapi # Required by PC/getpathp
	)

	if(KLAYGE_PLATFORM_WINDOWS_STORE)
		target_link_libraries(KlayGE_ScriptEngine_Python
			PRIVATE
				advapi32 # Required by RegKey*
		)
	endif()
endif()

ADD_DEPENDENCIES(AllInEngine KlayGE_ScriptEngine_Python)
