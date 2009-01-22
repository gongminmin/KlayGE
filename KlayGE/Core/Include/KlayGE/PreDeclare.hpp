#ifndef _PREDECLARE_HPP
#define _PREDECLARE_HPP

#include <boost/smart_ptr.hpp>

namespace KlayGE
{
	class Context;
	typedef boost::shared_ptr<std::istream> ResIdentifierPtr;
	class ResLoader;
	template <typename Key, typename Type, class Traits, class Allocator>
	class MapVector;
	template <typename Key, class Traits, class Allocator>
	class SetVector;
	class Exception;
	template <typename Sender, typename EventArg>
	class Event;

	class half;
	template <typename T, int N>
	class Vector_T;
	typedef Vector_T<int, 1> int1;
	typedef Vector_T<int, 2> int2;
	typedef Vector_T<int, 3> int3;
	typedef Vector_T<int, 4> int4;
	typedef Vector_T<float, 1> float1;
	typedef Vector_T<float, 2> float2;
	typedef Vector_T<float, 3> float3;
	typedef Vector_T<float, 4> float4;
	template <typename T>
	class Size_T;
	typedef Size_T<float> Size;
	template <typename T>
	class Rect_T;
	typedef Rect_T<float> Rect;
	template <typename T>
	class Matrix4_T;
	typedef Matrix4_T<float> float4x4;
	template <typename T>
	class Quaternion_T;
	typedef Quaternion_T<float> Quaternion;
	template <typename T>
	class Plane_T;
	typedef Plane_T<float> Plane;
	template <typename T>
	class Color_T;
	typedef Color_T<float> Color;
	template <typename T>
	class Bound_T;
	typedef Bound_T<float> Bound;
	template <typename T>
	class Sphere_T;
	typedef Sphere_T<float> Sphere;
	template <typename T>
	class Box_T;
	typedef Box_T<float> Box;

	class SceneManager;
	typedef boost::shared_ptr<SceneManager> SceneManagerPtr;
	class SceneNode;
	typedef boost::shared_ptr<SceneNode> SceneNodePtr;
	class SceneObject;
	typedef boost::shared_ptr<SceneObject> SceneObjectPtr;

	struct ElementInitData;
	class Camera;
	typedef boost::shared_ptr<Camera> CameraPtr;
	class FirstPersonCameraController;
	class Font;
	typedef boost::shared_ptr<Font> FontPtr;
	class RenderEngine;
	typedef boost::shared_ptr<RenderEngine> RenderEnginePtr;
	struct RenderSettings;
	class Renderable;
	typedef boost::shared_ptr<Renderable> RenderablePtr;
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
	class TextureLoader;
	typedef boost::shared_ptr<TextureLoader> TextureLoaderPtr;
	class FrameBuffer;
	typedef boost::shared_ptr<FrameBuffer> FrameBufferPtr;
	class RenderView;
	typedef boost::shared_ptr<RenderView> RenderViewPtr;
	class GraphicsBuffer;
	typedef boost::shared_ptr<GraphicsBuffer> GraphicsBufferPtr;
	class RenderLayout;
	typedef boost::shared_ptr<RenderLayout> RenderLayoutPtr;
	class RenderGraphicsBuffer;
	typedef boost::shared_ptr<RenderGraphicsBuffer> RenderGraphicsBufferPtr;
	struct Viewport;
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
	class KMesh;
	typedef boost::shared_ptr<KMesh> KMeshPtr;
	struct RenderDeviceCaps;
	class Query;
	typedef boost::shared_ptr<Query> QueryPtr;
	class OcclusionQuery;
	typedef boost::shared_ptr<OcclusionQuery> OcclusionQueryPtr;
	class PostProcess;
	typedef boost::shared_ptr<PostProcess> PostProcessPtr;
	class GammaCorrectionProcess;
	typedef boost::shared_ptr<GammaCorrectionProcess> GammaCorrectionProcessPtr;
	class SeparableBlurPostProcess;
	typedef boost::shared_ptr<SeparableBlurPostProcess> SeparableBlurPostProcessPtr;
	class Downsampler2x2PostProcess;
	typedef boost::shared_ptr<Downsampler2x2PostProcess> Downsampler2x2PostProcessPtr;
	class BrightPassDownsampler2x2PostProcess;
	typedef boost::shared_ptr<BrightPassDownsampler2x2PostProcess> BrightPassDownsampler2x2PostProcessPtr;
	class BlurXPostProcess;
	typedef boost::shared_ptr<BlurXPostProcess> BlurXPostProcessPtr;
	class BlurYPostProcess;
	typedef boost::shared_ptr<BlurYPostProcess> BlurYPostProcessPtr;
	class BlurPostProcess;
	typedef boost::shared_ptr<BlurPostProcess> BlurPostProcessPtr;
	class SumLumPostProcess;
	typedef boost::shared_ptr<SumLumPostProcess> SumLumPostProcessPtr;
	class SumLumLogPostProcess;
	typedef boost::shared_ptr<SumLumLogPostProcess> SumLumLogPostProcessPtr;
	class SumLumIterativePostProcess;
	typedef boost::shared_ptr<SumLumIterativePostProcess> SumLumIterativePostProcessPtr;
	class AdaptedLumPostProcess;
	typedef boost::shared_ptr<AdaptedLumPostProcess> AdaptedLumPostProcessPtr;
	class ToneMappingPostProcess;
	typedef boost::shared_ptr<ToneMappingPostProcess> ToneMappingPostProcessPtr;
	class HDRPostProcess;
	typedef boost::shared_ptr<HDRPostProcess> HDRPostProcessPtr;
	class SATSeparableScanPostProcess;
	typedef boost::shared_ptr<SATSeparableScanPostProcess> SATSeparableScanPostProcessPtr;
	class SATScanXPostProcess;
	typedef boost::shared_ptr<SATScanXPostProcess> SATScanXPostProcessPtr;
	class SATScanYPostProcess;
	typedef boost::shared_ptr<SATScanYPostProcess> SATScanYPostProcessPtr;
	class SummedAreaTablePostProcess;
	typedef boost::shared_ptr<SummedAreaTablePostProcess> SummedAreaTablePostProcessPtr;
	template <typename ParticleType>
	class ParticleSystem;

	class UIManager;
	typedef boost::shared_ptr<UIManager> UIManagerPtr;
	class UIElement;
	typedef boost::shared_ptr<UIElement> UIElementPtr;
	class UIControl;
	typedef boost::shared_ptr<UIControl> UIControlPtr;
	class UIDialog;
	typedef boost::shared_ptr<UIDialog> UIDialogPtr;
	class UIStatic;
	class UIButton;
	class UICheckBox;
	class UIRadioButton;
	class UISlider;
	class UIScrollBar;
	class UIListBox;
	class UIComboBox;
	class UIEditBox;

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
