#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_PLATFORM_IOS

#include <KFL/PreDeclare.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/Window.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <UIKit/UIView.h>
#include <GLKit/GLKit.h>

@interface KlayGEView : GLKView
{
	KlayGE::array<UITouch*, 16> touch_state_;
	KlayGE::Window* window_;
}
- (id) initWithFrame:(CGRect)frame window:(KlayGE::Window*)window;
@end

namespace KlayGE
{
	Window::Window(std::string const & name, RenderSettings const & settings)
		: active_(false), ready_(false), closed_(false)
	{
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		
		CGRect bounds = [[UIScreen mainScreen] bounds];
		glk_view_ = [[KlayGEView alloc] initWithFrame:bounds window:this];
		glk_view_.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		switch (settings.color_fmt)
		{
		case EF_ARGB8:
			glk_view_.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
			break;

		case EF_ABGR8:
		case EF_A2BGR10:
		default:
			BOOST_ASSERT(false);
			break;
		}
		switch (settings.depth_stencil_fmt)
		{
		case EF_D16:
			glk_view_.drawableDepthFormat = GLKViewDrawableDepthFormat16;
			glk_view_.drawableStencilFormat = GLKViewDrawableStencilFormatNone;
			break;

		case EF_D24S8:
			glk_view_.drawableDepthFormat = GLKViewDrawableDepthFormat24;
			glk_view_.drawableStencilFormat = GLKViewDrawableStencilFormat8;
			break;

		case EF_D32F:
		default:
			BOOST_ASSERT(false);
			break;
		}

		glk_view_.enableSetNeedsDisplay = NO;
		[glk_view_ bindDrawable];

		GLKViewController * viewController = [[GLKViewController alloc] initWithNibName:nil bundle:nil];
		viewController.view = glk_view_;
		viewController.preferredFramesPerSecond = 60;
		[viewController setTitle:[NSString stringWithCString:name.c_str() encoding:[NSString defaultCStringEncoding]]];

		UIWindow *window = [[UIWindow alloc] initWithFrame:bounds];
		window.rootViewController = viewController;
		window.backgroundColor = [UIColor blackColor];
		[window makeKeyAndVisible];

		CGRect rect = glk_view_.frame;
		left_ = 0;
		top_ = 0;
		width_ = rect.size.width;
		height_ = rect.size.height;

		[pool release];
	}

	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false)
	{
		UNREF_PARAM(name);
		UNREF_PARAM(settings);
		UNREF_PARAM(native_wnd);
		LogWarn("Unimplemented Window::Window");
	}

	Window::~Window()
	{
		LogWarn("Unimplemented Window::~Window");
	}

	void Window::BindDrawable()
	{
		[glk_view_ bindDrawable];
	}

	void Window::FlushBuffer()
	{
		[glk_view_.context presentRenderbuffer:GL_RENDERBUFFER];
	}

	void Window::PumpEvents()
	{
		const CFTimeInterval seconds = 0.000002;
		
		SInt32 result;
		do
		{
			result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, TRUE);
		} while (result == kCFRunLoopRunHandledSource);

		do
		{
			result = CFRunLoopRunInMode((CFStringRef)UITrackingRunLoopMode, seconds, TRUE);
		} while(result == kCFRunLoopRunHandledSource);
	}

	uint2 Window::GetGLKViewSize()
	{
		CGRect rect = glk_view_.frame;
		return KlayGE::uint2(rect.size.width, rect.size.height);
	}
}

@implementation KlayGEView

- (id) initWithFrame:(CGRect)frame window:(KlayGE::Window*)window
{
	self = [super initWithFrame:frame];
	if (self)
	{
		for (int i = 0; i < 16; ++ i)
		{
			touch_state_[i] = nil;
		}
		window_ = window;
	}
	return self;
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	UNREF_PARAM(event);
	for (UITouch* touch in touches)
	{
		int idx = -1;
		for (int i = 0; i < 16; ++ i)
		{
			if (nil == touch_state_[i])
			{
				touch_state_[i] = touch;
				idx = i;
				break;
			}
		}
		if (idx >= 0)
		{
			CGPoint point = [touch locationInView:self];
			KlayGE::int2 pt(point.x, point.y);
			window_->OnPointerDown()(*window_, pt, idx + 1);
		}
	}
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
	UNREF_PARAM(event);
	for (UITouch* touch in touches)
	{
		int idx = -1;
		for (int i = 0; i < 16; ++ i)
		{
			if (touch_state_[i] == touch)
			{
				idx = i;
				break;
			}
		}
		if (idx >= 0)
		{
			CGPoint point = [touch locationInView:self];
			KlayGE::int2 pt(point.x, point.y);
			window_->OnPointerUpdate()(*window_, pt, idx + 1, true);
		}
	}
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	UNREF_PARAM(event);
	for (UITouch* touch in touches)
	{
		int idx = -1;
		for (int i = 0; i < 16; ++ i)
		{
			if(touch_state_[i] == touch)
			{
				idx = i;
				touch_state_[i] = nil;
				break;
			}
		}
		if (idx >= 0)
		{
			CGPoint point = [touch locationInView:self];
			KlayGE::int2 pt(point.x, point.y);
			window_->OnPointerUp()(*window_, pt, idx + 1);
		}
	}
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
	UNREF_PARAM(event);
	for (UITouch* touch in touches)
	{
		int idx = -1;
		for (int i = 0; i < 16; ++ i)
		{
			if (touch_state_[i] == touch)
			{
				idx = i;
				touch_state_[i] = nil;
				break;
			}
		}
		if (idx >= 0)
		{
			CGPoint point = [touch locationInView:self];
			KlayGE::int2 pt(point.x, point.y);
			window_->OnPointerUp()(*window_, pt, idx + 1);
		}
	}
}
@end

#endif
