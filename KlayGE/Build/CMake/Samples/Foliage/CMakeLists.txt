DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Grass1/grass.imposter.rt0.dds" "4b57e9eec4ef81b72df7ff7f378d3e202c3fc87b")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Grass1/grass.imposter.rt1.dds" "477f64508a879f9c0fb962e1ed8222e110c2451e")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Grass1/grass.glb" "443A4D7D297D2A29859602E38E218B59DC16B262")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Grass1/Grass1.jpg" "DF8634644BA1348833F4C5EC92ED10E064C17435")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Grass1/Grass4.jpg" "6B7C83B975D313A5E9F7C3A7DB6E4839AAB9300C")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Grass2/WC_Euphorbia-larica_2.imposter.rt0.dds" "3eaf350da568d0feac1efe7886cba6c96ae179b9")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Grass2/WC_Euphorbia-larica_2.imposter.rt1.dds" "5e8b57d7221867dc980a25debc7392dfebc416b1")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Grass2/WC_Euphorbia-larica_2.glb" "12FCAB994E1DF29095AD07F3D797100683B1C48A")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Grass2/WC_Euphorbia-larica.png" "B880CC3951ABE55109027C08133619EDD43FBA49")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree1/bark1.jpg" "1952F403EA18CD82C668A042B8A709C711531D1D")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree1/bark1_nmp.jpg" "10662EF40E004609E5E95B3E9B24FF4B40EFAEA8")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree1/leafs1.png" "7185A5F6736CAE1C40E3F6BC9E9DA42904711139")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree1/tree1a.imposter.rt0.dds" "8b78706bd6ce1f74653269188ba5c8918c3b9416")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree1/tree1a.imposter.rt1.dds" "9e856325058f28638063d269d05c5739a62192bc")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree1/tree1a.meshml" "7E5C76B64C1F22D7B4446EE25AE2DF062FD3E89D")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree2/bark2.jpg" "A5681E2A3A15B4572A1E0D5E961ED6D64E998308")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree2/bark2_nmp.jpg" "6D55327CA4B4D6339AD8E2AA196678BE4031377C")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree2/leafs2.png" "C1DCFB74B17C421ED5220F85C87B0771B548BCF8")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree2/tree2a.imposter.rt0.dds" "c4c6fd4b7edd5a065facf0d6c94a543f2c04e192")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree2/tree2a.imposter.rt1.dds" "80f1dcd24381269945d616c34a85b9d989d70be0")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Tree2/tree2a.meshml" "D572A677155F7AF55A0984E7305AD77407F91A5B")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/DH001cross_filtered_c.dds" "0bcb4b159a3f1f5255b00722378d0c851bd7333a")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/DH001cross_filtered_y.dds" "9416d0a724293bc0a48f5627eadc7bd812fe649a")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/GrassGreenTexture0002.dds" "5e20a4c450848b237941d6015ec517d5c2c0a7fe")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/Ground.dds" "825200b353086a39d8a97cb7fad8c46f335fee84")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/probability.dds" "0078aed8dad5bf6750f39df873608db11cd0a4da")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/RealSand40BoH.dds" "1fcb18a6f441b3775b064f819b64118ed654c4b5")
DOWNLOAD_DEPENDENCY("KlayGE/Samples/media/Foliage/snow_DM.dds" "a01473c33d27ef663ff47992071de98c9ed42b24")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Foliage/Foliage.cpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Foliage/FoliageTerrain.cpp
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Foliage/Foliage.hpp
	${KLAYGE_PROJECT_DIR}/Samples/src/Foliage/FoliageTerrain.hpp
)

SET(EFFECT_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/FoggySkyBox.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/FoliageDistribution.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/GBufferFoliage.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/GBufferFoliageImpostor.fxml
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/ProceduralTerrain.fxml
)

SET(POST_PROCESSORS
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/ProceduralTerrain.ppml
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Foliage.uiml
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
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/Fog.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LensEffects.ppml
	${KLAYGE_PROJECT_DIR}/media/PostProcessors/LightShaft.ppml
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
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Fog.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/Font.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensEffects.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LensFlare.kfx
	${KLAYGE_PROJECT_DIR}/media/RenderFX/LightShaft.kfx
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
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass1/grass.impml
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass1/grass.imposter.rt0.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass1/grass.imposter.rt1.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass1/grass.glb.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass1/Grass1.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass1/Grass4.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass2/WC_Euphorbia-larica_2.impml
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass2/WC_Euphorbia-larica_2.imposter.rt0.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass2/WC_Euphorbia-larica_2.imposter.rt1.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass2/WC_Euphorbia-larica_2.glb.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Grass2/WC_Euphorbia-larica.png.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree1/tree1a.impml
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree1/tree1a.imposter.rt0.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree1/tree1a.imposter.rt1.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree1/tree1a.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree1/bark1.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree1/bark1_nmp.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree1/leafs1.png.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree2/tree2a.impml
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree2/tree2a.imposter.rt0.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree2/tree2a.imposter.rt1.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree2/tree2a.meshml.model_bin
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree2/bark2.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree2/bark2_nmp.jpg.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Tree2/leafs2.png.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/DH001cross_filtered_c.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/DH001cross_filtered_y.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/FoggySkyBox.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/FoliageDistribution.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/GBufferFoliage.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/GBufferFoliage+GBufferTwoSided.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/GBufferFoliageImpostor.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/GrassGreenTexture0002.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/Ground.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/probability.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/ProceduralTerrain.kfx
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/RealSand40BoH.dds
	${KLAYGE_PROJECT_DIR}/Samples/media/Foliage/snow_DM.dds
)

IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	SET(PACKAGE_GUID "14DCF351-5CDC-41E6-8761-07D4858D421C")
ENDIF()

SETUP_SAMPLE(Foliage)
