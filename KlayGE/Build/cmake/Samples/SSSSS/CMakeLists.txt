DOWNLOAD_FILE("KlayGE/Samples/media/SSSSS/Infinite-Level_02.meshml" "36afc56909efad1732ba709807cebaac6e3cc4f2" "9c8ddad9f840270735d8b9670f96131d41897744")
DOWNLOAD_FILE("KlayGE/Samples/media/SSSSS/Infinite-Level_02_Tangent_SmoothUV.dds" "cbda47a1678ce70b6720856736100979d469e159" "8f49e8ff2ab19756ebf3ba2b6213b57d20e3d203")
DOWNLOAD_FILE("KlayGE/Samples/media/SSSSS/Map-COL.dds" "cbda47a1678ce70b6720856736100979d469e159" "ddbeb5a0bf60ce04160c3572da2365c49ac15c7d")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SSSSS.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/SSSSS/SSSSS.hpp
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/SSSSS.uiml
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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/GBuffer+GBufferSSS.kfx
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
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci1.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci2.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci3.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci4.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci5.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci6.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci8.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/ScifiRoom/Sci9.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/Infinite-Level_02.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/Infinite-Level_02_Tangent_SmoothUV.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/SSSSS/Map-COL.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "B0CE0295-2665-474E-9DEA-9BC15AB32047")
ENDIF()

SETUP_SAMPLE(SSSSS)
