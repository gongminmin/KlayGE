FUNCTION(GetOriginalAssetName item search_ext asset_result)
	GET_FILENAME_COMPONENT(item_directory ${item} DIRECTORY)
	GET_FILENAME_COMPONENT(item_file_base_name ${item} NAME_WE)
	GET_FILENAME_COMPONENT(item_ext_name ${item} EXT)
	STRING(FIND ${item_ext_name} ${search_ext} search_ext_pos REVERSE)
	IF(${search_ext_pos} GREATER 0)
		STRING(SUBSTRING ${item_ext_name} 0 ${search_ext_pos} original_ext)
		SET(${asset_result} ${item_directory}/${item_file_base_name}${original_ext} PARENT_SCOPE)
	ELSE()
		SET(${asset_result} "" PARENT_SCOPE)
	ENDIF()
ENDFUNCTION()

FUNCTION(GetOriginalModelName item result)
	GetOriginalAssetName(${item} ".model_bin" asset_result)
	IF(NOT ${asset_result} STREQUAL "")
		SET(${result} "${asset_result}" PARENT_SCOPE)
	ELSE()
		SET(${result} "" PARENT_SCOPE)
	ENDIF()
ENDFUNCTION()

FUNCTION(GetOriginalImageName item result)
	GetOriginalAssetName(${item} ".dds" asset_result)
	IF(NOT ${asset_result} STREQUAL "")
		SET(${result} "${asset_result}" PARENT_SCOPE)
	ELSE()
		SET(${result} "" PARENT_SCOPE)
	ENDIF()
ENDFUNCTION()

