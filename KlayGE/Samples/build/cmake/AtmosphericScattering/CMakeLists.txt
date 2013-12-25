SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/AtmosphericScattering/AtmosphericScattering.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/AtmosphericScattering/AtmosphericScattering.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/AtmosphericScattering.fxml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/AtmosphericScattering.uiml
)

IF(KLAYGE_BUILD_PLATFORM_WINRT)
	SET(PACKAGE_GUID "be19c46c-d114-4f14-b908-71599c66f7c7")
	SET(DISPLAY_NAME "Atmospheric Scattering")
	SET(SHORT_NAME "AtmosphericScattering")

	SET(CONTENT_FILES
		${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
		${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
		${KLAYGE_PROJECT_DIR}/media/Models/directional_light_proxy.meshml.model_bin
		${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
		${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
		${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
		${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
		${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/SelectMode.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
		${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
		${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
		${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
		${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/Logo.png
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/SmallLogo.png
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/SplashScreen.png
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/StoreLogo.png
		${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/AtmosphericScattering.kfx
		${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/earth_color.dds
		${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/geosphere.meshml.model_bin
		${KLAYGE_PROJECT_DIR}/Samples/media/AtmosphericScattering/ocean_mask.dds
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
ELSE()
	SET(RESOURCE_FILES
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
		${KLAYGE_PROJECT_DIR}/klayge_logo.ico
	)
ENDIF()

SETUP_SAMPLE(AtmosphericScattering)
