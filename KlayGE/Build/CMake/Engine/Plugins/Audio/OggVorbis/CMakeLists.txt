SET(LIB_NAME KlayGE_AudioDataSource_OggVorbis)

SET(OV_ADS_SOURCE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Src/AudioDataSource/OggVorbis/OggVorbisSource.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/AudioDataSource/OggVorbis/OggVorbisSourceFactory.cpp
)

SET(OV_ADS_SOURCE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OggVorbis/OggVorbisSource.hpp
)

SOURCE_GROUP("Source Files" FILES ${OV_ADS_SOURCE_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${OV_ADS_SOURCE_HEADER_FILES})

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
	${OV_ADS_SOURCE_SOURCE_FILES} ${OV_ADS_SOURCE_HEADER_FILES}
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
		vorbisfile vorbis ogg
)

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
