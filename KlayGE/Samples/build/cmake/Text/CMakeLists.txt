SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Text/Text.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Text/Text.hpp
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Text/Text.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SelectMode.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Text/text.txt
)

IF(KLAYGE_BUILD_PLATFORM_WINRT)
	SET(PACKAGE_GUID "0cf69a0d-fa24-4075-8971-82c545f4af61")
	SET(DISPLAY_NAME "Text")
	SET(SHORT_NAME "Text")
	
	SET(CONTENT_FILES ${CONTENT_FILES}
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/Logo.png
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/SmallLogo.png
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/SplashScreen.png
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/StoreLogo.png
	)

	SET(DEBUG_CONTENT_FILES
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/7zxa${CMAKE_DEBUG_POSTFIX}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/LZMA${CMAKE_DEBUG_POSTFIX}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/kfont_${KLAYGE_COMPILER_NAME}_${KLAYGE_ARCH_NAME}${CMAKE_DEBUG_POSTFIX}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/KlayGE_Core_${KLAYGE_COMPILER_NAME}${CMAKE_DEBUG_POSTFIX}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Render/KlayGE_RenderEngine_D3D11_${KLAYGE_COMPILER_NAME}${CMAKE_DEBUG_POSTFIX}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Scene/KlayGE_Scene_OCTree_${KLAYGE_COMPILER_NAME}${CMAKE_DEBUG_POSTFIX}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Input/KlayGE_InputEngine_MsgInput_${KLAYGE_COMPILER_NAME}${CMAKE_DEBUG_POSTFIX}.dll
	)
	SET(RELEASE_CONTENT_FILES
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/7zxa.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/LZMA.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/kfont_${KLAYGE_COMPILER_NAME}_${KLAYGE_ARCH_NAME}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/KlayGE_Core_${KLAYGE_COMPILER_NAME}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Render/KlayGE_RenderEngine_D3D11_${KLAYGE_COMPILER_NAME}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Scene/KlayGE_Scene_OCTree_${KLAYGE_COMPILER_NAME}.dll
		${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Input/KlayGE_InputEngine_MsgInput_${KLAYGE_COMPILER_NAME}.dll
	)
ELSEIF(KLAYGE_BUILD_PLATFORM_ANDROID)
ELSE()
	SET(RESOURCE_FILES
		${KLAYGE_PROJECT_DIR}/Samples/media/Text/text.txt
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
		${KLAYGE_PROJECT_DIR}/klayge_logo.ico
	)
ENDIF()

SETUP_SAMPLE(Text)
