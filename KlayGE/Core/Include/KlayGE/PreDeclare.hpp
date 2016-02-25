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
	typedef std::shared_ptr<ResLoadingDesc> ResLoadingDescPtr;
	class ResLoader;
	class PerfRange;
	typedef std::shared_ptr<PerfRange> PerfRangePtr;
	class PerfProfiler;
	typedef std::shared_ptr<PerfProfiler> PerfProfilerPtr;

	class SceneManager;
	class SceneNode;
	typedef std::shared_ptr<SceneNode> SceneNodePtr;
	class SceneObject;
	typedef std::shared_ptr<SceneObject> SceneObjectPtr;
	class SceneObjectHelper;
	typedef std::shared_ptr<SceneObjectHelper> SceneObjectHelperPtr;
	class SceneObjectSkyBox;
	typedef std::shared_ptr<SceneObjectSkyBox> SceneObjectSkyBoxPtr;
	class SceneObjectLightSourceProxy;
	typedef std::shared_ptr<SceneObjectLightSourceProxy> SceneObjectLightSourceProxyPtr;
	class SceneObjectCameraProxy;
	typedef std::shared_ptr<SceneObjectCameraProxy> SceneObjectCameraProxyPtr;

	struct ElementInitData;
	class Camera;
	typedef std::shared_ptr<Camera> CameraPtr;
	class CameraController;
	typedef std::shared_ptr<CameraController> CameraControllerPtr;
	class FirstPersonCameraController;
	typedef std::shared_ptr<FirstPersonCameraController> FirstPersonCameraControllerPtr;
	class TrackballCameraController;
	typedef std::shared_ptr<TrackballCameraController> TrackballCameraControllerPtr;
	class CameraPathController;
	typedef std::shared_ptr<CameraPathController> CameraPathControllerPtr;
	class Font;
	typedef std::shared_ptr<Font> FontPtr;
	class RenderEngine;
	struct RenderSettings;
	struct RenderMaterial;
	typedef std::shared_ptr<RenderMaterial> RenderMaterialPtr;
	class Renderable;
	typedef std::shared_ptr<Renderable> RenderablePtr;
	class RenderableHelper;
	typedef std::shared_ptr<RenderableHelper> RenderableHelperPtr;
	class RenderablePoint;
	typedef std::shared_ptr<RenderablePoint> RenderablePointPtr;
	class RenderableLine;
	typedef std::shared_ptr<RenderableLine> RenderableLinePtr;
	class RenderableTriangle;
	typedef std::shared_ptr<RenderableTriangle> RenderableTrianglePtr;
	class RenderableTriBox;
	typedef std::shared_ptr<RenderableTriBox> RenderableTriBoxPtr;
	class RenderableLineBox;
	typedef std::shared_ptr<RenderableLineBox> RenderableLineBoxPtr;
	class RenderableSkyBox;
	typedef std::shared_ptr<RenderableSkyBox> RenderableSkyBoxPtr;
	class RenderablePlane;
	typedef std::shared_ptr<RenderablePlane> RenderablePlanePtr;
	class RenderDecal;
	typedef std::shared_ptr<RenderDecal> RenderDecalPtr;
	class RenderEffect;
	typedef std::shared_ptr<RenderEffect> RenderEffectPtr;
	class RenderTechnique;
	typedef std::shared_ptr<RenderTechnique> RenderTechniquePtr;
	class RenderPass;
	typedef std::shared_ptr<RenderPass> RenderPassPtr;
	class RenderEffectConstantBuffer;
	typedef std::shared_ptr<RenderEffectConstantBuffer> RenderEffectConstantBufferPtr;
	class RenderEffectParameter;
	typedef std::shared_ptr<RenderEffectParameter> RenderEffectParameterPtr;
	class RenderVariable;
	typedef std::shared_ptr<RenderVariable> RenderVariablePtr;
	class RenderEffectAnnotation;
	typedef std::shared_ptr<RenderEffectAnnotation> RenderEffectAnnotationPtr;
	struct RasterizerStateDesc;
	struct DepthStencilStateDesc;
	struct BlendStateDesc;
	struct SamplerStateDesc;
	class RasterizerStateObject;
	typedef std::shared_ptr<RasterizerStateObject> RasterizerStateObjectPtr;
	class DepthStencilStateObject;
	typedef std::shared_ptr<DepthStencilStateObject> DepthStencilStateObjectPtr;
	class BlendStateObject;
	typedef std::shared_ptr<BlendStateObject> BlendStateObjectPtr;
	class SamplerStateObject;
	typedef std::shared_ptr<SamplerStateObject> SamplerStateObjectPtr;
	class ShaderObject;
	typedef std::shared_ptr<ShaderObject> ShaderObjectPtr;
	class Texture;
	typedef std::shared_ptr<Texture> TexturePtr;
	class TexCompression;
	typedef std::shared_ptr<TexCompression> TexCompressionPtr;
	class TexCompressionBC1;
	typedef std::shared_ptr<TexCompressionBC1> TexCompressionBC1Ptr;
	class TexCompressionBC2;
	typedef std::shared_ptr<TexCompressionBC2> TexCompressionBC2Ptr;
	class TexCompressionBC3;
	typedef std::shared_ptr<TexCompressionBC3> TexCompressionBC3Ptr;
	class TexCompressionBC4;
	typedef std::shared_ptr<TexCompressionBC4> TexCompressionBC4Ptr;
	class TexCompressionBC5;
	typedef std::shared_ptr<TexCompressionBC5> TexCompressionBC5Ptr;
	class TexCompressionBC6U;
	typedef std::shared_ptr<TexCompressionBC6U> TexCompressionBC6UPtr;
	class TexCompressionBC6S;
	typedef std::shared_ptr<TexCompressionBC6S> TexCompressionBC6SPtr;
	class TexCompressionBC7;
	typedef std::shared_ptr<TexCompressionBC7> TexCompressionBC7Ptr;
	class TexCompressionETC1;
	typedef std::shared_ptr<TexCompressionETC1> TexCompressionETC1Ptr;
	class TexCompressionETC2RGB8;
	typedef std::shared_ptr<TexCompressionETC2RGB8> TexCompressionETC2RGB8Ptr;
	class TexCompressionETC2RGB8A1;
	typedef std::shared_ptr<TexCompressionETC2RGB8A1> TexCompressionETC2RGB8A1Ptr;
	class TexCompressionETC2R11;
	typedef std::shared_ptr<TexCompressionETC2R11> TexCompressionETC2R11Ptr;
	class TexCompressionETC2RG11;
	typedef std::shared_ptr<TexCompressionETC2RG11> TexCompressionETC2RG11Ptr;
	class JudaTexture;
	typedef std::shared_ptr<JudaTexture> JudaTexturePtr;
	class FrameBuffer;
	typedef std::shared_ptr<FrameBuffer> FrameBufferPtr;
	class RenderView;
	typedef std::shared_ptr<RenderView> RenderViewPtr;
	class UnorderedAccessView;
	typedef std::shared_ptr<UnorderedAccessView> UnorderedAccessViewPtr;
	class GraphicsBuffer;
	typedef std::shared_ptr<GraphicsBuffer> GraphicsBufferPtr;
	class RenderLayout;
	typedef std::shared_ptr<RenderLayout> RenderLayoutPtr;
	class RenderGraphicsBuffer;
	typedef std::shared_ptr<RenderGraphicsBuffer> RenderGraphicsBufferPtr;
	struct Viewport;
	typedef std::shared_ptr<Viewport> ViewportPtr;
	class RenderFactory;
	class RenderModel;
	typedef std::shared_ptr<RenderModel> RenderModelPtr;
	class StaticMesh;
	typedef std::shared_ptr<StaticMesh> StaticMeshPtr;
	class SkinnedModel;
	typedef std::shared_ptr<SkinnedModel> SkinnedModelPtr;
	class SkinnedMesh;
	typedef std::shared_ptr<SkinnedMesh> SkinnedMeshPtr;
	class RenderableLightSourceProxy;
	typedef std::shared_ptr<RenderableLightSourceProxy> RenderableLightSourceProxyPtr;
	class RenderableCameraProxy;
	typedef std::shared_ptr<RenderableCameraProxy> RenderableCameraProxyPtr;
	class LightSource;
	typedef std::shared_ptr<LightSource> LightSourcePtr;
	class AmbientLightSource;
	typedef std::shared_ptr<AmbientLightSource> AmbientLightSourcePtr;
	class PointLightSource;	
	typedef std::shared_ptr<PointLightSource> PointLightSourcePtr;
	class SpotLightSource;
	typedef std::shared_ptr<SpotLightSource> SpotLightSourcePtr;
	class DirectionalLightSource;
	typedef std::shared_ptr<DirectionalLightSource> DirectionalLightSourcePtr;
	class SunLightSource;
	typedef std::shared_ptr<SunLightSource> SunLightSourcePtr;
	class SphereAreaLightSource;
	typedef std::shared_ptr<SphereAreaLightSource> SphereAreaLightSourcePtr;
	class TubeAreaLightSource;
	typedef std::shared_ptr<TubeAreaLightSource> TubeAreaLightSourcePtr;
	struct RenderDeviceCaps;
	class Query;
	typedef std::shared_ptr<Query> QueryPtr;
	class OcclusionQuery;
	typedef std::shared_ptr<OcclusionQuery> OcclusionQueryPtr;
	class ConditionalRender;
	typedef std::shared_ptr<ConditionalRender> ConditionalRenderPtr;
	class PostProcess;
	typedef std::shared_ptr<PostProcess> PostProcessPtr;
	class PostProcessChain;
	typedef std::shared_ptr<PostProcessChain> PostProcessChainPtr;
	class SeparableBoxFilterPostProcess;
	typedef std::shared_ptr<SeparableBoxFilterPostProcess> SeparableBoxFilterPostProcessPtr;
	class SeparableGaussianFilterPostProcess;
	typedef std::shared_ptr<SeparableGaussianFilterPostProcess> SeparableGaussianFilterPostProcessPtr;
	class SeparableBilateralFilterPostProcess;
	typedef std::shared_ptr<SeparableBilateralFilterPostProcess> SeparableBilateralFilterPostProcessPtr;
	template <typename T>
	class BlurPostProcess;
	class SumLumPostProcess;
	typedef std::shared_ptr<SumLumPostProcess> SumLumPostProcessPtr;
	class SumLumLogPostProcess;
	typedef std::shared_ptr<SumLumLogPostProcess> SumLumLogPostProcessPtr;
	class SumLumLogPostProcessCS;
	typedef std::shared_ptr<SumLumLogPostProcessCS> SumLumLogPostProcessCSPtr;
	class SumLumIterativePostProcess;
	typedef std::shared_ptr<SumLumIterativePostProcess> SumLumIterativePostProcessPtr;
	class AdaptedLumPostProcess;
	typedef std::shared_ptr<AdaptedLumPostProcess> AdaptedLumPostProcessPtr;
	class AdaptedLumPostProcessCS;
	typedef std::shared_ptr<AdaptedLumPostProcessCS> AdaptedLumPostProcessCSPtr;
	class ImageStatPostProcess;
	typedef std::shared_ptr<ImageStatPostProcess> ImageStatPostProcessPtr;
	class LensEffectsPostProcess;
	typedef std::shared_ptr<LensEffectsPostProcess> LensEffectsPostProcessPtr;
	class LensEffectsPostProcessCS;
	typedef std::shared_ptr<LensEffectsPostProcessCS> LensEffectsPostProcessCSPtr;
	class ToneMappingPostProcess;
	typedef std::shared_ptr<ToneMappingPostProcess> ToneMappingPostProcessPtr;
	class HDRPostProcess;
	typedef std::shared_ptr<HDRPostProcess> HDRPostProcessPtr;
	class SATSeparableScanSweepPostProcess;
	typedef std::shared_ptr<SATSeparableScanSweepPostProcess> SATSeparableScanSweepPostProcessPtr;
	class SATPostProcess;
	typedef std::shared_ptr<SATPostProcess> SATPostProcessPtr;
	class SATPostProcessCS;
	typedef std::shared_ptr<SATPostProcessCS> SATPostProcessCSPtr;
	class TAAPostProcess;
	typedef std::shared_ptr<TAAPostProcess> TAAPostProcessPtr;
	class SSVOPostProcess;
	typedef std::shared_ptr<SSVOPostProcess> SSVOPostProcessPtr;
	struct Particle;
	typedef std::shared_ptr<Particle> ParticlePtr;
	class ParticleEmitter;
	typedef std::shared_ptr<ParticleEmitter> ParticleEmitterPtr;
	class ParticleUpdater;
	typedef std::shared_ptr<ParticleUpdater> ParticleUpdaterPtr;
	class ParticleSystem;
	typedef std::shared_ptr<ParticleSystem> ParticleSystemPtr;
	class InfTerrainRenderable;
	typedef std::shared_ptr<InfTerrainRenderable> InfTerrainRenderablePtr;
	class InfTerrainSceneObject;
	typedef std::shared_ptr<InfTerrainSceneObject> InfTerrainSceneObjectPtr;
	class HQTerrainRenderable;
	typedef std::shared_ptr<HQTerrainRenderable> HQTerrainRenderablePtr;
	class HQTerrainSceneObject;
	typedef std::shared_ptr<HQTerrainSceneObject> HQTerrainSceneObjectPtr;
	class LensFlareRenderable;
	typedef std::shared_ptr<LensFlareRenderable> LensFlareRenderablePtr;
	class LensFlareSceneObject;
	typedef std::shared_ptr<LensFlareSceneObject> LensFlareSceneObjectPtr;
	class DeferredRenderingLayer;
	class MultiResLayer;
	typedef std::shared_ptr<MultiResLayer> MultiResLayerPtr;
	class IndirectLightingLayer;
	typedef std::shared_ptr<IndirectLightingLayer> IndirectLightingLayerPtr;
	class MultiResSILLayer;
	typedef std::shared_ptr<MultiResSILLayer> MultiResSILLayerPtr;
	class SSGILayer;
	typedef std::shared_ptr<SSGILayer> SSGILayerPtr;
	class CascadedShadowLayer;
	typedef std::shared_ptr<CascadedShadowLayer> CascadedShadowLayerPtr;
	class PSSMCascadedShadowLayer;
	typedef std::shared_ptr<PSSMCascadedShadowLayer> PSSMCascadedShadowLayerPtr;
	class SDSMCascadedShadowLayer;
	typedef std::shared_ptr<SDSMCascadedShadowLayer> SDSMCascadedShadowLayerPtr;
	class GpuFft;
	typedef std::shared_ptr<GpuFft> GpuFftPtr;
	class GpuFftPS;
	typedef std::shared_ptr<GpuFftPS> GpuFftPSPtr;
	class GpuFftCS4;
	typedef std::shared_ptr<GpuFftCS4> GpuFftCS4Ptr;
	class GpuFftCS5;
	typedef std::shared_ptr<GpuFftCS4> GpuFftCS5Ptr;
	class SSGIPostProcess;
	typedef std::shared_ptr<SSGIPostProcess> SSGIPostProcessPtr;
	class SSRPostProcess;
	typedef std::shared_ptr<SSRPostProcess> SSRPostProcessPtr;
	class LightShaftPostProcess;
	typedef std::shared_ptr<LightShaftPostProcess> LightShaftPostProcessPtr;
	class TransientBuffer;
	typedef std::shared_ptr<TransientBuffer> TransientBufferPtr;
	class Fence;
	typedef std::shared_ptr<Fence> FencePtr;

	class UIManager;
	class UIElement;
	typedef std::shared_ptr<UIElement> UIElementPtr;
	class UIControl;
	typedef std::shared_ptr<UIControl> UIControlPtr;
	class UIDialog;
	typedef std::shared_ptr<UIDialog> UIDialogPtr;
	class UIStatic;
	typedef std::shared_ptr<UIStatic> UIStaticPtr;
	class UIButton;
	typedef std::shared_ptr<UIButton> UIButtonPtr;
	class UITexButton;
	typedef std::shared_ptr<UITexButton> UITexButtonPtr;
	class UICheckBox;
	typedef std::shared_ptr<UICheckBox> UICheckBoxPtr;
	class UIRadioButton;
	typedef std::shared_ptr<UIRadioButton> UIRadioButtonPtr;
	class UISlider;
	typedef std::shared_ptr<UISlider> UISliderPtr;
	class UIScrollBar;
	typedef std::shared_ptr<UIScrollBar> UIScrollBarPtr;
	class UIListBox;
	typedef std::shared_ptr<UIListBox> UIListBoxPtr;
	class UIComboBox;
	typedef std::shared_ptr<UIComboBox> UIComboBoxPtr;
	class UIEditBox;
	typedef std::shared_ptr<UIEditBox> UIEditBoxPtr;
	class UIPolylineEditBox;
	typedef std::shared_ptr<UIPolylineEditBox> UIPolylineEditBoxPtr;
	class UIProgressBar;
	typedef std::shared_ptr<UIProgressBar> UIProgressBarPtr;

	class Socket;
	class Lobby;
	class Player;

	class AudioEngine;
	class AudioBuffer;
	typedef std::shared_ptr<AudioBuffer> AudioBufferPtr;
	class SoundBuffer;
	class MusicBuffer;
	class AudioDataSource;
	typedef std::shared_ptr<AudioDataSource> AudioDataSourcePtr;
	class AudioFactory;
	class AudioDataSourceFactory;

	class App3DFramework;
	class Window;
	typedef std::shared_ptr<Window> WindowPtr;

	class InputEngine;
	class InputDevice;
	typedef std::shared_ptr<InputDevice> InputDevicePtr;
	class InputKeyboard;
	typedef std::shared_ptr<InputKeyboard> InputKeyboardPtr;
	class InputMouse;
	typedef std::shared_ptr<InputMouse> InputMousePtr;
	class InputJoystick;
	typedef std::shared_ptr<InputJoystick> InputJoystickPtr;
	class InputTouch;
	typedef std::shared_ptr<InputTouch> InputTouchPtr;
	class InputSensor;
	typedef std::shared_ptr<InputSensor> InputSensorPtr;
	class InputFactory;
	struct InputActionParam;
	typedef std::shared_ptr<InputActionParam> InputActionParamPtr;
	struct InputKeyboardActionParam;
	typedef std::shared_ptr<InputKeyboardActionParam> InputKeyboardActionParamPtr;
	struct InputMouseActionParam;
	typedef std::shared_ptr<InputMouseActionParam> InputMouseActionParamPtr;
	struct InputJoystickActionParam;
	typedef std::shared_ptr<InputJoystickActionParam> InputJoystickActionParamPtr;
	struct InputTouchActionParam;
	typedef std::shared_ptr<InputTouchActionParam> InputTouchActionParamPtr;
	struct InputSensorActionParam;
	typedef std::shared_ptr<InputSensorActionParam> InputSensorActionParamPtr;

	class ShowEngine;
	class ShowFactory;

	class ScriptEngine;
	class ScriptFactory;
}

#endif			// _KLAYGE_PREDECLARE_HPP
