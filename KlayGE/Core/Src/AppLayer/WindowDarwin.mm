#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_PLATFORM_DARWIN

#include <KFL/PreDeclare.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/Window.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#import <CoreServices/CoreServices.h>
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

@interface KlayGEWindow : NSWindow
- (BOOL)canBecomeKeyWindow;
- (BOOL)canBecomeMainWindow;
@end
@implementation KlayGEWindow
- (BOOL)canBecomeKeyWindow
{
	return YES;
}

- (BOOL)canBecomeMainWindow
{
	return YES;
}
@end

@interface KlayGEWindowListener : NSResponder<NSWindowDelegate>
{
	NSWindow *ns_window;
	KlayGE::Window *app_window;
}

-(id)initWithAppWindow:(KlayGE::Window*)_window;
-(void)listen:(NSWindow*)_window;
-(void)close;

-(BOOL)windowShouldClose:(id)sender;
-(void)windowDidResize:(NSNotification*)aNotification;
-(void)windowDidBecomeKey:(NSNotification*)aNotification;
-(void)windowDidResignKey:(NSNotification*)aNotification;

-(void)mouseDown:(NSEvent*)theEvent;
-(void)rightMouseDown:(NSEvent*)theEvent;
-(void)otherMouseDown:(NSEvent*)theEvent;
-(void)mouseUp:(NSEvent*)theEvent;
-(void)rightMouseUp:(NSEvent*)theEvent;
-(void)otherMouseUp:(NSEvent*)theEvent;
-(void)mouseMoved:(NSEvent*)theEvent;
-(void)mouseDragged:(NSEvent*)theEvent;
-(void)rightMouseDragged:(NSEvent*)theEvent;
-(void)otherMouseDragged:(NSEvent*)theEvent;
-(void)scrollWheel:(NSEvent*)theEvent;
-(void)keyDown:(NSEvent*)theEvent;
-(void)keyUp:(NSEvent*)theEvent;
@end
 
@interface KlayGEESView : NSView
{
}
- (void) render;
@end

namespace KlayGE
{
	static void RegisterApp()
	{
		NSAutoreleasePool* pool;
		ProcessSerialNumber psn = { 0, kCurrentProcess };
		TransformProcessType(&psn, kProcessTransformToForegroundApplication);
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"	// Ignore SetFrontProcess's deprecation
#endif
		SetFrontProcess(&psn);
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_8
#pragma GCC diagnostic pop
#endif
		pool = [[NSAutoreleasePool alloc] init];
		if (nil == NSApp)
		{
			[NSApplication sharedApplication];
			
			[NSApp finishLaunching];
			NSDictionary *appDefaults = [[NSDictionary alloc] initWithObjectsAndKeys:
										 [NSNumber numberWithBool:NO], @"AppleMomentumScrollSupported",
										 [NSNumber numberWithBool:NO], @"ApplePressAndHoldEnabled",
										 nil];
			[[NSUserDefaults standardUserDefaults] registerDefaults:appDefaults];
		}
		
		[pool release];
	}

