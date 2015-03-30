DOWNLOAD_FILE("KlayGE/klayge_logo.png")
DOWNLOAD_FILE("KlayGE/media/Fonts/gkai00mp.kfont")
DOWNLOAD_FILE("KlayGE/media/Models/ambient_light_proxy.meshml")
DOWNLOAD_FILE("KlayGE/media/Models/camera_proxy.meshml")
DOWNLOAD_FILE("KlayGE/media/Models/directional_light_proxy.meshml")
DOWNLOAD_FILE("KlayGE/media/Models/indirect_light_proxy.meshml")
DOWNLOAD_FILE("KlayGE/media/Models/point_light_proxy.meshml")
DOWNLOAD_FILE("KlayGE/media/Models/spot_light_proxy.meshml")
DOWNLOAD_FILE("KlayGE/media/Models/tube_light_proxy.meshml")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/fBm5_grad_tex.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/fBm5_tex.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/flare.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/klayge_logo.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/klayge_logo_sm.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/lens_effects_imag.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/lens_effects_real.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/noise_grad3_perm.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/noise_grad4_perm.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/noise_simplex.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/powered_by_klayge.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/ui.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/2D/white_noise_tex.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/3D/color_grading.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/Cube/Lake_CraterLake03_filtered_c.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/Cube/Lake_CraterLake03_filtered_y.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/Cube/rnl_cross_c.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/Cube/rnl_cross_y.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/Cube/uffizi_cross_filtered_c.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/Cube/uffizi_cross_filtered_y.dds")
DOWNLOAD_FILE("KlayGE/media/Textures/Juda/klayge_logo.jdt")

SET(LIB_NAME ${KLAYGE_CORELIB_NAME})

SET(KERNEL_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/Context.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/KlayGE.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/PerfProfiler.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/ResLoader.cpp
)

SET(KERNEL_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Context.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/PreDeclare.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/PerfProfiler.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ResLoader.hpp
)

SOURCE_GROUP("Kernel\\Source Files" FILES ${KERNEL_SOURCE_FILES})
SOURCE_GROUP("Kernel\\Header Files" FILES ${KERNEL_HEADER_FILES})


SET(RENDERING_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Camera.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/CameraController.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/CascadedShadowLayer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/DeferredRenderingLayer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/ElementFormat.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/FFT.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Font.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/FrameBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/GraphicsBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/HDRPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/HeightMap.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/IndirectLightingLayer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/InfTerrain.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/JudaTexture.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/LensFlare.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Light.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/LightShaft.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Mesh.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/MultiResLayer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/ParticleSystem.cpp
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
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SkyBox.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SSGIPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SSRPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SSVOPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/TexCompression.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/TexCompressionBC.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/TexCompressionETC.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Texture.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Viewport.cpp
)

SET(RENDERING_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Camera.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/CameraController.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/CascadedShadowLayer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/DeferredRenderingLayer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ElementFormat.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/FFT.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Font.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/FrameBuffer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/GraphicsBuffer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/HDRPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/HeightMap.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/IndirectLightingLayer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/InfTerrain.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/JudaTexture.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/LensFlare.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Light.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/LightShaft.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Mesh.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/MultiResLayer.hpp
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
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SkyBox.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SSGIPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SSRPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SSVOPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/TexCompression.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/TexCompressionBC.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/TexCompressionETC.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Texture.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Viewport.hpp
)

SET(RENDERING_EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/media/RenderFX/2D.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/BilateralBlur.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Bone.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/CameraProxy.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/CascadedShadow.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ColorGrading.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy2LightBuffer.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Decal.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DeferredRendering.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DeferredRenderingDebug.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DeferredRenderingUtil.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Depth.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/FFT.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Fog.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/FXAA.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GammaCorrection.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBuffer.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBufferNoSkinning.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBufferSkinning64.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBufferSkinning128.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/InfTerrain.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/JudaTexture.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensFlare.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightIndexedDeferredRendering.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightIndexedDeferredRendering4.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightIndexedDeferredRendering32.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Lighting.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightShaft.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/MultiRes.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Noise.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Particle.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PointSprite.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostProcess.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Quaternion.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/RenderableHelper.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/RSM2VPLs.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SkyBox.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Sort.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSGI.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSR.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSVO.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Stereoscopic.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SAT.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/TAA.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/TileBasedDeferredRendering.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/util.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/VPLsLighting.fxml
)

SET(RENDERING_POSTPROCESSOR_FILES
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/CascadedShadow.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/ColorGrading.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy2LightBuffer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Depth.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/FFT.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Fog.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/FXAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/GammaCorrection.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LightShaft.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/MultiRes.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/RSM2VPLs.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SAT.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Sort.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Stereoscopic.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/TAA.ppml
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
	${KLAYGE_PROJECT_DIR}/Core/Src/Script/ScriptFactory.cpp
)

