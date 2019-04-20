DOWNLOAD_DEPENDENCY("KlayGE/klayge_logo.png" "bfdd6d15f5698d09f72306d1a54ec78c7d50c8dd")
DOWNLOAD_DEPENDENCY("KlayGE/media/Fonts/gkai00mp.kfont" "25ed05b2f28159840523d44f98bbb4034999c1c1")
DOWNLOAD_DEPENDENCY("KlayGE/media/Models/AmbientLightProxy.glb" "B0F65861730C3538A9E615AB0D38AEBF06CD759A")
DOWNLOAD_DEPENDENCY("KlayGE/media/Models/CameraProxy.glb" "092269909F3B26457616D1AC0B9E59E029B66EFE")
DOWNLOAD_DEPENDENCY("KlayGE/media/Models/DirectionalLightProxy.glb" "20FC54C4570C3655065E1CDF0877D1C08E1D637F")
DOWNLOAD_DEPENDENCY("KlayGE/media/Models/IndirectLightProxy.glb" "81F92384D96A5501982179F458588624F39E069B")
DOWNLOAD_DEPENDENCY("KlayGE/media/Models/PointLightProxy.glb" "8F5DB5C3B100214D5C9D9AE0A5C3227710A95073")
DOWNLOAD_DEPENDENCY("KlayGE/media/Models/SpotLightProxy.glb" "C0E44F1827A48C99B335944CE9F6D98811ABABAB")
DOWNLOAD_DEPENDENCY("KlayGE/media/Models/TubeLightProxy.glb" "495A9CE81FC81EB96B4D7341FA3A383C78C5094F")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/fBm5_grad_tex.dds" "D51AB2BFEC5BC93FBD5C1AFBE8AE30D823938C45")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/fBm5_tex.dds" "d3c1f21fc2d6bcdd4a445f8fcd7f8f59d12c395f")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/flare.dds" "1438372d81bea4f721a6e664caf0c0a41f11f674")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/klayge_logo.dds" "fc5795e65bb5e3ed34d56cb1ad7eb720588cb87a")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/klayge_logo_sm.dds" "b3fbf21b5f5b51851b8c0fda68607fac550dff55")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/lens_effects_imag.dds" "81ab87a46cb35c4b820e56b36cfd61028350e78b")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/lens_effects_real.dds" "d335f9b72bfbfbc27420334d5d4ca74b6da7a4b9")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/noise_grad3_perm.dds" "40918db9f36fb360ca30377afa1ddde059c4ce7e")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/noise_grad4_perm.dds" "a2c879dd71f5b96eef5286bf20b3ba38937d4e0d")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/noise_simplex.dds" "83b8b76f165a00e998ff13e39c40c27393d3e271")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/powered_by_klayge.dds" "21346648b14a0366de7abd628393e6de0253317d")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/SMAAAreaTex.dds" "6edf67ea2e127b5cab8a8b37d20ab05c54d0c757")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/SMAASearchTex.dds" "49dece98e469fa2629412e6226ff75abe2b889b0")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/ui.dds" "7bdafaf02f9f5f2fef748d38c12c84212004cda6")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/2D/white_noise_tex.dds" "5739284048778c68fb862d957f0d956430b5c279")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/3D/color_grading.dds" "44bee46635485aef2068095a45cab5a9fbc6d838")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/Cube/Lake_CraterLake03_filtered_c.dds" "579143e79a6361d26926d967b6c4511127c0137b")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/Cube/Lake_CraterLake03_filtered_y.dds" "4978e7b023d80d42111afea10d3f10be37980e25")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/Cube/rnl_cross_filtered_c.dds" "34afb02245ebaa9a79f897bcac977ce3aee60db2")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/Cube/rnl_cross_filtered_y.dds" "dceb26325178a3701c264ffa558984f55229d610")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/Cube/uffizi_cross_filtered_c.dds" "0dbeab782f1e2989709c4a3df834397fa1749d92")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/Cube/uffizi_cross_filtered_y.dds" "c515b8aed52a6eafee96ea4d32b60eaf746ea985")
DOWNLOAD_DEPENDENCY("KlayGE/media/Textures/Juda/klayge_logo.jdt" "4fed908fd641b6c536fee9f733dba37d4531079a")

SET(LIB_NAME ${KLAYGE_CORELIB_NAME})

SET(BASE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Base/Context.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Base/HWDetect.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Base/PerfProfiler.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Base/ResLoader.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Base/Signal.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Base/TableGen/Tables.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Base/TableGen/TableGen.py
)

SET(BASE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Context.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/HWDetect.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/PreDeclare.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/PerfProfiler.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ResLoader.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SALWrapper.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Signal.hpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Base/TableGen/Tables.hpp
)

