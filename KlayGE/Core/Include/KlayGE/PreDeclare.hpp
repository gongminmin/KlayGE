#ifndef _KLAYGEPREDECLARE_HPP
#define _KLAYGEPREDECLARE_HPP

#include <boost/smart_ptr.hpp>

namespace KlayGE
{
	class Context;
	template <typename T>
	class COMPtr;
	template <typename T>
	class bintree;
	template <typename T>
	class tree;
	typedef boost::shared_ptr<std::istream> ResIdentifierPtr;
	class ResLoader;

	class Timer;

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
	class Color;
	class Bound;
	class Sphere;
	class Box;

	class Camera;
	class Font;
	typedef boost::shared_ptr<Font> FontPtr;
	struct Light;
	struct Material;
	class RenderEngine;
	typedef boost::shared_ptr<RenderEngine> RenderEnginePtr;
	class RenderTarget;
	typedef boost::shared_ptr<RenderTarget> RenderTargetPtr;
	class RenderSettings;
	class RenderWindow;
	typedef boost::shared_ptr<RenderWindow> RenderWindowPtr;
	class Renderable;
	typedef boost::shared_ptr<Renderable> RenderablePtr;
	class RenderEffect;
	typedef boost::shared_ptr<RenderEffect> RenderEffectPtr;
	class RenderEffectParameter;
	typedef boost::shared_ptr<RenderEffectParameter> RenderEffectParameterPtr;
	class SceneManager;
	typedef boost::shared_ptr<SceneManager> SceneManagerPtr;
	class SceneNode;
	typedef boost::shared_ptr<SceneNode> SceneNodePtr;
	class Texture;
	typedef boost::shared_ptr<Texture> TexturePtr;
	class RenderTexture;
	typedef boost::shared_ptr<RenderTexture> RenderTexturePtr;
	class VertexStream;
	typedef boost::shared_ptr<VertexStream> VertexStreamPtr;
	class IndexStream;
	typedef boost::shared_ptr<IndexStream> IndexStreamPtr;
	class RenderBuffer;
	typedef boost::shared_ptr<RenderBuffer> RenderBufferPtr;
	struct Viewport;
	class RenderFactory;
	typedef boost::shared_ptr<RenderFactory> RenderFactoryPtr;
	class StaticMesh;
	typedef boost::shared_ptr<StaticMesh> StaticMeshPtr;
	class BoneMesh;
	typedef boost::shared_ptr<BoneMesh> BoneMeshPtr;

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
	class CDAudio;
	class AudioFactory;
	typedef boost::shared_ptr<AudioFactory> AudioFactoryPtr;

	class App3DFramework;

	class InputEngine;
	typedef boost::shared_ptr<InputEngine> InputEnginePtr;
	class InputDevice;
	typedef boost::shared_ptr<InputDevice> InputDevicePtr;
	class InputFactory;
	typedef boost::shared_ptr<InputFactory> InputFactoryPtr;

	class ShowEngine;
	typedef boost::shared_ptr<ShowEngine> ShowEnginePtr;
}

#endif			// _PREDECLARE_HPP