	Window::Window(std::string const & name, RenderSettings const & settings)
		: active_(false), ready_(false), closed_(false), keep_screen_on_(settings.keep_screen_on),
			dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity)
	{
		RegisterApp();

		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

		NSScreen* mainDisplay = [NSScreen mainScreen];
		NSRect initContentRect = NSMakeRect(settings.left, settings.top, settings.width, settings.height);

#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_11
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"	// Ignore NSResizableWindowMask's deprecation
#endif
		NSUInteger initStyleMask = NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask | NSResizableWindowMask;
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_11
#pragma GCC diagnostic pop
#endif

		// TODO: full screen support
		ns_window_ = [[KlayGEWindow alloc] initWithContentRect:initContentRect
											 styleMask:initStyleMask
											 backing:NSBackingStoreBuffered
											 defer:YES
											 screen:mainDisplay];

		[ns_window_ setAcceptsMouseMovedEvents:YES];
		[ns_window_ setTitle:[NSString stringWithCString:name.c_str() encoding:[NSString defaultCStringEncoding]]];

		NSRect content_rect = [ns_window_ contentRectForFrameRect:ns_window_.frame];
		left_ = 0;
		top_ = 0;
		width_ = content_rect.size.width;
		height_ = content_rect.size.height;
		ns_view_ = nullptr;

		[pool release];
	}

	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false), keep_screen_on_(settings.keep_screen_on),
			dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity)
	{
		KFL_UNUSED(name);
		KFL_UNUSED(settings);
		KFL_UNUSED(native_wnd);
		LogWarn("Unimplemented Window::Window");
	}

	Window::~Window()
	{
		[ns_window_listener_ close];
	}

	void Window::BindListeners()
	{
		ns_window_listener_ = [[KlayGEWindowListener alloc] initWithAppWindow:this];
		[ns_window_listener_ listen:ns_window_];
	}

	void Window::CreateGLView(RenderSettings const & settings)
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		int r_size, a_size, d_size, s_size;
		switch (settings.color_fmt)
		{
			case EF_ARGB8:
			case EF_ABGR8:
				r_size = 8;
				a_size = 8;
				break;
				
			case EF_A2BGR10:
				r_size = 10;
				a_size = 2;
				break;
				
			default:
				r_size = 0;
				a_size = 0;
				BOOST_ASSERT(false);
				break;
		}
		switch (settings.depth_stencil_fmt)
		{
			case EF_D16:
				d_size = 16;
				s_size = 0;
				break;
				
			case EF_D24S8:
				d_size = 24;
				s_size = 8;
				break;
				
			case EF_D32F:
				d_size = 32;
				s_size = 0;
				break;
				
			default:
				d_size = 0;
				s_size = 0;
				break;
		}

		std::vector<NSOpenGLPixelFormatAttribute> visual_attr;
		visual_attr.push_back(NSOpenGLPFAColorSize);
		visual_attr.push_back(r_size * 3);
		visual_attr.push_back(NSOpenGLPFAAlphaSize);
		visual_attr.push_back(a_size);
		if (d_size > 0)
		{
			visual_attr.push_back(NSOpenGLPFADepthSize);
			visual_attr.push_back(d_size);
		}
		if (s_size > 0)
		{
			visual_attr.push_back(NSOpenGLPFAStencilSize);
			visual_attr.push_back(s_size);
		}
		visual_attr.push_back(NSOpenGLPFADoubleBuffer);
		if (settings.sample_count > 1)
		{
			visual_attr.push_back(NSOpenGLPFAMultisample);
			visual_attr.push_back(1);
			visual_attr.push_back(NSOpenGLPFASamples);
			visual_attr.push_back(settings.sample_count);
		}
		visual_attr.push_back(NSOpenGLPFAOpenGLProfile);
		visual_attr.push_back(NSOpenGLProfileVersion4_1Core);
		visual_attr.push_back(0);

		NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:&visual_attr[0]];
		ns_view_ = [[NSOpenGLView alloc] initWithFrame:NSMakeRect(0, 0, width_, height_) pixelFormat:pixel_format];
		[pixel_format release];

		[ns_window_ setContentView:ns_view_];
		[ns_window_ makeKeyAndOrderFront:nil];

		[[(NSOpenGLView*)ns_view_ openGLContext] makeCurrentContext];	// Create GL Context
		[[(NSOpenGLView*)ns_view_ openGLContext] setView:ns_view_];		// initilize fbo 0

		[pool release];
	}

	void Window::CreateGLESView()
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		ns_view_ = [[::NSView alloc] initWithFrame:NSMakeRect(0, 0, width_, height_)];

		[ns_window_ setContentView:ns_view_];
		[ns_window_ makeKeyAndOrderFront:nil];

		[pool release];
	}
	
	void Window::PumpEvents()
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		for (;;)
		{
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_11
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"	// Ignore NSAnyEventMask's deprecation
#endif
			NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_11
#pragma GCC diagnostic pop
#endif
			if (nil == event)
			{
				break;
			}
			
			[NSApp sendEvent:event];
		}
		[pool release];
	}
	
	void Window::FlushBuffer()
	{
		[[(NSOpenGLView*)ns_view_ openGLContext]flushBuffer];
	}

	uint2 Window::GetNSViewSize()
	{
		NSRect rect = ns_view_.frame;
		return KlayGE::uint2(rect.size.width, rect.size.height);
	}
}

@implementation KlayGEWindowListener

- (id)initWithAppWindow:(KlayGE::Window*) _window
{
	self = [super init];
	if (self)
	{
		app_window = _window;
		ns_window = nil;
	}
	return self;
}

