DOWNLOAD_FILE("KlayGE/Samples/media/MotionBlurDoF/bokeh_pattern.dds" "cbda47a1678ce70b6720856736100979d469e159" "a0841e627bca18d05c2f6d9c0e714b1b20a8604a")

FIND_PACKAGE(PythonInterp)

SET(EXE_NAME "MotionBlurDoF")
SET(KLAYGE_ROOT_DIR "${KLAYGE_ROOT_DIR}")
SET(MEDIA_DIR "${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/")
SET(PY_SRCS "\"MotionBlurDoF_init\"")
CONFIGURE_FILE(
	${KLAYGE_ROOT_DIR}/cmake/MakePyZip.py.in
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MakePyZip.py
	@ONLY
)

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/MotionBlurDoF/MotionBlurDoF.cpp
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF_init.py
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/MotionBlurDoF/MotionBlurDoF.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/DepthOfFieldPP.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurPP.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/DepthOfField.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/Models/indirect_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/Models/point_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/Models/spot_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Depth.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SAT.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Depth.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SAT.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/teapot.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/bokeh_pattern.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/DepthOfFieldPP.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoF.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurPP.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoFPy.zip
)

SET_SOURCE_FILES_PROPERTIES(${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoFPy.zip PROPERTIES GENERATED 1)

SET(DEBUG_CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Script/KlayGE_ScriptEngine_Python${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.dll
)

SET(RELEASE_CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/${KLAYGE_PLATFORM_NAME}/Script/KlayGE_ScriptEngine_Python${KLAYGE_OUTPUT_SUFFIX}.dll
)

IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(PACKAGE_GUID "BB6372FD-18EE-4A48-97F9-06F22F626BEE")
ENDIF()

SETUP_SAMPLE(MotionBlurDoF)

ADD_CUSTOM_COMMAND(TARGET MotionBlurDoF
	PRE_BUILD
	COMMAND "${PYTHON_EXECUTABLE}" "${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MakePyZip.py")

IF(NOT KLAYGE_PLATFORM_WINDOWS_RUNTIME AND NOT KLAYGE_PLATFORM_ANDROID)
	ADD_CUSTOM_COMMAND(TARGET MotionBlurDoF
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoFPy.zip ${KLAYGE_BIN_DIR})

	INSTALL(FILES ${KLAYGE_PROJECT_DIR}/Samples/media/MotionBlurDoF/MotionBlurDoFPy.zip
		DESTINATION ${KLAYGE_BIN_DIR}
	)
ENDIF()
