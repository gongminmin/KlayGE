if (NOT KLAYGE_PLATFORM_WINDOWS)
	return()
endif()

ADD_LIBRARY(KlayGE_AudioEngine_XAudio SHARED
	Source/XAAudio.hpp
	Source/XAAudioEngine.cpp
	Source/XAAudioFactory.cpp
	Source/XAMusicBuffer.cpp
	Source/XASoundBuffer.cpp
)

SET_TARGET_PROPERTIES(KlayGE_AudioEngine_XAudio PROPERTIES
	OUTPUT_NAME KlayGE_AudioEngine_XAudio${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Audio"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_AudioEngine_XAudio PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_AudioEngine_XAudio
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_AudioEngine_XAudio
	PRIVATE
		KlayGE_Core
		dxsdk
)

if(KLAYGE_COMPILER_GCC)
	target_link_libraries(KlayGE_AudioEngine_XAudio
		PRIVATE
			xaudio2_8
	)
endif()

ADD_DEPENDENCIES(AllInEngine KlayGE_AudioEngine_XAudio)
