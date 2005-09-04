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
	typedef Vector_T<float, 2> Vector2;
	typedef Vector_T<float, 3> Vector3;
	typedef Vector_T<float, 4> Vector4;
	template <typename T>
	class Size_T;
	typedef Size_T<float> Size;
	template <typename T>
	class Rect_T;
	typedef Rect_T<float> Rect;
	template <typename T>
	class Matrix4_T;
	typedef Matrix4_T<float> Matrix4;
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

	class Camera;
	typedef boost::shared_ptr<Camera> CameraPtr;
	class FirstPersonCameraController;
	class Font;
	typedef boost::shared_ptr<Font> FontPtr;
	class RenderEngine;
	typedef boost::shared_ptr<RenderEngine> RenderEnginePtr;
	class RenderTarget;
	typedef boost::shared_ptr<RenderTarget> RenderTargetPtr;
	struct RenderSettings;
	class RenderWindow;
	typedef boost::shared_ptr<RenderWindow> RenderWindowPtr;
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
	class SceneManager;
	typedef boost::shared_ptr<SceneManager> SceneManagerPtr;
	class SceneNode;
	typedef boost::shared_ptr<SceneNode> SceneNodePtr;
	class Texture;
	typedef boost::shared_ptr<Texture> TexturePtr;
	class Sampler;
	typedef boost::shared_ptr<Sampler> SamplerPtr;
	class RenderTexture;
	typedef boost::shared_ptr<RenderTexture> RenderTexturePtr;
	class VertexStream;
	typedef boost::shared_ptr<VertexStream> VertexStreamPtr;
	class IndexStream;
	typedef boost::shared_ptr<IndexStream> IndexStreamPtr;
	class VertexBuffer;
	typedef boost::shared_ptr<VertexBuffer> VertexBufferPtr;
	class RenderVertexStream;
	typedef boost::shared_ptr<RenderVertexStream> RenderVertexStreamPtr;
	struct Viewport;
	class RenderFactory;
	typedef boost::shared_ptr<RenderFactory> RenderFactoryPtr;
	class StaticMesh;
	typedef boost::shared_ptr<StaticMesh> StaticMeshPtr;
	class BoneMesh;
	typedef boost::shared_ptr<BoneMesh> BoneMeshPtr;
	class KMesh;
	typedef boost::shared_ptr<KMesh> KMeshPtr;
	struct RenderDeviceCaps;

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

	class InputEngine;
	typedef boost::shared_ptr<InputEngine> InputEnginePtr;
	class InputDevice;
	typedef boost::shared_ptr<InputDevice> InputDevicePtr;
	class InputKeyboard;
	class InputMouse;
	class InputJoystick;
	class InputFactory;
	typedef boost::shared_ptr<InputFactory> InputFactoryPtr;

	class ShowEngine;
	typedef boost::shared_ptr<ShowEngine> ShowEnginePtr;

	class ScriptModule;
	class RegisterModule;
	class ScriptEngine;
}

#endif			// _PREDECLARE_HPP
