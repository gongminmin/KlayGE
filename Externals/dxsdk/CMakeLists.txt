UpdateExternalLib("dxsdk" "https://github.com/apitrace/dxsdk.git" "d964b66467aaa734edbc24326da8119f5f063dd3")

add_library(dxsdk INTERFACE)
target_include_directories(dxsdk
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/dxsdk/Include
)

if(KLAYGE_PLATFORM_WINDOWS AND (NOT KLAYGE_COMPILER_MSVC))
	target_compile_definitions(dxsdk
		INTERFACE
			-D__REQUIRED_RPCNDR_H_VERSION__=475
	)
endif()
