#ifndef _KLAYGEPREDECLARE_HPP
#define _KLAYGEPREDECLARE_HPP

namespace KlayGE
{
	class Engine;
	class MemoryLib;
	template <typename T>
	class SharePtr;
	template <typename T>
	class COMPtr;
	template <typename T>
	class SharePtr;
	template <typename T>
	class alloc;
	template <typename T>
	class bintree;
	template <typename T>
	class tree;

	class Timer;

	class MathLib;
	template <int N, typename T>
	class Vector_T;
	typedef Vector_T<2, float> Vector2;
	typedef Vector_T<3, float> Vector3;
	typedef Vector_T<4, float> Vector4;
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
	typedef SharePtr<Font> FontPtr;
	struct Light;
	struct Material;
	class RenderEngine;
	typedef SharePtr<RenderEngine> RenderEnginePtr;
	class RenderTarget;
	typedef SharePtr<RenderTarget> RenderTargetPtr;
	class RenderWindowSettings;
	class RenderWindow;
	typedef SharePtr<RenderWindow> RenderWindowPtr;
	class Renderable;
	typedef SharePtr<Renderable> RenderablePtr;
	class RenderEffect;
	typedef SharePtr<RenderEffect> RenderEffectPtr;
	class RenderTechnique;
	typedef SharePtr<RenderTechnique> RenderTechniquePtr;
	class SceneManager;
	class Texture;
	typedef SharePtr<Texture> TexturePtr;
	class RenderTexture;
	typedef SharePtr<RenderTexture> RenderTexturePtr;
	class VertexBuffer;
	typedef SharePtr<VertexBuffer> VertexBufferPtr;
	struct Viewport;
	class RenderFactory;
	typedef SharePtr<RenderFactory> RenderFactoryPtr;

	class Socket;
	class Lobby;
	class Player;

	class AudioEngine;
	typedef SharePtr<AudioEngine> AudioEnginePtr;
	class AudioBuffer;
	typedef SharePtr<AudioBuffer> AudioBufferPtr;
	class SoundBuffer;
	class MusicBuffer;
	class AudioDataSource;
	typedef SharePtr<AudioDataSource> AudioDataSourcePtr;
	class CDAudio;
	class AudioFactory;
	typedef SharePtr<AudioFactory> AudioFactoryPtr;

	class CPUInfo;
	class Crc32;
	class Pkt;
	class UnPkt;
	class ResKeyManager;
	class WaveFile;
	typedef SharePtr<WaveFile> WaveFilePtr;
	class VFile;
	typedef SharePtr<VFile>	VFilePtr;

	class App3DFramework;

	class InputEngine;
	typedef SharePtr<InputEngine> InputEnginePtr;
	class InputDevice;
	typedef SharePtr<InputDevice> InputDevicePtr;
	class InputFactory;
	typedef SharePtr<InputFactory> InputFactoryPtr;

	class ShowEngine;
	typedef SharePtr<ShowEngine> ShowEnginePtr;
}

#endif			// _PREDECLARE_HPP