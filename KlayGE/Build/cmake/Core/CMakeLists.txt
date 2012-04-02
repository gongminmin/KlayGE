SET(LIB_NAME ${KLAYGE_CORELIB_NAME})

SET(KERNEL_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/Context.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/CpuInfo.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/DllLoader.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/KlayGE.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/Log.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/ResLoader.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/ThrowErr.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/Timer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/Util.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/XMLDom.cpp
)

SET(KERNEL_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/aligned_allocator.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/atomic.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/config/auto_link.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/COMPtr.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Config.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Context.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/CpuInfo.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/DllLoader.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Log.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/MapVector.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/PreDeclare.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ResLoader.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SetVector.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/thread.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ThrowErr.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Timer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Trace.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Types.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Util.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/XMLDom.hpp
)

SOURCE_GROUP("Kernel\\Source Files" FILES ${KERNEL_SOURCE_FILES})
SOURCE_GROUP("Kernel\\Header Files" FILES ${KERNEL_HEADER_FILES})


SET(MATH_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Math/Math.cpp
)

SET(MATH_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Detail/MathHelper.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/AABBox.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Bound.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Color.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Frustum.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/half.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Math.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Matrix.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Noise.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/OBBox.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Plane.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Quaternion.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Rect.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Size.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Sphere.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Vector.hpp
)

SOURCE_GROUP("Math Library\\Source Files" FILES ${MATH_SOURCE_FILES})
SOURCE_GROUP("Math Library\\Header Files" FILES ${MATH_HEADER_FILES})


SET(RENDERING_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/BlockCompression.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Camera.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/CameraController.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/DeferredRenderingLayer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Font.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/FrameBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/FXAAPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/GraphicsBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/HDRPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/HeightMap.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/InfTerrain.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/JudaTexture.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/LensFlare.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Light.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Mesh.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/PostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Query.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Renderable.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderableHelper.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderEffect.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderEngine.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderFactory.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderLayout.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderStateObject.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderView.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SATPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/ShaderObject.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SSGIPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SSVOPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/TAAPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Texture.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Viewport.cpp
)

SET(RENDERING_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/BlockCompression.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Camera.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/CameraController.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/DeferredRenderingLayer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ElementFormat.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Font.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/FrameBuffer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/FXAAPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/GraphicsBuffer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/HDRPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Heightmap.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/InfTerrain.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/JudaTexture.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/LensFlare.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Light.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Mesh.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ParticleSystem.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/PostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Query.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Renderable.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderableHelper.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderDeviceCaps.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderEffect.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderEngine.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderFactory.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderLayout.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderSettings.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderStateObject.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderView.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SATPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ShaderObject.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SSGIPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SSVOPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/TAAPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Texture.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Viewport.hpp
)

SET(RENDERING_EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/media/RenderFX/2D.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/BilateralBlur.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Bone.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ColorGrading.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy2LightBuffer.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/CustomMipMap.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DeferredRendering.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DeferredRenderingUtil.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DepthToSM.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DownSample.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Fog.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/FXAA.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GammaCorrection.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBuffer.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GlowMerger.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/InfTerrain.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/JudaTexture.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensFlare.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Lighting.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Noise.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PointSprite.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostProcess.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Quaternion.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/RenderableHelper.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/RSM2VPLs.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SetSubsplatStencil.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSGI.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSVO.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Stereoscopic.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SummedAreaTable.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/TAA.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Upsampling.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/util.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/VPLsLighting.fxml
)

SET(RENDERING_POSTPROCESSOR_FILES
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/ColorGrading.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy2LightBuffer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/CustomMipMap.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/DepthToSM.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Downsampler2x2.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/GammaCorrection.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/GlowMerger.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/RSM2VPLs.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Upsampling.ppml
)

SOURCE_GROUP("Rendering System\\Source Files" FILES ${RENDERING_SOURCE_FILES})
SOURCE_GROUP("Rendering System\\Header Files" FILES ${RENDERING_HEADER_FILES})
SOURCE_GROUP("Rendering System\\Effect Files" FILES ${RENDERING_EFFECT_FILES})
SOURCE_GROUP("Rendering System\\Post Processors" FILES ${RENDERING_POSTPROCESSOR_FILES})


