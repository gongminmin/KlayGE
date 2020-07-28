#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_PLATFORM_IOS

#include <KFL/PreDeclare.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/Window.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <UIKit/UIKit.h>
#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES2/gl.h>

@interface KlayGEView : UIView
{
	std::array<UITouch*, 16> touch_state_;
	KlayGE::Window* window_;
}
@property(readonly) CAEAGLLayer* eagl_layer;
@property(readonly) EAGLContext* context;

- (id) initWithFrame:(CGRect)frame window:(KlayGE::Window*)window;
@end

namespace KlayGE
{
	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false), keep_screen_on_(settings.keep_screen_on),
			dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity)
	{
		KFL_UNUSED(settings);
		KFL_UNUSED(native_wnd);

		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

		CGRect bounds = [[UIScreen mainScreen] bounds];
		eagl_view_ = [[KlayGEView alloc] initWithFrame:bounds window:this];

		UIViewController* view_controller = [[UIViewController alloc] initWithNibName:nil bundle:nil];
		view_controller.view = eagl_view_;
		[view_controller setTitle:[NSString stringWithCString:name.c_str() encoding:[NSString defaultCStringEncoding]]];

		UIWindow* window = [[UIWindow alloc] initWithFrame:bounds];
		window.rootViewController = view_controller;
		window.backgroundColor = [UIColor blackColor];
		[window makeKeyAndVisible];

		CGRect rect = eagl_view_.frame;
		left_ = 0;
		top_ = 0;
		width_ = rect.size.width;
		height_ = rect.size.height;

		[pool release];
	}

	Window::~Window()
	{
		LogWarn("Unimplemented Window::~Window");
	}

	void Window::CreateColorRenderBuffer(ElementFormat pf)
	{
		NSString* format = nil;
		switch (pf)
		{
		case EF_ARGB8:
			format = kEAGLColorFormatRGBA8;
			break;
		case EF_ABGR8:
		case EF_A2BGR10:
		default:
			BOOST_ASSERT(false);
			break;
		}
		[[eagl_view_ eagl_layer] setDrawableProperties:[NSDictionary dictionaryWithObjectsAndKeys:format, kEAGLDrawablePropertyColorFormat, nil]];
		[[eagl_view_ context] renderbufferStorage:GL_RENDERBUFFER fromDrawable:[eagl_view_ eagl_layer]];
	}

	void Window::FlushBuffer()
	{
		[[eagl_view_ context] presentRenderbuffer:GL_RENDERBUFFER];
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

	uint2 Window::GetGLKViewSize() const
	{
		CGRect rect = eagl_view_.frame;
		return KlayGE::uint2(rect.size.width, rect.size.height);
	}
}

@implementation KlayGEView

@synthesize context, eagl_layer;

- (id) initWithFrame:(CGRect)frame window:(KlayGE::Window*)window
{
	self = [super initWithFrame:frame];
	if (self)
	{
		eagl_layer = (CAEAGLLayer*)self.layer;
		eagl_layer.opaque = YES;

		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
		if (!context || ![EAGLContext setCurrentContext:context])
		{
			context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
			[EAGLContext setCurrentContext:context];
		}

		for (int i = 0; i < 16; ++ i)
		{
			touch_state_[i] = nil;
		}
		window_ = window;
	}
	return self;
}

+ (Class)layerClass
{
	return [CAEAGLLayer class];
}


- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	KFL_UNUSED(event);

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
	KFL_UNUSED(event);
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
	KFL_UNUSED(event);

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
	KFL_UNUSED(event);

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
