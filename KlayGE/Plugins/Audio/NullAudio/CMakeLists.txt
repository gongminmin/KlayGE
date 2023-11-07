ADD_LIBRARY(KlayGE_AudioEngine_NullAudio ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/NullAudio.hpp
	Source/NullAudioEngine.cpp
	Source/NullAudioFactory.cpp
	Source/NullMusicBuffer.cpp
	Source/NullSoundBuffer.cpp
)

SET_TARGET_PROPERTIES(KlayGE_AudioEngine_NullAudio PROPERTIES
	OUTPUT_NAME KlayGE_AudioEngine_NullAudio${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Audio"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_AudioEngine_NullAudio PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_AudioEngine_NullAudio
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_AudioEngine_NullAudio
	PRIVATE
		KlayGE_Core
)

ADD_DEPENDENCIES(AllInEngine KlayGE_AudioEngine_NullAudio)
