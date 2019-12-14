DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/diffuse.dds" "95430e9a0e2100f0d4c721cc6a49b4a39df8de34")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/height.dds" "8a5475aa5e5e5c64dc06804c63325c82e6ea7866")
#DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/na_length.dds" "cd9a5826b056bf8d37a9d3ecb210d17a8b1607b3")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/normal.dds" "226e1bff32280d0a598d537c9547940bd07fd506")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/DetailedSurface/occlusion.dds" "FC495C94B1A161EDC9DFE37E5E10E8C4E3E9B317")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DetailedSurfaceDR/DetailedSurfaceDR.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/DetailedSurfaceDR/DetailedSurfaceDR.hpp
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurfaceDR/DetailedSurfaceDR.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/Models/IndirectLightProxy.glb.model_bin
	${KLAYGE_PROJECT_DIR}/media/Models/PointLightProxy.glb.model_bin
	${KLAYGE_PROJECT_DIR}/media/Models/SpotLightProxy.glb.model_bin
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy2LightBuffer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Depth.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/DepthOfField.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/MotionBlur.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/MultiRes.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/RSM2VPLs.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SAT.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SMAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SSVO.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/TAA.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/ToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Translucency.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/VarianceDepthMap.ppml
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Blur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/CascadedShadow.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ClusteredDeferredRendering.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Copy2LightBuffer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DeferredRendering.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DeferredRenderingDebug.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Depth.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/DepthOfField.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBuffer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBuffer+GBufferFlatTess.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBuffer+GBufferParallaxOcclusionMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBuffer+GBufferSmoothTess.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightIndexedDeferredRendering.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/MotionBlur.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/MultiRes.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Particle.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PredefinedCBuffers.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/RSM2VPLs.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SAT.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SkyBox.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SMAA.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSGI.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSR.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSS.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SSVO.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/SumLum.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/TAA.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/ToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/VarianceDepthMap.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/VPLsLighting.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/UI.kfx
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/BokehPattern.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAAAreaTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAASearchTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/white_noise_tex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/teapot.glb.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/diffuse.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/height.dds
	#${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/na_length.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/normal.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurface/occlusion.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurfaceDR/Bump.mtlml
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurfaceDR/FlatTessellation.mtlml
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurfaceDR/None.mtlml
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurfaceDR/Parallax.mtlml
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurfaceDR/ParallaxOcclusion.mtlml
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurfaceDR/SmoothTessellation.mtlml
	${KLAYGE_PROJECT_DIR}/Samples/media/DetailedSurfaceDR/DetailedSurfaceDR.uiml
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "E151422C-DE1D-4F08-8C5E-1F256BC0A4A3")
ENDIF()

SETUP_SAMPLE(DetailedSurfaceDR)
