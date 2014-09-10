#include <KlayGE/KlayGE.hpp>

#include <iostream>

#ifdef KLAYGE_PLATFORM_DARWIN

#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/Window.hpp>

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

@interface KlayGEWindow : NSWindow
{
	void* mouseParam;
	BOOL firstContent;
	int status;
}
@property(assign) void *mouseParam;
@property(assign) BOOL firstContent;
@property(readwrite) int status;
- (void)cvMouseEvent:(NSEvent *)event;
@end

@interface KlayGEView : NSOpenGLView
{
	CVDisplayLinkRef displayLink;
}
- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime;
@end

namespace KlayGE
{
	static NSApplication *application = nil;
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
			NSString *windowName = [NSString stringWithCString:name.c_str() encoding:[NSString defaultCStringEncoding]];
			// TODO: full screen
			NSRect initContentRect = NSMakeRect(settings.left, settings.top, settings.width, settings.height);
			if (mainDisplay)
			{
				NSRect dispFrame = [mainDisplay visibleFrame];
				initContentRect.origin.y = dispFrame.size.height - 20;
			}
			
			KlayGEWindow *window = [[KlayGEWindow alloc] initWithContentRect:initContentRect
												styleMask:NSTitledWindowMask | NSMiniaturizableWindowMask
												backing:NSBackingStoreBuffered
												defer:YES
												screen:mainDisplay];
			
			[window setFrameTopLeftPoint:initContentRect.origin];
			
			[window setFirstContent:YES];
			
			[window setHasShadow:YES];
			[window setAcceptsMouseMovedEvents:YES];
			[window useOptimizedDrawing:YES];
			[window setTitle:windowName];
			
			d_window_ = static_cast<void*>(window);
			//[application run];
			top_ = initContentRect.origin.x;
			left_ = initContentRect.origin.y;
			width_ = initContentRect.size.width;
			height_ = initContentRect.size.height;
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
	
	void* Window::CreateView() const
	{
		NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pf_];
		KlayGEView* view = [[KlayGEView alloc] initWithFrame:NSMakeRect(0, 0, width_, height_) pixelFormat:pixelFormat];
		return view;
	}
	
	void Window::SetView(void* view) const
	{
		[static_cast<KlayGEWindow*>(d_window_) setContentView:static_cast<KlayGEView*>(view)];
		[static_cast<KlayGEWindow*>(d_window_) makeKeyAndOrderFront:nil];
	}
	void Window::RunLoop()
	{
		[application run];
	}
}

@implementation KlayGEWindow

//@synthesize mouseCallback;
@synthesize mouseParam;
@synthesize firstContent;
@synthesize status;

- (void)cvMouseEvent:(NSEvent *)event
{
	//NSLog(@"%@", event);
	/*
	if(!mouseCallback)
		return;
	
	int flags = 0;
	if([event modifierFlags] & NSShiftKeyMask)		flags |= CV_EVENT_FLAG_SHIFTKEY;
	if([event modifierFlags] & NSControlKeyMask)	flags |= CV_EVENT_FLAG_CTRLKEY;
	if([event modifierFlags] & NSAlternateKeyMask)	flags |= CV_EVENT_FLAG_ALTKEY;
	
	if([event type] == NSLeftMouseDown)	{[self cvSendMouseEvent:event type:CV_EVENT_LBUTTONDOWN flags:flags | CV_EVENT_FLAG_LBUTTON];}
	if([event type] == NSLeftMouseUp)	{[self cvSendMouseEvent:event type:CV_EVENT_LBUTTONUP   flags:flags | CV_EVENT_FLAG_LBUTTON];}
	if([event type] == NSRightMouseDown){[self cvSendMouseEvent:event type:CV_EVENT_RBUTTONDOWN flags:flags | CV_EVENT_FLAG_RBUTTON];}
	if([event type] == NSRightMouseUp)	{[self cvSendMouseEvent:event type:CV_EVENT_RBUTTONUP   flags:flags | CV_EVENT_FLAG_RBUTTON];}
	if([event type] == NSOtherMouseDown){[self cvSendMouseEvent:event type:CV_EVENT_MBUTTONDOWN flags:flags];}
	if([event type] == NSOtherMouseUp)	{[self cvSendMouseEvent:event type:CV_EVENT_MBUTTONUP   flags:flags];}
	if([event type] == NSMouseMoved)	{[self cvSendMouseEvent:event type:CV_EVENT_MOUSEMOVE   flags:flags];}
	if([event type] == NSLeftMouseDragged) {[self cvSendMouseEvent:event type:CV_EVENT_MOUSEMOVE   flags:flags | CV_EVENT_FLAG_LBUTTON];}
	if([event type] == NSRightMouseDragged)	{[self cvSendMouseEvent:event type:CV_EVENT_MOUSEMOVE   flags:flags | CV_EVENT_FLAG_RBUTTON];}
	if([event type] == NSOtherMouseDragged)	{[self cvSendMouseEvent:event type:CV_EVENT_MOUSEMOVE   flags:flags | CV_EVENT_FLAG_MBUTTON];}
	 */
}

- (void)keyDown:(NSEvent *)theEvent
{
	[super keyDown:theEvent];
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

- (void)otherMouseDragged:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	[self cvMouseEvent:theEvent];
}

@end

@implementation KlayGEView

// https://developer.apple.com/library/mac/qa/qa1385/_index.html
- (void)prepareOpenGL
{
	// Synchronize buffer swaps with vertical refresh rate
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
	// Create a display link capable of being used with all active displays
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	
	// Set the renderer output callback function
	CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
	
	// Set the display link for the current renderer
	CGLContextObj cglContext = static_cast<CGLContextObj>([[self openGLContext] CGLContextObj]);
	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)([[self pixelFormat] CGLPixelFormatObj]);
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
	
	// Activate the display link
	CVDisplayLinkStart(displayLink);
}

// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now,
		const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	CVReturn result = [(KlayGEView*)displayLinkContext getFrameForTime:outputTime];
	return kCVReturnSuccess;
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime
{
	NSOpenGLContext    *currentContext = [self openGLContext];
	[currentContext makeCurrentContext];
	
	// must lock GL context because display link is threaded
	CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);
	
	// Add your drawing codes here
	
	[currentContext flushBuffer];
	
	CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
	
	return kCVReturnSuccess;
}

- (void)dealloc
{
	// Release the display link
	CVDisplayLinkRelease(displayLink);
	
	[super dealloc];
}
@end

#endif
