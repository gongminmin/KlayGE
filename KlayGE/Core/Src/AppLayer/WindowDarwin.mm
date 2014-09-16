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
	KlayGE::Window* win;
}
@property(assign) KlayGE::Window* win;
@end

@interface KlayGEView : NSOpenGLView
{
	KlayGE::RenderEngine* re;
	CVDisplayLinkRef displayLink;
}
@property(assign) KlayGE::RenderEngine* re;
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
		pf_ = new NSOpenGLPixelFormatAttribute[visual_attr.size()+1];
		std::copy(visual_attr.begin(), visual_attr.end(), pf_);
		pf_[visual_attr.size()] = 0;
		
		if (!wasInitialized)
		{
			InitSystem();
		}
		
		@autoreleasepool
		{
			NSScreen* mainDisplay = [NSScreen mainScreen];
			NSString* windowName = [NSString stringWithCString:name.c_str() encoding:[NSString defaultCStringEncoding]];
			// TODO: full screen
			NSRect initContentRect = NSMakeRect(settings.left, settings.top, settings.width, settings.height);
			if (mainDisplay)
			{
				NSRect dispFrame = [mainDisplay visibleFrame];
				initContentRect.origin.y = dispFrame.size.height - 20;
			}
			
			KlayGEWindow* window = [[KlayGEWindow alloc] initWithContentRect:initContentRect
												styleMask:NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask
												backing:NSBackingStoreBuffered
												defer:YES
												screen:mainDisplay];
			
			[window setFrameTopLeftPoint:initContentRect.origin];
			
			[window setHasShadow:YES];
			[window setAcceptsMouseMovedEvents:YES];
			[window useOptimizedDrawing:YES];
			[window setTitle:windowName];

			[window setDelegate:window];
			[window setWin:this];
			
			d_window_ = static_cast<void*>(window);
			//[application run];
			top_ = initContentRect.origin.x;
			left_ = initContentRect.origin.y;
			width_ = initContentRect.size.width;
			height_ = initContentRect.size.height;
		}
	}

	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false)
	{
		// TODO
		std::cout<<"test2";
		//NSLog(@"test2");
	}

	Window::~Window()
	{
		// TODO
		std::cout<<"test3";
		//NSLog(@"test3");
	}
	
	void Window::CreateView()
	{
		NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pf_];
		KlayGEView* view = [[KlayGEView alloc] initWithFrame:NSMakeRect(0, 0, width_, height_) pixelFormat:pixelFormat];
		d_view_ = static_cast<void*>(view);

		[static_cast<KlayGEWindow*>(d_window_) setContentView:(view)];
		[static_cast<KlayGEWindow*>(d_window_) makeKeyAndOrderFront:nil];
	}
	
	void Window::RunLoop(RenderEngine& re)
	{
		[static_cast<KlayGEView*>(d_view_) setRe:&re];

		active_ = true;
		[application run];
	}
}

@implementation KlayGEWindow
@synthesize win;

- (void)windowWillClose:(NSNotification*)notification
{
	[(KlayGEView*)[self contentView] setRe:nil];
	[KlayGE::application stop:nil];
}

- (void)keyDown:(NSEvent*)theEvent
{
	NSLog(@"%@", theEvent);
}

- (void)rightMouseDragged:(NSEvent*)theEvent
{
	NSLog(@"%@", theEvent);
}

- (void)rightMouseUp:(NSEvent*)theEvent
{
	NSLog(@"%@", theEvent);
}

- (void)rightMouseDown:(NSEvent*)theEvent
{
	NSLog(@"%@", theEvent);
}

- (void)mouseMoved:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, point.y);
	win->OnPointerUpdate()(*win, pt, 0, false);
}

- (void)mouseDragged:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, point.y);
	win->OnPointerUpdate()(*win, pt, 0, true);
}

- (void)mouseUp:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, point.y);
	win->OnPointerUp()(*win, pt, 0);
}

- (void)mouseDown:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, point.y);
	win->OnPointerDown()(*win, pt, 0);
}

@end

@implementation KlayGEView

@synthesize re;

- (id)init
{
	self = [super init];
	if (self)
	{
		re = nil;
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
	
	if (re != nil)
	{
		re->Refresh();
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
