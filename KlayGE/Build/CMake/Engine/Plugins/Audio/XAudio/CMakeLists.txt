if (NOT KLAYGE_PLATFORM_WINDOWS)
	return()
endif()

SET(LIB_NAME KlayGE_AudioEngine_XAudio)

SET(XAUDIO_AE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Audio/XAudio/XAAudioEngine.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Audio/XAudio/XAAudioFactory.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Audio/XAudio/XAMusicBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/Audio/XAudio/XASoundBuffer.cpp
)

SET(XAUDIO_AE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/XAudio/XAAudio.hpp
)

SOURCE_GROUP("Source Files" FILES ${XAUDIO_AE_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${XAUDIO_AE_HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} SHARED
	${XAUDIO_AE_SOURCE_FILES} ${XAUDIO_AE_HEADER_FILES}
)

target_include_directories(${LIB_NAME}
	PRIVATE
		${KLAYGE_PROJECT_DIR}/Plugins/Include
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	PROJECT_LABEL ${LIB_NAME}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Audio"
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
		dxsdk
)

if(KLAYGE_COMPILER_GCC)
	target_link_libraries(${LIB_NAME}
		PRIVATE
			xaudio2_8
	)
endif()

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
