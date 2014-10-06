/**
 * @file PreDeclare.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _KLAYGE_PREDECLARE_HPP
#define _KLAYGE_PREDECLARE_HPP

#pragma once

namespace KlayGE
{
	struct ContextCfg;
	class Context;
	class ResLoadingDesc;
	typedef shared_ptr<ResLoadingDesc> ResLoadingDescPtr;
	class ResLoader;
	class PerfRange;
	typedef shared_ptr<PerfRange> PerfRangePtr;
	class PerfProfiler;
	typedef shared_ptr<PerfProfiler> PerfProfilerPtr;

	class SceneManager;
	typedef shared_ptr<SceneManager> SceneManagerPtr;
	class SceneNode;
	typedef shared_ptr<SceneNode> SceneNodePtr;
	class SceneObject;
	typedef shared_ptr<SceneObject> SceneObjectPtr;
	class SceneObjectHelper;
	typedef shared_ptr<SceneObjectHelper> SceneObjectHelperPtr;
	class SceneObjectSkyBox;
	typedef shared_ptr<SceneObjectSkyBox> SceneObjectSkyBoxPtr;
	class SceneObjectLightSourceProxy;
	typedef shared_ptr<SceneObjectLightSourceProxy> SceneObjectLightSourceProxyPtr;
	class SceneObjectCameraProxy;
	typedef shared_ptr<SceneObjectCameraProxy> SceneObjectCameraProxyPtr;

	struct ElementInitData;
	class Camera;
	typedef shared_ptr<Camera> CameraPtr;
	class CameraController;
	typedef shared_ptr<CameraController> CameraControllerPtr;
	class FirstPersonCameraController;
	typedef shared_ptr<FirstPersonCameraController> FirstPersonCameraControllerPtr;
	class TrackballCameraController;
	typedef shared_ptr<TrackballCameraController> TrackballCameraControllerPtr;
	class CameraPathController;
	typedef shared_ptr<CameraPathController> CameraPathControllerPtr;
	class Font;
	typedef shared_ptr<Font> FontPtr;
	class RenderEngine;
	typedef shared_ptr<RenderEngine> RenderEnginePtr;
	struct RenderSettings;
	struct RenderMaterial;
	typedef shared_ptr<RenderMaterial> RenderMaterialPtr;
	class Renderable;
	typedef shared_ptr<Renderable> RenderablePtr;
	class RenderableHelper;
	typedef shared_ptr<RenderableHelper> RenderableHelperPtr;
	class RenderablePoint;
	typedef shared_ptr<RenderablePoint> RenderablePointPtr;
	class RenderableLine;
	typedef shared_ptr<RenderableLine> RenderableLinePtr;
	class RenderableTriangle;
	typedef shared_ptr<RenderableTriangle> RenderableTrianglePtr;
	class RenderableTriBox;
	typedef shared_ptr<RenderableTriBox> RenderableTriBoxPtr;
	class RenderableLineBox;
	typedef shared_ptr<RenderableLineBox> RenderableLineBoxPtr;
	class RenderableSkyBox;
	typedef shared_ptr<RenderableSkyBox> RenderableSkyBoxPtr;
	class RenderablePlane;
	typedef shared_ptr<RenderablePlane> RenderablePlanePtr;
	class RenderDecal;
	typedef shared_ptr<RenderDecal> RenderDecalPtr;
	class RenderEffect;
	typedef shared_ptr<RenderEffect> RenderEffectPtr;
	class RenderTechnique;
	typedef shared_ptr<RenderTechnique> RenderTechniquePtr;
	class RenderPass;
	typedef shared_ptr<RenderPass> RenderPassPtr;
	class RenderEffectConstantBuffer;
	typedef shared_ptr<RenderEffectConstantBuffer> RenderEffectConstantBufferPtr;
	class RenderEffectParameter;
	typedef shared_ptr<RenderEffectParameter> RenderEffectParameterPtr;
	class RenderVariable;
	typedef shared_ptr<RenderVariable> RenderVariablePtr;
	class RenderEffectAnnotation;
	typedef shared_ptr<RenderEffectAnnotation> RenderEffectAnnotationPtr;
	struct RasterizerStateDesc;
	struct DepthStencilStateDesc;
	struct BlendStateDesc;
	struct SamplerStateDesc;
	class RasterizerStateObject;
	typedef shared_ptr<RasterizerStateObject> RasterizerStateObjectPtr;
	class DepthStencilStateObject;
	typedef shared_ptr<DepthStencilStateObject> DepthStencilStateObjectPtr;
	class BlendStateObject;
	typedef shared_ptr<BlendStateObject> BlendStateObjectPtr;
	class SamplerStateObject;
	typedef shared_ptr<SamplerStateObject> SamplerStateObjectPtr;
	class ShaderObject;
	typedef shared_ptr<ShaderObject> ShaderObjectPtr;
	class Texture;
	typedef shared_ptr<Texture> TexturePtr;
	class TexCompression;
	typedef shared_ptr<TexCompression> TexCompressionPtr;
	class TexCompressionBC1;
	typedef shared_ptr<TexCompressionBC1> TexCompressionBC1Ptr;
	class TexCompressionBC2;
	typedef shared_ptr<TexCompressionBC2> TexCompressionBC2Ptr;
	class TexCompressionBC3;
	typedef shared_ptr<TexCompressionBC3> TexCompressionBC3Ptr;
	class TexCompressionBC4;
	typedef shared_ptr<TexCompressionBC4> TexCompressionBC4Ptr;
	class TexCompressionBC5;
	typedef shared_ptr<TexCompressionBC5> TexCompressionBC5Ptr;
	class TexCompressionETC1;
	typedef shared_ptr<TexCompressionETC1> TexCompressionETC1Ptr;
	class TexCompressionETC2RGB8;
	typedef shared_ptr<TexCompressionETC2RGB8> TexCompressionETC2RGB8Ptr;
	class TexCompressionETC2RGB8A1;
	typedef shared_ptr<TexCompressionETC2RGB8A1> TexCompressionETC2RGB8A1Ptr;
	class TexCompressionETC2R11;
	typedef shared_ptr<TexCompressionETC2R11> TexCompressionETC2R11Ptr;
	class TexCompressionETC2RG11;
	typedef shared_ptr<TexCompressionETC2RG11> TexCompressionETC2RG11Ptr;
	class JudaTexture;
	typedef shared_ptr<JudaTexture> JudaTexturePtr;
	class FrameBuffer;
	typedef shared_ptr<FrameBuffer> FrameBufferPtr;
	class RenderView;
	typedef shared_ptr<RenderView> RenderViewPtr;
	class UnorderedAccessView;
	typedef shared_ptr<UnorderedAccessView> UnorderedAccessViewPtr;
	class GraphicsBuffer;
	typedef shared_ptr<GraphicsBuffer> GraphicsBufferPtr;
	class RenderLayout;
	typedef shared_ptr<RenderLayout> RenderLayoutPtr;
	class RenderGraphicsBuffer;
	typedef shared_ptr<RenderGraphicsBuffer> RenderGraphicsBufferPtr;
	struct Viewport;
	typedef shared_ptr<Viewport> ViewportPtr;
	class RenderFactory;
	typedef shared_ptr<RenderFactory> RenderFactoryPtr;
	class RenderModel;
	typedef shared_ptr<RenderModel> RenderModelPtr;
	class StaticMesh;
	typedef shared_ptr<StaticMesh> StaticMeshPtr;
	class SkinnedModel;
	typedef shared_ptr<SkinnedModel> SkinnedModelPtr;
	class SkinnedMesh;
	typedef shared_ptr<SkinnedMesh> SkinnedMeshPtr;
	class RenderableLightSourceProxy;
	typedef shared_ptr<RenderableLightSourceProxy> RenderableLightSourceProxyPtr;
	class RenderableCameraProxy;
	typedef shared_ptr<RenderableCameraProxy> RenderableCameraProxyPtr;
	class LightSource;
	typedef shared_ptr<LightSource> LightSourcePtr;
	class AmbientLightSource;
	typedef shared_ptr<AmbientLightSource> AmbientLightSourcePtr;
	class PointLightSource;	
	typedef shared_ptr<PointLightSource> PointLightSourcePtr;
	class SpotLightSource;
	typedef shared_ptr<SpotLightSource> SpotLightSourcePtr;
	class DirectionalLightSource;
	typedef shared_ptr<DirectionalLightSource> DirectionalLightSourcePtr;
	class SunLightSource;
	typedef shared_ptr<SunLightSource> SunLightSourcePtr;
	struct RenderDeviceCaps;
	class Query;
	typedef shared_ptr<Query> QueryPtr;
	class OcclusionQuery;
	typedef shared_ptr<OcclusionQuery> OcclusionQueryPtr;
	class ConditionalRender;
	typedef shared_ptr<ConditionalRender> ConditionalRenderPtr;
	class PostProcess;
	typedef shared_ptr<PostProcess> PostProcessPtr;
	class PostProcessChain;
	typedef shared_ptr<PostProcessChain> PostProcessChainPtr;
	class SeparableBoxFilterPostProcess;
	typedef shared_ptr<SeparableBoxFilterPostProcess> SeparableBoxFilterPostProcessPtr;
	class SeparableGaussianFilterPostProcess;
	typedef shared_ptr<SeparableGaussianFilterPostProcess> SeparableGaussianFilterPostProcessPtr;
	class SeparableBilateralFilterPostProcess;
	typedef shared_ptr<SeparableBilateralFilterPostProcess> SeparableBilateralFilterPostProcessPtr;
	template <typename T>
	class BlurPostProcess;
	class SumLumPostProcess;
	typedef shared_ptr<SumLumPostProcess> SumLumPostProcessPtr;
	class SumLumLogPostProcess;
	typedef shared_ptr<SumLumLogPostProcess> SumLumLogPostProcessPtr;
	class SumLumLogPostProcessCS;
	typedef shared_ptr<SumLumLogPostProcessCS> SumLumLogPostProcessCSPtr;
	class SumLumIterativePostProcess;
	typedef shared_ptr<SumLumIterativePostProcess> SumLumIterativePostProcessPtr;
	class AdaptedLumPostProcess;
	typedef shared_ptr<AdaptedLumPostProcess> AdaptedLumPostProcessPtr;
	class AdaptedLumPostProcessCS;
	typedef shared_ptr<AdaptedLumPostProcessCS> AdaptedLumPostProcessCSPtr;
	class ImageStatPostProcess;
	typedef shared_ptr<ImageStatPostProcess> ImageStatPostProcessPtr;
	class LensEffectsPostProcess;
	typedef shared_ptr<LensEffectsPostProcess> LensEffectsPostProcessPtr;
	class LensEffectsPostProcessCS;
	typedef shared_ptr<LensEffectsPostProcessCS> LensEffectsPostProcessCSPtr;
	class ToneMappingPostProcess;
	typedef shared_ptr<ToneMappingPostProcess> ToneMappingPostProcessPtr;
	class HDRPostProcess;
	typedef shared_ptr<HDRPostProcess> HDRPostProcessPtr;
	class SATSeparableScanSweepPostProcess;
	typedef shared_ptr<SATSeparableScanSweepPostProcess> SATSeparableScanSweepPostProcessPtr;
	class SATPostProcess;
	typedef shared_ptr<SATPostProcess> SATPostProcessPtr;
	class SATPostProcessCS;
	typedef shared_ptr<SATPostProcessCS> SATPostProcessCSPtr;
	class TAAPostProcess;
	typedef shared_ptr<TAAPostProcess> TAAPostProcessPtr;
	class SSVOPostProcess;
	typedef shared_ptr<SSVOPostProcess> SSVOPostProcessPtr;
	struct Particle;
	typedef shared_ptr<Particle> ParticlePtr;
	class ParticleEmitter;
	typedef shared_ptr<ParticleEmitter> ParticleEmitterPtr;
	class ParticleUpdater;
	typedef shared_ptr<ParticleUpdater> ParticleUpdaterPtr;
	class ParticleSystem;
	typedef shared_ptr<ParticleSystem> ParticleSystemPtr;
	class InfTerrainRenderable;
	typedef shared_ptr<InfTerrainRenderable> InfTerrainRenderablePtr;
	class InfTerrainSceneObject;
	typedef shared_ptr<InfTerrainSceneObject> InfTerrainSceneObjectPtr;
	class HQTerrainRenderable;
	typedef shared_ptr<HQTerrainRenderable> HQTerrainRenderablePtr;
	class HQTerrainSceneObject;
	typedef shared_ptr<HQTerrainSceneObject> HQTerrainSceneObjectPtr;
	class LensFlareRenderable;
	typedef shared_ptr<LensFlareRenderable> LensFlareRenderablePtr;
	class LensFlareSceneObject;
	typedef shared_ptr<LensFlareSceneObject> LensFlareSceneObjectPtr;
	class DeferredRenderingLayer;
	typedef shared_ptr<DeferredRenderingLayer> DeferredRenderingLayerPtr;
	class MultiResLayer;
	typedef shared_ptr<MultiResLayer> MultiResLayerPtr;
	class IndirectLightingLayer;
	typedef shared_ptr<IndirectLightingLayer> IndirectLightingLayerPtr;
	class MultiResSILLayer;
	typedef shared_ptr<MultiResSILLayer> MultiResSILLayerPtr;
	class SSGILayer;
	typedef shared_ptr<SSGILayer> SSGILayerPtr;
	class CascadedShadowLayer;
	typedef shared_ptr<CascadedShadowLayer> CascadedShadowLayerPtr;
	class PSSMCascadedShadowLayer;
	typedef shared_ptr<PSSMCascadedShadowLayer> PSSMCascadedShadowLayerPtr;
	class SDSMCascadedShadowLayer;
	typedef shared_ptr<SDSMCascadedShadowLayer> SDSMCascadedShadowLayerPtr;
	class GpuFft;
	typedef shared_ptr<GpuFft> GpuFftPtr;
	class GpuFftPS;
	typedef shared_ptr<GpuFftPS> GpuFftPSPtr;
	class GpuFftCS4;
	typedef shared_ptr<GpuFftCS4> GpuFftCS4Ptr;
	class GpuFftCS5;
	typedef shared_ptr<GpuFftCS4> GpuFftCS5Ptr;
	class SSGIPostProcess;
	typedef shared_ptr<SSGIPostProcess> SSGIPostProcessPtr;
	class SSRPostProcess;
	typedef shared_ptr<SSRPostProcess> SSRPostProcessPtr;
	class LightShaftPostProcess;
	typedef shared_ptr<LightShaftPostProcess> LightShaftPostProcessPtr;

	class UIManager;
	typedef shared_ptr<UIManager> UIManagerPtr;
	class UIElement;
	typedef shared_ptr<UIElement> UIElementPtr;
	class UIControl;
	typedef shared_ptr<UIControl> UIControlPtr;
	class UIDialog;
	typedef shared_ptr<UIDialog> UIDialogPtr;
	class UIStatic;
	typedef shared_ptr<UIStatic> UIStaticPtr;
	class UIButton;
	typedef shared_ptr<UIButton> UIButtonPtr;
	class UITexButton;
	typedef shared_ptr<UITexButton> UITexButtonPtr;
	class UICheckBox;
	typedef shared_ptr<UICheckBox> UICheckBoxPtr;
	class UIRadioButton;
	typedef shared_ptr<UIRadioButton> UIRadioButtonPtr;
	class UISlider;
	typedef shared_ptr<UISlider> UISliderPtr;
	class UIScrollBar;
	typedef shared_ptr<UIScrollBar> UIScrollBarPtr;
	class UIListBox;
	typedef shared_ptr<UIListBox> UIListBoxPtr;
	class UIComboBox;
	typedef shared_ptr<UIComboBox> UIComboBoxPtr;
	class UIEditBox;
	typedef shared_ptr<UIEditBox> UIEditBoxPtr;
	class UIPolylineEditBox;
	typedef shared_ptr<UIPolylineEditBox> UIPolylineEditBoxPtr;
	class UIProgressBar;
	typedef shared_ptr<UIProgressBar> UIProgressBarPtr;

	class Socket;
	class Lobby;
	class Player;

	class AudioEngine;
	typedef shared_ptr<AudioEngine> AudioEnginePtr;
	class AudioBuffer;
	typedef shared_ptr<AudioBuffer> AudioBufferPtr;
	class SoundBuffer;
	class MusicBuffer;
	class AudioDataSource;
	typedef shared_ptr<AudioDataSource> AudioDataSourcePtr;
	class AudioFactory;
	typedef shared_ptr<AudioFactory> AudioFactoryPtr;
	class AudioDataSourceFactory;
	typedef shared_ptr<AudioDataSourceFactory> AudioDataSourceFactoryPtr;

	class App3DFramework;
	class Window;
	typedef shared_ptr<Window> WindowPtr;

	class InputEngine;
	typedef shared_ptr<InputEngine> InputEnginePtr;
	class InputDevice;
	typedef shared_ptr<InputDevice> InputDevicePtr;
	class InputKeyboard;
	typedef shared_ptr<InputKeyboard> InputKeyboardPtr;
	class InputMouse;
	typedef shared_ptr<InputMouse> InputMousePtr;
	class InputJoystick;
	typedef shared_ptr<InputJoystick> InputJoystickPtr;
	class InputTouch;
	typedef shared_ptr<InputTouch> InputTouchPtr;
	class InputSensor;
	typedef shared_ptr<InputSensor> InputSensorPtr;
	class InputFactory;
	typedef shared_ptr<InputFactory> InputFactoryPtr;
	struct InputActionParam;
	typedef shared_ptr<InputActionParam> InputActionParamPtr;
	struct InputKeyboardActionParam;
	typedef shared_ptr<InputKeyboardActionParam> InputKeyboardActionParamPtr;
	struct InputMouseActionParam;
	typedef shared_ptr<InputMouseActionParam> InputMouseActionParamPtr;
	struct InputJoystickActionParam;
	typedef shared_ptr<InputJoystickActionParam> InputJoystickActionParamPtr;
	struct InputTouchActionParam;
	typedef shared_ptr<InputTouchActionParam> InputTouchActionParamPtr;
	struct InputSensorActionParam;
	typedef shared_ptr<InputSensorActionParam> InputSensorActionParamPtr;

	class ShowEngine;
	typedef shared_ptr<ShowEngine> ShowEnginePtr;
	class ShowFactory;
	typedef shared_ptr<ShowFactory> ShowFactoryPtr;

	class ScriptEngine;
	typedef shared_ptr<ScriptEngine> ScriptEnginePtr;
	class ScriptFactory;
	typedef shared_ptr<ScriptFactory> ScriptFactoryPtr;
}

#endif			// _KLAYGE_PREDECLARE_HPP