SET(AUDIO_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Audio/AudioBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Audio/AudioDataSource.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Audio/AudioEngine.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Audio/AudioFactory.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Audio/MusicBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Audio/SoundBuffer.cpp
)

SET(AUDIO_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Audio.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/AudioDataSource.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/AudioFactory.hpp
)

SOURCE_GROUP("Audio System\\Source Files" FILES ${AUDIO_SOURCE_FILES})
SOURCE_GROUP("Audio System\\Header Files" FILES ${AUDIO_HEADER_FILES})


SET(NETWORK_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Net/Lobby.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Net/Player.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Net/Socket.cpp
)

SET(NETWORK_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Lobby.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/NetMsg.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Player.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Socket.hpp
)

SOURCE_GROUP("Network System\\Source Files" FILES ${NETWORK_SOURCE_FILES})
SOURCE_GROUP("Network System\\Header Files" FILES ${NETWORK_HEADER_FILES})


SET(SCRIPT_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Script/Script.cpp
)

SET(SCRIPT_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Script.hpp
)

SOURCE_GROUP("Script System\\Source Files" FILES ${SCRIPT_SOURCE_FILES})
SOURCE_GROUP("Script System\\Header Files" FILES ${SCRIPT_HEADER_FILES})


SET(SHOW_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Show/ShowEngine.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Show/ShowFactory.cpp
)

SET(SHOW_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Show.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ShowFactory.hpp
)

SOURCE_GROUP("Show System\\Source Files" FILES ${SHOW_SOURCE_FILES})
SOURCE_GROUP("Show System\\Header Files" FILES ${SHOW_HEADER_FILES})


SET(INPUT_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Input/InputActionMap.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Input/InputDevice.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Input/InputEngine.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Input/InputFactory.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Input/Joystick.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Input/Keyboard.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Input/Mouse.cpp
)

SET(INPUT_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Input.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/InputFactory.hpp
)

SOURCE_GROUP("Input System\\Source Files" FILES ${INPUT_SOURCE_FILES})
SOURCE_GROUP("Input System\\Header Files" FILES ${INPUT_HEADER_FILES})


SET(SCENE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Scene/SceneManager.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Scene/SceneObject.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Scene/SceneObjectHelper.cpp
)

SET(SCENE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SceneManager.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SceneNode.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SceneObject.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SceneObjectHelper.hpp
)

SOURCE_GROUP("Scene Management\\Source Files" FILES ${SCENE_SOURCE_FILES})
SOURCE_GROUP("Scene Management\\Header Files" FILES ${SCENE_HEADER_FILES})


SET(PACKING_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/ArchiveExtractCallback.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/ArchiveOpenCallback.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/Extract7z.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/LZMACodec.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/Streams.cpp
)

SET(PACKING_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/ArchiveExtractCallback.hpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/ArchiveOpenCallback.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Extract7z.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/LZMACodec.hpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/Streams.hpp
)

SOURCE_GROUP("Packing System\\Source Files" FILES ${PACKING_SOURCE_FILES})
SOURCE_GROUP("Packing System\\Header Files" FILES ${PACKING_HEADER_FILES})


SET(UI_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UI.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UIButton.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UICheckBox.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UIComboBox.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UIEditBox.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UIListBox.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UIPolylineEditBox.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UIProgressBar.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UIRadioButton.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UIScrollBar.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UISlider.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UIStatic.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/UI/UITexButton.cpp
)

SET(UI_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/UI.hpp
)

SET(UI_EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.fxml
)

SOURCE_GROUP("UI System\\Source Files" FILES ${UI_SOURCE_FILES})
SOURCE_GROUP("UI System\\Header Files" FILES ${UI_HEADER_FILES})
SOURCE_GROUP("UI System\\Effect Files" FILES ${UI_EFFECT_FILES})