IF(KLAYGE_IS_DEV_PLATFORM)
	SET(BASE_SOURCE_FILES ${BASE_SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Src/Base/DevHelper.cpp
	)

	SET(BASE_HEADER_FILES ${BASE_HEADER_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/DevHelper.hpp
	)
ENDIF()

SOURCE_GROUP("Base\\Source Files" FILES ${BASE_SOURCE_FILES})
SOURCE_GROUP("Base\\Header Files" FILES ${BASE_HEADER_FILES})

SET_SOURCE_FILES_PROPERTIES(${KLAYGE_PROJECT_DIR}/Core/Src/Base/TableGen/Tables.cpp PROPERTIES GENERATED 1)
SET_SOURCE_FILES_PROPERTIES(${KLAYGE_PROJECT_DIR}/Core/Src/Base/TableGen/Tables.hpp PROPERTIES GENERATED 1)

SET(RENDERING_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Blitter.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Camera.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/CameraController.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/CascadedShadowLayer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/DeferredRenderingLayer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/DistanceField.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/ElementFormat.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Fence.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/FFT.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Font.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/FrameBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/GraphicsBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/HDRPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/HeightMap.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Imposter.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/IndirectLightingLayer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/InfTerrain.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/JudaTexture.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/LensFlare.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Light.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/LightShaft.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Mesh.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/MotionBlur.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/MultiResLayer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/ParticleSystem.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/PostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Query.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Renderable.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderableHelper.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderDeviceCaps.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderEffect.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderEngine.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderFactory.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderLayout.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderMaterial.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderStateObject.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/RenderView.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SATPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/ShaderObject.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SkyBox.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SSGIPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SSRPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SSSBlur.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/SSVOPostProcess.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/TexCompression.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/TexCompressionBC.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/TexCompressionETC.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Texture.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/TransientBuffer.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Render/Viewport.cpp
)

SET(RENDERING_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Blitter.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Camera.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/CameraController.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/CascadedShadowLayer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/DeferredRenderingLayer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/DistanceField.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ElementFormat.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Fence.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/FFT.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Font.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/FrameBuffer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/GraphicsBuffer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/HDRPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/HeightMap.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Imposter.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/IndirectLightingLayer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/InfTerrain.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/JudaTexture.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/LensFlare.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Light.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/LightShaft.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Mesh.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/MotionBlur.hpp
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
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderMaterial.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderSettings.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderStateObject.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/RenderView.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SATPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/ShaderObject.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SkyBox.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SSGIPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SSRPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SSSBlur.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SSVOPostProcess.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/TexCompression.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/TexCompressionBC.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/TexCompressionETC.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Texture.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/TransientBuffer.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Viewport.hpp
)

SET(RENDERING_EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/media/RenderFX/2D.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/BilateralBlur.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blitter.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Bone.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/CameraProxy.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/CascadedShadow.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ClusteredDeferredRendering.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ColorGrading.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ColorSpaceUtil.fxml
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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBufferFlatTess.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBufferLine.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBufferSkinning.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBufferSmoothTess.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBufferSSS.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBufferTwoSided.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/HDRDisplay.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/InfTerrain.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Imposter.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/JudaTexture.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensFlare.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightIndexedDeferredRendering.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Lighting.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightShaft.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/MotionBlur.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/MultiRes.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Noise.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Particle.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PointSprite.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostProcess.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PrefixSum.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Quaternion.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/RenderableHelper.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/RSM2VPLs.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SkyBox.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SMAA.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Sort.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSGI.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSR.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSS.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSVO.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Stereoscopic.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SAT.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/TAA.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Tessellation.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/util.fxml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/VarianceDepthMap.fxml
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
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/HDRDisplay.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LightShaft.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/MotionBlur.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/MultiRes.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/RSM2VPLs.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SAT.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SMAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Sort.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Stereoscopic.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/TAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/ToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Translucency.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/VarianceDepthMap.ppml
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
	${KLAYGE_PROJECT_DIR}/Core/Src/Scene/SceneComponent.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Scene/SceneManager.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Scene/SceneNode.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Scene/SceneNodeHelper.cpp
)

SET(SCENE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SceneComponent.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SceneManager.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SceneNode.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/SceneNodeHelper.hpp
)

SOURCE_GROUP("Scene Management\\Source Files" FILES ${SCENE_SOURCE_FILES})
SOURCE_GROUP("Scene Management\\Header Files" FILES ${SCENE_HEADER_FILES})


SET(PACKING_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/ArchiveExtractCallback.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/ArchiveOpenCallback.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/LZMACodec.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/Package.cpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/Streams.cpp
)

SET(PACKING_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/LZMACodec.hpp
	${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/Package.hpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/ArchiveExtractCallback.hpp
	${KLAYGE_PROJECT_DIR}/Core/Src/Pack/ArchiveOpenCallback.hpp
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
IF(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	SET(APP_SOURCE_FILES ${APP_SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/WindowWin.cpp
	)
ELSEIF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(APP_SOURCE_FILES ${APP_SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Core/Src/AppLayer/WindowWinStore.cpp
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


ADD_DEFINITIONS(-DKLAYGE_CORE_SOURCE)
IF(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_LINUX)
	FIND_PATH(WINE_PATH "winegcc" #searching for 'wine' may result in include path
		PATHS /usr/local/bin opt/local/sbin /usr/bin /bin /usr/sbin /sbin /usr/local/bin)

	IF(NOT WINE_PATH)
		MESSAGE(FATAL_ERROR "Cound NOT find wine")
	ENDIF()
	ADD_DEFINITIONS(-DWINE_PATH=${WINE_PATH}/)
ENDIF()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/7z)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/UniversalDXSDK/Include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../KFL/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Core/Include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../kfont/include)
IF(KLAYGE_PLATFORM_LINUX)
	INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../glloader/include)
ENDIF()
IF(KLAYGE_PLATFORM_ANDROID)
	INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/android_native_app_glue)
ENDIF()
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../KFL/lib/${KLAYGE_PLATFORM_NAME})
if(KLAYGE_PLATFORM_WINDOWS OR (KLAYGE_PREFERRED_LIB_TYPE STREQUAL "STATIC"))
	LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../kfont/lib/${KLAYGE_PLATFORM_NAME})
endif()
IF(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_LINUX)
	LINK_DIRECTORIES(${KLAYGE_BIN_DIR})
ENDIF()

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
	${BASE_SOURCE_FILES} ${BASE_HEADER_FILES}
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

IF(KLAYGE_PLATFORM_WINDOWS)
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
		ws2_32)
ELSEIF(KLAYGE_PLATFORM_DARWIN)
	FIND_LIBRARY(APPKIT AppKit "/")
	FIND_LIBRARY(OPENGL OpenGL "/")
	FIND_LIBRARY(COREVIDEO CoreVideo "/")
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
		${APPKIT} ${OPENGL} ${COREVIDEO})
