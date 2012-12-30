#ifndef _PREDECLARE_HPP
#define _PREDECLARE_HPP

#pragma once

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011)
#endif
#include <boost/smart_ptr.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace KlayGE
{
	struct ContextCfg;
	class Context;
	class ResLoadingDesc;
	typedef boost::shared_ptr<ResLoadingDesc> ResLoadingDescPtr;
	class ResLoader;

	class SceneManager;
	typedef boost::shared_ptr<SceneManager> SceneManagerPtr;
	class SceneNode;
	typedef boost::shared_ptr<SceneNode> SceneNodePtr;
	class SceneObject;
	typedef boost::shared_ptr<SceneObject> SceneObjectPtr;
	class SceneObjectHelper;
	typedef boost::shared_ptr<SceneObjectHelper> SceneObjectHelperPtr;
	class SceneObjectSkyBox;
	typedef boost::shared_ptr<SceneObjectSkyBox> SceneObjectSkyBoxPtr;
	class SceneObjectHDRSkyBox;
	typedef boost::shared_ptr<SceneObjectHDRSkyBox> SceneObjectHDRSkyBoxPtr;
	class SceneObjectLightSourceProxy;
	typedef boost::shared_ptr<SceneObjectLightSourceProxy> SceneObjectLightSourceProxyPtr;
	class SceneObjectCameraProxy;
	typedef boost::shared_ptr<SceneObjectCameraProxy> SceneObjectCameraProxyPtr;

	struct ElementInitData;
	class Camera;
	typedef boost::shared_ptr<Camera> CameraPtr;
	class CameraController;
	typedef boost::shared_ptr<CameraController> CameraControllerPtr;
	class FirstPersonCameraController;
	typedef boost::shared_ptr<FirstPersonCameraController> FirstPersonCameraControllerPtr;
	class TrackballCameraController;
	typedef boost::shared_ptr<TrackballCameraController> TrackballCameraControllerPtr;
	class CameraPathController;
	typedef boost::shared_ptr<CameraPathController> CameraPathControllerPtr;
	class Font;
	typedef boost::shared_ptr<Font> FontPtr;
	class RenderEngine;
	typedef boost::shared_ptr<RenderEngine> RenderEnginePtr;
	struct RenderSettings;
	struct RenderMaterial;
	typedef boost::shared_ptr<RenderMaterial> RenderMaterialPtr;
	class Renderable;
	typedef boost::shared_ptr<Renderable> RenderablePtr;
	class RenderableHelper;
	typedef boost::shared_ptr<RenderableHelper> RenderableHelperPtr;
	class RenderablePoint;
	typedef boost::shared_ptr<RenderablePoint> RenderablePointPtr;
	class RenderableLine;
	typedef boost::shared_ptr<RenderableLine> RenderableLinePtr;
	class RenderableTriangle;
	typedef boost::shared_ptr<RenderableTriangle> RenderableTrianglePtr;
	class RenderableTriBox;
	typedef boost::shared_ptr<RenderableTriBox> RenderableTriBoxPtr;
	class RenderableLineBox;
	typedef boost::shared_ptr<RenderableLineBox> RenderableLineBoxPtr;
	class RenderableSkyBox;
	typedef boost::shared_ptr<RenderableSkyBox> RenderableSkyBoxPtr;
	class RenderableHDRSkyBox;
	typedef boost::shared_ptr<RenderableHDRSkyBox> RenderableHDRSkyBoxPtr;
	class RenderablePlane;
	typedef boost::shared_ptr<RenderablePlane> RenderablePlanePtr;
	class RenderDecal;
	typedef boost::shared_ptr<RenderDecal> RenderDecalPtr;
	class RenderEffect;
	typedef boost::shared_ptr<RenderEffect> RenderEffectPtr;
	class RenderTechnique;
	typedef boost::shared_ptr<RenderTechnique> RenderTechniquePtr;
	class RenderPass;
	typedef boost::shared_ptr<RenderPass> RenderPassPtr;
	class RenderEffectParameter;
	typedef boost::shared_ptr<RenderEffectParameter> RenderEffectParameterPtr;
	class RenderVariable;
	typedef boost::shared_ptr<RenderVariable> RenderVariablePtr;
	class RenderEffectAnnotation;
	typedef boost::shared_ptr<RenderEffectAnnotation> RenderEffectAnnotationPtr;
	struct RasterizerStateDesc;
	struct DepthStencilStateDesc;
	struct BlendStateDesc;
	struct SamplerStateDesc;
	class RasterizerStateObject;
	typedef boost::shared_ptr<RasterizerStateObject> RasterizerStateObjectPtr;
	class DepthStencilStateObject;
	typedef boost::shared_ptr<DepthStencilStateObject> DepthStencilStateObjectPtr;
	class BlendStateObject;
	typedef boost::shared_ptr<BlendStateObject> BlendStateObjectPtr;
	class SamplerStateObject;
	typedef boost::shared_ptr<SamplerStateObject> SamplerStateObjectPtr;
	class ShaderObject;
	typedef boost::shared_ptr<ShaderObject> ShaderObjectPtr;
	class Texture;
	typedef boost::shared_ptr<Texture> TexturePtr;
	class JudaTexture;
	typedef boost::shared_ptr<JudaTexture> JudaTexturePtr;
	class FrameBuffer;
	typedef boost::shared_ptr<FrameBuffer> FrameBufferPtr;
	class RenderView;
	typedef boost::shared_ptr<RenderView> RenderViewPtr;
	class UnorderedAccessView;
	typedef boost::shared_ptr<UnorderedAccessView> UnorderedAccessViewPtr;
	class GraphicsBuffer;
	typedef boost::shared_ptr<GraphicsBuffer> GraphicsBufferPtr;
	class RenderLayout;
	typedef boost::shared_ptr<RenderLayout> RenderLayoutPtr;
	class RenderGraphicsBuffer;
	typedef boost::shared_ptr<RenderGraphicsBuffer> RenderGraphicsBufferPtr;
	struct Viewport;
	typedef boost::shared_ptr<Viewport> ViewportPtr;
	class RenderFactory;
	typedef boost::shared_ptr<RenderFactory> RenderFactoryPtr;
	class RenderModel;
	typedef boost::shared_ptr<RenderModel> RenderModelPtr;
	class StaticMesh;
	typedef boost::shared_ptr<StaticMesh> StaticMeshPtr;
	class SkinnedModel;
	typedef boost::shared_ptr<SkinnedModel> SkinnedModelPtr;
	class SkinnedMesh;
	typedef boost::shared_ptr<SkinnedMesh> SkinnedMeshPtr;
	class RenderableLightSourceProxy;
	typedef boost::shared_ptr<RenderableLightSourceProxy> RenderableLightSourceProxyPtr;
	class LightSource;
	typedef boost::shared_ptr<LightSource> LightSourcePtr;
	class AmbientLightSource;
	typedef boost::shared_ptr<AmbientLightSource> AmbientLightSourcePtr;
	class PointLightSource;	
	typedef boost::shared_ptr<PointLightSource> PointLightSourcePtr;
	class SpotLightSource;
	typedef boost::shared_ptr<SpotLightSource> SpotLightSourcePtr;
	class DirectionalLightSource;
	typedef boost::shared_ptr<DirectionalLightSource> DirectionalLightSourcePtr;
	struct RenderDeviceCaps;
	class Query;
	typedef boost::shared_ptr<Query> QueryPtr;
	class OcclusionQuery;
	typedef boost::shared_ptr<OcclusionQuery> OcclusionQueryPtr;
	class ConditionalRender;
	typedef boost::shared_ptr<ConditionalRender> ConditionalRenderPtr;
	class PostProcess;
	typedef boost::shared_ptr<PostProcess> PostProcessPtr;
	class PostProcessChain;
	typedef boost::shared_ptr<PostProcessChain> PostProcessChainPtr;
	class SeparableBoxFilterPostProcess;
	typedef boost::shared_ptr<SeparableBoxFilterPostProcess> SeparableBoxFilterPostProcessPtr;
	class SeparableGaussianFilterPostProcess;
	typedef boost::shared_ptr<SeparableGaussianFilterPostProcess> SeparableGaussianFilterPostProcessPtr;
	class SeparableBilateralFilterPostProcess;
	typedef boost::shared_ptr<SeparableBilateralFilterPostProcess> SeparableBilateralFilterPostProcessPtr;
	template <typename T>
	class BlurPostProcess;
	class SumLumPostProcess;
	typedef boost::shared_ptr<SumLumPostProcess> SumLumPostProcessPtr;
	class SumLumLogPostProcess;
	typedef boost::shared_ptr<SumLumLogPostProcess> SumLumLogPostProcessPtr;
	class SumLumLogPostProcessCS;
	typedef boost::shared_ptr<SumLumLogPostProcess> SumLumLogPostProcessCSPtr;
	class SumLumIterativePostProcess;
	typedef boost::shared_ptr<SumLumIterativePostProcess> SumLumIterativePostProcessPtr;
	class AdaptedLumPostProcess;
	typedef boost::shared_ptr<AdaptedLumPostProcess> AdaptedLumPostProcessPtr;
	class AdaptedLumPostProcessCS;
	typedef boost::shared_ptr<AdaptedLumPostProcessCS> AdaptedLumPostProcessCSPtr;
	class ImageStatPostProcess;
	typedef boost::shared_ptr<ImageStatPostProcess> ImageStatPostProcessPtr;
	class LensEffectsPostProcess;
	typedef boost::shared_ptr<LensEffectsPostProcess> LensEffectsPostProcessPtr;
	class LensEffectsPostProcessCS;
	typedef boost::shared_ptr<LensEffectsPostProcessCS> LensEffectsPostProcessCSPtr;
	class ToneMappingPostProcess;
	typedef boost::shared_ptr<ToneMappingPostProcess> ToneMappingPostProcessPtr;
	class HDRPostProcess;
	typedef boost::shared_ptr<HDRPostProcess> HDRPostProcessPtr;
	class SATSeparableScanSweepPostProcess;
	typedef boost::shared_ptr<SATSeparableScanSweepPostProcess> SATSeparableScanSweepPostProcessPtr;
	class SATPostProcess;
	typedef boost::shared_ptr<SATPostProcess> SATPostProcessPtr;
	class SATPostProcessCS;
	typedef boost::shared_ptr<SATPostProcessCS> SATPostProcessCSPtr;
	class TAAPostProcess;
	typedef boost::shared_ptr<TAAPostProcess> TAAPostProcessPtr;
	class SSVOPostProcess;
	typedef boost::shared_ptr<SSVOPostProcess> SSVOPostProcessPtr;
	template <typename ParticleType>
	class ParticleSystem;
	class InfTerrainRenderable;
	typedef boost::shared_ptr<InfTerrainRenderable> InfTerrainRenderablePtr;
	class InfTerrainSceneObject;
	typedef boost::shared_ptr<InfTerrainSceneObject> InfTerrainSceneObjectPtr;
	class LensFlareRenderable;
	typedef boost::shared_ptr<LensFlareRenderable> LensFlareRenderablePtr;
	class LensFlareSceneObject;
	typedef boost::shared_ptr<LensFlareSceneObject> LensFlareSceneObjectPtr;
	class DeferredRenderingLayer;
	typedef boost::shared_ptr<DeferredRenderingLayer> DeferredRenderingLayerPtr;
	class GpuFft;
	typedef boost::shared_ptr<GpuFft> GpuFftPtr;
	class GpuFftPS;
	typedef boost::shared_ptr<GpuFftPS> GpuFftPSPtr;
	class GpuFftCS4;
	typedef boost::shared_ptr<GpuFftCS4> GpuFftCS4Ptr;
	class GpuFftCS5;
	typedef boost::shared_ptr<GpuFftCS4> GpuFftCS5Ptr;
	class SSGIPostProcess;
	typedef boost::shared_ptr<SSGIPostProcess> SSGIPostProcessPtr;
	class SSRPostProcess;
	typedef boost::shared_ptr<SSRPostProcess> SSRPostProcessPtr;
	class LightShaftPostProcess;
	typedef boost::shared_ptr<LightShaftPostProcess> LightShaftPostProcessPtr;

	class UIManager;
	typedef boost::shared_ptr<UIManager> UIManagerPtr;
	class UIElement;
	typedef boost::shared_ptr<UIElement> UIElementPtr;
	class UIControl;
	typedef boost::shared_ptr<UIControl> UIControlPtr;
	class UIDialog;
	typedef boost::shared_ptr<UIDialog> UIDialogPtr;
	class UIStatic;
	typedef boost::shared_ptr<UIStatic> UIStaticPtr;
	class UIButton;
	typedef boost::shared_ptr<UIButton> UIButtonPtr;
	class UITexButton;
	typedef boost::shared_ptr<UITexButton> UITexButtonPtr;
	class UICheckBox;
	typedef boost::shared_ptr<UICheckBox> UICheckBoxPtr;
	class UIRadioButton;
	typedef boost::shared_ptr<UIRadioButton> UIRadioButtonPtr;
	class UISlider;
	typedef boost::shared_ptr<UISlider> UISliderPtr;
	class UIScrollBar;
	typedef boost::shared_ptr<UIScrollBar> UIScrollBarPtr;
	class UIListBox;
	typedef boost::shared_ptr<UIListBox> UIListBoxPtr;
	class UIComboBox;
	typedef boost::shared_ptr<UIComboBox> UIComboBoxPtr;
	class UIEditBox;
	typedef boost::shared_ptr<UIEditBox> UIEditBoxPtr;
	class UIPolylineEditBox;
	typedef boost::shared_ptr<UIPolylineEditBox> UIPolylineEditBoxPtr;
	class UIProgressBar;
	typedef boost::shared_ptr<UIProgressBar> UIProgressBarPtr;

	class Socket;
	class Lobby;
	class Player;

	class AudioEngine;
	typedef boost::shared_ptr<AudioEngine> AudioEnginePtr;
	class AudioBuffer;
	typedef boost::shared_ptr<AudioBuffer> AudioBufferPtr;
	class SoundBuffer;
	class MusicBuffer;
	class AudioDataSource;
	typedef boost::shared_ptr<AudioDataSource> AudioDataSourcePtr;
	class AudioFactory;
	typedef boost::shared_ptr<AudioFactory> AudioFactoryPtr;
	class AudioDataSourceFactory;
	typedef boost::shared_ptr<AudioDataSourceFactory> AudioDataSourceFactoryPtr;

	class App3DFramework;
	class Window;
	typedef boost::shared_ptr<Window> WindowPtr;

	class InputEngine;
	typedef boost::shared_ptr<InputEngine> InputEnginePtr;
	class InputDevice;
	typedef boost::shared_ptr<InputDevice> InputDevicePtr;
	class InputKeyboard;
	typedef boost::shared_ptr<InputKeyboard> InputKeyboardPtr;
	class InputMouse;
	typedef boost::shared_ptr<InputMouse> InputMousePtr;
	class InputJoystick;
	typedef boost::shared_ptr<InputJoystick> InputJoystickPtr;
	class InputFactory;
	typedef boost::shared_ptr<InputFactory> InputFactoryPtr;

	class ShowEngine;
	typedef boost::shared_ptr<ShowEngine> ShowEnginePtr;
	class ShowFactory;
	typedef boost::shared_ptr<ShowFactory> ShowFactoryPtr;

	class ScriptModule;
	class RegisterModule;
	class ScriptEngine;
}

#endif			// _PREDECLARE_HPP