SET(APP_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/App3D.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/Window.cpp
)

SET(APP_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/App3D.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Window.hpp
)

SOURCE_GROUP("App Layer\\Source Files" FILES ${APP_SOURCE_FILES})
SOURCE_GROUP("App Layer\\Header Files" FILES ${APP_HEADER_FILES})


ADD_DEFINITIONS(-DKLAYGE_BUILD_DLL -DKLAYGE_CORE_SOURCE)

INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/boost)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/Python/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/rapidxml)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/7z)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Core/Include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../kfont/include)
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/boost/lib_${KLAYGE_COMPILER_NAME}${KLAYGE_COMPILER_VERSION}_${KLAYGE_ARCH_NAME}/lib)
IF(KLAYGE_ARCH_NAME MATCHES "x64")
	LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/Python/libs/amd64)
ELSE()
	LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/Python/libs)
ENDIF()
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/lib/${KLAYGE_PLATFORM_NAME})
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../kfont/lib/${KLAYGE_PLATFORM_NAME})

ADD_LIBRARY(${LIB_NAME} SHARED
	${KERNEL_SOURCE_FILES} ${KERNEL_HEADER_FILES}
	${MATH_SOURCE_FILES} ${MATH_HEADER_FILES}
	${RENDERING_SOURCE_FILES} ${RENDERING_HEADER_FILES} ${RENDERING_EFFECT_FILES} ${RENDERING_POSTPROCESSOR_FILES}
	${AUDIO_SOURCE_FILES} ${AUDIO_HEADER_FILES}
	${NETWORK_SOURCE_FILES} ${NETWORK_HEADER_FILES}
	${SCRIPT_SOURCE_FILES} ${SCRIPT_HEADER_FILES}
	${SHOW_SOURCE_FILES} ${SHOW_HEADER_FILES}
	${INPUT_SOURCE_FILES} ${INPUT_HEADER_FILES}
	${SCENE_SOURCE_FILES} ${SCENE_HEADER_FILES}
	${PACKING_SOURCE_FILES} ${PACKING_HEADER_FILES}
	${UI_SOURCE_FILES} ${UI_HEADER_FILES} ${UI_EFFECT_FILES}
	${APP_SOURCE_FILES} ${APP_HEADER_FILES}
)

IF(MSVC)
	# hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
	SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES IMPORT_PREFIX "../")

	SET(EXTRA_LINKED_LIBRARIES wsock32)
ELSE()
	SET(EXTRA_LINKED_LIBRARIES
		boost_date_time boost_filesystem boost_signals boost_system boost_thread
	)
ENDIF()

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${KLAYGE_OUTPUT_DIR}
	PROJECT_LABEL ${LIB_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_NAME_POSTFIX}
)

ADD_PRECOMPILED_HEADER("KlayGE/KlayGE.hpp" "${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/KlayGE.cpp")

TARGET_LINK_LIBRARIES(${LIB_NAME}
	debug python32_d optimized python32
	${EXTRA_LINKED_LIBRARIES}
)


GET_PROPERTY(OUT_LOCATION_DEBUG TARGET ${LIB_NAME} PROPERTY LOCATION_DEBUG)
ADD_CUSTOM_COMMAND(TARGET ${LIB_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${OUT_LOCATION_DEBUG} ${KLAYGE_BIN_DIR})

GET_PROPERTY(OUT_LOCATION_RELEASE TARGET ${LIB_NAME} PROPERTY LOCATION_RELEASE)
ADD_CUSTOM_COMMAND(TARGET ${LIB_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${OUT_LOCATION_RELEASE} ${KLAYGE_BIN_DIR})

 
INSTALL(TARGETS ${LIB_NAME}
	RUNTIME DESTINATION ${KLAYGE_BIN_DIR}
	LIBRARY DESTINATION ${KLAYGE_BIN_DIR}
	ARCHIVE DESTINATION ${KLAYGE_OUTPUT_DIR}
)


SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER "KlayGE Core")