ELSEIF(KLAYGE_PLATFORM_LINUX)
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
		X11 GL
		debug glloader${KLAYGE_OUTPUT_SUFFIX}_d optimized glloader${KLAYGE_OUTPUT_SUFFIX})
ENDIF()

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_OUTPUT_DIR}
	RUNTIME_OUTPUT_DIRECTORY ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_BIN_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${KLAYGE_BIN_DIR}
	LIBRARY_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_BIN_DIR}
	LIBRARY_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_BIN_DIR}
	LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_BIN_DIR}
	LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_BIN_DIR}
	PROJECT_LABEL ${LIB_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Core"
)

KLAYGE_ADD_PRECOMPILED_HEADER(${LIB_NAME} "${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp")

TARGET_LINK_LIBRARIES(${LIB_NAME}
	${EXTRA_LINKED_LIBRARIES}
	debug kfont${KLAYGE_OUTPUT_SUFFIX}_d optimized kfont${KLAYGE_OUTPUT_SUFFIX}
	debug KFL${KLAYGE_OUTPUT_SUFFIX}_d optimized KFL${KLAYGE_OUTPUT_SUFFIX}
	${KLAYGE_FILESYSTEM_LIBRARY}
)

add_dependencies(${LIB_NAME} KFL kfont 7zxa LZMA TableGen)
if(KLAYGE_IS_DEV_PLATFORM)
	if((NOT (KLAYGE_ARCH_NAME STREQUAL "arm")) AND (NOT (KLAYGE_ARCH_NAME STREQUAL "arm64")))
		add_dependencies(${LIB_NAME} UniversalDXSDKRedist)
	endif()
endif()
if(KLAYGE_PLATFORM_LINUX)
	add_dependencies(${LIB_NAME} glloader)
elseif(KLAYGE_PLATFORM_ANDROID)
	add_dependencies(${LIB_NAME} android_native_app_glue)
endif()
if(KLAYGE_WITH_BOOST_FILESYSTEM)
	add_dependencies(${LIB_NAME} filesystem)
endif()

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})

if(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
	set(rc_lib_name "KlayGE_RC")
	add_library(${rc_lib_name} OBJECT 
		${KLAYGE_PROJECT_DIR}/media/KlayGE.rc
		${KLAYGE_PROJECT_DIR}/klayge_logo.ico
	)
	set_target_properties(${rc_lib_name} PROPERTIES
		FOLDER "KlayGE/Engine/Core"
	)

	add_dependencies(AllInEngine ${rc_lib_name})
endif()
