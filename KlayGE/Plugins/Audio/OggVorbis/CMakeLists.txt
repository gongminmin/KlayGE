SET(LIB_NAME KlayGE_AudioDataSource_OggVorbis)

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/OggVorbisSource.cpp
	Source/OggVorbisSource.hpp
	Source/OggVorbisSourceFactory.cpp
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

target_precompile_headers(${LIB_NAME}
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(${LIB_NAME}
	PRIVATE
		KlayGE_Core
		vorbisfile vorbis ogg
)

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
