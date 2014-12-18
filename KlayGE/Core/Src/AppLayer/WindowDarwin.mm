#include <KlayGE/KlayGE.hpp>

#include <iostream>

#ifdef KLAYGE_PLATFORM_DARWIN

#include <KFL/PreDeclare.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/Window.hpp>
#include <KlayGE/RenderEngine.hpp>

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

@interface KlayGEWindow : NSWindow<NSWindowDelegate>
{
	KlayGE::Window* window_;
}
@property(assign) KlayGE::Window* window_;
@end

@interface KlayGEView : NSOpenGLView
{
	KlayGE::RenderEngine* render_engine_;
	CVDisplayLinkRef displayLink;
}
@property(assign) KlayGE::RenderEngine* render_engine_;
- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime;
@end

namespace KlayGE
{
	static NSApplication* application = nil;
	static bool wasInitialized = false;
	
	int InitSystem()
	{
		wasInitialized = true;
		
		@autoreleasepool
		{
			application = [NSApplication sharedApplication];
		
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
		
#ifndef NSAppKitVersionNumber10_5
#define NSAppKitVersionNumber10_5 949
#endif
			if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_5)
			{
				[application setActivationPolicy:NSApplicationActivationPolicyRegular];
			}
#endif
		}

		return 0;
	}

	Window::Window(std::string const & name, RenderSettings const & settings)
		: active_(false), ready_(false), closed_(false)
	{
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
		visual_attr.push_back(r_size);
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
		// https://searchcode.com/codesearch/view/70048394/#l-182
		if (settings.sample_count > 1)
		{
			visual_attr.push_back(NSOpenGLPFAMultisample);
			visual_attr.push_back(1);
			visual_attr.push_back(NSOpenGLPFASamples);
			visual_attr.push_back(settings.sample_count);
		}
		// TODO: OpenGL 3.x core will crash since Cg can only compile old version
		// visual_attr.push_back(NSOpenGLPFAOpenGLProfile);
		// visual_attr.push_back(NSOpenGLProfileVersion3_2Core);
		visual_attr.push_back(0);
		pixel_format_ = [[NSOpenGLPixelFormat alloc] initWithAttributes:&visual_attr[0]];
		
		if (!wasInitialized)
		{
			InitSystem();
		}
		
		@autoreleasepool
		{
			NSScreen* mainDisplay = [NSScreen mainScreen];
			NSString* windowName = [NSString stringWithCString:name.c_str() encoding:[NSString defaultCStringEncoding]];

			NSRect initContentRect = NSMakeRect(settings.left, settings.top, settings.width, settings.height);
			NSUInteger initStyleMask = NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask | NSResizableWindowMask;
			if (settings.full_screen)
			{
				// https://developer.apple.com/library/mac/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_fullscreen/opengl_cgl.html#//apple_ref/doc/uid/TP40001987-CH210-SW6
				initContentRect = [mainDisplay frame];
				initStyleMask = NSBorderlessWindowMask;
			}
			d_window_ = [[KlayGEWindow alloc] initWithContentRect:initContentRect
												styleMask:initStyleMask
												backing:NSBackingStoreBuffered
												defer:YES
												screen:mainDisplay];
            
			if(settings.full_screen)
			{
				[d_window_ setLevel:NSMainMenuWindowLevel + 1];
			}
			else
			{
				[d_window_ setFrameTopLeftPoint:initContentRect.origin];
			}

			[d_window_ setHasShadow:YES];
			[d_window_ setAcceptsMouseMovedEvents:YES];
			[d_window_ useOptimizedDrawing:YES];
			[d_window_ setTitle:windowName];

			[d_window_ setDelegate:d_window_];
			[d_window_ setWindow_:this];

			top_ = d_window_.frame.origin.x;
			left_ = d_window_.frame.origin.y;
			width_ = d_window_.frame.size.width;
			height_ = d_window_.frame.size.height;
		}
	}

	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false)
	{
		// TODO
		std::cout << "Unimplemented Window::Window" << std::endl;
	}

	Window::~Window()
	{
		// TODO
		std::cout << "Unimplemented Window::~Window" << std::endl;
	}
	
	void Window::CreateView()
	{
		// TODO: opengl 3.3
		d_view_ = [[KlayGEView alloc] initWithFrame:NSMakeRect(0, 0, width_, height_) pixelFormat:pixel_format_];

		[d_window_ setContentView:d_view_];
		[d_window_ makeKeyAndOrderFront:nil];
	}
	
	void Window::RunLoop(RenderEngine& re)
	{
		[d_view_ setRender_engine_:&re];
		[application run];
	}

	void Window::HandleCMD(int32_t cmd)
	{
		switch (cmd)
		{
		case 0:
			OnClose()(*this);
			active_ = false;
			ready_ = false;
			closed_ = true;

			[d_view_ setRender_engine_:nil];
			[application stop:nil];
			break;

		case 1:
			active_ = true;
			ready_ = true;
			OnActive()(*this, true);
			break;

		case 2:
			active_ = false;
			OnActive()(*this, false);
			break;

		case 3:
			top_ = d_window_.frame.origin.x;
			left_ = d_window_.frame.origin.y;
			width_ = d_window_.frame.size.width;
			height_ = d_window_.frame.size.height;
			active_ = true;
			ready_ = true;
			// TODO: why
			//OnSize()(*this, true);
			break;
		}
	}
}

@implementation KlayGEWindow
@synthesize window_;

- (void)windowWillClose:(NSNotification*)notification
{
	window_->HandleCMD(0);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
	window_->HandleCMD(1);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
	window_->HandleCMD(2);
}

- (void)windowDidResize:(NSNotification *)notification
{
	window_->HandleCMD(3);
}

- (void)windowDidMove:(NSNotification *)notification
{
	window_->HandleCMD(3);
}

- (void)mouseMoved:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, self.frame.size.height - point.y);
	window_->OnPointerUpdate()(*window_, pt, 1, false);
}

- (void)mouseDragged:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, self.frame.size.height - point.y);
	window_->OnPointerUpdate()(*window_, pt, 1, true);
}

- (void)mouseUp:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, self.frame.size.height - point.y);
	window_->OnPointerUp()(*window_, pt, 1);
}

- (void)mouseDown:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, self.frame.size.height - point.y);
	window_->OnPointerDown()(*window_, pt, 1);
}

@end

@implementation KlayGEView
@synthesize render_engine_;

- (id)init
{
	self = [super init];
	if (self)
	{
		render_engine_ = nil;
	}
	return self;
}

// https://developer.apple.com/library/mac/qa/qa1385/_index.html
- (void)prepareOpenGL
{
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	
	CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
	
	CGLContextObj cglContext = static_cast<CGLContextObj>([[self openGLContext] CGLContextObj]);
	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)([[self pixelFormat] CGLPixelFormatObj]);
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
	
	CVDisplayLinkStart(displayLink);
}

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now,
		const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	CVReturn result = [(KlayGEView*)displayLinkContext getFrameForTime:outputTime];
	return kCVReturnSuccess;
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime
{
	NSOpenGLContext* currentContext = [self openGLContext];
	[currentContext makeCurrentContext];
	
	CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);
	
	if (render_engine_ != nil)
	{
		render_engine_->Refresh();
	}

	[currentContext flushBuffer];
	
	CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
	
	return kCVReturnSuccess;
}

- (void)dealloc
{
	CVDisplayLinkRelease(displayLink);
	
	[super dealloc];
}
@end

#endif