SET(SCRIPT_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Script.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ScriptFactory.hpp
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
	${KLAYGE_PROJECT_DIR}/Core/Src/Input/Sensor.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Input/Touch.cpp
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
)
IF(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	SET(APP_SOURCE_FILES ${APP_SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/WindowWin.cpp
	)
ELSEIF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET(APP_SOURCE_FILES ${APP_SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/WindowWinRT.cpp
	)
ELSEIF(KLAYGE_PLATFORM_ANDROID)
	SET(APP_SOURCE_FILES ${APP_SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/WindowAndroid.cpp
	)
ELSEIF(KLAYGE_PLATFORM_LINUX)
	SET(APP_SOURCE_FILES ${APP_SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/WindowLinux.cpp
	)
ELSEIF(KLAYGE_PLATFORM_DARWIN)
	SET(APP_SOURCE_FILES ${APP_SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/WindowDarwin.mm
	)
ELSEIF(KLAYGE_PLATFORM_IOS)
	SET(APP_SOURCE_FILES ${APP_SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/WindowIOS.mm
	)
ENDIF()

SET(APP_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/App3D.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Window.hpp
)

SOURCE_GROUP("App Layer\\Source Files" FILES ${APP_SOURCE_FILES})
SOURCE_GROUP("App Layer\\Header Files" FILES ${APP_HEADER_FILES})


ADD_DEFINITIONS(-DKLAYGE_BUILD_DLL -DKLAYGE_CORE_SOURCE)
IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	ADD_DEFINITIONS(-ZW)
ENDIF()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/7z)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../KFL/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Core/Include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../KFL/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../kfont/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../MeshMLLib/include)
IF(KLAYGE_PLATFORM_LINUX)
	INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../glloader/include)
ENDIF()
IF(KLAYGE_PLATFORM_ANDROID)
	INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/android_native_app_glue)
	INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Plugins/Include)
ENDIF()
IF(KLAYGE_PLATFORM_IOS)
	INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Plugins/Include)
ENDIF()
LINK_DIRECTORIES(${Boost_LIBRARY_DIR})
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/lib/${KLAYGE_PLATFORM_NAME})
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../KFL/lib/${KLAYGE_PLATFORM_NAME})
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../kfont/lib/${KLAYGE_PLATFORM_NAME})
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../MeshMLLib/lib/${KLAYGE_PLATFORM_NAME})
IF(KLAYGE_PLATFORM_LINUX)
	LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../glloader/lib/${KLAYGE_PLATFORM_NAME})
ENDIF()

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
	${KERNEL_SOURCE_FILES} ${KERNEL_HEADER_FILES}
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

IF(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES} wsock32)
ELSEIF(KLAYGE_PLATFORM_DARWIN)
	FIND_LIBRARY(APPKIT AppKit "/")
	FIND_LIBRARY(OPENGL OpenGL "/")
	FIND_LIBRARY(COREVIDEO CoreVideo "/")
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES} ${APPKIT} ${OPENGL} ${COREVIDEO})
ELSEIF(KLAYGE_PLATFORM_LINUX)
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES} X11 GL GLU
		debug glloader${KLAYGE_OUTPUT_SUFFIX}_d optimized glloader${KLAYGE_OUTPUT_SUFFIX})
ENDIF()

IF(NOT MSVC)
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
		debug MeshMLLib${KLAYGE_OUTPUT_SUFFIX}_d optimized MeshMLLib${KLAYGE_OUTPUT_SUFFIX}
		debug kfont${KLAYGE_OUTPUT_SUFFIX}_d optimized kfont${KLAYGE_OUTPUT_SUFFIX}
		debug KFL${KLAYGE_OUTPUT_SUFFIX}_d optimized KFL${KLAYGE_OUTPUT_SUFFIX}
		${Boost_CHRONO_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})
ENDIF()

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_OUTPUT_DIR}
	PROJECT_LABEL ${LIB_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
)
IF(KLAYGE_PLATFORM_WINDOWS_RUNTIME)
	SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES VS_WINRT_EXTENSIONS TRUE)
ENDIF()
IF(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_IOS OR KLAYGE_PLATFORM_LINUX)
	SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${KLAYGE_OUTPUT_DIR}
		LIBRARY_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_OUTPUT_DIR}
		LIBRARY_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_OUTPUT_DIR}
		LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_OUTPUT_DIR}
		LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_OUTPUT_DIR})
ENDIF()

ADD_PRECOMPILED_HEADER(${LIB_NAME} "KlayGE/KlayGE.hpp" "${KLAYGE_PROJECT_DIR}/Core/Include" "${KLAYGE_PROJECT_DIR}/Core/Src/Kernel/KlayGE.cpp")

TARGET_LINK_LIBRARIES(${LIB_NAME}
	${EXTRA_LINKED_LIBRARIES}
)

IF(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	ADD_POST_BUILD(${LIB_NAME} "")

	INSTALL(TARGETS ${LIB_NAME}
		RUNTIME DESTINATION ${KLAYGE_BIN_DIR}
		LIBRARY DESTINATION ${KLAYGE_BIN_DIR}
		ARCHIVE DESTINATION ${KLAYGE_OUTPUT_DIR}
	)
ENDIF()

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER "KlayGE Core")