# Required variables:
# - SOURCE_FILES/HEADER_FILES/RESOURCE_FILES
# - EFFECT_FILES/POST_PROCESSORS/UI_FILES
#
MACRO(SETUP_TUTORIAL EXE_NAME)
	IF(KLAYGE_PLATFORM_WINDOWS_STORE)
		SET(SHORT_NAME ${EXE_NAME})

		IF(PERMISSION_LOCATION)
			SET(PERMISSION_LOCATION_STR "<DeviceCapability Name=\"location\" />")
		ENDIF()
		
		SET(APPXMANIFEST_TYPE "WinStore")

		CONFIGURE_FILE(
			${KLAYGE_CMAKE_MODULE_DIR}/${APPXMANIFEST_TYPE}/Package_${KLAYGE_COMPILER_NAME}${KLAYGE_COMPILER_VERSION}.appxmanifest.in
			${CMAKE_CURRENT_BINARY_DIR}/Package.appxmanifest
			@ONLY
		)

		SET(CONTENT_FILES ${CONTENT_FILES}
			${CMAKE_CURRENT_BINARY_DIR}/Package.appxmanifest
			${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/Logo.png
			${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/SmallLogo44.png
			${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/SplashScreen.png
			${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/StoreLogo.png
			${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/Wide310x150Logo.png
		)

		SET(DEBUG_CONTENT_FILES ${DEBUG_CONTENT_FILES}
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/7zxa${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/LZMA${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/kfont${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/KlayGE_Core${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Render/KlayGE_RenderEngine_D3D11${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Render/KlayGE_RenderEngine_D3D12${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Scene/KlayGE_Scene_OCTree${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Input/KlayGE_InputEngine_MsgInput${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Audio/KlayGE_AudioEngine_XAudio${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Audio/KlayGE_AudioDataSource_OggVorbis${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Show/KlayGE_ShowEngine_MFShow${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
		)
		SET(RELEASE_CONTENT_FILES ${RELEASE_CONTENT_FILES}
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/7zxa${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/LZMA${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/kfont${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/KlayGE_Core${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Render/KlayGE_RenderEngine_D3D11${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Render/KlayGE_RenderEngine_D3D12${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Scene/KlayGE_Scene_OCTree${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Input/KlayGE_InputEngine_MsgInput${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Audio/KlayGE_AudioEngine_XAudio${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Audio/KlayGE_AudioDataSource_OggVorbis${KLAYGE_OUTPUT_SUFFIX}.dll
			${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Show/KlayGE_ShowEngine_MFShow${KLAYGE_OUTPUT_SUFFIX}.dll
		)

		SET(RESOURCE_FILES ${RESOURCE_FILES}
			${CONTENT_FILES} ${DEBUG_CONTENT_FILES} ${RELEASE_CONTENT_FILES}
			${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGE_TemporaryKey.pfx)

		SET_SOURCE_FILES_PROPERTIES(${CONTENT_FILES} ${POST_PROCESSORS} ${UI_FILES}
			PROPERTIES
			VS_DEPLOYMENT_CONTENT 1)
		SET_SOURCE_FILES_PROPERTIES(${DEBUG_CONTENT_FILES}
			PROPERTIES
			VS_DEPLOYMENT_CONTENT $<CONFIG:Debug>)
		SET_SOURCE_FILES_PROPERTIES(${RELEASE_CONTENT_FILES}
			PROPERTIES
			VS_DEPLOYMENT_CONTENT $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>)
	ELSEIF(KLAYGE_PLATFORM_ANDROID)
		IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
			SET(PROJ_NAME "${EXE_NAME}_d")
		ELSE()
			SET(PROJ_NAME "${EXE_NAME}")
		ENDIF()
		SET(SHORT_NAME ${PROJ_NAME})

		STRING(TOLOWER ${SHORT_NAME} SHORT_NAME_LOWER_CASE)
		CONFIGURE_FILE(
			${KLAYGE_CMAKE_MODULE_DIR}/Android/AndroidManifest.xml.in
			${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml
			@ONLY
		)
		CONFIGURE_FILE(
			${KLAYGE_CMAKE_MODULE_DIR}/Android/strings.xml.in
			${CMAKE_CURRENT_BINARY_DIR}/res/values/strings.xml
			@ONLY
		)
	ELSEIF(KLAYGE_PLATFORM_IOS)
		IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
			SET(PROJ_NAME "${EXE_NAME}_d")
		ELSE()
			SET(PROJ_NAME "${EXE_NAME}")
		ENDIF()
		SET(SHORT_NAME ${PROJ_NAME})

		STRING(TOLOWER ${SHORT_NAME} SHORT_NAME_LOWER_CASE)

		SET(MACOSX_BUNDLE_ICON_FILE "klayge_logo.png")
		SET(MACOSX_BUNDLE_INFO_STRING "${SHORT_NAME}")
		SET(MACOSX_BUNDLE_GUI_IDENTIFIER "org.klayge.${SHORT_NAME}")
		SET(MACOSX_BUNDLE_BUNDLE_NAME ${SHORT_NAME})
	ELSEIF(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
		SET(RESOURCE_FILES ${RESOURCE_FILES}
			$<TARGET_OBJECTS:KlayGE_RC>)
	ENDIF()

	IF(KLAYGE_PLATFORM_WINDOWS_STORE OR KLAYGE_PLATFORM_ANDROID OR KLAYGE_PLATFORM_IOS)
		FOREACH(item ${CONTENT_FILES} ${DEBUG_CONTENT_FILES} ${RELEASE_CONTENT_FILES})
			GET_FILENAME_COMPONENT(ITEM_EXT_NAME ${item} EXT)
			STRING(TOLOWER ${ITEM_EXT_NAME} ITEM_EXT_NAME)
			SET(set_generated OFF)
			IF((ITEM_EXT_NAME STREQUAL ".kfx") OR (ITEM_EXT_NAME STREQUAL ".dll"))
				SET(set_generated ON)
			ELSE()
				STRING(FIND ${ITEM_EXT_NAME} ".model_bin" search_ext_pos REVERSE)
				IF(search_ext_pos GREATER 0)
					SET(set_generated ON)
				ELSE()
					STRING(FIND ${ITEM_EXT_NAME} ".dds" search_ext_pos REVERSE)
					IF(search_ext_pos GREATER 0)
						SET(set_generated ON)
					ENDIF()
				ENDIF()
			ENDIF()

			IF(set_generated)
				SET_SOURCE_FILES_PROPERTIES(${item} PROPERTIES GENERATED 1)
			ENDIF()
		ENDFOREACH()
	ENDIF()

	SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
	SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
	SOURCE_GROUP("Resource Files" FILES ${RESOURCE_FILES})
	SOURCE_GROUP("Effect Files" FILES ${EFFECT_FILES})
	SOURCE_GROUP("Post Processors" FILES ${POST_PROCESSORS})
	SOURCE_GROUP("UI Files" FILES ${UI_FILES})

	IF(KLAYGE_PLATFORM_ANDROID)
		ADD_LIBRARY(${EXE_NAME} SHARED ${SOURCE_FILES})
	ELSE()
		ADD_EXECUTABLE(${EXE_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} ${EFFECT_FILES} ${POST_PROCESSORS} ${UI_FILES})
	ENDIF()

	if(KLAYGE_COMPILER_CLANGCL)
		target_compile_options(${EXE_NAME}
			PRIVATE
				-Wno-inconsistent-missing-override)
	endif()

	set(archive_output_dir ${KLAYGE_OUTPUT_DIR})
	if(KLAYGE_PLATFORM_ANDROID)
		set(library_output_dir ${CMAKE_CURRENT_BINARY_DIR}/libs/${KLAYGE_ARCH_NAME})
	else()
		set(library_output_dir ${KLAYGE_BIN_DIR})
	endif()

	SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ${archive_output_dir}
		ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${archive_output_dir}
		ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${archive_output_dir}
		ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${archive_output_dir}
		ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${archive_output_dir}
		LIBRARY_OUTPUT_DIRECTORY ${library_output_dir}
		LIBRARY_OUTPUT_DIRECTORY_DEBUG ${library_output_dir}
		LIBRARY_OUTPUT_DIRECTORY_RELEASE ${library_output_dir}
		LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${library_output_dir}
		LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${library_output_dir}
		PROJECT_LABEL ${EXE_NAME}
		DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
		FOLDER "KlayGE/Tutorials"
	)
	IF(KLAYGE_PLATFORM_ANDROID)
		SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES OUTPUT_NAME ${EXE_NAME})
	ELSE()
		SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES OUTPUT_NAME ${EXE_NAME}${KLAYGE_OUTPUT_SUFFIX})
	ENDIF()
	if(KLAYGE_IS_DEV_PLATFORM)
		set_target_properties(${EXE_NAME} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY ${KLAYGE_BIN_DIR}
			RUNTIME_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_BIN_DIR}
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_BIN_DIR}
			RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_BIN_DIR}
			RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_BIN_DIR}
		)
	endif()

	IF(KLAYGE_PLATFORM_ANDROID)
		target_link_libraries(${EXE_NAME}
			PRIVATE
				-Wl,--whole-archive
				KlayGE_RenderEngine_OpenGLES
				KlayGE_Scene_OCTree
				KlayGE_InputEngine_MsgInput
				KlayGE_AudioEngine_OpenAL
				KlayGE_ShowEngine_NullShow
				KlayGE_ScriptEngine_NullScript
				KlayGE_AudioDataSource_NullAudioDataSource
				android_native_app_glue
				-Wl,--no-whole-archive
				SampleCommon
				OpenAL
		)
	ELSEIF(KLAYGE_PLATFORM_IOS)
		FIND_LIBRARY(FOUNDATION Foundation "/")
		FIND_LIBRARY(QUARTZCORE QuartzCore "/")
		FIND_LIBRARY(UIKIT UIKit "/")
		FIND_LIBRARY(OPENGLES OpenGLES "/")
		FIND_LIBRARY(COREFOUNDATION CoreFoundation "/")
		FIND_LIBRARY(GLKIT GLKit "/")

		target_link_libraries(${EXE_NAME}
			PRIVATE
				${FOUNDATION} ${QUARTZCORE} ${UIKIT} ${OPENGLES} ${COREFOUNDATION} ${GLKIT}
				KlayGE_RenderEngine_OpenGLES
				KlayGE_Scene_OCTree
				KlayGE_InputEngine_MsgInput
				KlayGE_AudioEngine_NullAudio
				KlayGE_ShowEngine_NullShow
				KlayGE_ScriptEngine_NullScript
				KlayGE_AudioDataSource_NullAudioDataSource
				SampleCommon
		)
	ELSE()
		target_link_libraries(${EXE_NAME}
			PRIVATE
				SampleCommon
		)
	ENDIF()
	ADD_DEPENDENCIES(${EXE_NAME} SampleCommon)

	if(KLAYGE_SHADER_PLATFORM_NAME MATCHES "auto")
		if(KLAYGE_PLATFORM_WINDOWS)
			set(target_platform "d3d_11_0")
		elseif(KLAYGE_PLATFORM_ANDROID OR KLAYGE_PLATFORM_IOS)
			set(target_platform "gles_3_0")
		elseif(KLAYGE_PLATFORM_DARWIN)
			set(target_platform "gl_4_1")
		else()
			set(target_platform "gl_4_6")
		endif()
	else()
		set(target_platform ${KLAYGE_SHADER_PLATFORM_NAME})
	endif()

	IF(KLAYGE_PLATFORM_WINDOWS_STORE)
		FOREACH(item ${CONTENT_FILES})
			GET_FILENAME_COMPONENT(ITEM_EXT_NAME ${item} EXT)
			STRING(TOLOWER ${ITEM_EXT_NAME} ITEM_EXT_NAME)
			GET_FILENAME_COMPONENT(ITEM_DIRECTORY ${item} DIRECTORY)
			GET_FILENAME_COMPONENT(ITEM_FILE_BASE_NAME ${item} NAME_WE)
			IF(ITEM_EXT_NAME STREQUAL ".kfx")
				ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${KLAYGE_HOST_BIN_DIR}/FxmlJit -P ${target_platform} -I ${ITEM_DIRECTORY}/${ITEM_FILE_BASE_NAME}.fxml)
			ELSE()
				set(original_asset )
				GetOriginalModelName(${item} original_model)
				if(NOT ${original_model} STREQUAL "")
					set(original_asset ${original_model})
				else()
					GetOriginalImageName(${item} original_image)
					if(NOT ${original_image} STREQUAL "")
						set(original_asset ${original_image})
					endif()
				endif()
				if(NOT ${original_asset} STREQUAL "")
					ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${KLAYGE_HOST_BIN_DIR}/PlatformDeployer -P ${target_platform} -I ${original_asset})
				endif()
			ENDIF()
		ENDFOREACH()
	ELSEIF(KLAYGE_PLATFORM_ANDROID)
		FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets)
		ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/drawable-xhdpi-icon.png ${CMAKE_CURRENT_BINARY_DIR}/res/drawable-xhdpi/icon.png)
		ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/drawable-hdpi-icon.png ${CMAKE_CURRENT_BINARY_DIR}/res/drawable-hdpi/icon.png)
		ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/drawable-mdpi-icon.png ${CMAKE_CURRENT_BINARY_DIR}/res/drawable-mdpi/icon.png)
		ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/drawable-ldpi-icon.png ${CMAKE_CURRENT_BINARY_DIR}/res/drawable-ldpi/icon.png)
		FOREACH(item ${CONTENT_FILES} ${POST_PROCESSORS} ${UI_FILES})
			GET_FILENAME_COMPONENT(ITEM_FILE_NAME ${item} NAME)
			GET_FILENAME_COMPONENT(ITEM_EXT_NAME ${item} EXT)
			STRING(TOLOWER ${ITEM_EXT_NAME} ITEM_EXT_NAME)
			GET_FILENAME_COMPONENT(ITEM_DIRECTORY ${item} DIRECTORY)
			GET_FILENAME_COMPONENT(ITEM_FILE_BASE_NAME ${item} NAME_WE)
			IF(ITEM_EXT_NAME STREQUAL ".kfx")
				ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${KLAYGE_HOST_BIN_DIR}/FxmlJit -P ${target_platform} -I ${ITEM_DIRECTORY}/${ITEM_FILE_BASE_NAME}.fxml -D ${CMAKE_CURRENT_BINARY_DIR}/assets)
			ELSE()
				set(original_asset )
				GetOriginalModelName(${item} original_model)
				if(NOT ${original_model} STREQUAL "")
					set(original_asset ${original_model})
				else()
					GetOriginalImageName(${item} original_image)
					if(NOT ${original_image} STREQUAL "")
						set(original_asset ${original_image})
					endif()
				endif()
				if(NOT ${original_asset} STREQUAL "")
					ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${KLAYGE_HOST_BIN_DIR}/PlatformDeployer -P ${target_platform} -I ${original_asset} -D ${CMAKE_CURRENT_BINARY_DIR}/assets)
				else()
					ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${item} ${CMAKE_CURRENT_BINARY_DIR}/assets/${ITEM_FILE_NAME})
				endif()
			ENDIF()
		ENDFOREACH()

		ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND android update project -p . -s -t android-${ANDROID_NATIVE_API_LEVEL} -n KlayGE_${PROJ_NAME})

		ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} POST_BUILD COMMAND ant debug)
		ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy bin\\KlayGE_${PROJ_NAME}-debug.apk ${KLAYGE_BIN_DIR})
	ELSEIF(KLAYGE_PLATFORM_IOS)
		FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources)
		ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${KLAYGE_PROJECT_DIR}/klayge_logo.png ${CMAKE_CURRENT_BINARY_DIR}/Resources/klayge_logo.png)
		FOREACH(item ${CONTENT_FILES} ${POST_PROCESSORS} ${UI_FILES})
			GET_FILENAME_COMPONENT(ITEM_FILE_NAME ${item} NAME)
			GET_FILENAME_COMPONENT(ITEM_EXT_NAME ${item} EXT)
			STRING(TOLOWER ${ITEM_EXT_NAME} ITEM_EXT_NAME)
			GET_FILENAME_COMPONENT(ITEM_DIRECTORY ${item} DIRECTORY)
			GET_FILENAME_COMPONENT(ITEM_FILE_BASE_NAME ${item} NAME_WE)
			IF(ITEM_EXT_NAME STREQUAL ".kfx")
				ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND cd ${KLAYGE_HOST_BIN_DIR} && ./FxmlJit -P ${target_platform} -I ${ITEM_DIRECTORY}/${ITEM_FILE_BASE_NAME}.fxml -D ${CMAKE_CURRENT_BINARY_DIR}/Resources)
			ELSE()
				set(original_asset )
				GetOriginalModelName(${item} original_model)
				if(NOT ${original_model} STREQUAL "")
					set(original_asset ${original_model})
				else()
					GetOriginalImageName(${item} original_image)
					if(NOT ${original_image} STREQUAL "")
						set(original_asset ${original_image})
					endif()
				endif()
				if(NOT ${original_asset} STREQUAL "")
					ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND cd ${KLAYGE_HOST_BIN_DIR} && ./PlatformDeployer -P ${target_platform} -I ${original_asset} -D ${CMAKE_CURRENT_BINARY_DIR}/Resources)
				else()
					ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${item} ${CMAKE_CURRENT_BINARY_DIR}/Resources/${ITEM_FILE_NAME})
				endif()
			ENDIF()
		ENDFOREACH()

		SET(APP_NAME \${TARGET_BUILD_DIR}/\${FULL_PRODUCT_NAME})
		ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/Resources ${APP_NAME}
		)
	ENDIF()

	IF(NOT KLAYGE_PLATFORM_WINDOWS_STORE AND NOT KLAYGE_PLATFORM_ANDROID AND NOT KLAYGE_PLATFORM_IOS)
		CREATE_PROJECT_USERFILE(KlayGE ${EXE_NAME})
	ENDIF()
ENDMACRO(SETUP_TUTORIAL)

ADD_SUBDIRECTORY(DistanceMapping)
ADD_SUBDIRECTORY(Fractal)
ADD_SUBDIRECTORY(InputCaps)
ADD_SUBDIRECTORY(RasterizationOrder)
ADD_SUBDIRECTORY(Refract)
ADD_SUBDIRECTORY(Tessellation)
ADD_SUBDIRECTORY(Tutor1)
ADD_SUBDIRECTORY(Tutor2)
ADD_SUBDIRECTORY(VertexDisplacement)
