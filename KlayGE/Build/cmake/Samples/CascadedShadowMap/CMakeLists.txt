DOWNLOAD_FILE("KlayGE/Samples/media/CascadedShadowMap/katapult.meshml" "cbda47a1678ce70b6720856736100979d469e159" "3610e9ef72a662cd8c97043cdd8e57724c3b6afa")
DOWNLOAD_FILE("KlayGE/Samples/media/CascadedShadowMap/p12_e.dds" "cbda47a1678ce70b6720856736100979d469e159" "100339339154778c263f99259706e3cfc7769191")
DOWNLOAD_FILE("KlayGE/Samples/media/CascadedShadowMap/p12_f.dds" "cbda47a1678ce70b6720856736100979d469e159" "8c4a8f351ee0e05e071130be8e40ee1274c8a1d3")
DOWNLOAD_FILE("KlayGE/Samples/media/CascadedShadowMap/p12_fb.dds" "cbda47a1678ce70b6720856736100979d469e159" "cb65cbc2de7514745521a25fcd97cace712a2939")
DOWNLOAD_FILE("KlayGE/Samples/media/CascadedShadowMap/plane.meshml" "cbda47a1678ce70b6720856736100979d469e159" "026bb36bcdc4d29c041521a4256f5b1236f4ff7a")
DOWNLOAD_FILE("KlayGE/Samples/media/CascadedShadowMap/stones.dds" "cbda47a1678ce70b6720856736100979d469e159" "8656637ad71427f8926cfa9cae1ee3626f9bd5dd")
DOWNLOAD_FILE("KlayGE/Samples/media/CascadedShadowMap/tau_bump.dds" "cbda47a1678ce70b6720856736100979d469e159" "512f0b5fed7569cc10ff6533f45cbca6b9b5c20b")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/CascadedShadowMap/CascadedShadowMap.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/CascadedShadowMap/CascadedShadowMap.hpp
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/CascadedShadowMap/CascadedShadowMap.uiml
)

SET(CONTENT_FILES
	${KLAYGE_PROJECT_DIR}/bin/KlayGE.cfg
	${KLAYGE_PROJECT_DIR}/media/Fonts/gkai00mp.kfont
	${KLAYGE_PROJECT_DIR}/media/Models/indirect_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/Models/point_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/Models/spot_light_proxy.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Copy2LightBuffer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Depth.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/MultiRes.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/PostToneMapping.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Resizer.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/RSM2VPLs.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/SMAA.ppml
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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBuffer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightSourceProxy.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightIndexedDeferredRendering.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/MultiRes.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Particle.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/PostToneMapping.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Resizer.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/RSM2VPLs.kfx
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
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAAAreaTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAASearchTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/white_noise_tex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/Lake_CraterLake03_filtered_c.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/Cube/Lake_CraterLake03_filtered_y.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CascadedShadowMap/katapult.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/CascadedShadowMap/plane.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/CascadedShadowMap/p12_e.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CascadedShadowMap/p12_f.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CascadedShadowMap/p12_fb.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CascadedShadowMap/stones.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/CascadedShadowMap/tau_bump.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "1E41AEDF-AA5F-4D9D-930E-938C8B40B48E")
ENDIF()

SETUP_SAMPLE(CascadedShadowMap)