- (void)listen:(NSWindow *)_window
{
	ns_window = _window;

	NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
	NSView *view = [ns_window contentView];

	if ([ns_window delegate] != nil)
	{
		[center addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:ns_window];
		[center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:ns_window];
		[center addObserver:self selector:@selector(windowDidResignKey:) name:NSWindowDidResignKeyNotification object:ns_window];
	}
	else
	{
		[ns_window setDelegate:self];
	}
	
	[ns_window setNextResponder:self];
	[ns_window setAcceptsMouseMovedEvents:YES];
	
	[view setNextResponder:self];
	
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_12
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"	// Ignore setAcceptsTouchEvents's deprecation
#endif
	if ([view respondsToSelector:@selector(setAcceptsTouchEvents:)])
	{
		[view setAcceptsTouchEvents:YES];
	}
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_12
#pragma GCC diagnostic pop
#endif
}

- (void)close
{
	NSView *view = [ns_window contentView];
	
	NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
	
	if ([ns_window delegate] != self)
	{
		[center removeObserver:self name:NSWindowDidResizeNotification object:ns_window];
		[center removeObserver:self name:NSWindowDidBecomeKeyNotification object:ns_window];
		[center removeObserver:self name:NSWindowDidResignKeyNotification object:ns_window];
	}
	else
	{
		[ns_window setDelegate:nil];
	}
		
	if ([ns_window nextResponder] == self)
	{
		[ns_window setNextResponder:nil];
	}
	if ([view nextResponder] == self)
	{
		[view setNextResponder:nil];
	}
}

- (BOOL)windowShouldClose:(id) sender
{
	KFL_UNUSED(sender);
	app_window->OnClose()(*app_window);
	app_window->Active(false);
	app_window->Ready(false);
	app_window->Closed(true);
	return NO;
}

- (void)windowDidResize:(NSNotification*) aNotification
{
	KFL_UNUSED(aNotification);
	app_window->Active(true);
	app_window->Ready(true);
	app_window->OnSize()(*app_window, true);
}

- (void)windowDidBecomeKey:(NSNotification*) aNotification
{
	KFL_UNUSED(aNotification);
	app_window->Active(true);
	app_window->Ready(true);
	app_window->OnSize()(*app_window, true);
}

- (void)windowDidResignKey:(NSNotification*) aNotification
{
	KFL_UNUSED(aNotification);
	app_window->Active(false);
	app_window->OnActive()(*app_window, false);
}

- (void)mouseDown:(NSEvent*) theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, app_window->Height() - point.y);
	app_window->OnPointerDown()(*app_window, pt, [theEvent buttonNumber] + 1);
}

- (void)rightMouseDown:(NSEvent*) theEvent
{
	[self mouseDown:theEvent];
}

- (void)otherMouseDown:(NSEvent*) theEvent
{
	[self mouseDown:theEvent];
}

- (void)mouseUp:(NSEvent*) theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, app_window->Height() - point.y);
	app_window->OnPointerUp()(*app_window, pt, [theEvent buttonNumber] + 1);
}

- (void)rightMouseUp:(NSEvent*) theEvent
{
	[self mouseUp:theEvent];
}

- (void)otherMouseUp:(NSEvent*) theEvent
{
	[self mouseUp:theEvent];
}

- (void)mouseMoved:(NSEvent*) theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, app_window->Height() - point.y);
	app_window->OnPointerUpdate()(*app_window, pt, [theEvent buttonNumber]+1, false);
}

- (void)mouseDragged:(NSEvent*) theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, app_window->Height() - point.y);
	app_window->OnPointerUpdate()(*app_window, pt, [theEvent buttonNumber]+1, true);
}

- (void)rightMouseDragged:(NSEvent*) theEvent
{
	[self mouseDragged:theEvent];
}

- (void)otherMouseDragged:(NSEvent*) theEvent
{
	[self mouseDragged:theEvent];
}

- (void)scrollWheel:(NSEvent*) theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, app_window->Height() - point.y);
	app_window->OnPointerWheel()(*app_window, pt, [theEvent buttonNumber]+1, -[theEvent deltaY]);
}

- (void)keyDown:(NSEvent*) theEvent
{
	NSString* characters = [theEvent charactersIgnoringModifiers];
	for (unsigned int i = 0; i < [characters length]; ++ i)
	{
		unichar keyChar = [characters characterAtIndex:0];
		app_window->OnKeyDown()(*app_window, static_cast<wchar_t>(keyChar));
	}
}

- (void)keyUp:(NSEvent*) theEvent
{
	NSString* characters = [theEvent charactersIgnoringModifiers];
	for (unsigned int i = 0; i < [characters length]; ++ i)
	{
		unichar keyChar = [characters characterAtIndex:0];
		app_window->OnKeyUp()(*app_window, static_cast<wchar_t>(keyChar));
	}
}

@end

#endif
