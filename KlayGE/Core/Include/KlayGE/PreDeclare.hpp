#ifndef _KLAYGEPREDECLARE_HPP
#define _KLAYGEPREDECLARE_HPP

namespace KlayGE
{
	class Engine;
	class MemoryLib;
	template <typename T>
	class SharedPtr;
	template <typename T>
	class COMPtr;
	template <typename T>
	class SharedPtr;
	template <typename T>
	class alloc;
	template <typename T>
	class bintree;
	template <typename T>
	class tree;

	class Timer;

	class MathLib;
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
	class Matrix4;
	class Quaternion;
	template <typename T>
	class Plane_T;
	typedef Plane_T<float> Plane;
	class Color;
	class Box;

	class Camera;
	class Font;
	typedef SharedPtr<Font> FontPtr;
	struct Light;
	struct Material;
	class RenderEngine;
	typedef SharedPtr<RenderEngine> RenderEnginePtr;
	class RenderTarget;
	typedef SharedPtr<RenderTarget> RenderTargetPtr;
	class RenderWindowSettings;
	class RenderWindow;
	typedef SharedPtr<RenderWindow> RenderWindowPtr;
	class Renderable;
	typedef SharedPtr<Renderable> RenderablePtr;
	class RenderEffect;
	typedef SharedPtr<RenderEffect> RenderEffectPtr;
	class SceneManager;
	class Texture;
	typedef SharedPtr<Texture> TexturePtr;
	class RenderTexture;
	typedef SharedPtr<RenderTexture> RenderTexturePtr;
	class VertexStream;
	typedef SharedPtr<VertexStream> VertexStreamPtr;
	class IndexStream;
	typedef SharedPtr<IndexStream> IndexStreamPtr;
	class RenderBuffer;
	typedef SharedPtr<RenderBuffer> RenderBufferPtr;
	struct Viewport;
	class RenderFactory;
	typedef SharedPtr<RenderFactory> RenderFactoryPtr;

	class Socket;
	class Lobby;
	class Player;

	class AudioEngine;
	typedef SharedPtr<AudioEngine> AudioEnginePtr;
	class AudioBuffer;
	typedef SharedPtr<AudioBuffer> AudioBufferPtr;
	class SoundBuffer;
	class MusicBuffer;
	class AudioDataSource;
	typedef SharedPtr<AudioDataSource> AudioDataSourcePtr;
	class CDAudio;
	class AudioFactory;
	typedef SharedPtr<AudioFactory> AudioFactoryPtr;

	class CPUInfo;
	class Crc32;
	class Pkt;
	class UnPkt;
	class ResKeyManager;
	class WaveFile;
	typedef SharedPtr<WaveFile> WaveFilePtr;
	class VFile;
	typedef SharedPtr<VFile>	VFilePtr;

	class App3DFramework;

	class InputEngine;
	typedef SharedPtr<InputEngine> InputEnginePtr;
	class InputDevice;
	typedef SharedPtr<InputDevice> InputDevicePtr;
	class InputFactory;
	typedef SharedPtr<InputFactory> InputFactoryPtr;

	class ShowEngine;
	typedef SharedPtr<ShowEngine> ShowEnginePtr;
}

#endif			// _PREDECLARE_HPP