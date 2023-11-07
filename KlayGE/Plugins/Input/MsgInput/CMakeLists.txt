IF(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	SET(KLAYGE_LibOVR_PATH "" CACHE STRING "Path of LibOVR")
	IF(KLAYGE_LibOVR_PATH)
		SET(KLAYGE_LIBOVR_FOUND TRUE)
	ENDIF()
ENDIF()

IF(KLAYGE_COMPILER_MSVC)
	SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
ENDIF()

ADD_LIBRARY(KlayGE_InputEngine_MsgInput ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/MIJoystick.cpp
	Source/MIKeyboard.cpp
	Source/MIMouse.cpp
	Source/MInput.hpp
	Source/MInputEngine.cpp
	Source/MInputFactory.cpp
	Source/MISensor.cpp
	Source/MITouch.cpp
	Source/MIXInput.cpp
	Source/MIOVR.cpp
)

if(KLAYGE_PLATFORM_WINDOWS AND KLAYGE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(KlayGE_InputEngine_MsgInput
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

IF(KLAYGE_LIBOVR_FOUND)
	target_compile_definitions(KlayGE_InputEngine_MsgInput
		PRIVATE
			-DKLAYGE_HAVE_LIBOVR
	)

	target_include_directories(KlayGE_InputEngine_MsgInput
		PRIVATE
			${KLAYGE_LibOVR_PATH}/Include
	)
ENDIF()

SET_TARGET_PROPERTIES(KlayGE_InputEngine_MsgInput PROPERTIES
	OUTPUT_NAME KlayGE_InputEngine_MsgInput${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Input"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_InputEngine_MsgInput PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_InputEngine_MsgInput
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_InputEngine_MsgInput
	PRIVATE
		KlayGE_Core
)
if(KLAYGE_PLATFORM_WINDOWS)
	target_link_libraries(KlayGE_InputEngine_MsgInput
		PRIVATE
			dxsdk
			xinput
	)
endif()
if(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	target_link_libraries(KlayGE_InputEngine_MsgInput
		PRIVATE
			locationapi sensorsapi
	)
endif()
if(KLAYGE_LIBOVR_FOUND)
	target_link_libraries(KlayGE_InputEngine_MsgInput
		PRIVATE
			winmm
			debug ${KLAYGE_LibOVR_PATH}/Lib/${KLAYGE_VS_PLATFORM_NAME}/libovrd64.lib
			optimized ${KLAYGE_LibOVR_PATH}/Lib/${KLAYGE_VS_PLATFORM_NAME}/libovr64.lib
	)	
endif()

ADD_DEPENDENCIES(AllInEngine KlayGE_InputEngine_MsgInput)
