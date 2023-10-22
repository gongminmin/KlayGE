if (NOT KLAYGE_IS_DEV_PLATFORM)
	return()
endif()

SET(LIB_NAME KlayGE_DevHelper)

SET(PLAT_CONF_FILES
	${KLAYGE_PROJECT_DIR}/media/PlatConf/d3d_11_0.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/d3d_11_1.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/d3d_12_0.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/d3d_12_1.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/gl_4_1.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/gl_4_2.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/gl_4_3.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/gl_4_4.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/gl_4_5.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/gl_4_6.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/gles_3_0.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/gles_3_1.plat
	${KLAYGE_PROJECT_DIR}/media/PlatConf/gles_3_2.plat
)

SOURCE_GROUP("Resource Files\\PlatConf" FILES ${PLAT_CONF_FILES})

ADD_LIBRARY(${LIB_NAME} SHARED
	Include/KlayGE/DevHelper/DevHelper.hpp
	Include/KlayGE/DevHelper/MeshConverter.hpp
	Include/KlayGE/DevHelper/MeshMetadata.hpp
	Include/KlayGE/DevHelper/PlatformDefinition.hpp
	Include/KlayGE/DevHelper/TexConverter.hpp
	Include/KlayGE/DevHelper/TexMetadata.hpp

	Source/DevHelper.cpp
	Source/ImagePlane.cpp
	Source/ImagePlane.hpp
	Source/MeshConverter.cpp
	Source/MeshMetadata.cpp
	Source/MetadataUtil.cpp
	Source/MetadataUtil.hpp
	Source/PlatformDefinition.cpp
	Source/TexConverter.cpp
	Source/TexMetadata.cpp
	Source/TexMetadata.cpp

	${PLAT_CONF_FILES}
)

target_compile_definitions(${LIB_NAME}
	PRIVATE
		-DKLAYGE_PLUGIN_DEV_HELPER_SOURCE
)

target_include_directories(${LIB_NAME}
	PUBLIC
		${KLAYGE_PROJECT_DIR}/Plugins/DevHelper/Include
)

SET(LIB_OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX})

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	PROJECT_LABEL ${LIB_NAME}
	OUTPUT_NAME ${LIB_OUTPUT_NAME}
	FOLDER "KlayGE/Engine/Plugins/DevHelper"
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
		assimp
		FreeImage
		${KLAYGE_CORELIB_NAME}
)

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
