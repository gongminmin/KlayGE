#ifndef _RENDERTARGET_HPP
#define _RENDERTARGET_HPP

#include <KlayGE/Viewport.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	class RenderTarget
	{
	public:
		RenderTarget();
		virtual ~RenderTarget();

		virtual int Left() const;
		virtual int Top() const;
		virtual int Width() const;
		virtual int Height() const;
		virtual int ColorDepth() const;

		virtual void Update();

		virtual const Viewport& GetViewport() const;
		virtual Viewport& GetViewport();
		virtual void SetViewport(const Viewport& viewport);

		virtual float FPS() const;

		// Gets a custom (maybe platform-specific) attribute.
		virtual void CustomAttribute(const std::string& name, void* pData) = 0;

		virtual bool Active() const;
		virtual void Active(bool state);

		virtual bool RequiresTextureFlipping() const = 0;

	protected:
		void UpdateStats();

		int		left_;
		int		top_;
		int		width_;
		int		height_;
		int		colorDepth_;
		bool	isDepthBuffered_;

		// Stats
		float	FPS_;
		float	frameTime_;

		bool	active_;

		Viewport viewport_;
	};
}

#endif			// _RENDERTARGET_HPP
