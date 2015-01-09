#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_PLATFORM_DARWIN

#include <KFL/PreDeclare.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/Window.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

@interface KlayGEWindow : NSWindow<NSWindowDelegate>
{
	KlayGE::Window* window_;
}
- (void)setWindow:(KlayGE::Window*)window;
@end

@interface KlayGEView : NSOpenGLView
{
	CVDisplayLinkRef displayLink;
}
- (void) startDisplayLink;
- (void) stopDisplayLink;
- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime;
@end
 
@interface KlayGEESView : NSView
{
}
- (void) render;
@end

namespace KlayGE
{
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
		visual_attr.push_back(NSOpenGLProfileVersion3_2Core);
		visual_attr.push_back(0);
		pixel_format_ = [[[NSOpenGLPixelFormat alloc] initWithAttributes:&visual_attr[0]] autorelease];

		NSScreen* mainDisplay = [NSScreen mainScreen];
		NSRect initContentRect = NSMakeRect(settings.left, settings.top, settings.width, settings.height);
		NSUInteger initStyleMask = NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask | NSResizableWindowMask;
		
		// TODO: full screen support
		KlayGEWindow *ns_window = [[KlayGEWindow alloc] initWithContentRect:initContentRect
											 styleMask:initStyleMask
											 backing:NSBackingStoreBuffered
											 defer:YES
											 screen:mainDisplay];

		[ns_window setAcceptsMouseMovedEvents:YES];
		[ns_window setTitle:[NSString stringWithCString:name.c_str() encoding:[NSString defaultCStringEncoding]]];

		[ns_window setWindow:this];
		[ns_window setDelegate:ns_window];

		NSRect content_rect = [ns_window contentRectForFrameRect:ns_window.frame];
		left_ = 0;
		top_ = 0;
		width_ = content_rect.size.width;
		height_ = content_rect.size.height;

		ns_view_ = nullptr;
		ns_es_view_ = nullptr;
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

	void Window::CreateGLView()
	{
		ns_view_ = [[KlayGEView alloc] initWithFrame:NSMakeRect(0, 0, width_, height_) pixelFormat:pixel_format_];

		[ns_window setContentView:ns_view_];
		[ns_window makeKeyAndOrderFront:nil];
	}

	void Window::CreateGLESView()
	{
		ns_es_view_ = [[KlayGEESView alloc] initWithFrame:NSMakeRect(0, 0, width_, height_)];

		[ns_window setContentView:ns_es_view_];
		[ns_window makeKeyAndOrderFront:nil];
	}
	
	void Window::StartRunLoop()
	{
		if (ns_view_)
		{
			[ns_view_ startDisplayLink];
		}
	}

	void Window::StopRunLoop()
	{
		if (ns_view_)
		{
			[ns_view_ stopDisplayLink];
		}
	}
	
	void Window::FlushBuffer()
	{
		if (ns_view_)
		{
			[[ns_view_ openGLContext] flushBuffer];
		}
	}

	uint2 Window::GetNSViewSize()
	{
		if (ns_view_)
		{
			NSRect rect = ns_view_.frame;
			return KlayGE::uint2(rect.size.width, rect.size.height);
		}
		else
		{
			return KlayGE::uint2(0, 0);
		}
	}
}

@implementation KlayGEWindow
- (void)setWindow:(KlayGE::Window*)window
{
	window_ = window;
}

- (void)windowWillClose:(NSNotification*)notification
{
	(void)notification;
	window_->StopRunLoop();

	window_->OnClose()(*window_);
	window_->Active(false);
	window_->Ready(false);
	window_->Closed(true);
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
	(void)notification;
	window_->Active(true);
	window_->Ready(true);
	window_->OnActive()(*window_, true);
}

- (void)windowDidResignKey:(NSNotification*)notification
{
	(void)notification;
	window_->Active(false);
	window_->OnActive()(*window_, false);
}

- (void)windowDidResize:(NSNotification*)notification
{
	(void)notification;
	window_->Active(true);
	window_->Ready(true);
	window_->OnSize()(*window_, true);
}

- (void)mouseMoved:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, window_->Height() - point.y);
	window_->OnPointerUpdate()(*window_, pt, 1, false);
}

- (void)mouseDragged:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, window_->Height() - point.y);
	window_->OnPointerUpdate()(*window_, pt, 1, true);
}

- (void)mouseUp:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, window_->Height() - point.y);
	window_->OnPointerUp()(*window_, pt, 1);
}

- (void)mouseDown:(NSEvent*)theEvent
{
	NSPoint point = [theEvent locationInWindow];
	KlayGE::int2 pt(point.x, window_->Height() - point.y);
	window_->OnPointerDown()(*window_, pt, 1);
}

- (void)keyDown:(NSEvent *)theEvent
{
	NSString *characters = [theEvent charactersIgnoringModifiers];
	for(unsigned int i = 0; i < [characters length]; i++)
	{
		unichar keyChar = [characters characterAtIndex:0];
		window_->OnKeyDown()(*window_, static_cast<wchar_t>(keyChar));
	}
}

- (void)keyUp:(NSEvent *)theEvent
{
	NSString *characters = [theEvent charactersIgnoringModifiers];
	for(unsigned int i = 0; i < [characters length]; i++)
	{
		unichar keyChar = [characters characterAtIndex:0];
		window_->OnKeyUp()(*window_, static_cast<wchar_t>(keyChar));
	}
}

@end

@implementation KlayGEView

- (id)init
{
	self = [super init];
	return self;
}

- (void)prepareOpenGL
{
	GLint swapInt = 0;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);

	CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);

	CGLContextObj cglContext = static_cast<CGLContextObj>([[self openGLContext] CGLContextObj]);
	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)([[self pixelFormat] CGLPixelFormatObj]);
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
}

- (void) startDisplayLink
{
	CVDisplayLinkStart(displayLink);
}

- (void) stopDisplayLink
{
	CVDisplayLinkStop(displayLink);
}

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, CVTimeStamp const * now,
		CVTimeStamp const * outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	CVReturn result = [(KlayGEView*)displayLinkContext getFrameForTime:outputTime];
	return kCVReturnSuccess;
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime
{
	NSOpenGLContext* currentContext = [self openGLContext];
	[currentContext makeCurrentContext];

	CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);
	KlayGE::Context::Instance().RenderFactoryInstance().RenderEngineInstance().Refresh();
	CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);

	return kCVReturnSuccess;
}

- (void)dealloc
{
	CVDisplayLinkRelease(displayLink);

	[super dealloc];
}
@end

@implementation KlayGEESView

- (void) render
{
	KlayGE::Context::Instance().RenderFactoryInstance().RenderEngineInstance().Refresh();
}

@end

#endif
