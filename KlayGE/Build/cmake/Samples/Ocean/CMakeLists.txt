DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/DH001cross_filtered_c.dds" "224f6a5f538391161658928d43917d2a7fc5f1ea" "0bcb4b159a3f1f5255b00722378d0c851bd7333a")
DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/DH001cross_filtered_y.dds" "224f6a5f538391161658928d43917d2a7fc5f1ea" "9416d0a724293bc0a48f5627eadc7bd812fe649a")
DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/Dirt.dds" "cbda47a1678ce70b6720856736100979d469e159" "0980bdbf018a6126cfce0cdde9229458c14e30b4")
DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/grass.dds" "cbda47a1678ce70b6720856736100979d469e159" "99497d6d0617e14eecd8c25e16e2f31eb9011195")
DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/GrassGreenTexture0002.dds" "cbda47a1678ce70b6720856736100979d469e159" "5e20a4c450848b237941d6015ec517d5c2c0a7fe")
DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/OceanDisplacement.dds" "20e84d895bf9be7ef66d691e987e43a2fdb9857d" "7cca595feb8ed0f4b89443c59b28717d7ff987bb")
DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/OceanDisplacementParam.dds" "20e84d895bf9be7ef66d691e987e43a2fdb9857d" "fa6186c185d597dc0724ca68fa1b7b3eefb2acc0")
DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/OceanGradient.dds" "20e84d895bf9be7ef66d691e987e43a2fdb9857d" "0527570befa4ec1b8039a919cc0cc6077f5c6e57")
DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/RealSand40BoH.dds" "cbda47a1678ce70b6720856736100979d469e159" "1fcb18a6f441b3775b064f819b64118ed654c4b5")
DOWNLOAD_FILE("KlayGE/Samples/media/Ocean/snow_DM.dds" "cbda47a1678ce70b6720856736100979d469e159" "a01473c33d27ef663ff47992071de98c9ed42b24")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/Ocean.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/OceanSimulator.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/ProceduralTerrain.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/Ocean.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/OceanSimulator.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Ocean/ProceduralTerrain.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/Ocean.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/OceanSimulator.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/ProceduralTerrain.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/ProceduralTerrain.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/Ocean.uiml
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
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Fog.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LightShaft.ppml
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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Fog.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensFlare.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightShaft.kfx
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
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/fBm5_grad_tex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/fBm5_tex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/flare.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/noise_grad3_perm.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/noise_grad4_perm.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/noise_simplex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/powered_by_klayge.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAAAreaTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/SMAASearchTex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/ui.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/2D/white_noise_tex.dds
	${KLAYGE_PROJECT_DIR}/media/Textures/3D/color_grading.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/DH001cross_filtered_c.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/DH001cross_filtered_y.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/Dirt.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/grass.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/GrassGreenTexture0002.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/Ocean.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/OceanDisplacement.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/OceanDisplacementParam.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/OceanGradient.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/OceanSimulator.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/ProceduralTerrain.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/RealSand40BoH.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Ocean/snow_DM.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "034D4D85-760C-4044-B018-9747A3F9769B")
ENDIF()

SETUP_SAMPLE(Ocean)
