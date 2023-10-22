IF(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	SET(KLAYGE_LibOVR_PATH "" CACHE STRING "Path of LibOVR")
	IF(KLAYGE_LibOVR_PATH)
		SET(KLAYGE_LIBOVR_FOUND TRUE)
	ENDIF()
ENDIF()

SET(LIB_NAME KlayGE_InputEngine_MsgInput)

IF(KLAYGE_COMPILER_MSVC)
	SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
ENDIF()

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
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
	target_compile_options(${LIB_NAME}
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

IF(KLAYGE_LIBOVR_FOUND)
	target_compile_definitions(${LIB_NAME}
		PRIVATE
			-DKLAYGE_HAVE_LIBOVR
	)

	target_include_directories(${LIB_NAME}
		PRIVATE
			${KLAYGE_LibOVR_PATH}/Include
	)
ENDIF()

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	PROJECT_LABEL ${LIB_NAME}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Input"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(${LIB_NAME} PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

KLAYGE_ADD_PRECOMPILED_HEADER(${LIB_NAME} "${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp")

target_link_libraries(${LIB_NAME}
	PRIVATE
		${KLAYGE_CORELIB_NAME}
)
if(KLAYGE_PLATFORM_WINDOWS)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			dxsdk
			xinput
	)
endif()
if(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			locationapi sensorsapi
	)
endif()
if(KLAYGE_LIBOVR_FOUND)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			winmm
			debug ${KLAYGE_LibOVR_PATH}/Lib/${KLAYGE_VS_PLATFORM_NAME}/libovrd64.lib
			optimized ${KLAYGE_LibOVR_PATH}/Lib/${KLAYGE_VS_PLATFORM_NAME}/libovr64.lib
	)	
endif()

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
